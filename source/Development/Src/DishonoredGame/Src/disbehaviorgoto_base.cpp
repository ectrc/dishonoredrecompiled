// DishonoredGame/src/disbehaviorgoto_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (19), 2012 rvas:
//   0x745530  public: static void __cdecl UDisBehaviorGoTo_Base::InitializePrivateStaticClassUDisBehaviorGoTo_Base(void)
//   0x745550  public: virtual void __thiscall UDisBehaviorGoTo_Base::OnBehaviorStart(void)
//   0x745560  public: virtual void __thiscall UDisBehaviorGoTo_Base::OnBehaviorStop(unsigned int)
//   0x7455c0  protected: void __thiscall UDisBehaviorGoTo_Base::PrepareGoTo(void)
//   0x745650  public: virtual unsigned char const * __thiscall UDisBehaviorGoTo_Base::BuildFilterStimMask(void)const
//   0x745690  protected: unsigned int __thiscall UDisBehaviorGoTo_Base::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x7456c0  protected: unsigned int __thiscall UDisBehaviorGoTo_Base::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x7456e0  protected: void __thiscall UDisBehaviorGoTo_Base::SetFixedDestination(class FVector const &, class FRotator const &, unsigned int, unsigned int)
//   0x745760  private: virtual struct FDisLocoRequest * __thiscall UDisBehaviorGoTo_Base::GetDesiresLocoRequest(void)
//   0x745770  public: virtual unsigned int __thiscall UDisBehaviorGoTo_Base::IsPlayerAllowedToPushMe(void)const
//   0x746ce0  public: virtual void __thiscall UDisBehaviorGoTo_Base::OnBehaviorPause(unsigned int)
//   0x746d10  public: virtual void __thiscall UDisBehaviorGoTo_Base::OnBehaviorResume(void)
//   0x746d40  protected: void __thiscall UDisBehaviorGoTo_Base::Start_TurningToFace(void)
//   0x746db0  protected: unsigned int __thiscall UDisBehaviorGoTo_Base::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x74c8c0  public: virtual void __thiscall UDisBehaviorGoTo_Base::TickBehavior(float)
//   0x74c900  public: virtual unsigned char const * __thiscall UDisBehaviorGoTo_Base::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74c990  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo_Base::GetFilterStimDelegate(enum EAIStimID)
//   0x756f10  public: static class UClass * __cdecl UDisBehaviorGoTo_Base::GetPrivateStaticClassUDisBehaviorGoTo_Base(wchar_t const *)
//   0x759d00  public: static class UClass * __cdecl UDisBehaviorGoTo_Base::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent FV (PHASE18 FV): the behaviour that walks an NPC to one place.

	The loop, in retail's order:
	  * the subclass (UDisBehaviorGoTo for Kismet) fills the destination from its stim and calls PrepareGoTo, which
	    states one locomotion desire - follow the actor, or walk to the fixed point - at the behaviour's priority;
	  * the locomotion component reports arrival as EAIStimID_DestinationReached, which FilterDestinationReached turns
	    into m_bHasReachedDestination when the arrival is exact;
	  * TickBehavior sees it on stage 0 and calls Start_TurningToFace: with m_bDoRotationAlign a face-to desire toward
	    the destination's yaw and stage 1, waiting for EAIStimID_RotationReached (stage 2); without it stage 2 at once;
	  * the next tick on stage 2 raises m_bIsFinished and m_bSuccess (stage 3), the brain sees IsBehaviorFinished and
	    pops the behaviour, and OnBehaviorStop tells the Kismet action Success - or Interrupted when the brain popped it
	    for another reason first. A pathing failure tries retail's teleport spell and otherwise reports Failed.

	What is NOT here, each with what it costs:
	  * AttemptTeleportSpell (2012 0x832a50): an NPC that cannot path to its mark reports Failed to Kismet at once
	    instead of blinking there first.
	  * FArkComponentFaceTo. The component has no body in this tree (Engine/Src/arkcomponentfaceto.cpp is a skeleton),
	    so a face-to desire never produces EAIStimID_RotationReached; Start_TurningToFace therefore does not wait on it
	    when the pawn has no face-to component and goes to stage 2 directly, named once. With the component in, the
	    retail wait is the path taken.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "fvdiag.h"
#include "arkcomponentlocomotion.h"

