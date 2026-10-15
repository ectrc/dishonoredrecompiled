/*=============================================================================
	arkanimnodeblendpose_dis.cpp: UArkAnimNodeBlendPose, the node that captures the pose it outputs and blends the live
	tree back in from that capture - the matinee blend-in and blend-out of the pawns (ADishonoredNPCPawn::m_pBlendPoseNode,
	ADishonoredPlayerPawn::m_pMatineeBlender, both the anim tree's 'PoseBlender').

	The class is an Engine-package class declared in DishonoredGameEngineShims.h (Engine/Src/arkanimnodeblendpose.cpp stays
	a skeleton: an Engine unit cannot name it), so the bodies live here.

	agent FZ (PHASE18 FZ): SetEnable 2012 0x54a250 / 2013 0x9099c0 (56 B), BlendNow 0x54ffb0 / 2013 0x90f8c0 (201 B).
	agent HB (PHASE22 HB): InitAnim 2012 0x1c3040 (242 B; 2013 0x91b0a0, 252 B, slot 80 of ??_7UArkAnimNodeBlendPose@@6B@ -
	the same body, the 2013 TArray realloc takes one more argument), TickAnim 0x550080 / 2013 0x90f990 (729 B both),
	SetActiveChild 0x550360 / 2013 0x90fc70 (1001 B both), ForceDisable 0x54a290 (17 B), and the node's Edge tree builder
	BuildEdgeAnimTree 0x55dd40 (770 B; 2013 0x91d150, 784 B, slot 91, the same branches) written for the CPU as GetBoneAtoms.
=============================================================================*/
#include "DishonoredGame.h"
#include "fkmatineediag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_ArkAnimNodeBlendPose()
{
}

// DISHONORED(bringup): agent HB. -hbnocapture keeps the node from capturing its output (the blend then falls back to the
// reference-pose leaf, which BlendNow refuses to start): an A/B switch for the matinee pose blend.
static UBOOL DisHBCaptureDisabled()
{
	static INT Disabled = -1;
	if( Disabled < 0 )
	{
		Disabled = ( appStrfind( appCmdLine(), TEXT("-hbnocapture") ) != NULL ) ? 1 : 0;
	}
	return Disabled != 0;
}

// DISHONORED(bringup): agent HB. -hbbasepose: TickAnim and GetBoneAtoms fall back to UAnimNodeBlendBase's (the tree before HB).
static UBOOL DisHBBasePose()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-hbbasepose") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

// DISHONORED(written): agent HB. -fkdiag: one line per BlendNow, then one a second while the blend runs, and one when it ends.
void UArkAnimNodeBlendPose::ReportBlend( const TCHAR* Event )
{
	if( !DisFKDiagEnabled() )
	{
		return;
	}
	const AActor* Owner = ( SkelComponent != NULL ) ? SkelComponent->GetOwner() : NULL;
	debugf( TEXT("DISHONORED(bringup): fkdiag blendpose %s %s %.2f captured %d bones w %.2f/%.2f enabled %d blend %d world %.2f"),
		( Owner != NULL ) ? *Owner->GetName() : *GetName(), Event, BlendTimeToGo, m_PreviousPose.Num() / (INT)sizeof(FBoneAtom),
		Children.Num() > 0 ? Children(0).Weight : -1.f, Children.Num() > 1 ? Children(1).Weight : -1.f,
		(INT)m_bEnabled, m_BlendPose.Num() / (INT)sizeof(FBoneAtom), GWorld ? GWorld->GetTimeSeconds() : 0.f );
}

// DISHONORED(port): 2012 rva 0x1c3040. UAnimNodeBlendList::InitAnim's body (the 2012 linker folded the two): the target
// weights sized to the children with child 0 the target, and child 0 made active at once when no child has a weight.
void UArkAnimNodeBlendPose::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	const UBOOL bCookedEnabled = m_bEnabled;
	Super::InitAnim( MeshComp, Parent );
	if( TargetWeight.Num() != Children.Num() )
	{
		TargetWeight.Empty( Children.Num() );
		TargetWeight.AddZeroed( Children.Num() );
		if( TargetWeight.Num() > 0 )
		{
			TargetWeight(0) = 1.f;
		}
	}
	if( GetChildWeightTotal() <= ZERO_ANIMWEIGHT_THRESH )
	{
		SetActiveChild( ActiveChildIndex, 0.f );
	}
	if( DisFKDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag blendpose %s %s init: %d children (%s), w %.2f/%.2f, m_bEnabled cooked %d"),
			( MeshComp != NULL && MeshComp->GetOwner() != NULL ) ? *MeshComp->GetOwner()->GetName() : TEXT("?"), *NodeName.ToString(), Children.Num(),
			( Children.Num() > 1 && Children(1).Anim != NULL ) ? *Children(1).Anim->GetClass()->GetName() : TEXT("child 1 empty"),
			Children.Num() > 0 ? Children(0).Weight : -1.f, Children.Num() > 1 ? Children(1).Weight : -1.f, (INT)bCookedEnabled );
	}
}

