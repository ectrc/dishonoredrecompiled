/*=============================================================================
	AkContinuousPBI.h - CAkContinuousPBI (akcontinuouspbi.cpp): the playing instance of a continuous random / sequence
	container's item, which schedules the container's next item (a CAkActionPlayAndContinue) for its transition: on
	its end (none / delay), before its end (cross-fades, trigger rate) or sample-accurately.

	DISHONORED(port): agent IH3, fork C (wave 26). 2012 PDB layout: CAkPBI (0x158) + 0x60 = 0x1B8 (CreatePBI allocates
	0x1B8); the transition mode is the low 4 bits of the dword at +432, the three flags bits 0-2 of the byte at +436.
=============================================================================*/
#pragma once

#include "AkContainers.h"

class CAkContinuousPBI : public CAkPBI
{
public:
	CAkContinuousPBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, ContParams & in_Cparameters, UserParams & in_rUserParams, PlayHistory & in_rPlayHistory, bool in_bIsFirst, AkUniqueID in_SeqID, CAkPBIAware * in_pInstigator, const PriorityInfoCurrent & in_rPriority, bool in_bTargetFeedback, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter );
	virtual ~CAkContinuousPBI();

	using CAkPBI::_Stop;
	virtual void _Stop( AkPBIStopMode in_eStopMode = AkPBIStopMode_Normal, bool in_bIsFromTransition = false, bool in_bHasNotStarted = false );
	virtual AKRESULT Init( AkPathInfo * in_pPathInfo );
	virtual void Term( bool in_bFailedToInit );
	virtual void SetEstimatedLength( AkReal32 in_fEstimatedLength );
	virtual void PlayToEnd( CAkParameterNodeBase * in_pNode );
	virtual void SeekTimeAbsolute( AkTimeMs in_iPosition, bool in_bSnapToMarker );
	virtual void SeekPercent( AkReal32 in_fPercent, bool in_bSnapToMarker );
	virtual void SetPauseStateForContinuous( bool in_bIsPaused );
	virtual void PrepareSampleAccurateTransition();
	virtual void PrepareNextToPlay( bool in_bIsPreliminary );

	void PrepareNextPlayHistory( PlayHistory & in_rPlayHistory );
	AkTransitionMode TransitionMode() const { return (AkTransitionMode)m_eTransitionMode; }

	static AkUniqueID m_CalSeqID;

	CAkContinuationList *	m_spContList;
	PlayHistory				m_PlayHistoryForNextToPlay;
	CAkPBIAware *			m_pInstigator;
	AkReal32				m_fTransitionTime;
	AkUniqueID				m_ulNextElementToPlay;
	AkInt32					m_eTransitionMode : 4;
	AkUInt8					m_bIsFirstPlay : 1;
	AkUInt8					m_bIsContinuousPaused : 1;
	AkUInt8					m_bIsNextPrepared : 1;

protected:
	bool CanSeek();
};
