// DishonoredGame/src/dismusicstate_combat.cpp
// PDB functions attributed to this file (6):
//   0x8b8d20  public: static class UClass * __cdecl UDisMusicState_Combat::GetPrivateStaticClassUDisMusicState_Combat(wchar_t const *)
//   0x8b8db0  private: void __thiscall UDisMusicState_Combat::ChooseCombatIntensity(void)
//   0x8bc810  public: static void __cdecl UDisMusicState_Combat::InitializePrivateStaticClassUDisMusicState_Combat(void)
//   0x8bc830  private: virtual void __thiscall UDisMusicState_Combat::OnEnterState(class UDishonoredNativeState *)
//   0x8bc850  private: virtual void __thiscall UDisMusicState_Combat::TickState(float)
//   0x8c0f60  public: static class UClass * __cdecl UDisMusicState_Combat::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile; 2013 twins on each body.

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateCombat()
{
}

FDisMusicState_Combat_Param::FDisMusicState_Combat_Param( FLOAT InitialCombatIntensityValue )
:	FDisMusicState_Base_Param( UDisMusicState_Combat::StaticClass() )
,	m_fInitialCombatIntensityValue( InitialCombatIntensityValue )
{
}

/** 2012 0x8b8c80 / 2013 0x848600 (the body retail shares with the chase parameter). */
void FDisMusicState_Combat_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisMusicState_Base_Param::OnPending( PendingState, ManagedObject );
	( (UDisMusicState_Combat*)PendingState )->m_fCombatIntensityValue = m_fInitialCombatIntensityValue;
}

/** 2012 0x8b8db0 / 2013 0x8486d0: the tweaks' thresholds pick the level; a change posts its event. */
void UDisMusicState_Combat::ChooseCombatIntensity()
{
	INT Level = m_fCombatIntensityValue >= m_pMusicEvents->m_fCombatIntensity[1] ? 1 : 0;
	if( m_fCombatIntensityValue >= m_pMusicEvents->m_fCombatIntensity[2] )
	{
		Level = 2;
	}
	if( m_fCombatIntensityValue >= m_pMusicEvents->m_fCombatIntensity[3] )
	{
		Level = 3;
	}
	if( m_CombatIntensityLevel != Level )
	{
		m_CombatIntensityLevel = (BYTE)Level;
		DisMusicPostStateEvent( TEXT("Combat"), m_pMusicEvents->m_pStartCombat[Level] );
	}
}

/** 2012 0x8bc830 / 2013 0x84d1d0. */
void UDisMusicState_Combat::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	m_CombatIntensityLevel = 4;
	ChooseCombatIntensity();
}

/** 2012 0x8bc850 / 2013 0x84d1f0: the level is re-chosen only while this is still the current state. */
void UDisMusicState_Combat::TickState( FLOAT DeltaSeconds )
{
	UDishonoredNativeState::TickState( DeltaSeconds );
	delegateTickCallback( this, DeltaSeconds );
	if( m_pStateMachine->IsCurState( GetClass(), TRUE ) )
	{
		ChooseCombatIntensity();
	}
}
