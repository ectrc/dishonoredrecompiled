// DishonoredGame/src/disaibrainprocess.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (14): the ten below plus the four class-registration helpers.

// ---- agent CG ports (PHASE9 CG): UDisAIBrainProcess, the brain's always-on processes ----
//
// A brain process is a piece of the brain that runs whatever behaviour is active: the attention process, the panic
// process, the battle sense, the hideout watcher, the rat crusher, the wolfhound counter. Unlike a sub-process it
// belongs to the brain rather than to a behaviour, it is built once by UDishonoredAIBrain::InitBrain_Processes from the
// brain tweaks' m_BrainProcessTweaks, and it gets first refusal on every stim before any behaviour sees it.

#include "DishonoredGame.h"
#include "aistimstruct.h"
#include "dishonoredutilities_saveload.h"
#include "dishonoredutilities_saveload_ai.h"

// DISHONORED(port): 2013 rva 0x789580-shaped accessor pair (2012: the 7-byte getter and the 16-byte setter). The
// process's own tweaks pointer is m_pBrainProcessTweaks; every subclass inherits it, so unlike the pickup tweaks that
// cost agent AU 147 pickups there is no per-subclass storage to miss here.
UDisTweaksBase* UDisAIBrainProcess::GetTweaks_Derived()
{
	return m_pBrainProcessTweaks;
}

void UDisAIBrainProcess::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pBrainProcessTweaks = (UDisTweaks_AIBrainProcess*)_pTweaks;
}

// DISHONORED(port): 2013 rva 0x77e3d0 (2012 0x78fd60): the tweaks, the owning brain and its pawn, the filter mask and
// then the subclass's own init. The pawn is taken from the brain rather than passed in, so a process built before the
// brain has a pawn would hold NULL - which is why InitBrain_Processes runs after m_pOwningPawn is set.
void UDisAIBrainProcess::InitBrainProcess( UDishonoredAIBrain* const _pOwningBrain, UDisTweaks_AIBrainProcess* const _pBrainProcTweaks )
{
	if( GetTweaks_Derived() != _pBrainProcTweaks )
	{
		SetTweaks_Derived( _pBrainProcTweaks );
		ApplyTweakChanges();
	}
	m_pOwningBrain = _pOwningBrain;
	m_pOwningPawn = _pOwningBrain ? _pOwningBrain->GetOwningPawn() : NULL;
	m_pFilterStimMask = (FPointer)BuildFilterStimMask();
	InitBrainProcess_Derived();
}

// DISHONORED(port): 2013 rva 0x77c1b0 (2012 0x78d7b0): the subclass first, then the two back-pointers are dropped, so
// a terminated process cannot reach a brain that is going away.
void UDisAIBrainProcess::TermBrainProcess()
{
	TermBrainProcess_Derived();
	m_pOwningPawn = NULL;
	m_pOwningBrain = NULL;
}

// DISHONORED(port): 2013 rva 0x77c170 (2012 0x78d770) / 0x77c190 (0x78d790)
void UDisAIBrainProcess::TickBrainProcess( FLOAT _fDeltaSeconds )
{
	TickBrainProcess_Derived( _fDeltaSeconds );
}

void UDisAIBrainProcess::RefreshBrainProcess( FLOAT _fTimeSinceLastThought )
{
	RefreshBrainProcess_Derived( _fTimeSinceLastThought );
}

// DISHONORED(port): 2013 rva 0x77c1d0 (2012 0x78d7d0): the mask is rebuilt on load because it points at static data
// whose address is not saved.
void UDisAIBrainProcess::PostGameLoad_BrainProcess()
{
	m_pFilterStimMask = (FPointer)BuildFilterStimMask();
	PostGameLoad_BrainProcess_Derived();
}

// DISHONORED(port): 2013 rva 0x77c1f0 (2012 0x78d7f0) / 0x77c200 (0x78d800)
void UDisAIBrainProcess::OnOtherActorTerminated_AIBrainProcess( const AActor& _rActor )
{
	OnOtherActorTerminated_AIBrainProcess_Derived( _rActor );
}

void UDisAIBrainProcess::OnDifficultyChange_AIBrainProcess()
{
	OnDifficultyChange_AIBrainProcess_Derived();
}

// DISHONORED(port): 2013 rva 0x78d1e0 (2012 0x79b070): the process's filter mask says which stim ids it cares about,
// and its filter delegate then decides. A process that returns TRUE swallows the stim: no behaviour ever sees it.
UBOOL UDisAIBrainProcess::FilterAIStim_BrainProcess( const FAIStimStruct& _rStim )
{
	const BYTE* Mask = (const BYTE*)m_pFilterStimMask;
	if( !Mask || ( Mask[_rStim.m_StimID] & 1 ) == 0 )
	{
		return FALSE;
	}
	const FDisStimPredicateDelegate Filter = GetFilterStimDelegate_BrainProcess( _rStim.m_StimID );
	return Filter.IsBound() ? Filter( _rStim ) : FALSE;
}
/*-----------------------------------------------------------------------------
	DisSaveLoad. DISHONORED(port): agent EJ (PHASE12 EJ). GameSave (2013 rva 0x736290, retail vtable slot 69)
	is not ported, for agent ED's reason - the writing half of the object layer does not exist here. It is the
	mirror of the body below: the properties, then DisSaveAISubTweakReference (0x7312a0).
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7362c0 (2012 0x798830, byte-identical), retail vtable slot 70. Two reads: the
// process's own script properties, then which of the owning brain tweaks' brain-process tweaks it runs on.
void UDisAIBrainProcess::GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location )
{
	DisSaveLoadObject( _rArchive, this );
	DisLoadAISubTweakReference< UDisTweaks_AIBrainProcess, UDisTweaks_AIBrain >( _rArchive, m_pBrainProcessTweaks,
		&UDisTweaks_AIBrain::m_BrainProcessTweaks );
}