// DISHONORED(port): 2013 rva 0x6e4d50 (2012 0x745550): a fresh go-to is neither finished nor a success.
void UDisBehaviorGoTo_Base::OnBehaviorStart()
{
	m_bIsFinished = FALSE;
	m_bSuccess = FALSE;
}

// DISHONORED(port): 2013 rva 0x6e4d60 (2012 0x745560): the Kismet action learns how the go-to ended, once.
void UDisBehaviorGoTo_Base::OnBehaviorStop( UBOOL _bIsBeingTerminated )
{
	if( m_pAssociatedKismetGoToAction )
	{
		UDisSeqAct_AIGoToActor* Action = m_pAssociatedKismetGoToAction;
		ADishonoredNPCController* Controller = m_pOwningBrain ? m_pOwningBrain->m_pOwningController : NULL;
		FVDIAG( "goto stop %s action %s %s", m_pOwningBrain && m_pOwningBrain->m_pOwningPawn ? *m_pOwningBrain->m_pOwningPawn->GetName() : TEXT("?"), *Action->GetName(), m_bSuccess ? TEXT("Success") : TEXT("Interrupted") );
		if( Controller )
		{
			Controller->SetGoToActionStatus( m_bSuccess ? AIGoToActorOutput_Success : AIGoToActorOutput_Interrupted, Action );
		}
		m_pAssociatedKismetGoToAction = NULL;
	}
}

// DISHONORED(port): 2013 rva 0x6e4de0 (2012 0x7455c0, 136 B): stage 0, no face-to, and the one locomotion desire. The
// end threshold is -1 (the component's default) and the funnel multiplier 1; following an actor is always an accurate
// stop, a fixed point only when the destination asked for one.
void UDisBehaviorGoTo_Base::PrepareGoTo()
{
	m_bIsFinished = FALSE;
	m_bHasReachedDestination = FALSE;
	m_GoToStage = 0;
	ClearFaceToDesire();
	if( m_pDestinationActor )
	{
		SetLocoActorDesire( m_pDestinationActor, m_TransitSpeed, -1.f, 1.f, TRUE, FALSE );
	}
	else
	{
		SetLocoLocationDesire( m_DestinationLocation, m_TransitSpeed, -1.f, 1.f, m_bAccurateStop, FALSE );
	}
	FVDIAG( "goto prepare %s -> %s at %.0f/%.0f/%.0f speed %d accurate %d align %d", m_pOwningBrain && m_pOwningBrain->m_pOwningPawn ? *m_pOwningBrain->m_pOwningPawn->GetName() : TEXT("?"),
		m_pDestinationActor ? *m_pDestinationActor->GetName() : TEXT("(fixed)"),
		m_pDestinationActor ? m_pDestinationActor->Location.X : m_DestinationLocation.X, m_pDestinationActor ? m_pDestinationActor->Location.Y : m_DestinationLocation.Y,
		m_pDestinationActor ? m_pDestinationActor->Location.Z : m_DestinationLocation.Z, (INT)m_TransitSpeed, (INT)m_bAccurateStop, (INT)m_bDoRotationAlign );
}

// DISHONORED(port): 2013 rva 0x6e4ef0 (2012 0x7456e0): a fixed point replaces any actor.
void UDisBehaviorGoTo_Base::SetFixedDestination( const FVector& _rDestinationLocation, const FRotator& _rDestinationRotation, UBOOL _bAccurateStop, UBOOL _bExactRotation )
{
	m_pDestinationActor = NULL;
	m_DestinationLocation = _rDestinationLocation;
	m_DestinationRotation = _rDestinationRotation;
	m_bAccurateStop = _bAccurateStop;
	m_bExactRotation = _bExactRotation;
}

