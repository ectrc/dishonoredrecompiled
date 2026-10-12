// Engine/src/interptrackinstlocomotion.cpp
// DISHONORED(port): agent GA (PHASE18 package GA). The half of UInterpTrackInstLocomotion that does not touch the
// locomotion component (which is declared in DishonoredGame: the other half is DishonoredGame/Src/
// interptracklocomotion_game.cpp). Decompiles in build/agentGA/decomp2012 and decomp2013; both databases name every
// function below at the sizes quoted.
// PDB functions attributed to this file (12):
//   0x53bb70  public: static void __cdecl UInterpTrackInstLocomotion::InitializePrivateStaticClassUInterpTrackInstLocomotion(void)
//   0x53bb90  public: virtual void __thiscall UInterpTrackInstLocomotion::TermTrackInst(class UInterpTrack *)           - game unit
//   0x53bc00  public: void __thiscall UInterpTrackInstLocomotion::CallbackLocoEvent(int, enum EArkCpntLocoEvent)       - game unit
//   0x53bc40  public: virtual void __thiscall UInterpTrackInstLocomotion::LoadData(class FArchive &)                   - not ported (the save layer's)
//   0x53cdc0  public: static class UClass * __cdecl UInterpTrackInstLocomotion::GetPrivateStaticClassUInterpTrackInstLocomotion(wchar_t const *)
//   0x53ce50  public: void __thiscall UInterpTrackInstLocomotion::StopCurrentLocoRequest(class FArkComponentLocomotion * const, int) - game unit
//   0x53efc0  public: static class UClass * __cdecl UInterpTrackInstLocomotion::StaticClassNoInline(void)
//   0x53eff0  public: void __thiscall UInterpTrackInstLocomotion::CalcInitialTransform(class UInterpTrack *, unsigned int)
//   0x53f5e0  public: unsigned int __thiscall UInterpTrackInstLocomotion::UpdateTrackInst_Jump(float)
//   0x53f760  public: void __thiscall UInterpTrackInstLocomotion::UpdateStopObsoleteLocoRequest(class FArkComponentLocomotion * const, float) - game unit
//   0x542380  public: virtual void __thiscall UInterpTrackInstLocomotion::InitTrackInst(class UInterpTrack *)
//   0x5423d0  public: void __thiscall UInterpTrackInstLocomotion::ComputeLocoProp(struct FArkCpntLocoGoToActorProp &, class FArkComponentLocomotion *, class AActor const * const, class UInterpTrackLocomotionKeyProperties const * const, float) - game unit
// and, attributed to interptracklocomotion.cpp: 0x53fa50 NeedsSynchronizing, 0x542630 UpdateTrackInst (game unit).

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"

// UnInterpolation.cpp's file-static GetBaseMatrix (the Move track's), which the Locomotion instance's initial
// transform uses too: a based actor's frame is its base's, or the bone it is attached to.
static FMatrix DisLocoBaseMatrix( AActor* Actor )
{
	if( !Actor->Base )
	{
		return FMatrix::Identity;
	}
	if( Actor->BaseSkelComponent )
	{
		const INT BoneIndex = Actor->BaseSkelComponent->MatchRefBone( Actor->BaseBoneName );
		if( BoneIndex != INDEX_NONE )
		{
			return Actor->BaseSkelComponent->GetBoneMatrix( BoneIndex );
		}
	}
	return FRotationTranslationMatrix( Actor->Base->Rotation, Actor->Base->Location );
}

APawn* UInterpTrackInstLocomotion::GetGroupPawn() const
{
	UInterpGroupInst* GrInst = Cast<UInterpGroupInst>( GetOuter() );
	return GrInst ? GrInst->GetGroupPawn() : NULL;
}

// 2012 0x542380 (68 B), 2013 0x503790: retail does not call the base - the track is remembered, the initial transform
// computed, the goal and request bookkeeping reset.
void UInterpTrackInstLocomotion::InitTrackInst( UInterpTrack* InTrack )
{
	Track = InTrack;
	CalcInitialTransform( InTrack, FALSE );
	m_CurrentGoalKeyIndex = INDEX_NONE;
	m_LastReachedGoalKeyIndex = INDEX_NONE;
	m_LastPosition = -1.f;
	m_iCurrentLocoRequestId = INDEX_NONE;
	m_iPendingStopLocoRequestId = INDEX_NONE;
}

