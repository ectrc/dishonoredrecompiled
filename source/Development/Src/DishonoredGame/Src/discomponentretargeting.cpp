// DishonoredGame/src/discomponentretargeting.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x8af8b0  public: __thiscall FDisComponentRetargeting::FDisComponentRetargeting(void)
//   0x8af970  public: virtual unsigned long __thiscall FDisComponentRetargeting::GetMemoryFootprint(void)const
//   0x8b37e0  public: void __thiscall FDisComponentRetargeting::UpdateRetargeting(float)
//   0x8d0eb0  public: void __thiscall FDisComponentRetargeting::StopRetargeting(void)
//   0x8d10f0  protected: void __thiscall FDisComponentRetargeting::OnStopRetargeting(class FArkGameEvent const &)
//   0x8d1100  protected: void __thiscall FDisComponentRetargeting::SetMasterActor(class AActor * const)
//   0x8d2330  public: virtual void __thiscall FDisComponentRetargeting::Stopping(void)
//   0x8d23c0  public: virtual void __thiscall FDisComponentRetargeting::PreAsyncWorkTick(float)
//   0x8d2400  public: void __thiscall FDisComponentRetargeting::StartRetargeting(class AActor * const, class FName, class FName, unsigned int, unsigned int, float, unsigned int)

#include "DishonoredGame.h"
#include "discomponentretargeting.h"
#include "arkcomponentpolicy.h"
#include "arkcomponentmeshoffset.h"
#include "arkgameeventdispatcher.h"

ARKCOMPONENT_IMPLEMENT_TYPE( FDisComponentRetargeting )

namespace
{
// DISHONORED(port): the EDisGameEventType both actors are watched for (2012 0x8d2400 / 2013 0xc88440: `_EventType = 13`,
// RegisterToObjectEvent per sender); any of them raising it ends the retargeting
const INT RetargetingStopEventType = 13;

UBOOL DisRetargetDiag()
{
	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("gadiag") );
	return bDiag;
}

FArkComponentPreAsyncWorkPolicy* DisRetargetPolicy()
{
	return ( GWorld && GWorld->m_pComponentManager ) ? &GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>() : NULL;
}
}

// DISHONORED(port): 2012 0x8af8b0 (98 B) / 2013 0xc66c90. m_bManageVolumeCollisions is not written (retail's): StartRetargeting
// sets it before anything reads it.
FDisComponentRetargeting::FDisComponentRetargeting()
	: m_iMasterBoneIndex( INDEX_NONE )
	, m_iSlaveBoneIndex( INDEX_NONE )
	, m_pMasterSkocket( NULL )
	, m_pMaster( NULL )
	, m_pMasterPawn( NULL )
	, m_pSlave( NULL )
	, m_pMasterSkeletalComp( NULL )
	, m_pSlaveSkeletalComp( NULL )
	, m_OldRMM( RMM_Ignore )
	, m_OldRMRM( RMRM_Ignore )
	, m_bIsRetargeting( FALSE )
	, m_bAllowRotation( TRUE )
	, m_bKeepActorCloser( FALSE )
	, m_bIsInTick( FALSE )
	, m_bShouldbeStopped( FALSE )
	, m_fTimeLeft( 0.f )
{
}

// DISHONORED(port): 2012 0x8d2330 (130 B) / 2013 0xc88370: out of the manager's PreAsyncWork policy, both stop events dropped
void FDisComponentRetargeting::Stopping()
{
	if( FArkComponentPreAsyncWorkPolicy* Policy = DisRetargetPolicy() )
	{
		Policy->Unregister( this );
	}
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pMaster, this, &FDisComponentRetargeting::OnStopRetargeting );
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pSlave, this, &FDisComponentRetargeting::OnStopRetargeting );
	}
}

