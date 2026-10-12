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
	(channel i is child i+1, as on UAnimNodeSlot). TickAnim, GetBoneAtoms, OnRemoveChild, BuildEdgeAnimTree
	(PS3 EdgeAnim), OnPaste and RenameChildConnectors are the base class's or editor-only and are not
	ported: the class inherits them. The class was a DishonoredGame shim until this wave; it is an Engine
	class in retail and is declared in EngineAnimClasses.h now.
-----------------------------------------------------------------------------*/

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"

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
void UArkAnimNodeStack::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim( MeshComp, Parent );
	ForceDontUseRefPoseOnChild( TRUE );
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

// 2012 rva 0x576570 (536 B). Retail's reading of the child: Children[i]+64 bit 2 is m_bIsAnimSeq, the
// node's +204 is AnimSeqName, +228 AnimSeq, +212 Rate, +216 the flag word (bLooping 0x8,
// bMatineePlaying 0x10), +220 CurrentTime, +224 PreviousTime; vtable +384 is SetAnim, +388 SetPosition.
void UArkAnimNodeStack::MAT_SetAnimPosition( INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bFireNotifies, UBOOL bLooping, UBOOL bEnableRootMotion )
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
	SeqNode->PreviousTime = SeqNode->CurrentTime;
	SeqNode->bMatineePlaying = ( GIsGame && InAnimSeqName != NAME_None ) ? 1 : 0;
	SeqNode->SetPosition( InPosition, bFireNotifies );

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

// 2012 rva 0x576790 (277 B). One child: it takes weight 1. Otherwise channel i (child i+1) takes its
// weight from the slot info, clamped to [0,1] and 0 when the info has no entry for it. Retail writes
// nothing to child 0 here; see the note below.
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
	FLOAT ChannelTotal = 0.f;
	for( INT ChildIndex = Children.Num() - 1; ChildIndex >= 1; ChildIndex-- )
	{
		const INT ChannelIndex = ChildIndex - 1;
		FLOAT ChannelWeight = 0.f;
		if( ChannelIndex < SlotInfo.ChannelWeights.Num() && SlotInfo.ChannelWeights(ChannelIndex) >= 0.f )
		{
			ChannelWeight = Min( SlotInfo.ChannelWeights(ChannelIndex), 1.f );
		}
		Children(ChildIndex).Weight = ChannelWeight;
		ChannelTotal += ChannelWeight;
	}
	// DISHONORED(deviation, agent FS): this tree's UAnimNodeBlendBase::GetBoneAtoms is the reference
	// weighted sum, which only renormalises for BlendType != ABT_Linear; with child 0 left at the
	// asset's weight a channel at 1.0 would be summed on top of the source pose. Retail's base does
	// not write child 0 here either, so the complement below stands in for whatever its blend does
	// with the stack - the same complement UAnimNodeSlot::MAT_SetAnimWeights computes. To verify
	// against retail: read the stack's Children weights live during the Boat matinee with dismod.
	Children(0).Weight = Max( 1.f - ChannelTotal, 0.f );

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
