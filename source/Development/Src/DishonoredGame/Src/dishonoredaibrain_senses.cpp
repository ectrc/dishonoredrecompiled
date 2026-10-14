// DishonoredGame/src/dishonoredaibrain_senses.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG), rewritten by agent GI (PHASE20 GI): the brain's senses pass ----

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x717880 (2012 0x7562d0, 118 B), decompiled in both databases by agent GI. Agent CG's reading of this
// pass ("asks the controller's FDisComponentVisionNPC what it can see") was wrong: the vision component is ticked by the
// controller's component container, not from here, and reports through ADishonoredNPCController::StartSeeingVisibleThing /
// StopSeeingVisibleThing -> UDishonoredAIBrain::HandleTargetSighted / HandleTargetUnsighted (dishonoredaibrain.cpp). What this
// pass does is: rebuild the final sense masks (2013 inlines BuildFinalSenseMask here as sub_B00F70), and unless the pawn is
// frozen by Bend Time, fire the pending PlayerHeard Kismet event, fire the approach / loiter / look-at dialog hooks, run down
// the magical incident, and tick the brain's own head-track look-at request.
// DISHONORED(bringup): FDisComponentVisionNPC (discomponentvisionnpc.cpp) is still a skeleton, so no sighting ever arrives
// here from retail's path; see disaibrainprocessattention.cpp for what reaches the attention process instead.
void UDishonoredAIBrain::TickBrain_Senses( FLOAT _fDeltaSeconds )
{
	BuildFinalSenseMask();
	// DISHONORED(bringup): the per-actor DisIsActorFrozenByBendTimePower is the global DisIsBendTimeOn() here (agent GD).
	if( DisIsBendTimeOn() )
	{
		return;
	}
	CheckForImportantKismetEvents();
	CheckForImportantDialog();
	if( m_PendingMagicalIncident.m_pMagicUsingPawn )
	{
		m_PendingMagicalIncident.m_fMagicalIncidentExpirationTimer -= _fDeltaSeconds;
		if( m_PendingMagicalIncident.m_fMagicalIncidentExpirationTimer < 0.f )
		{
			m_PendingMagicalIncident.m_pMagicUsingPawn = NULL;
			m_PendingMagicalIncident.m_pMagicalIncidentClass = NULL;
		}
	}
	m_LookAtRequest.TickRequest( _fDeltaSeconds );
}