// DISHONORED(port): 2012 0x8d23c0 (57 B) / 2013 0xc88400: a stop asked for during the update is carried out after it
void FDisComponentRetargeting::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	m_bIsInTick = TRUE;
	UpdateRetargeting( _fTimeStep );
	m_bIsInTick = FALSE;
	if( m_bShouldbeStopped )
	{
		StopRetargeting();
	}
}

// DISHONORED(port): 2012 0x8d10f0 (8 B)
void FDisComponentRetargeting::OnStopRetargeting( const FArkGameEvent& _rEvent )
{
	StopRetargeting();
}

// DISHONORED(port): 2012 0x8d1100 (116 B) / 2013 0xc87690. A pawn master gives its body mesh, a matinee skeletal-mesh actor (the
// boat) its SkeletalMeshComponent; any other actor leaves the master mesh as it was (retail's).
void FDisComponentRetargeting::SetMasterActor( AActor* const _pActor )
{
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pMaster, this, &FDisComponentRetargeting::OnStopRetargeting );
	}
	m_pMaster = _pActor;
	ADishonoredPawn* MasterPawn = Cast<ADishonoredPawn>( _pActor );
	ASkeletalMeshActorMAT* MasterMAT = Cast<ASkeletalMeshActorMAT>( _pActor );
	if( MasterPawn )
	{
		m_pMasterPawn = MasterPawn;
		m_pMasterSkeletalComp = MasterPawn->Mesh;
	}
	else if( MasterMAT )
	{
		m_pMasterSkeletalComp = MasterMAT->SkeletalMeshComponent;
	}
}

