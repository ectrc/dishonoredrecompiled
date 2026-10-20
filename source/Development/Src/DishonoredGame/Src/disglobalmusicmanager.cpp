// DishonoredGame/src/disglobalmusicmanager.cpp
// PDB functions attributed to this file (14):
//   0x8af1e0  public: static void __cdecl UDisGlobalMusicManager::InitializePrivateStaticClassUDisGlobalMusicManager(void)
//   0x8b16b0  public: void __thiscall UDisGlobalMusicManager::Term_GlobalMusic(void)
//   0x8b16f0  private: virtual void __thiscall UDisGlobalMusicManager::SetTweaks_Derived(class UDisTweaksBase *)
//   0x8b8950  unsigned int __cdecl Music::IsEnemyNearPlayer(float)
//   0x8bc6a0  public: static class UClass * __cdecl UDisGlobalMusicManager::GetPrivateStaticClassUDisGlobalMusicManager(wchar_t const *)
//   0x8c0f00  public: static class UClass * __cdecl UDisGlobalMusicManager::StaticClassNoInline(void)
//   0x8c8310  public: void __thiscall UDisGlobalMusicManager::Init_GlobalMusic(void)
//   0x8c8390  private: void __thiscall UDisGlobalMusicManager::ChooseNewState(class UDishonoredNativeState * const)
//   0x8c8540  public: virtual void __thiscall UDisGlobalMusicManager::TickCallback_Suspense(class UDishonoredNativeState *, float)
//   0x8c8550  public: virtual void __thiscall UDisGlobalMusicManager::TickCallback_Combat(class UDishonoredNativeState *, float)
//   0x8c8570  public: virtual void __thiscall UDisGlobalMusicManager::TickCallback_Chase(class UDishonoredNativeState *, float)
//   0x8d1e50  void __cdecl Music::CollectCombatIntensities(struct TMemStackArray<enum EDisCombatMusicIntensityObjectType> &)
//   0x8d2180  struct Music::CalcCombatIntensityResult __cdecl Music::CalcCombatIntensity(class UDisTweaks_MusicEvents const * const)
//   0x8d22c0  public: void __thiscall UDisGlobalMusicManager::Tick_GlobalMusic(float)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile; the 2013 twins (match_2012_2013.csv) are on each body.
// Deviations (named):
//   * Music::CollectCombatIntensities ports the attentive-brain half only. Its second half walks the crowd population manager's
//     skeletal rats (TDisCrowdAgentIterator<ADisGameCrowdAgentSkeletalRat>, 2012 0x8cd280) and adds every rat attacking the
//     player; that iterator and the crowd agents are unported here, so no rat contributes and m_bAttackedByRats stays clear.
//     The 2013 body (0x85b190) differs from 2012's (ratio 0.70); 2012's is ported.
//   * -akdiag / -ih4diag print the map's music events at init and every state's event when it is posted (DisMusicPostStateEvent).

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "disattentioninfo.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisGlobalMusicManager()
{
}

UBOOL DisIH4DiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( ParseParam( appCmdLine(), TEXT("akdiag") ) || ParseParam( appCmdLine(), TEXT("ih4diag") ) ) ? 1 : 0;
	}
	return Enabled;
}

void DisMusicPostStateEvent( const TCHAR* _StateName, UAkEvent* _pEvent )
{
	IH4DIAG( "music state %s posts %s on the player pawn", _StateName, _pEvent ? *_pEvent->GetName() : TEXT("<none>") );
	if( _pEvent )
	{
		ADishonoredPlayerPawn::s_pInstance->PostAkEvent( _pEvent );
	}
}

namespace Music
{
	struct CalcCombatIntensityResult
	{
		FLOAT m_fTotalIntensity;
		UBOOL m_bIsFightingRats;
	};

	/** 2012 0x8b8950 / 2013 0x8483c0: a living NPC within range whom the relationship test calls the player's enemy. */
	UBOOL IsEnemyNearPlayer( FLOAT _fRange )
	{
		ADishonoredPlayerPawn* pPlayerPawn = ADishonoredPlayerPawn::s_pInstance;
		UBOOL bResult = FALSE;
		const FLOAT fRangeSq = _fRange * _fRange;
		for( APawn* pPawn = GWorld->GetFirstPawn(); pPawn && !bResult; pPawn = pPawn->NextPawn )
		{
			ADishonoredNPCPawn* pNPC = Cast<ADishonoredNPCPawn>( pPawn );
			if( !pNPC || pNPC->IsDead() )
			{
				continue;
			}
			if( fRangeSq >= ( pPlayerPawn->Location - pNPC->Location ).SizeSquared() )
			{
				bResult = pNPC->DetermineRelationship( *pPlayerPawn ) == Relationship_Enemy;
			}
		}
		return bResult;
	}

