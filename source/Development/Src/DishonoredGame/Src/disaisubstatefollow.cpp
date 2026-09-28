// DishonoredGame/src/disaisubstatefollow.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateFollow ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	UDisAISubStateFollow

	DISHONORED(retail): the only sub-state with no parameter of its own. A behaviour puts it in its machine and then calls
	Configure directly, which is why m_bHaveNewParameters exists: the configure and the next refresh are separate events.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x787d70. Passing no parameters means "use the slot's tweaks", which is how a follow order
// from Kismet and one from a behaviour end up with the same distance and angle.
void UDisAISubStateFollow::Configure( ADishonoredPawn* const _pFollowedPawn, const FFollowParameters* const _pOverwriteFollowParameters )
{
	m_pFollowedPawn = _pFollowedPawn;

	const FFollowParameters* Parameters = _pOverwriteFollowParameters;
	if( !Parameters )
	{
		const UDisTweaks_AISubState_Follow* Tweaks = Cast<UDisTweaks_AISubState_Follow>( GetTweaks_Derived() );
		if( !Tweaks )
		{
			Tweaks = (const UDisTweaks_AISubState_Follow*)UDisTweaks_AISubState_Follow::StaticClass()->GetDefaultObject();
		}
		Parameters = &Tweaks->m_FollowParameters;
	}
	m_FollowParameters = *Parameters;
	m_bHaveNewParameters = TRUE;
}

// DISHONORED(port): 2012 rva 0x768c80. Two jobs: notice that what we were following has died, and - only once, when the
// parameters are new - state the follow desire. A follow is the one loco desire that is not re-stated every thought,
// because the loco layer's follow form keeps tracking the actor by itself.
void UDisAISubStateFollow::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	if( m_pFollowedPawn && DisIsPawnDead( m_pFollowedPawn ) )
	{
		m_pFollowedPawn = NULL;
		delegateNoMoreTargetCallback();
	}

	if( m_bHaveNewParameters && m_pFollowedPawn )
	{
		SetLocoFollowDesire( m_pFollowedPawn, m_FollowParameters.m_eDesiredSpeed,
			m_FollowParameters.m_fAngle, m_FollowParameters.m_fDistance );
		m_bHaveNewParameters = FALSE;
	}
}

// DISHONORED(port): 2012 rva 0x773970. The seventh delegate: the six every sub-state has are registered by
// UDishonoredAIBehavior::RegisterCallbacks, and this one is registered by the behaviour that uses it - with the same name
// composition, "NoMoreTargetCallback" + m_StateSuffix.
void UDisAISubStateFollow::RegisterDelegate_NoMoreTarget( UDishonoredAIBehavior* const _pOwningBehavior )
{
	const FString FunctionName = FString( TEXT("NoMoreTargetCallback") ) + m_StateSuffix.GetNameString();
	const FName TargetFunction( *FunctionName, FNAME_Add, TRUE );
	if( _pOwningBehavior && _pOwningBehavior->FindFunction( TargetFunction ) )
	{
		__NoMoreTargetCallback__Delegate.Object = _pOwningBehavior;
		__NoMoreTargetCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__NoMoreTargetCallback__Delegate.Object = NULL;
		__NoMoreTargetCallback__Delegate.FunctionName = NAME_None;
	}
}