// DISHONORED(port): 2012 rva 0x550080. The disable timer SetEnable( FALSE ) starts, BlendList's full-weight-on-relevance, then
// the weights toward their targets - held one frame by m_bMustBlendNextFrame, which BlendNow raises so the pose is captured first.
void UArkAnimNodeBlendPose::TickAnim( FLOAT DeltaSeconds )
{
	if( DisHBBasePose() )
	{
		Super::TickAnim( DeltaSeconds );
		return;
	}
	if( m_fDisableTimer > 0.f )
	{
		m_fDisableTimer -= DeltaSeconds;
		if( m_fDisableTimer <= 0.f )
		{
			m_bEnabled = FALSE;
			SetActiveChild( 0, 0.f );
		}
	}
	if( bJustBecameRelevant && bForceChildFullWeightWhenBecomingRelevant && Children.IsValidIndex( ActiveChildIndex ) && Children(ActiveChildIndex).Weight != 1.f )
	{
		SetActiveChild( ActiveChildIndex, 0.f );
	}
	if( BlendTimeToGo > 0.f && !m_bMustBlendNextFrame )
	{
		if( DeltaSeconds < BlendTimeToGo )
		{
			for( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
			{
				Children(ChildIndex).Weight += ( TargetWeight(ChildIndex) - Children(ChildIndex).Weight ) / BlendTimeToGo * DeltaSeconds;
			}
			BlendTimeToGo -= DeltaSeconds;
		}
		else
		{
			BlendTimeToGo = 0.f;
			for( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
			{
				Children(ChildIndex).Weight = TargetWeight(ChildIndex);
			}
			ReportBlend( TEXT("done") );
		}
		if( BlendTimeToGo > 0.f && DisFKDiagEnabled() )
		{
			static TMap<const UObject*, FLOAT> LastReportAt;
			const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
			FLOAT* Last = LastReportAt.Find( this );
			if( Last == NULL || Now - *Last >= 1.f )
			{
				LastReportAt.Set( this, Now );
				ReportBlend( TEXT("blending") );
			}
		}
	}
	m_bMustBlendNextFrame = FALSE;
	Super::TickAnim( DeltaSeconds );
}

// DISHONORED(port): 2012 rva 0x550360. UAnimNodeBlendList::SetActiveChild as Arkane built it: the not-rendered skip tests
// GIsGame (the reference engine tests !GIsEditor), and ActiveChildIndex is clamped into range before the weight scaling.
void UArkAnimNodeBlendPose::SetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	if( ChildIndex < 0 || ChildIndex >= Children.Num() )
	{
		ChildIndex = 0;
	}
	if( BlendTime > 0.f )
	{
		ActiveChildIndex = ( ActiveChildIndex < 0 ) ? 0 : Min( ActiveChildIndex, Children.Num() - 1 );
		if( ( bForceChildFullWeightWhenBecomingRelevant && bJustBecameRelevant )
			|| ( bSkipBlendWhenNotRendered && SkelComponent != NULL && !SkelComponent->bRecentlyRendered && GIsGame ) )
		{
			BlendTime = 0.f;
		}
		else if( Children(ActiveChildIndex).Anim == Children(ChildIndex).Anim )
		{
			BlendTime *= 1.f - Children(ActiveChildIndex).Weight;
		}
		else
		{
			BlendTime *= 1.f - Children(ChildIndex).Weight;
		}
	}
	for( INT Index = 0; Index < Children.Num(); Index++ )
	{
		const FLOAT Target = ( Index == ChildIndex ) ? 1.f : 0.f;
		TargetWeight(Index) = Target;
		if( BlendTime == 0.f )
		{
			Children(Index).Weight = Target;
		}
	}
	BlendTimeToGo = BlendTime;
	ActiveChildIndex = ChildIndex;
	if( bPlayActiveChild && Children.IsValidIndex( ActiveChildIndex ) )
	{
		UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>( Children(ActiveChildIndex).Anim );
		if( SeqNode != NULL )
		{
			SeqNode->ReplayAnim();
		}
	}
}

// DISHONORED(port): 2012 rva 0x54a250. Enabling arms the capture at once; disabling starts a one-second timer TickAnim counts down.
void UArkAnimNodeBlendPose::SetEnable( UBOOL bEnable )
{
	if( bEnable )
	{
		m_bEnabled = TRUE;
		m_fDisableTimer = -1.f;
	}
	else
	{
		m_fDisableTimer = 1.f;
	}
}

// DISHONORED(port): 2012 rva 0x54a290: the disable timer expires on the next tick.
void UArkAnimNodeBlendPose::ForceDisable()
{
	m_fDisableTimer = 0.0001f;
}

// DISHONORED(port): 2012 rva 0x54ffb0. Only an enabled node holding a captured pose blends: the next evaluation freezes that
// capture (m_bDoBlend), child 1 (the frozen pose) goes to weight 1 and child 0 (the live tree) to 0, and TickAnim takes them
// back to the targets over BlendTime, starting a frame later when bWaitForNextFrame.
void UArkAnimNodeBlendPose::BlendNow( FLOAT BlendTime, UBOOL bWaitForNextFrame )
{
	if( !m_bEnabled || m_PreviousPose.Num() == 0 || Children.Num() < 2 )
	{
		ReportBlend( TEXT("BlendNow refused") );
		return;
	}
	m_bDoBlend = TRUE;
	Children(0).Weight = 0.f;
	Children(1).Weight = 1.f;
	ActiveChildIndex = 0;
	m_bMustBlendNextFrame = bWaitForNextFrame ? TRUE : FALSE;
	BlendTimeToGo = BlendTime;
	ReportBlend( TEXT("BlendNow") );
}

// DISHONORED(port): agent HB. UArkAnimNodeBlendPose::BuildEdgeAnimTree, 2012 rva 0x55dd40 / 2013 0x91d150, written for the CPU
// evaluator. Retail builds one Edge branch: left = child 0's subtree, right = a leaf, op 0 (linear), alpha = child 1's Weight
// (child 1's node is never evaluated); every child's BlendWeight is set to its Weight first.
//   m_bDoBlend   the capture of the previous evaluation (m_PreviousPose) becomes the frozen pose (m_BlendPose), flag cleared.
//   m_bEnabled   the branch carries flags 4 and userVal = m_PreviousPose sized (numJoints + 1) * 48: _edgeAnimProcessCommandList
//                (2012 0x6238c0, command 3) blends only when alpha > 0 and then copies the branch's output into userVal - the
//                node captures what it outputs, every evaluation.
//   disabled     with child 1 at weight 0 the frozen pose is freed.
//   the leaf     alpha >= 0.0001 with a frozen pose: flags 2, the frozen pose copied in (command 5), every joint defined but the
//                locomotion joint; otherwise flags 4 with no header: the skeleton's base pose, same definition.
// So the output is lerp( live, frozen or reference, alpha ) per bone, and the root motion is the live tree's (the right leaf
// never defines the locomotion joint). The capture here is FBoneAtom per bone index (this evaluator's atoms), not Edge joints.
void UArkAnimNodeBlendPose::GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys )
{
	if( DisHBBasePose() )
	{
		Super::GetBoneAtoms( Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
		return;
	}
	if( GetCachedResults( Atoms, RootMotionDelta, bHasRootMotion, CurveKeys, DesiredBones.Num() ) )
	{
		return;
	}
	for( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		Children(ChildIndex).BlendWeight = Children(ChildIndex).Weight;
	}
	const TArray<FMeshBone>& RefSkeleton = SkelComponent->SkeletalMesh->RefSkeleton;
	if( Children.Num() > 0 && Children(0).Anim != NULL )
	{
		GetChildBoneAtoms( 0, Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
	}
	else
	{
		RootMotionDelta.SetIdentity();
		bHasRootMotion = 0;
		FillWithRefPose( Atoms, DesiredBones, RefSkeleton );
	}

	const INT PoseBytes = Min( Atoms.Num(), RefSkeleton.Num() ) * sizeof(FBoneAtom);
	const FLOAT RightWeight = ( Children.Num() > 1 ) ? Children(1).Weight : 0.f;
	if( m_bDoBlend )
	{
		m_BlendPose = m_PreviousPose;
		m_bDoBlend = FALSE;
	}
	if( !m_bEnabled && RightWeight == 0.f )
	{
		m_BlendPose.Empty();
	}
	if( RightWeight > 0.f )
	{
		const UBOOL bFrozen = RightWeight >= 0.0001f && m_BlendPose.Num() == PoseBytes;
		FBoneAtomArray RefAtoms;
		if( !bFrozen )
		{
			RefAtoms.Add( RefSkeleton.Num() );
			FillWithRefPose( RefAtoms, DesiredBones, RefSkeleton );
		}
		for( INT BoneSlot = 0; BoneSlot < DesiredBones.Num(); BoneSlot++ )
		{
			const INT BoneIndex = DesiredBones(BoneSlot);
			// the frozen pose is a BYTE array (8-byte aligned); FBoneAtom is a 16-byte aligned SSE type, so it is copied out
			FBoneAtom Right;
			appMemcpy( &Right, bFrozen ? &m_BlendPose( BoneIndex * sizeof(FBoneAtom) ) : (const BYTE*)&RefAtoms(BoneIndex), sizeof(FBoneAtom) );
			Atoms(BoneIndex).Blend( Atoms(BoneIndex), Right, RightWeight );
		}
	}
	if( m_bEnabled && !DisHBCaptureDisabled() )
	{
		if( m_PreviousPose.Num() != PoseBytes )
		{
			m_PreviousPose.Empty( PoseBytes );
			m_PreviousPose.Add( PoseBytes );
		}
		appMemcpy( m_PreviousPose.GetData(), Atoms.GetData(), PoseBytes );
	}
	SaveCachedResults( Atoms, RootMotionDelta, bHasRootMotion, CurveKeys, DesiredBones.Num() );
}
