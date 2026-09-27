#pragma once
// DishonoredGame/inc/dishonoredutilities_ai.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (1):
//   0x7483e0  private: virtual class FArchive & __thiscall FDisArchiveCheckForBadActors::operator<<(class UObject * &)

// ---- agent CG ports (PHASE9 CG) ----

/**
 * DISHONORED(port): 2013 rva 0x6ea520 (2012 0x7483e0). An object-reference collector that answers one question: does
 * the thing it was run over name an actor that has been terminated or is pending kill? UDishonoredAIBrain::FlushStimQueue
 * runs a stim through it before processing it, which is how a stim queued two seconds ago can never hand the AI a
 * pointer to an actor that died in between. Retail declares it beside this one operator<<; the two result members and
 * the ArIsObjectReferenceCollector flag come from FlushStimQueue's own setup of it (2013 rva 0x716eb0).
 */
class FDisArchiveCheckForBadActors : public FArchive
{
public:
	AActor*		m_pFoundTerminatedActor;
	AActor*		m_pFoundPendingKillActor;

	FDisArchiveCheckForBadActors()
		: m_pFoundTerminatedActor( NULL )
		, m_pFoundPendingKillActor( NULL )
	{
		ArIsObjectReferenceCollector = TRUE;
	}

	void Reset()
	{
		m_pFoundTerminatedActor = NULL;
		m_pFoundPendingKillActor = NULL;
	}

	UBOOL FoundBadActor() const
	{
		return m_pFoundTerminatedActor != NULL || m_pFoundPendingKillActor != NULL;
	}

	// DISHONORED(port): 2013 rva 0x6ea520: an actor that is pending kill or that has already been marked for deletion
	// is recorded; anything else passes through untouched.
	virtual FArchive& operator<<( UObject*& Object )
	{
		AActor* Actor = Cast<AActor>( Object );
		if( Actor )
		{
			if( Actor->IsPendingKill() )
			{
				m_pFoundPendingKillActor = Actor;
			}
			else if( Actor->bDeleteMe )
			{
				m_pFoundTerminatedActor = Actor;
			}
		}
		return *this;
	}
};

// ---- agent CG (PHASE9 CG): the AI globals ----
// Bodies in Src/dishonoredutilities_accessors.cpp, beside agent AU's twelve.
class UDishonoredGlobalAIManager* DisGetGlobalAIManagerUnchecked();
/** DISHONORED(bringup): names, once per call site, a place where retail drives the desires system (IDisDesiresInterface)
    that this tree does not have. Defined in Src/disaisubprocess.cpp. */
void DisAINoteDesiresGap( const TCHAR* Site );
FLOAT DisGetAppropriateWorldTime( const class AActor* const _pActor );

// ---- agent CG: DisIsPawnDead ----
/** DISHONORED(port): the test retail spells ADishonoredPawn::IsDead (2013 rva 0x74a260, 2012 0x7561d0). That method is
    not declared in the tree (agent AJ's ADishonoredPawn cpptext does not carry it and no other unit needs it yet), so
    the AI package uses this one-line equivalent and names the retail function. Retail's body is the same three tests:
    no pawn, zero health, or already marked for deletion. */
inline UBOOL DisIsPawnDead( const class APawn* _pPawn )
{
	return _pPawn == NULL || _pPawn->Health <= 0 || _pPawn->bDeleteMe;
}
