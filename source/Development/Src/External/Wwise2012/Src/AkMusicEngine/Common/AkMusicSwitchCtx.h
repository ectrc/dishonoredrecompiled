/*=============================================================================
	AkMusicSwitchCtx.h - CAkMusicSwitchCtx, CAkMusicSwitchTransition and CAkNothingCtx (akmusicswitchctx.cpp).

	A switch context follows its container's switch / state group. Its queue of transitions holds the destination
	contexts in play order: the active one, then the scheduled ones, each with its pending play command (at its sync time
	minus its look-ahead) and the stop command it imposes on the one before. A switch change finds the transition rule
	from what will be playing to the new destination, the first valid sync point (by the rule's sync type, honouring
	"same time" / matching cue entries and transition segments), schedules the transition there and cancels the
	transitions it supersedes. The "nothing" context is a silent chain used for an empty switch.

	DISHONORED(port): agent IH4 (wave 26). Layouts (2012 PDB): CAkMusicSwitchTransition 0x48 (align 8; cutoff +8, play
	command +24, stop command +48, bits +64), CAkMusicSwitchCtx 0x78 (CAkMatrixAwareCtx 80 + CAkSwitchAware vptr +80 +
	m_pSwitchCntrNode +84 + m_targetSwitchID +88 + m_eGroupType +92 + m_delayedSwitchID +96 + m_queueTransitions +100 +
	m_itActiveSwitch +112 + m_bWasReferenced +116), CAkNothingCtx 0x68.
=============================================================================*/
#pragma once

#include "AkMatrixAwareCtx.h"
#include "AkSegmentChain.h"
#include <AkSwitchAware.h>

class CAkMusicSwitchCntr;
class CAkScheduleWindow;
struct AkAssociatedAction;

struct AkSwitchPlayCmd
{
	AkInt64		iRelativeTime;
	AkMusicFade	fadeParams;
};

struct AkSwitchStopCmd
{
	AkInt64		iRelativeTime;
	TransParams	transParams;
};

class CAkMusicSwitchTransition
{
public:
	explicit CAkMusicSwitchTransition( CAkMatrixAwareCtx * in_pDestCtx );

	void Cancel();
	void Dispose();
	void ScheduleToPlay( AkInt64 in_iSyncTime, const AkMusicFade & in_fadeParams, AkInt32 in_iLookAheadTime );
	void ScheduleToStop( AkInt64 in_iSyncTime, AkInt64 in_iStopTime, AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, bool in_bCutoff );
	void Restore( AkInt64 in_iCurrentTime, AkUInt32 in_uMaxFadeInDuration, bool in_bUseMaxFadeInDuration );
	void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo );

	CAkMusicSwitchTransition *	pNextItem;
	CAkMatrixAwareCtx *			m_pDestCtx;
	AkCutoffInfo				m_cutoffInfo;
	AkSwitchPlayCmd				m_cmdPlay;
	AkSwitchStopCmd				m_cmdStop;
	AkUInt8						m_bCmdPlayPending : 1;
	AkUInt8						m_bCmdStopPending : 1;
	AkUInt8						m_bWasScheduledToStop : 1;
};

class CAkNothingCtx : public CAkChainCtx
{
public:
	explicit CAkNothingCtx( CAkMusicCtx * in_parent ) : CAkChainCtx( in_parent ), m_bWasReferenced( false ) {}

	virtual void OnStopped();
	virtual CAkMusicNode * Node() { return NULL; }
	virtual bool IsValid() { return true; }

	bool m_bWasReferenced;
};

class CAkMusicSwitchCtx : public CAkMatrixAwareCtx, public CAkSwitchAware
{
public:
	typedef AkListBare<CAkMusicSwitchTransition> TransQueue;

	CAkMusicSwitchCtx( CAkMusicSwitchCntr * in_pSwitchNode, CAkMusicCtx * in_pParentCtx );
	virtual ~CAkMusicSwitchCtx();

