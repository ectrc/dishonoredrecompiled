// Engine/src/arkanimnodestack.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x574d70  public: static void __cdecl UArkAnimNodeStack::InitializePrivateStaticClassUArkAnimNodeStack(void)
//   0x574d90  public: virtual void __thiscall UArkAnimNodeStack::TickAnim(float)
//   0x574db0  public: virtual void __thiscall UArkAnimNodeStack::GetBoneAtoms(class TArray<class FBoneAtom, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &, class TArray<unsigned char, class FDefaultAllocator> const &, class FBoneAtom &, int &)
//   0x574dc0  public: virtual void __thiscall UArkAnimNodeStack::OnRemoveChild(int)
//   0x576570  public: void __thiscall UArkAnimNodeStack::MAT_SetAnimPosition(int, class FName, float, unsigned int, unsigned int, unsigned int)
//   0x576790  public: void __thiscall UArkAnimNodeStack::MAT_SetAnimWeights(struct FAnimSlotInfo const &)
//   0x5768b0  public: void __thiscall UArkAnimNodeStack::MAT_FinishAnimControl(void)
//   0x576920  public: unsigned short __thiscall UArkAnimNodeStack::BuildEdgeAnimTreeEx(struct UAnimNode::FEdgeAnimTreeContext &, int)
//   0x576aa0  public: void __thiscall UArkAnimNodeStack::ForceDontUseRefPoseOnChild(unsigned int)
//   0x5782c0  public: virtual void __thiscall UArkAnimNodeStack::InitAnim(class USkeletalMeshComponent *, class UAnimNodeBlendBase *)
//   0x5782f0  public: virtual unsigned short __thiscall UArkAnimNodeStack::BuildEdgeAnimTree(struct UAnimNode::FEdgeAnimTreeContext &)
//   0x578310  public: virtual void __thiscall UArkAnimNodeStack::OnPaste(void)
//   0x579e20  public: virtual void __thiscall UArkAnimNodeStack::RenameChildConnectors(void)
//   0x57c120  public: static class UClass * __cdecl UArkAnimNodeStack::GetPrivateStaticClassUArkAnimNodeStack(wchar_t const *)
//   0x57c920  public: static class UClass * __cdecl UArkAnimNodeStack::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	DISHONORED(port): agent FS (PHASE17 package FS). Arkane's matinee slot node.

	Retail's pawns have no UAnimNodeSlot in their trees for Matinee to drive: APawn::SlotNodes and
	ASkeletalMeshActorMAT::SlotNodes are TArray<UArkAnimNodeStack*> (retail_sdk_layout.json @1056 / @644)
	and APawn::MAT_SetAnimPosition (2012 rva 0x2d0880) calls the stack's MAT_SetAnimPosition straight.
	Until this unit had bodies a StretchAnimControl track on any Dishonored pawn animated nothing.

	Child 0 is the tree the pawn's own animation comes down; children 1..N-1 are the matinee channels
	(channel i is child i+1, as on UAnimNodeSlot). TickAnim and OnRemoveChild are the base class's, OnPaste and
	RenameChildConnectors editor-only: the class inherits them. GetBoneAtoms is retail's 9-byte jump to the base,
	which retail PC never reaches: its skeleton is evaluated through BuildEdgeAnimTree / BuildEdgeAnimTreeEx, and
	GetBoneAtoms here is that Edge builder written for the CPU evaluator (agent HA). The class was a DishonoredGame
	shim until wave 20; it is an Engine class in retail and is declared in EngineAnimClasses.h now.
-----------------------------------------------------------------------------*/

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationEncodingFormat_EdgeAnim.h"

IMPLEMENT_CLASS(UArkAnimNodeStack);

/** -fkdiag (agent FK's switch): what the matinee hands this node, once a second per node. */
static UBOOL DisFSStackDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-fkdiag") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