// DISHONORED(port): 2013 0xc88440 (578 B; 2012 0x8d2400, 569 B). Registers with the manager's PreAsyncWork policy (the 2013
// GetPolicy is the ICF-folded `mov eax,ecx`), takes the master, makes the owner the slave (or, when the owner is the master,
// the player), watches both for the stop event, resolves the master's socket (else bone) and the slave's bone, remembers the
// slave mesh's root-motion modes, and for the owner: collision off and root motion ignored when it is the master, the mesh
// offset locked and zeroed (2013 also zeroes the mesh offset's pitch: 2013 0xb498c0 -> 0x8f1c10, not in 2012), the rotation
// intents locked at DisFaceToPriority_ReTargeting and cancelled, PHYS_None. Then one update with no time.
// DISHONORED(written): retail reads the master mesh unchecked and calls the NPC rotation lock on the owner unchecked; a master
// without a skeletal mesh is refused here (named once), and the rotation calls are made on an NPC owner only.
void FDisComponentRetargeting::StartRetargeting( AActor* const _pMaster, FName _MasterBoneName, FName _SlaveBoneName, UBOOL _bAllowRotation,
	UBOOL _bKeepActorCloser, FLOAT _fRetargetTime, UBOOL _bManageVolumeCollisions )
{
	if( FArkComponentPreAsyncWorkPolicy* Policy = DisRetargetPolicy() )
	{
		Policy->Register( this );
	}
	m_bAllowRotation = _bAllowRotation;
	m_fTimeLeft = _fRetargetTime;
	m_bKeepActorCloser = _bKeepActorCloser;
	m_bManageVolumeCollisions = _bManageVolumeCollisions;
	SetMasterActor( _pMaster );

	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	ADishonoredPawn* Owner = Cast<ADishonoredPawn>( m_pOwner );
	if( Dispatcher )
	{
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pSlave, this, &FDisComponentRetargeting::OnStopRetargeting );
	}
	if( _pMaster == m_pOwner )
	{
		m_pSlave = ADishonoredPlayerPawn::s_pInstance;
		Owner = Cast<ADishonoredPawn>( _pMaster );
	}
	else
	{
		m_pSlave = Owner;
	}
	m_pSlaveSkeletalComp = m_pSlave ? m_pSlave->Mesh : NULL;
	if( m_pMasterSkeletalComp == NULL || m_pSlaveSkeletalComp == NULL )
	{
		static UBOOL bWarned = FALSE;
		if( !bWarned )
		{
			bWarned = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(written): FDisComponentRetargeting::StartRetargeting: %s has no skeletal mesh to retarget %s onto"),
				_pMaster ? *_pMaster->GetName() : TEXT("none"), m_pOwner ? *m_pOwner->GetName() : TEXT("none") );
		}
		if( FArkComponentPreAsyncWorkPolicy* Policy = DisRetargetPolicy() )
		{
			Policy->Unregister( this );
		}
		return;
	}
	if( Dispatcher )
	{
		Dispatcher->RegisterToObjectEvent( RetargetingStopEventType, m_pMaster, this, &FDisComponentRetargeting::OnStopRetargeting );
		Dispatcher->RegisterToObjectEvent( RetargetingStopEventType, m_pSlave, this, &FDisComponentRetargeting::OnStopRetargeting );
	}

	m_pMasterSkocket = m_pMasterSkeletalComp->SkeletalMesh ? m_pMasterSkeletalComp->SkeletalMesh->FindSocket( _MasterBoneName ) : NULL;
	if( m_pMasterSkocket == NULL )
	{
		m_iMasterBoneIndex = m_pMasterSkeletalComp->MatchRefBone( _MasterBoneName );
	}
	m_iSlaveBoneIndex = m_pSlaveSkeletalComp->MatchRefBone( _SlaveBoneName );
	m_OldRMM = m_pSlaveSkeletalComp->RootMotionMode;
	m_OldRMRM = m_pSlaveSkeletalComp->RootMotionRotationMode;

	if( Owner )
	{
		if( Owner == m_pMaster )
		{
			if( m_bManageVolumeCollisions )
			{
				Owner->SetCollision( FALSE, FALSE, FALSE );
			}
			Owner->ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore );
		}
		Owner->LockMeshTranslation( TRUE, ADishonoredPawn::eMeshOffsetDisable_ByRetargeting );
		Owner->SetMeshTranslation( FVector::ZeroVector, _fRetargetTime );
		FArkComponentMeshOffset* MeshOffset = FArkComponentMeshOffset::GetMeshOffsetComponent( Owner, TRUE );
		if( MeshOffset && Owner->Physics != PHYS_RigidBody )
		{
			MeshOffset->ForcePitch( 0 );
		}
		if( ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( Owner ) )
		{
			NPC->LockNPCRotation( DisFaceToPriority_ReTargeting, this, GetInternalName() );
			NPC->CancelAllNPCRotations();
		}
		Owner->setPhysics( PHYS_None );
	}
	m_bIsRetargeting = TRUE;
	m_bShouldbeStopped = FALSE;
	if( DisRetargetDiag() )
	{
		debugf( TEXT("gadiag retarget %s master %s socket %s anchor %s (bone %i/%i) rot %i closer %i time %.2f volumes %i start"),
			m_pSlave ? *m_pSlave->GetName() : TEXT("none"), *m_pMaster->GetName(), m_pMasterSkocket ? *_MasterBoneName.ToString() : TEXT("none"),
			*_SlaveBoneName.ToString(), m_iMasterBoneIndex, m_iSlaveBoneIndex, m_bAllowRotation, m_bKeepActorCloser, m_fTimeLeft, m_bManageVolumeCollisions );
	}
	UpdateRetargeting( 0.f );
}