	/** 2012 0x8d1e50, the attentive-brain half (see the banner): every enemy brain busted on the player adds the combat
	    intensity type of its pawn's tweaks. */
	void CollectCombatIntensities( TArray< BYTE, TMemStackAllocator< GMainThreadMemStack > >& _rOut_IntensityContributors )
	{
		DIS_MEMSTACK_ARRAY( UDishonoredAIBrain* ) Brains;
		ADishonoredPlayerPawn::s_pInstance->GetAttentiveBrains( Brains, Relationship_Enemy, DAL_Busted );
		for( INT BrainIndex = 0; BrainIndex < Brains.Num(); BrainIndex++ )
		{
			ADishonoredPawn* pPawn = (ADishonoredPawn*)Brains( BrainIndex )->m_pOwningPawn;
			UDisTweaks_NPCPawn* pTweaks = (UDisTweaks_NPCPawn*)pPawn->GetTweaks_Derived();
			if( !pTweaks )
			{
				pTweaks = UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_NPCPawn>();
			}
			_rOut_IntensityContributors.AddItem( pTweaks->m_CombatIntensityType );
		}
	}

	/** 2012 0x8d2180 / 2013 0x85b4d0: the tweaks' contribution of every contributor; a rat marks the fight as a rat attack. */
	CalcCombatIntensityResult CalcCombatIntensity( const UDisTweaks_MusicEvents* const _pMusicTweaks )
	{
		FMemMark Mark( GMainThreadMemStack );
		CalcCombatIntensityResult Result;
		{
			TArray< BYTE, TMemStackAllocator< GMainThreadMemStack > > IntensityTypes;
			CollectCombatIntensities( IntensityTypes );
			Result.m_fTotalIntensity = 0.f;
			Result.m_bIsFightingRats = FALSE;
			for( INT TypeIndex = 0; TypeIndex < IntensityTypes.Num(); TypeIndex++ )
			{
				const BYTE Type = IntensityTypes( TypeIndex );
				Result.m_fTotalIntensity += _pMusicTweaks->m_fCombatIntensityContribution[Type];
				if( Type == DCMIOT_Rat )
				{
					Result.m_bIsFightingRats = TRUE;
				}
			}
		}
		Mark.Pop();
		return Result;
	}
}

/** The GetTweaks_Derived-or-class-default read every retail caller inlines. */
UDisTweaks_MusicEvents* UDisGlobalMusicManager::GetMusicEventsTweaks()
{
	UDisTweaks_MusicEvents* pTweaks = (UDisTweaks_MusicEvents*)GetTweaks_Derived();
	return pTweaks ? pTweaks : UDisTweaks_MusicEvents::StaticClass()->GetDefaultObject<UDisTweaks_MusicEvents>();
}

/** The IDisTweaksInterface slot retail implements as a member read (inlined everywhere). */
UDisTweaksBase* UDisGlobalMusicManager::GetTweaks_Derived()
{
	return m_pMusicEventsTweaks;
}

/** 2012 0x8b16f0 / 2013 0x8418c0. */
void UDisGlobalMusicManager::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pMusicEventsTweaks = _pTweaks ? (UDisTweaks_MusicEvents*)_pTweaks : UDisTweaks_MusicEvents::StaticClass()->GetDefaultObject<UDisTweaks_MusicEvents>();
}

/** 2012 0x8c8310 / 2013 0x8529c0: the map's music tweaks, then the state machine starting in Exploration. */
void UDisGlobalMusicManager::Init_GlobalMusic()
{
	UDisTweaks_MusicEvents* pMapMusicEvents = DishonoredGetMapInfo()->m_pMapMusicEvents;
	if( GetTweaks_Derived() != pMapMusicEvents )
	{
		SetTweaks_Derived( pMapMusicEvents );
		ApplyTweakChanges();
	}
	IH4DIAG( "music manager init: map music events %s", pMapMusicEvents ? *pMapMusicEvents->GetPathName() : TEXT("<none>") );
	FDisMusicState_Exploration_Param Param;
	m_pMusicFSM->InitFSM( this, Param, NULL );
}

/** 2012 0x8b16b0 / 2013 0x841880. */
void UDisGlobalMusicManager::Term_GlobalMusic()
{
	m_pMusicFSM->DestroyFSM();
	if( GetTweaks_Derived() )
	{
		SetTweaks_Derived( NULL );
		ApplyTweakChanges();
	}
}

/** 2012 0x8c8390 / 2013 0x852a40: combat while there is combat intensity; else a rat attack while fighting rats; else, with an
    enemy within the suspense range, a chase straight after combat or a rat attack and suspense otherwise; else exploration.
    A change is requested only when the current state is not already of the chosen class. */
