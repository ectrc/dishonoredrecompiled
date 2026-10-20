// DishonoredGame/src/dismusicstate_chase.cpp
// PDB functions attributed to this file (7):
//   0x8b8bf0  public: static class UClass * __cdecl UDisMusicState_Chase::GetPrivateStaticClassUDisMusicState_Chase(wchar_t const *)
//   0x8b8c80  public: virtual void __thiscall FDisMusicState_Chase_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x8b8ca0  private: void __thiscall UDisMusicState_Chase::ChooseChaseIntensity(void)
//   0x8bc760  public: static void __cdecl UDisMusicState_Chase::InitializePrivateStaticClassUDisMusicState_Chase(void)
//   0x8bc780  private: virtual void __thiscall UDisMusicState_Chase::OnEnterState(class UDishonoredNativeState *)
//   0x8bc7a0  private: virtual void __thiscall UDisMusicState_Chase::TickState(float)
//   0x8c0f30  public: static class UClass * __cdecl UDisMusicState_Chase::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile; 2013 twins on each body.

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateChase()
{
}

FDisMusicState_Chase_Param::FDisMusicState_Chase_Param( FLOAT ChaseTime )
:	FDisMusicState_Base_Param( UDisMusicState_Chase::StaticClass() )
,	m_fChaseTime( ChaseTime )
{
}

/** 2012 0x8b8c80 / 2013 0x848600 (one body with the combat parameter's): the chase timer starts at the tweaks' chase time. */
void FDisMusicState_Chase_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisMusicState_Base_Param::OnPending( PendingState, ManagedObject );
	( (UDisMusicState_Chase*)PendingState )->m_fChaseTimer = m_fChaseTime;
}

/** 2012 0x8b8ca0 / 2013 0x848620: the first chase range holding an enemy picks the intensity and a change posts its event; no
    enemy within either range ends the chase. */
void UDisMusicState_Chase::ChooseChaseIntensity()
{
	INT Intensity = 0;
	while( !Music::IsEnemyNearPlayer( m_pMusicEvents->m_fEnemyRangeForChase[Intensity] ) )
	{
		if( ++Intensity >= 2 )
		{
			m_fChaseTimer = 0.f;
			return;
		}
	}
	if( m_ChaseIntensity != Intensity )
	{
		m_ChaseIntensity = (BYTE)Intensity;
		DisMusicPostStateEvent( TEXT("Chase"), m_pMusicEvents->m_pStartChase[Intensity] );
	}
}

/** 2012 0x8bc780 / 2013 0x84d140. */
void UDisMusicState_Chase::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	m_ChaseIntensity = 2;
	ChooseChaseIntensity();
}

/** 2012 0x8bc7a0 / 2013 0x84d160. */
void UDisMusicState_Chase::TickState( FLOAT DeltaSeconds )
{
	UDishonoredNativeState::TickState( DeltaSeconds );
	delegateTickCallback( this, DeltaSeconds );
	m_fChaseTimer -= DeltaSeconds;
	ChooseChaseIntensity();
}