/** -fkdiag (agent HA): the evaluation line, once a second per stack. */
static UBOOL DisHAStackEvalDiagDue( const UArkAnimNodeStack* Stack )
{
	if( !DisFSStackDiagEnabled() || GWorld == NULL )
	{
		return FALSE;
	}
	static TMap<const UObject*, FLOAT> LastReportAt;
	const FLOAT Now = GWorld->GetTimeSeconds();
	FLOAT* Last = LastReportAt.Find( Stack );
	if( Last != NULL && Now - *Last < 1.f )
	{
		return FALSE;
	}
	LastReportAt.Set( Stack, Now );
	return TRUE;
}

static FString DisFSStackWeights( const UArkAnimNodeStack* Stack )
{
	FString Out;
	for( INT ChildIndex = 0; ChildIndex < Stack->Children.Num(); ChildIndex++ )
	{
		Out += FString::Printf( TEXT("%s%.2f"), ChildIndex ? TEXT("/") : TEXT(""), Stack->Children(ChildIndex).Weight );
	}
	return Out;
}

// 2012 rva 0x5782c0 (35 B)
// DISHONORED(port): agent HA. Retail's Super::InitAnim is UAnimNodeBlendBase::InitAnim (2012 0x1c2ed0, 255 B), which ends by
// writing every child's m_bIsAnimSeq = ( Anim != NULL && Anim->IsA( UAnimNodeSequence ) ); this tree's blend base has no
// InitAnim of its own, so the stack - the one node that reads the bit (BuildEdgeAnimTreeEx, MAT_SetAnimPosition) - derives it here.
void UArkAnimNodeStack::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	FString CookedBits;
	for( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		CookedBits += Children(ChildIndex).m_bIsAnimSeq ? TEXT("1") : TEXT("0");
	}
	Super::InitAnim( MeshComp, Parent );
	FString DerivedBits;
	for( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		FAnimBlendChild& Child = Children(ChildIndex);
		Child.m_bIsAnimSeq = ( Child.Anim != NULL && Child.Anim->IsA( UAnimNodeSequence::StaticClass() ) ) ? 1 : 0;
		DerivedBits += Child.m_bIsAnimSeq ? TEXT("1") : TEXT("0");
	}
	ForceDontUseRefPoseOnChild( TRUE );

	if( DisFSStackDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag stack %s on %s init: %d children, w0 %.2f (cooked), m_bIsAnimSeq cooked %s derived %s"),
			*NodeName.ToString(), ( MeshComp != NULL && MeshComp->GetOwner() != NULL ) ? *MeshComp->GetOwner()->GetName() : TEXT("?"),
			Children.Num(), Children.Num() > 0 ? Children(0).Weight : -1.f, *CookedBits, *DerivedBits );
	}
}

// 2012 rva 0x576aa0 (138 B): every sequence child from 1 on gets bDontUseRefPoseOnNonDefinedBones
void UArkAnimNodeStack::ForceDontUseRefPoseOnChild( UBOOL bStatus )
{
	for( INT ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++ )
	{
		UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );
		if( SeqNode != NULL )
		{
			SeqNode->m_bDontUseRefPoseOnNonDefinedBones = bStatus ? 1 : 0;
		}
	}
}