void UDisGlobalMusicManager::ChooseNewState( UDishonoredNativeState* const _pCurrentState )
{
	UDisTweaks_MusicEvents* pTweaks = GetMusicEventsTweaks();
	FDisMusicState_Combat_Param CombatParam( m_fCombatIntensity );
	FDisMusicState_RatAttack_Param RatAttackParam;
	FDisMusicState_Suspense_Param SuspenseParam;
	FDisMusicState_Exploration_Param ExplorationParam;
	FDisMusicState_Chase_Param ChaseParam( pTweaks->m_fChaseTime );
	FDisNativeStateParam* pParam = &ExplorationParam;
	if( m_fCombatIntensity > 0.f )
	{
		pParam = &CombatParam;
	}
	else if( m_bAttackedByRats )
	{
		pParam = &RatAttackParam;
	}
	else if( Music::IsEnemyNearPlayer( pTweaks->m_fEnemyRangeForSuspense ) )
	{
		if( _pCurrentState->IsA( UDisMusicState_Combat::StaticClass() ) || _pCurrentState->IsA( UDisMusicState_RatAttack::StaticClass() ) )
		{
			pParam = &ChaseParam;
		}
		else
		{
			pParam = &SuspenseParam;
		}
	}
	if( pParam->m_pStateClass && !_pCurrentState->IsA( pParam->m_pStateClass ) )
	{
		m_pMusicFSM->RequestStateChange( *pParam, NULL, FALSE );
	}
}

/** 2012 0x8c8540 / 2013 0x852bf0: the Exploration, RatAttack and Suspense slots share this body (retail vtable 2012 0xd765e0,
    0xd765e4, 0xd765e8 hold the one folded function). */
void UDisGlobalMusicManager::TickCallback_Exploration( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	ChooseNewState( _pThisState );
}

void UDisGlobalMusicManager::TickCallback_RatAttack( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	ChooseNewState( _pThisState );
}

void UDisGlobalMusicManager::TickCallback_Suspense( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	ChooseNewState( _pThisState );
}

/** 2012 0x8c8550 / 2013 0x852c00: the combat state follows the manager's intensity. */
void UDisGlobalMusicManager::TickCallback_Combat( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	ChooseNewState( _pThisState );
	( (UDisMusicState_Combat*)_pThisState )->m_fCombatIntensityValue = m_fCombatIntensity;
}

/** 2012 0x8c8570 / 2013 0x852c20: the chase lasts until its timer runs out, combat resumes or rats attack. */
void UDisGlobalMusicManager::TickCallback_Chase( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	if( ( (UDisMusicState_Chase*)_pThisState )->m_fChaseTimer <= 0.f || m_fCombatIntensity > 0.f || m_bAttackedByRats )
	{
		ChooseNewState( _pThisState );
	}
}

/** 2012 0x8d22c0 / 2013 0x85b610. */
void UDisGlobalMusicManager::Tick_GlobalMusic( FLOAT _fDeltaSeconds )
{
	const Music::CalcCombatIntensityResult Result = Music::CalcCombatIntensity( GetMusicEventsTweaks() );
	m_bAttackedByRats = Result.m_bIsFightingRats ? 1 : 0;
	m_fCombatIntensity = Result.m_fTotalIntensity;
	m_pMusicFSM->TickStateMachine( _fDeltaSeconds );
}

// The natives the music states' tick delegates reach ("TickCallback" + the state's m_StateSuffix; 2013 execs 0x5e4870
// Exploration, 0x5e4900 Suspense, 0x5e4990 Combat, 0x1d0680 RatAttack): each forwards to its virtual.
void UDisGlobalMusicManager::execTickCallback_Exploration( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDishonoredNativeState, _pThisState );
	P_GET_FLOAT( _fDeltaSeconds );
	P_FINISH;
	TickCallback_Exploration( _pThisState, _fDeltaSeconds );
}

void UDisGlobalMusicManager::execTickCallback_RatAttack( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDishonoredNativeState, _pThisState );
	P_GET_FLOAT( _fDeltaSeconds );
	P_FINISH;
	TickCallback_RatAttack( _pThisState, _fDeltaSeconds );
}

void UDisGlobalMusicManager::execTickCallback_Suspense( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDishonoredNativeState, _pThisState );
	P_GET_FLOAT( _fDeltaSeconds );
	P_FINISH;
	TickCallback_Suspense( _pThisState, _fDeltaSeconds );
}

void UDisGlobalMusicManager::execTickCallback_Combat( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDishonoredNativeState, _pThisState );
	P_GET_FLOAT( _fDeltaSeconds );
	P_FINISH;
	TickCallback_Combat( _pThisState, _fDeltaSeconds );
}

void UDisGlobalMusicManager::execTickCallback_Chase( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDishonoredNativeState, _pThisState );
	P_GET_FLOAT( _fDeltaSeconds );
	P_FINISH;
	TickCallback_Chase( _pThisState, _fDeltaSeconds );
}