// DISHONORED(port): 2013 rva 0x6e8590 (2012 0x746d40, 99 B): the walk is over; with rotation alignment asked for, face the
// destination's yaw (the actor's if following one) at the component's default speed, exactly when the destination said so.
void UDisBehaviorGoTo_Base::Start_TurningToFace()
{
	ClearLocoDesire();
	if( m_bDoRotationAlign )
	{
		const INT Yaw = m_pDestinationActor ? m_pDestinationActor->Rotation.Yaw : m_DestinationRotation.Yaw;
		SetFaceToYawDesire( Yaw, -100.f, m_bExactRotation );
		m_GoToStage = 1;
		// DISHONORED(bringup): see the unit's head - no FArkComponentFaceTo, no EAIStimID_RotationReached, so the wait
		// would never end. The pawn keeps the yaw it arrived with and the go-to completes.
		ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
		if( !Pawn || !Pawn->m_pCpntFaceTo )
		{
			static UBOOL bNoted = FALSE;
			if( !bNoted )
			{
				bNoted = TRUE;
				debugf( TEXT("DISHONORED(bringup): UDisBehaviorGoTo_Base::Start_TurningToFace: %s has no FArkComponentFaceTo (Engine/Src/arkcomponentfaceto.cpp is not ported), so the rotation alignment the go-to asked for is skipped and the go-to completes on arrival"), Pawn ? *Pawn->GetName() : TEXT("?") );
			}
			m_GoToStage = 2;
		}
	}
	else
	{
		m_GoToStage = 2;
	}
}

// DISHONORED(port): 2013 rva 0x6edc00 (2012 0x74c8c0, 54 B): the three-stage completion described at the head.
void UDisBehaviorGoTo_Base::TickBehavior( FLOAT _fDeltaSeconds )
{
	if( DisFVDiagEnabled() && m_pOwningBrain && m_pOwningBrain->m_pOwningPawn )
	{
		static FLOAT s_fNextReport = 0.f;
		const FLOAT Now = GWorld && GWorld->GetWorldInfo() ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;
		if( Now >= s_fNextReport )
		{
			s_fNextReport = Now + 1.f;
			ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
			FArkComponentLocomotion* Loco = Pawn->GetComponentLocomotion();
			const FVector Dest = m_pDestinationActor ? m_pDestinationActor->Location : m_DestinationLocation;
			FVDIAG( "goto tick %s stage %i reached %i at %.0f/%.0f/%.0f dist %.0f base %s vel %.0f | loco req %i path %i/%i%s%s pathend %.0f speed %.0f/%.0f stuck %i",
				*Pawn->GetName(), (INT)m_GoToStage, (INT)m_bHasReachedDestination, Pawn->Location.X, Pawn->Location.Y, Pawn->Location.Z,
				( Pawn->Location - Dest ).Size2D(), Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"), Pawn->Velocity.Size2D(),
				Loco ? Loco->GetRequestsCount() : -1, Loco ? Loco->GetCurPathPointIdx() : -1, Loco ? Loco->GetPathPoints().Num() : -1,
				Loco && Loco->IsArrived() ? TEXT(" arrived") : TEXT(""), Loco && !Loco->HasComputedPath() ? TEXT(" nopath") : TEXT(""),
				Loco ? appSqrt( Loco->GetSq2DDistToPathEnd() ) : 0.f, Loco ? Loco->GetCurMoveSpeed() : 0.f, Loco ? Loco->GetTargetMoveSpeed() : 0.f,
				Loco ? (INT)Loco->IsStuckInAWall() : 0 );
		}
	}
	if( m_GoToStage == 0 )
	{
		if( m_bHasReachedDestination )
		{
			Start_TurningToFace();
		}
	}
	else if( m_GoToStage == 2 )
	{
		m_bIsFinished = TRUE;
		m_bSuccess = TRUE;
		m_GoToStage = 3;
		FVDIAG( "goto finished %s", m_pOwningBrain && m_pOwningBrain->m_pOwningPawn ? *m_pOwningBrain->m_pOwningPawn->GetName() : TEXT("?") );
	}
}