// DISHONORED(port): agent HA. UArkAnimNodeStack::BuildEdgeAnimTreeEx, 2012 rva 0x576920 (376 B) / 2013 rva 0x5350c0 (VA
// 0x9350c0, 376 B; slot 91 (+364) of ??_7UArkAnimNodeStack@@6B@ is BuildEdgeAnimTree 0x937220, 18 B, = Ex( Context, -1 ); the
// 2012 and 2013 bodies are the same 116 instructions, only the assert line number differs), written for the CPU evaluator.
// Retail PC evaluates skeletons through EdgeAnim and this is the stack's Edge tree, a chain of blend branches:
//   base     child 0's subtree (UAnimNode::GetEdgeAnimTree), or - no node - UAnimNode::BuildEdgeAnimTree (2012 0x19e6f0,
//            2013 0x5929f0): a leaf with flags 4 and no header, which _edgeAnimProcessCommandList (2012 0x6238c0, command 5)
//            fills with the skeleton's base pose, every joint defined, the locomotion joint not. Child 0's Weight is never read.
//   child i  (i >= 1) used iff its raw Weight > 0. A sequence child (m_bIsAnimSeq, +64 bit 2) whose AnimSeq (+228) is NULL is
//            skipped; a child with no node gets the base-pose leaf; any other node is its own subtree.
//   branch   bIsAdditive (+64 bit 1) -> op 1, the relative add-delta blend (_edgeAnimBlendJointsRelative 2012 0x613a90); else
//            op 0, the linear blend; alpha = Weight. _edgeAnimBlendPose (2012 0x6232f0) runs both over numJoints + 1 joints,
//            the last one being the locomotion joint, so root motion goes through the same branch.
// A sequence leaf leaves the bones its animation does not define undefined (weight 0) when m_bDontUseRefPoseOnNonDefinedBones
// is set or the sequence is additive (UAnimNodeSequence::BuildEdgeAnimTreeLeaf 2012 0x1a46b0 -> AnimLeafCallback 0x58fb00), and
// its locomotion joint is defined only when it extracts root motion; a blend keeps the left pose where the right is undefined.
// This replaces agent FT's stand-in, which put a sequence channel without an animation in as the reference pose at the
// channel's weight: the captain's T-pose under a played-out key (GJ3.log `ch3 None w1`).
void UArkAnimNodeStack::GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys )
{
	if( GetCachedResults( Atoms, RootMotionDelta, bHasRootMotion, CurveKeys, DesiredBones.Num() ) )
	{
		return;
	}
	const TArray<FMeshBone>& RefSkeleton = SkelComponent->SkeletalMesh->RefSkeleton;
	const UBOOL bDiag = DisHAStackEvalDiagDue( this );
	FString Diag;

	if( Children.Num() > 0 && Children(0).Anim != NULL )
	{
		GetChildBoneAtoms( 0, Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
		Diag = FString::Printf( TEXT("base %s"), *Children(0).Anim->GetClass()->GetName() );
	}
	else
	{
		RootMotionDelta.SetIdentity();
		bHasRootMotion = 0;
		FillWithRefPose( Atoms, DesiredBones, RefSkeleton );
		Diag = TEXT("base refpose");
	}

	FBoneAtomArray ChildAtoms;
	FBoneAtom ChildRootMotion;
	INT bChildHasRootMotion = 0;
	FCurveKeyArray ChildCurveKeys;
	TArray<BYTE> DefinedBones;
	for( INT ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++ )
	{
		const FAnimBlendChild& Child = Children(ChildIndex);
		const FLOAT SlotWeight = Child.Weight;
		if( !( SlotWeight > 0.f ) )
		{
			if( bDiag )
			{
				Diag += FString::Printf( TEXT(" | ch%d w %.2f skip(w0)"), ChildIndex - 1, SlotWeight );
			}
			continue;
		}
		UAnimNodeSequence* SeqNode = Child.m_bIsAnimSeq ? Cast<UAnimNodeSequence>( Child.Anim ) : NULL;
		if( SeqNode != NULL && SeqNode->AnimSeq == NULL )
		{
			if( bDiag )
			{
				Diag += FString::Printf( TEXT(" | ch%d w %.2f skip(noseq)"), ChildIndex - 1, SlotWeight );
			}
			continue;
		}
		if( ChildAtoms.Num() == 0 )
		{
			ChildAtoms.Add( RefSkeleton.Num() );
		}
		if( Child.Anim != NULL )
		{
			GetChildBoneAtoms( ChildIndex, ChildAtoms, DesiredBones, ChildRootMotion, bChildHasRootMotion, ChildCurveKeys );
		}
		else
		{
			FillWithRefPose( ChildAtoms, DesiredBones, RefSkeleton );
			ChildRootMotion.SetIdentity();
			bChildHasRootMotion = 0;
		}

		// DISHONORED(port): agent HA. The mask is the Edge leaf's (AnimLeafCallback's weight array: the joints the blob's hash list
		// names) for an Edge sequence; the AnimSet linkup (agent FT's test) only for a sequence the Edge path cannot evaluate.
		const TArray<INT>* BoneToTrackTable = NULL;
		const UBOOL bMaskUndefined = SeqNode != NULL && ( SeqNode->m_bDontUseRefPoseOnNonDefinedBones || SeqNode->AnimSeq->bIsAdditive );
		const UBOOL bEdgeMask = bMaskUndefined && FEdgeAnimSequencePose::GetDefinedBones( SeqNode->AnimSeq, SkelComponent->SkeletalMesh, DefinedBones );
		if( bMaskUndefined && !bEdgeMask && SeqNode->AnimSeq->GetAnimSet() != NULL )
		{
			const UAnimSet* AnimSet = SeqNode->AnimSeq->GetAnimSet();
			if( AnimSet->LinkupCache.IsValidIndex( SeqNode->AnimLinkupIndex ) )
			{
				BoneToTrackTable = &AnimSet->LinkupCache( SeqNode->AnimLinkupIndex ).BoneToTrackTable;
			}
		}
		INT NumMasked = 0;
		for( INT BoneSlot = 0; BoneSlot < DesiredBones.Num(); BoneSlot++ )
		{
			const INT BoneIndex = DesiredBones(BoneSlot);
			if( ( bEdgeMask && ( !DefinedBones.IsValidIndex( BoneIndex ) || DefinedBones(BoneIndex) == 0 ) )
				|| ( BoneToTrackTable != NULL && BoneToTrackTable->IsValidIndex( BoneIndex ) && (*BoneToTrackTable)(BoneIndex) == INDEX_NONE ) )
			{
				NumMasked++;
				continue;
			}
			if( Child.bIsAdditive )
			{
				FBoneAtom::BlendFromIdentityAndAccumulate( Atoms(BoneIndex), ChildAtoms(BoneIndex), ScalarRegister( SlotWeight ) );
			}
			else
			{
				Atoms(BoneIndex).Blend( Atoms(BoneIndex), ChildAtoms(BoneIndex), SlotWeight );
			}
		}

		if( bChildHasRootMotion )
		{
			if( Child.bIsAdditive )
			{
				if( !bHasRootMotion )
				{
					RootMotionDelta.SetIdentity();
				}
				FBoneAtom::BlendFromIdentityAndAccumulate( RootMotionDelta, ChildRootMotion, ScalarRegister( SlotWeight ) );
			}
			else if( bHasRootMotion )
			{
				RootMotionDelta.Blend( RootMotionDelta, ChildRootMotion, SlotWeight );
			}
			else
			{
				RootMotionDelta = ChildRootMotion;
			}
			bHasRootMotion = 1;
		}
		if( bDiag )
		{
			Diag += FString::Printf( TEXT(" | ch%d w %.2f %s%s%s %s mask %s %d/%d"), ChildIndex - 1, SlotWeight, Child.bIsAdditive ? TEXT("add") : TEXT("lerp"),
				Child.Anim == NULL ? TEXT(" refpose") : TEXT(""), bChildHasRootMotion ? TEXT(" rm") : TEXT(""),
				SeqNode != NULL ? *SeqNode->AnimSeqName.ToString() : ( Child.Anim != NULL ? *Child.Anim->GetClass()->GetName() : TEXT("-") ),
				bEdgeMask ? TEXT("edge") : ( BoneToTrackTable != NULL ? TEXT("linkup") : TEXT("none") ), NumMasked, DesiredBones.Num() );
		}
	}
	static TMap<const UObject*, INT> LastHadRootMotion;
	const INT* LastRootMotion = DisFSStackDiagEnabled() ? LastHadRootMotion.Find( this ) : NULL;
	const UBOOL bRootMotionEdge = DisFSStackDiagEnabled() && ( LastRootMotion != NULL ? *LastRootMotion : 0 ) != bHasRootMotion;
	if( DisFSStackDiagEnabled() )
	{
		LastHadRootMotion.Set( this, bHasRootMotion );
	}
	if( bDiag || bRootMotionEdge )
	{
		const FVector RootMotionTranslation = RootMotionDelta.GetTranslation();
		debugf( TEXT("DISHONORED(bringup): fkdiag stack %s %s eval %s | rm %d %.2f/%.2f/%.2f/%d"),
			( SkelComponent->GetOwner() != NULL ) ? *SkelComponent->GetOwner()->GetName() : TEXT("?"), *NodeName.ToString(), *Diag,
			bHasRootMotion, RootMotionTranslation.X, RootMotionTranslation.Y, RootMotionTranslation.Z, bHasRootMotion ? RootMotionDelta.GetRotation().Rotator().Yaw : 0 );
	}
	SaveCachedResults( Atoms, RootMotionDelta, bHasRootMotion, CurveKeys, DesiredBones.Num() );
}

// 2012 rva 0x576570 (536 B). Retail's reading of the child: Children[i]+64 bit 2 is m_bIsAnimSeq, the
// node's +204 is AnimSeqName, +228 AnimSeq, +212 Rate, +216 the flag word (bLooping 0x8,
// bMatineePlaying 0x10), +220 CurrentTime, +224 PreviousTime; vtable +384 is SetAnim, +388 SetPosition.
// DISHONORED(port): agent HA. 2013 VA 0x934cb0 (rva 0x534cb0, 629 B; 2013 vtable +388 SetAnim, +392 SetPosition, +484
// IssueNotifies): the previous position. Without notifies, or with FLT_MIN (no previous position, what the track passes
// outside its special root-motion path), PreviousTime = CurrentTime and SetPosition as 2012; otherwise the notifies between
// CurrentTime and InPosition are issued, then SetPosition, and PreviousTime is the caller's previous position.
void UArkAnimNodeStack::MAT_SetAnimPosition( INT ChannelIndex, FName InAnimSeqName, FLOAT InPrevPosition, FLOAT InPosition, UBOOL bFireNotifies, UBOOL bLooping, UBOOL bEnableRootMotion )
{
	const INT ChildIndex = ChannelIndex + 1;
	if( ChildIndex >= Children.Num() )
	{
		return;
	}
	FAnimBlendChild& Child = Children(ChildIndex);
	// retail tests the child's m_bIsAnimSeq type cache; nothing in this tree writes that bit, so the cast is the test
	UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>( Child.Anim );
	if( SeqNode == NULL )
	{
		return;
	}
	if( SeqNode->AnimSeqName != InAnimSeqName || SeqNode->AnimSeq == NULL )
	{
		SeqNode->SetAnim( InAnimSeqName );
		SeqNode->SetPosition( InPosition, FALSE );
	}
	Child.bMirrorSkeleton = FALSE;
	if( SkelComponent != NULL )
	{
		if( bEnableRootMotion )
		{
			SeqNode->SetRootBoneAxisOption( RBA_Translate, RBA_Translate, RBA_Translate );
			SeqNode->SetRootBoneRotationOption( RRO_Extract, RRO_Extract, RRO_Extract );
		}
		else
		{
			SeqNode->SetRootBoneAxisOption( RBA_Default, RBA_Default, RBA_Default );
			SeqNode->SetRootBoneRotationOption( RRO_Default, RRO_Default, RRO_Default );
		}
	}
	SeqNode->Rate = 1.f;
	SeqNode->bLooping = bLooping ? 1 : 0;
	SeqNode->bMatineePlaying = ( GIsGame && InAnimSeqName != NAME_None ) ? 1 : 0;
	if( !bFireNotifies || InPrevPosition == FLT_MIN )
	{
		SeqNode->PreviousTime = SeqNode->CurrentTime;
		SeqNode->SetPosition( InPosition, bFireNotifies );
	}
	else
	{
		SeqNode->IssueNotifies( InPosition - SeqNode->CurrentTime );
		SeqNode->SetPosition( InPosition, bFireNotifies );
		SeqNode->PreviousTime = InPrevPosition;
	}

	if( DisFSStackDiagEnabled() )
	{
		static TMap<const UObject*, FLOAT> LastReportAt;
		const FLOAT Now = GWorld->GetTimeSeconds();
		FLOAT* Last = LastReportAt.Find( this );
		if( Last == NULL || Now - *Last >= 1.f )
		{
			LastReportAt.Set( this, Now );
			debugf( TEXT("DISHONORED(bringup): fkdiag stack %s on %s: ch %d <- %s pos %.2f (seq now %s t %.2f, AnimSeq %s), weights %s"),
				*NodeName.ToString(), ( SkelComponent != NULL && SkelComponent->GetOwner() != NULL ) ? *SkelComponent->GetOwner()->GetName() : TEXT("?"),
				ChannelIndex, *InAnimSeqName.ToString(), InPosition, *SeqNode->AnimSeqName.ToString(), SeqNode->CurrentTime,
				( SeqNode->AnimSeq != NULL ) ? TEXT("found") : TEXT("NULL"), *DisFSStackWeights( this ) );
		}
	}
}

// 2012 rva 0x576790 (277 B), 2013 0x534f30 (277 B). One child: it takes weight 1. Otherwise channel i (child i+1) takes its
// weight from the slot info, clamped to [0,1] and 0 when the info has no entry for it. Child 0 is not written: the
// evaluation (GetBoneAtoms, BuildEdgeAnimTreeEx) never reads its weight.
void UArkAnimNodeStack::MAT_SetAnimWeights( const FAnimSlotInfo& SlotInfo )
{
	if( Children.Num() == 1 )
	{
		Children(0).Weight = 1.f;
		return;
	}
	if( Children.Num() < 2 )
	{
		return;
	}
	// DISHONORED(port): agent HA. Agent FS's child-0 complement ( 1 - sum of channels ) is gone: retail (2012 0x576790, 2013
	// 0x534f30) writes children N-1..1 only, and with it the base tree sat at weight 0 - never ticked - for the whole scene.
	for( INT ChildIndex = Children.Num() - 1; ChildIndex >= 1; ChildIndex-- )
	{
		const INT ChannelIndex = ChildIndex - 1;
		FLOAT ChannelWeight = 0.f;
		if( ChannelIndex < SlotInfo.ChannelWeights.Num() && SlotInfo.ChannelWeights(ChannelIndex) >= 0.f )
		{
			ChannelWeight = Min( SlotInfo.ChannelWeights(ChannelIndex), 1.f );
		}
		Children(ChildIndex).Weight = ChannelWeight;
	}

	if( DisFSStackDiagEnabled() )
	{
		static TMap<const UObject*, FLOAT> LastReportAt;
		const FLOAT Now = GWorld->GetTimeSeconds();
		FLOAT* Last = LastReportAt.Find( this );
		if( Last == NULL || Now - *Last >= 1.f )
		{
			LastReportAt.Set( this, Now );
			FString In;
			for( INT ChannelIndex = 0; ChannelIndex < SlotInfo.ChannelWeights.Num(); ChannelIndex++ )
			{
				In += FString::Printf( TEXT("%s%.2f"), ChannelIndex ? TEXT("/") : TEXT(""), SlotInfo.ChannelWeights(ChannelIndex) );
			}
			debugf( TEXT("DISHONORED(bringup): fkdiag stack %s weights <- [%s] -> children %s"), *NodeName.ToString(), *In, *DisFSStackWeights( this ) );
		}
	}
}

// 2012 rva 0x5768b0 (102 B): the matinee has let go of every channel
void UArkAnimNodeStack::MAT_FinishAnimControl()
{
	for( INT ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++ )
	{
		UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );
		if( SeqNode != NULL )
		{
			SeqNode->bMatineePlaying = 0;
		}
	}
}
