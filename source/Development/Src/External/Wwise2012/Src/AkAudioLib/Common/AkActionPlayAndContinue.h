/*=============================================================================
	AkActionPlayAndContinue.h - CAkActionPlayAndContinue (akactionplayandcontinue.cpp): the runtime action a continuous
	random / sequence container schedules for its next item (type 0x0503; no bank contains one).

	DISHONORED(port): agent IH2 (wave 26). The PDB layout and the members the audio manager calls on a pending
	action (NeedNotifyDelay 0x949460, Resume 0x9494e0, UnsetFadeBack 0x9494b0, the pause count). Everything that
	creates or runs one (Create, Execute, SetPlayStopTransition, BreakToNode's continuation walk) needs IH3's continuous
	containers and continuation lists, so it is IH3's: BreakToNode here only answers "break" as retail does for an
	action without a continuation list.
=============================================================================*/
#pragma once

#include "AkAudioMgr.h"
#include "AkPBI.h"

class CAkActionPlayAndContinue : public CAkActionPlay
{
public:
	bool NeedNotifyDelay() const { return m_bNeedNotifyDelay; }
	void Resume();
	void UnsetFadeBack( CAkContinuousPBI * in_pPBIToCheck );
	bool BreakToNode( AkUniqueID in_nodeID, CAkRegisteredObj * in_pGameObj, AkPendingAction * in_pPendingAction );

	PlaybackTransition		m_PBTrans;
	AkPathInfo				m_PathInfo;
	CAkContinuationList *	m_spContList;
	PlayHistory				m_PlayHistory;
	CAkContinuousPBI *		m_pPreviousPBI;
	AkTimeMs				m_FadeOutTimeForLast;
	AkPlaybackState			m_InitialPlaybackState;
	AkUInt32				m_SA_PBIID;
	bool					m_bNeedNotifyDelay;
	bool					m_bIsfirstPlay;
	AkPendingAction *		m_pTransitionOwner;
	CAkPBIAware *			m_pInstigator;
	AkUInt32				m_ulPauseCount;
	CAkParameterNodeBase *	m_pPathOwner;

private:
	CAkActionPlayAndContinue( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionPlay( in_eActionType, in_ulID ) {}
};

static const AkUInt16 AkActionType_Duck = 0x1820;