// DISHONORED(port): 2013 0xc6ace0 (4,824 B, unnamed; StartRetargeting's and PreAsyncWorkTick's callee) / 2012 0x8b37e0 (4,308 B).
// The slave mesh's world transform is taken with its translation/rotation offset zeroed (SetTransformedToWorld, vtable +516 in
// 2012 / +520 in 2013, then the offset restored without a second update), the master bone or socket and the slave bone read,
// and the mesh re-expressed in the slave bone's frame. With m_bAllowRotation off the master bone is first turned about the
// master's origin, in yaw only, until it points where the slave bone is. The mesh is then put so that its bone lies on the
// master bone, and the actor follows the mesh: over m_fTimeLeft the move is blended in (lerp, slerp). A non-NPC slave (the
// player) is far-moved; an NPC is moved (m_bKeepActorCloser) or set, its rotation written directly, the volumes it swept are
// touched and untouched with bCollideActors forced on, and what the move fell short of becomes the mesh translation.
void FDisComponentRetargeting::UpdateRetargeting( FLOAT _fTimeStep )
{
	USkeletalMeshComponent* SlaveComp = m_pSlaveSkeletalComp;
	const FVector SavedTranslation = SlaveComp->Translation;
	const FRotator SavedRotation = SlaveComp->Rotation;
	SlaveComp->Translation = FVector( 0.f, 0.f, 0.f );
	SlaveComp->Rotation = FRotator( 0, 0, 0 );
	SlaveComp->SetTransformedToWorld();
	const FMatrix SlaveMeshToWorld = SlaveComp->CalcCurrentLocalToWorld( m_pSlave->LocalToWorld() );
	FMatrix MasterBoneToWorld;
	if( m_pMasterSkocket )
	{
		m_pMasterSkocket->GetSocketMatrix( MasterBoneToWorld, m_pMasterSkeletalComp );
	}
	else
	{
		MasterBoneToWorld = m_pMasterSkeletalComp->GetBoneMatrix( m_iMasterBoneIndex );
	}
	const FMatrix SlaveBoneToWorld = SlaveComp->GetBoneMatrix( m_iSlaveBoneIndex );
	SlaveComp->Translation = SavedTranslation;
	SlaveComp->Rotation = SavedRotation;

	const FMatrix SlaveMeshInBone = SlaveMeshToWorld * SlaveBoneToWorld.Inverse();
	if( !m_bAllowRotation )
	{
		const FMatrix MasterToWorld = m_pMaster->LocalToWorld();
		const FMatrix MasterBoneInMaster = MasterBoneToWorld * MasterToWorld.Inverse();
		const FVector ToSlaveBone = FVector( SlaveBoneToWorld.M[3][0] - MasterToWorld.M[3][0], SlaveBoneToWorld.M[3][1] - MasterToWorld.M[3][1], 0.f ).SafeNormal();
		const FVector ToMasterBone = FVector( MasterBoneToWorld.M[3][0] - MasterToWorld.M[3][0], MasterBoneToWorld.M[3][1] - MasterToWorld.M[3][1], 0.f ).SafeNormal();
		const FLOAT CrossZ = ToSlaveBone.X * ToMasterBone.Y - ToMasterBone.X * ToSlaveBone.Y;
		FLOAT Angle = appAcos( Clamp( ToMasterBone | ToSlaveBone, -1.f, 1.f ) );
		if( CrossZ > 0.f )
		{
			Angle = -Angle;
		}
		MasterBoneToWorld = MasterBoneInMaster * FRotationTranslationMatrix( FRotator( 0, appTrunc( Angle * 10430.378f ), 0 ), FVector::ZeroVector ) * MasterToWorld;
	}
	const FMatrix NewSlaveMeshToWorld = SlaveMeshInBone * MasterBoneToWorld;
	const FMatrix NewSlaveToWorld = m_pSlave->LocalToWorld() * SlaveMeshToWorld.Inverse() * NewSlaveMeshToWorld;

	const FVector OldLocation = m_pSlave->Location;
	const FQuat OldQuat = m_pSlave->Rotation.Quaternion();
	FVector NewLocation = NewSlaveToWorld.GetOrigin();
	FQuat NewQuat = NewSlaveToWorld.Rotator().Quaternion();
	if( m_fTimeLeft > 0.f )
	{
		const FLOAT Alpha = Min( _fTimeStep / m_fTimeLeft, 1.f );
		m_fTimeLeft -= _fTimeStep;
		NewLocation = OldLocation + ( NewLocation - OldLocation ) * Alpha;
		NewQuat = SlerpQuat( OldQuat, NewQuat, Alpha );
	}
	const FRotator NewRotation = FQuatRotationTranslationMatrix( NewQuat, FVector::ZeroVector ).Rotator();

	// DISHONORED(written): retail's FarMoveActor carries a sixth Dishonored argument (-1.f) that this tree's has not
	if( !m_pSlave->IsA( ADishonoredNPCPawn::StaticClass() ) )
	{
		GWorld->FarMoveActor( m_pSlave, NewLocation, FALSE, TRUE, FALSE );
		m_pSlave->SetRotation( NewRotation );
		return;
	}
	if( !m_bKeepActorCloser || !m_pSlave->Move( NewLocation - m_pSlave->Location ) )
	{
		m_pSlave->SetLocation( NewLocation );
	}
	m_pSlave->Rotation = NewRotation;
	if( m_bManageVolumeCollisions )
	{
		FMemMark Mark( GMainThreadMemStack );
		const FVector Extent = m_pSlave->GetCylinderExtent();
		FCheckResult* Hits = GWorld->MultiLineCheck( GMainThreadMemStack, NewLocation, OldLocation, FVector( Extent.X * 1.001f, Extent.Y * 1.001f, Extent.Z ),
			TRACE_Volumes, m_pSlave );
		const UBOOL bOldCollideActors = m_pSlave->bCollideActors;
		m_pSlave->bCollideActors = TRUE;
		for( FCheckResult* Hit = Hits; Hit; Hit = Hit->GetNext() )
		{
			m_pSlave->BeginTouch( Hit->Actor, Hit->Component, Hit->Location, Hit->Normal, Hit->SourceComponent );
		}
		m_pSlave->UnTouchActors();
		m_pSlave->bCollideActors = bOldCollideActors;
		Mark.Pop();
	}
	const FVector Shortfall = NewLocation - m_pSlave->Location;
	m_pSlave->SetMeshTranslation( m_pSlave->LocalToWorld().Inverse().TransformNormal( Shortfall ), 0.f );

	if( DisRetargetDiag() && GWorld && GWorld->GetWorldInfo() )
	{
		static TMap<ADishonoredPawn*, FLOAT> s_NextReport;
		const FLOAT Now = GWorld->GetWorldInfo()->TimeSeconds;
		FLOAT* Next = s_NextReport.Find( m_pSlave );
		if( Next == NULL || Now >= *Next )
		{
			s_NextReport.Set( m_pSlave, Now + 1.f );
			const FVector InSocket = MasterBoneToWorld.InverseTransformFVector( m_pSlave->Location );
			const FMatrix AnchorNow = SlaveMeshInBone.Inverse() * SlaveComp->CalcCurrentLocalToWorld( m_pSlave->LocalToWorld() );
			const FLOAT AnchorError = ( AnchorNow.GetOrigin() - MasterBoneToWorld.GetOrigin() ).Size();
			const FLOAT QuatDot = Min( 1.f, Abs( FQuat( AnchorNow ) | FQuat( MasterBoneToWorld ) ) );
			debugf( TEXT("gadiag retarget %s on %s at %.1f/%.1f/%.1f yaw %i | in socket %.2f/%.2f/%.2f anchor err %.2f uu %.2f deg | socket at %.1f/%.1f/%.1f | moved %.2f meshT %.2f/%.2f/%.2f phys %i base %s"),
				*m_pSlave->GetName(), *m_pMaster->GetName(), m_pSlave->Location.X, m_pSlave->Location.Y, m_pSlave->Location.Z, m_pSlave->Rotation.Yaw & 0xFFFF,
				InSocket.X, InSocket.Y, InSocket.Z, AnchorError, 2.f * appAcos( QuatDot ) * ( 180.f / PI ),
				MasterBoneToWorld.M[3][0], MasterBoneToWorld.M[3][1], MasterBoneToWorld.M[3][2], ( NewLocation - OldLocation ).Size(),
				SlaveComp->Translation.X, SlaveComp->Translation.Y, SlaveComp->Translation.Z, (INT)m_pSlave->Physics,
				m_pSlave->Base ? *m_pSlave->Base->GetName() : TEXT("none") );
		}
	}
}