// 2012 0x53eff0 (1,510 B), 2013 0x4ffc70: the Move track's CalcInitialTransform for a group that walks - an AI group
// starts from its stage mark (plus the collision height and, in game, the mesh translation), any other group from the
// actor's own transform; a based actor's transform is taken into its base's frame. The 2012 body then derives
// InitialTM / InitialQuat and the reset transform the way UInterpTrackInstMove does.
// DISHONORED(bringup): the decompile's tail (the quaternion and the reset pair) was read from UInterpTrackInstMove's,
// which this instance's reflected members mirror; nothing in the Locomotion update reads them.
void UInterpTrackInstLocomotion::CalcInitialTransform( UInterpTrack* InTrack, UBOOL bZeroFromHere )
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GrInst->GetOuter() );
	AActor* Actor = GetGroupActor();
	if( !Actor )
	{
		return;
	}
	FMatrix ActorTM;
	UInterpGroupInstAI* AIGrInst = Cast<UInterpGroupInstAI>( GrInst );
	if( AIGrInst )
	{
		FRotator InitRot;
		FVector InitLoc = AIGrInst->GetStageMarkPosition( &InitRot );
		APawn* Pawn = GetGroupPawn();
		if( Pawn )
		{
			if( Pawn->CylinderComponent )
			{
				InitLoc.Z += Pawn->CylinderComponent->CollisionHeight;
			}
			if( GIsGame && Pawn->Mesh )
			{
				InitLoc.Z += Pawn->Mesh->Translation.Z;
			}
		}
		ActorTM = FRotationTranslationMatrix( InitRot, InitLoc );
	}
	else
	{
		ActorTM = FRotationTranslationMatrix( Actor->Rotation, Actor->Location );
	}
	if( Actor->GetBase() )
	{
		ActorTM = ActorTM * DisLocoBaseMatrix( Actor ).Inverse();
	}
	InitialTM = ActorTM;
	InitialQuat = FQuat( ActorTM );
	ResetLocation = Actor->Location;
	ResetRotation = Actor->Rotation;
}

// 2012 0x53fa50 (343 B), 2013 0x5006b0: StepInterp asks whether the matinee must hold. It must when the current (or,
// with none current, the first key the step crosses) goal key has WaitForGoalReached, has not been reached, and the
// step would pass its end: the position is clamped just short of that end.
UBOOL UInterpTrackInstLocomotion::NeedsSynchronizing( FLOAT _fOldPosition, FLOAT _fNewPosition, FLOAT& _fOutClampedPosition, UBOOL _bPreview )
{
	if( _bPreview || !GetGroupPawn() )
	{
		return FALSE;
	}
	const UInterpTrackLocomotion* LocoTrack = CastChecked<UInterpTrackLocomotion>( Track );
	INT KeyIndex = m_CurrentGoalKeyIndex;
	if( KeyIndex == INDEX_NONE )
	{
		for( KeyIndex = 0; KeyIndex < LocoTrack->LocoKeys.Num(); KeyIndex++ )
		{
			const FLOAT KeyStart = LocoTrack->LocoKeys(KeyIndex).StartTime;
			if( ( KeyStart > _fOldPosition || ( _fOldPosition == 0.f && KeyStart == 0.f ) ) && _fNewPosition >= KeyStart )
			{
				break;
			}
		}
		if( KeyIndex >= LocoTrack->LocoKeys.Num() )
		{
			return FALSE;
		}
	}
	const FLocomotionControlTrackKey& Key = LocoTrack->LocoKeys(KeyIndex);
	const FLOAT KeyEnd = Key.StartTime + Key.KeyLength;
	if( !Key.Properties || !Key.Properties->WaitForGoalReached || m_LastReachedGoalKeyIndex == KeyIndex || KeyEnd < _fOldPosition || _fNewPosition < KeyEnd )
	{
		return FALSE;
	}
	_fOutClampedPosition = KeyEnd - 0.0001f;
	return TRUE;
}

// 2012 0x53f5e0 (370 B), 2013 0x500260: a jump forward while the matinee is skipping is left to the normal update
// (FALSE); any other jump that leaves the current goal key behind (forward past its end, or backward into it) retires
// the request - the stop is deferred to the next update through m_iPendingStopLocoRequestId - and the last position is
// set just short of the new one so the next update re-evaluates the keys.
UBOOL UInterpTrackInstLocomotion::UpdateTrackInst_Jump( FLOAT NewPosition )
{
	USeqAct_Interp* Matinee = GetMatinee();
	if( Matinee && Matinee->m_bSkipping && NewPosition > m_LastPosition )
	{
		return FALSE;
	}
	const UInterpTrackLocomotion* LocoTrack = CastChecked<UInterpTrackLocomotion>( Track );
	if( m_CurrentGoalKeyIndex != INDEX_NONE && LocoTrack->LocoKeys.IsValidIndex( m_CurrentGoalKeyIndex ) )
	{
		const FLocomotionControlTrackKey& Key = LocoTrack->LocoKeys(m_CurrentGoalKeyIndex);
		if( m_LastPosition <= NewPosition )
		{
			if( Key.StartTime + Key.KeyLength < NewPosition )
			{
				m_iPendingStopLocoRequestId = m_iCurrentLocoRequestId;
				m_iCurrentLocoRequestId = INDEX_NONE;
				m_LastReachedGoalKeyIndex = m_CurrentGoalKeyIndex;
				m_CurrentGoalKeyIndex = INDEX_NONE;
			}
		}
		else if( Key.StartTime < NewPosition && NewPosition <= Key.StartTime + Key.KeyLength )
		{
			m_iPendingStopLocoRequestId = m_iCurrentLocoRequestId;
			m_iCurrentLocoRequestId = INDEX_NONE;
			m_LastReachedGoalKeyIndex = INDEX_NONE;
			m_CurrentGoalKeyIndex = INDEX_NONE;
		}
	}
	m_LastPosition = NewPosition - 0.0001f;
	return TRUE;
}