// DISHONORED(port): 2013 rva 0x6e4ea0 (2012 0x745690): only this behaviour's own request counts, and only an exact arrival.
UBOOL UDisBehaviorGoTo_Base::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = _rStim.m_pRequestOriginator == this;
	FVDIAG( "goto destination-reached stim for %s: mine %i exact %i at %.0f/%.0f/%.0f", m_pOwningBrain && m_pOwningBrain->m_pOwningPawn ? *m_pOwningBrain->m_pOwningPawn->GetName() : TEXT("?"), (INT)bMine, (INT)_rStim.m_bExact, _rStim.m_Location.X, _rStim.m_Location.Y, _rStim.m_Location.Z );
	if( bMine && _rStim.m_bExact )
	{
		m_bHasReachedDestination = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2013 rva 0x6e4ed0 (2012 0x7456c0)
UBOOL UDisBehaviorGoTo_Base::FilterRotationReached( const FAIStimStruct_RotationReached& _rStim )
{
	const UBOOL bMine = _rStim.m_pRequestOriginator == this;
	if( bMine )
	{
		m_GoToStage = 2;
	}
	return bMine;
}

// DISHONORED(port): 2013 rva 0x6e8600 (2012 0x746db0, 134 B): the mark is unreachable. Retail tries AttemptTeleportSpell(
// pawn, pawn, EDisTeleportIntention_Exact, destination ) and reports Failed to Kismet only when the blink is refused too;
// the spell (2012 0x832a50) is not ported, so Failed is reported at once. Never consumed (returns FALSE), so the
// sub-states see the same stim.
UBOOL UDisBehaviorGoTo_Base::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	if( _rStim.m_pRequestOriginator == this )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisBehaviorGoTo_Base::FilterPathingFail: AttemptTeleportSpell (2012 rva 0x832a50) is not ported; the go-to reports Failed without the blink") );
		}
		FVDIAG( "goto pathing failed %s", m_pOwningBrain && m_pOwningBrain->m_pOwningPawn ? *m_pOwningBrain->m_pOwningPawn->GetName() : TEXT("?") );
		if( m_pAssociatedKismetGoToAction && m_pOwningBrain && m_pOwningBrain->m_pOwningController )
		{
			m_pOwningBrain->m_pOwningController->SetGoToActionStatus( AIGoToActorOutput_Failed, m_pAssociatedKismetGoToAction );
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x6e4f70 (2012 0x745770): a walking NPC can be pushed by the player.
UBOOL UDisBehaviorGoTo_Base::IsPlayerAllowedToPushMe() const
{
	return TRUE;
}

// DISHONORED(port): 2012 rvas 0x746ce0 / 0x746d10: paused by another behaviour the pawn stops reading the danger steering
// (UDisSteeringInfluence_Danger::EnableDanger( TRUE, 2 ), 2012 0x765080: group flag 2 leaves m_DisableGroupFlags, and the
// influence is enabled when no flag is left); resumed it reads it again and re-states its desire from the start.
void UDisBehaviorGoTo_Base::OnBehaviorPause( UBOOL _bIsBeingTerminated )
{
	if( !_bIsBeingTerminated && m_pOwningBrain && m_pOwningBrain->m_pSteeringInfluence_Danger )
	{
		UDisSteeringInfluence_Danger* Danger = m_pOwningBrain->m_pSteeringInfluence_Danger;
		Danger->m_DisableGroupFlags &= ~2;
		Danger->m_bEnabled = Danger->m_DisableGroupFlags == 0;
	}
}

void UDisBehaviorGoTo_Base::OnBehaviorResume()
{
	if( m_pOwningBrain && m_pOwningBrain->m_pSteeringInfluence_Danger )
	{
		UDisSteeringInfluence_Danger* Danger = m_pOwningBrain->m_pSteeringInfluence_Danger;
		Danger->m_DisableGroupFlags |= 2;
		Danger->m_bEnabled = Danger->m_DisableGroupFlags == 0;
	}
	PrepareGoTo();
}

// DISHONORED(port): 2012 rva 0x745650: the three stims the base listens for. Retail's BuildBehaviorFilterStimMasks (0x74c900)
// copies this mask and rolls the sub-processes' and sub-states' masks into it; this tree's UDishonoredAIBehavior::
// BuildBehaviorFilterStimMasks does the same from BuildFilterStimMask.
const BYTE* UDisBehaviorGoTo_Base::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_RotationReached, EAIStimID_PathingFail };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x74c990
FDisStimPredicateDelegate UDisBehaviorGoTo_Base::GetFilterStimDelegate( BYTE _StimID )
{
	switch( _StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo_Base, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_RotationReached:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo_Base, FAIStimStruct_RotationReached, FilterRotationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo_Base, FAIStimStruct_PathingFail, FilterPathingFail );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(bringup, agent FV): every function above is a virtual or is called from one, so nothing references this
// object and MSVC would drop it out of the static library. Named from DishonoredGameRegistrants.cpp.
void DishonoredLinkDisBehaviorGoToBase()
{
}