// DISHONORED(port): 2013 0xc87430 (589 B; 2012 0x8d0eb0, 563 B). Deferred while the component is ticking; otherwise out of the
// policy, both events dropped, and the NPC (the slave, else the master) is given back: the mesh offset unlocked and - unless it
// is a ragdoll - dropped 100 uu onto whatever is below, walking again with the mesh translation corrected by the drop, its
// root-motion modes and collision restored when it was the master, the ReTargeting rotation lock released. 2013 skips the
// give-back for an actor that has terminated (m_bLegalToTerminate and m_bHasTerminated, AActor +300 bits 4 and 3), 2012 does not.
void FDisComponentRetargeting::StopRetargeting()
{
	if( !m_bIsRetargeting )
	{
		return;
	}
	if( m_bIsInTick )
	{
		m_bShouldbeStopped = TRUE;
		return;
	}
	if( FArkComponentPreAsyncWorkPolicy* Policy = DisRetargetPolicy() )
	{
		Policy->Unregister( this );
	}
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pMaster, this, &FDisComponentRetargeting::OnStopRetargeting );
		Dispatcher->UnregisterToObjectEvent( RetargetingStopEventType, m_pSlave, this, &FDisComponentRetargeting::OnStopRetargeting );
	}
	ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( m_pSlave );
	if( NPC == NULL )
	{
		NPC = Cast<ADishonoredNPCPawn>( m_pMaster );
	}
	if( NPC && ( !GIsGame || !NPC->m_bLegalToTerminate || !NPC->m_bHasTerminated ) )
	{
		NPC->LockMeshTranslation( FALSE, ADishonoredPawn::eMeshOffsetDisable_ByRetargeting );
		if( NPC->Physics != PHYS_RigidBody )
		{
			const FVector OldLocation = NPC->Location;
			FCheckResult Hit;
			GWorld->MoveActor( NPC, FVector( 0.f, 0.f, -100.f ), NPC->Rotation, 0, Hit );
			NPC->setPhysics( PHYS_Walking );
			const FVector Translation = NPC->Mesh->Translation - ( NPC->Location - OldLocation );
			NPC->SetMeshTranslation( Translation, 0.f );
			if( NPC == m_pMaster )
			{
				NPC->ChangeAnimStateMotionModes( (BYTE)m_OldRMM, (BYTE)m_OldRMRM );
				if( m_bManageVolumeCollisions )
				{
					NPC->SetCollision( TRUE, TRUE, FALSE );
				}
			}
			NPC->UnlockNPCRotation( DisFaceToPriority_ReTargeting );
		}
		if( DisRetargetDiag() )
		{
			debugf( TEXT("gadiag retarget %s master %s stop at %.1f/%.1f/%.1f yaw %i phys %i base %s"), *NPC->GetName(), m_pMaster ? *m_pMaster->GetName() : TEXT("none"),
				NPC->Location.X, NPC->Location.Y, NPC->Location.Z, NPC->Rotation.Yaw & 0xFFFF, (INT)NPC->Physics, NPC->Base ? *NPC->Base->GetName() : TEXT("none") );
		}
	}
	m_bIsRetargeting = FALSE;
	m_pMasterPawn = NULL;
	m_pMaster = NULL;
	m_pSlave = NULL;
	m_pMasterSkeletalComp = NULL;
	m_pSlaveSkeletalComp = NULL;
	m_bShouldbeStopped = FALSE;
}
