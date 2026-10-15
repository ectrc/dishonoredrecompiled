// Engine/src/arkcomponentmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x574af0  private: virtual struct FDisLocoRequest * __thiscall UDisAISubStateStand::GetDesiresLocoRequest(void)
//   0x579cf0  public: __thiscall FArkComponentManager::~FArkComponentManager(void)
//   0x57afa0  public: __thiscall FArkComponentManager::FArkComponentManager(void)
//   0x57b070  public: void __thiscall FArkComponentManager::Tick(float, unsigned char)

#include "EnginePrivate.h"
#include "arkcomponentpolicy.h"

/*-----------------------------------------------------------------------------
	DISHONORED(port): agent HC (PHASE22 HC). Retail's per-world component ticker. UWorld's four tick stages call Tick with
	the world's TickGroup after their actors (2012 UWorld::TickPreAW 0x26ef30 also calls it with 5, between TickActors
	and FEdgeAnimManager::End, for the JustBeforeProceduralAnim policy); 2012 0x57b070 (144 B) / 2013 0x93a6b0 (117 B,
	same switch). Stages 3 and 4 (post-update, effects) tick nothing.
-----------------------------------------------------------------------------*/
void FArkComponentManager::Tick( FLOAT _fTimeStep, BYTE _TickType )
{
	switch( _TickType )
	{
	case TG_PreAsyncWork:
		m_PreAsyncWorkPolicy.Tick( _fTimeStep );
		break;
	case TG_DuringAsyncWork:
		m_DuringAsyncWorkPolicy.Tick( _fTimeStep );
		m_TimeSlicedTickPolicy.Tick( _fTimeStep );
		break;
	case TG_PostAsyncWork:
		m_PostAsyncWorkPolicy.Tick( _fTimeStep );
		break;
	case 5:
		m_PreAsyncWorkJustBeforeProceduralAnimPolicy.Tick( _fTimeStep );
		break;
	default:
		break;
	}
	static UBOOL bReported = FALSE;
	if( !bReported && _TickType == TG_PreAsyncWork && m_PreAsyncWorkPolicy.Num() > 0 && ParseParam( appCmdLine(), TEXT("gadiag") ) )
	{
		bReported = TRUE;
		debugf( TEXT("gadiag cpntmgr %i preasync %i justbefore %i during %i postasync %i timesliced"), m_PreAsyncWorkPolicy.Num(),
			m_PreAsyncWorkJustBeforeProceduralAnimPolicy.Num(), m_DuringAsyncWorkPolicy.Num(), m_PostAsyncWorkPolicy.Num(), m_TimeSlicedTickPolicy.Num() );
	}
}

// DISHONORED(port): 2012 0x578120 (314 B). One round gives every registered component a slice; a round may span frames.
// Each frame ticks components from the back of the round's list until one says it is not done, the frame's 0.9 ms budget
// is spent or every registered component has had a slice this frame.
void FArkComponentTimeSlicedTickPolicy::Tick( FLOAT _fDeltaTime )
{
	const DOUBLE UpdateStartTime = appSeconds();
	const INT MaxUpdatedCount = m_Components.Num();
	INT UpdatedCount = 0;
	for( ;; )
	{
		if( m_ComponentsToTick.Num() == 0 )
		{
			m_ComponentsToTick += m_Components;
			if( m_ComponentsToTick.Num() == 0 )
			{
				break;
			}
		}
		IArkComponentTimeSlicedTick* Component = m_ComponentsToTick.Last();
		const DOUBLE TimeLeft = 0.0009 - ( appSeconds() - UpdateStartTime );
		if( UpdatedCount >= MaxUpdatedCount || TimeLeft <= 0.0 || !Component->TimeSlicedTick( _fDeltaTime, TimeLeft ) )
		{
			break;
		}
		m_ComponentsToTick.RemoveItem( Component );
		++UpdatedCount;
	}
}
