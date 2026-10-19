/*=============================================================================
	AkActionPlayAndContinue.h - CAkActionPlayAndContinue (akactionplayandcontinue.cpp): the runtime action a continuous
	random / sequence container schedules for its next item (type 0x0503; never in a bank, created by CAkContinuousPBI
	and CAkParameterNode::PlayAndContinueAlternate). 0xAC bytes (Create allocates 0xAC).

	DISHONORED(port): agent IH2 (wave 26) - the layout and the members the audio manager calls on a pending action
	(NeedNotifyDelay 0x949460, Resume 0x9494e0, UnsetFadeBack 0x9494b0, in AkAudioMgr.cpp); agent IH3 fork C - the rest
	(AkActionPlayAndContinue.cpp). m_spContList is retail's CAkSmartPtr as a raw pointer (same 4 bytes).
=============================================================================*/
#pragma once

#include "AkAudioMgr.h"
#include "AkPBI.h"

class CAkActionPlayAndContinue : public CAkActionPlay
{
public:
	static CAkActionPlayAndContinue * Create( AkActionType in_eActionType, AkUniqueID in_ulID, CAkContinuationList *& in_spContList );
	virtual ~CAkActionPlayAndContinue();
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	void GetHistArray( AkCntrHistArray & out_rHistArray );

	bool NeedNotifyDelay() const { return m_bNeedNotifyDelay; }
	void Resume();
	void UnsetFadeBack( CAkContinuousPBI * in_pPBIToCheck );
	bool BreakToNode( AkUniqueID in_nodeID, CAkRegisteredObj * in_pGameObj, AkPendingAction * in_pPendingAction );
	void SetHistory( PlayHistory & in_rPlayHistory );
	void SetInitialPlaybackState( AkPlaybackState in_eInitialPlaybackState );
	void SetInstigator( CAkPBIAware * in_pInstigator );
	void SetIsFirstPlay( bool in_bIsFirstPlay );
	void SetFadeBack( CAkContinuousPBI * in_pPBIToNotify, AkTimeMs in_CrossFadeTime );
	void SetSAInfo( AkUniqueID in_seqID );
	void StartAsPaused();
	AKRESULT SetPlayStopTransition( CAkTransition * in_pTransition, bool in_bTransitionFading, AkPendingAction * in_pTransitionOwner );
	AKRESULT SetPauseResumeTransition( CAkTransition * in_pTransition, bool in_bTransitionFading, AkPendingAction * in_pTransitionOwner );
	void SetPathInfo( AkPathInfo * in_pPathInfo );

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

protected:
	CAkActionPlayAndContinue( AkActionType in_eActionType, AkUniqueID in_ulID, CAkContinuationList *& in_spContList );
};