	AKRESULT Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams, AkUInt32 in_ulGroupID, AkGroupType in_eGroupType );

	virtual void OnPlayed();
	virtual void OnStopped();
	virtual void EndInit();
	virtual void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo );
	virtual AkInt32 Prepare( const AkMusicTransDestRule * in_pRule, AkInt32 in_iMinStartPosition, AkUInt32 & out_uSelectedCue, AkUInt32 in_uCueHashForMatchSrc );
	virtual void GetNextScheduleWindow( CAkScheduleWindow & io_window, bool in_bDoNotGrow );
	virtual void RefreshWindow( CAkScheduleWindow & io_window );
	virtual void QueryLookAheadInfo( AkInt64 & out_iPlayTime, AkInt64 & out_iPlayTimeAudible );
	virtual CAkMusicNode * Node();
	virtual bool CanRestartPlaying();
	virtual void CancelPlayback( AkInt64 in_iCurrentTime );
	virtual void PerformDelayedSwitchChange();
	virtual void SetSwitch( AkUInt32 in_Switch, CAkRegisteredObj * in_pGameObj = NULL );

	bool HasOrAscendentHasPendingTransition();

	CAkMusicSwitchCntr *			m_pSwitchCntrNode;
	AkUInt32						m_targetSwitchID;
	AkGroupType						m_eGroupType;
	AkUInt32						m_delayedSwitchID;
	TransQueue						m_queueTransitions;
	CAkMusicSwitchTransition *		m_itActiveSwitch;
	bool							m_bWasReferenced;

private:
	AkInt64 ComputeWorstMinSyncTime( AkInt64 in_iCurrentTime, const AkMusicTransSrcRule & in_rule1, AkInt32 in_iLookAhead1, CAkMatrixAwareCtx * in_pTransCtx, const AkMusicTransSrcRule & in_rule2, AkInt32 in_iLookAhead2 );
	const AkMusicTransitionRule & GetTransitionRule( const CAkScheduleWindow & in_source, CAkMatrixAwareCtx *& io_pDestContext, AkUInt32 & io_uSrcSegmentLookAhead );
	void SplitRule( const AkMusicTransitionRule & in_rule, AkMusicTransSrcRule & out_ruleFrom, AkMusicTransDestRule & out_ruleTo, AkMusicTransDestRule & out_ruleToTrans, AkMusicTransSrcRule & out_ruleFromTrans );
	void ScheduleTransition( AkInt64 in_iCurrentTime, CAkMusicSwitchTransition * in_pNewTransition, const AkMusicTransSrcRule & in_ruleFrom, const AkMusicTransDestRule & in_ruleTo, CAkScheduleWindow & io_source, AkInt32 in_iDestinationLookAhead, AkInt64 in_iSyncPoint, bool in_bIsFromTransitionCtx, AkListBareLight<AkAssociatedAction> & io_listCancelledActions );
	CAkMusicSwitchTransition * FindLastNonCancellableTransition();
	void MoveWindowToLastNonCancellableTransition( CAkMusicSwitchTransition * in_pLastNonCancellableTransition, CAkScheduleWindow & io_window );
	void TryPropagateDelayedSwitchChange();
	AKRESULT PrepareFirstContext( CAkMatrixAwareCtx * in_pCtx );
	void DequeueCancellableTransitions( CAkMusicSwitchTransition * in_pLastValidTransition, TransQueue & out_listCancelledTransitions );
	void Sync( CAkMusicSwitchTransition * in_itNewTransition );
	CAkMatrixAwareCtx * CreateMusicContext( AkUniqueID in_ID );
	CAkMatrixAwareCtx * CreateDestinationContext( AkUniqueID in_ID );
	void ScheduleSwitchTransition( AkUniqueID in_destinationID, CAkMatrixAwareCtx *& io_pNewContext );
	void PerformSwitchChange( AkUInt32 in_switchID );
	static CAkMusicSwitchTransition * NewTransition( CAkMatrixAwareCtx * in_pDestCtx );
};

static_assert( sizeof( CAkMusicSwitchTransition ) == 0x48, "CAkMusicSwitchTransition: 2012 PDB 0x48" );
static_assert( sizeof( CAkNothingCtx ) == 0x68, "CAkNothingCtx: 2012 PDB 0x68 (CreateDestinationContext mallocs 0x68)" );
static_assert( sizeof( CAkMusicSwitchCtx ) == 0x78, "CAkMusicSwitchCtx: 2012 PDB 0x78 (CreateContext mallocs 0x78)" );
