/*=============================================================================
	AkPBI.h - CAkPBI (akpbi.cpp/.h): one playing instance of a sound - the bridge between the upper engine (node
	hierarchy, playing manager, transitions) and the lower engine's voice (CAkVPLSrcCbxNode).

	DISHONORED(port): agent IH2 (wave 26). Data members are the 2012 PDB's in its order, 0x158 bytes; the bitfield bytes
	+226..+231 match the decompile's masks (m_State is AkCtxState in bits 3-6 of +226, m_bWasStopped bit 7 of +228, ...).
=============================================================================*/
#pragma once

#include "AkAudioMgr.h"
#include "AkLEngineStructs.h"
#include "AkRegistryMgr.h"

class CAkSoundBase;
class CAkSource;
class CAkVPLSrcCbxNode;
class CAkLimiter;
class CAkPath;
class AkFeedbackParams;
class CAkGen3DParams;
class CAkAttenuation;
class CAkPBIAware;

namespace AK
{
	class CAkBusCtx;
}

struct AkPBIModValues
{
	AkReal32	VolumeOffset;
	AkReal32	PitchOffset;
	AkReal32	LPFOffset;
};

struct AkSoundParams
{
	AkReal32	Volume;
	AkReal32	Pitch;
	AkReal32	LPF;
};

struct AkMutedMapItem
{
	void *		m_Identifier;
	AkUInt32	m_bIsGlobal : 1;
	AkUInt32	m_bIsPersistent : 1;

	bool operator==( const AkMutedMapItem & in_rOther ) const { return m_Identifier == in_rOther.m_Identifier && m_bIsGlobal == in_rOther.m_bIsGlobal; }
};

struct PlaybackTransition
{
	PlaybackTransition() : pvPSTrans( NULL ), pvPRTrans( NULL ), bIsPSTransFading( 0 ), bIsPRTransFading( 0 ) {}

	CAkTransition *	pvPSTrans;
	CAkTransition *	pvPRTrans;
	AkUInt8			bIsPSTransFading : 1;
	AkUInt8			bIsPRTransFading : 1;
};

struct AkCntrHistArray
{
	AkUInt32	uiArraySize;
	AkUInt16	aCntrHist[ 32 ];
};

typedef AkCntrHistArray CAkCntrHist;

struct PlayHistory
{
	AkCntrHistArray		HistArray;
	CAkBitArrayMax32	arrayIsContinuous;
};

struct PriorityInfo
{
	AkReal32	priority;
	AkReal32	distanceOffset;
};

struct AkPriorityStruct
{
	AkReal32	priority;
	AkUInt32	PBIID;
};

struct PriorityInfoCurrent
{
	AkPriorityStruct	currentPriority;
	PriorityInfo		priorityInfo;
};

struct AkPathInfo
{
	CAkPath *	pPBPath;
	AkUniqueID	PathOwnerID;
};

struct BaseGenParams
{
	AkReal32	m_fPAN_RL;
	AkReal32	m_fPAN_FR;
	AkReal32	m_fCenterPCT;
	bool		bIsPannerEnabled;
};

struct Prev2DParams
{
	BaseGenParams	prev2DParams;
	AkReal32		prevVolume;
	AkReal32		prevDryLevel;
	AkReal32		prevMotionVol;
};

enum KickFrom
{
	KickFrom_OverNodeLimit		= 0,
	KickFrom_OverGlobalLimit	= 1,
	KickFrom_OverMemoryLimit	= 2
};

enum AKVoiceLimiterType
{
	AKVoiceLimiter_Global	= 0,
	AKVoiceLimiter_AM		= 1,
	AKVoiceLimiter_Bus		= 2
};

enum AkCtxVirtualHandlingResult
{
	VirtualHandling_NotHandled				= 0,
	VirtualHandling_RequiresSeeking			= 1,
	VirtualHandling_ShouldStop				= 2
};

class CAkContinuationList;

struct ContParams
{
	CAkTransition *			pPlayStopTransition;
	CAkTransition *			pPauseResumeTransition;
	AkPathInfo *			pPathInfo;
	bool					bIsPlayStopTransitionFading;
	bool					bIsPauseResumeTransitionFading;
	CAkContinuationList *	spContList;
	AkUInt32				ulPauseCount;
};

/** The parameters of a play request travelling down the node hierarchy (akparameternodebase.h). */
struct AkPBIParams
{
	enum ePBIType
	{
		PBI					= 0,
		ContinuousPBI		= 1,
		DynamicSequencePBI	= 2
	};

	ePBIType			eType;
	CAkPBIAware *		pInstigator;
	CAkRegisteredObj *	pGameObj;
	TransParams *		pTransitionParameters;
	UserParams			userParams;
	PlayHistory			playHistory;
	AkPlaybackState		ePlaybackState;
	AkUInt32			uFrameOffset;
	ContParams *		pContinuousParams;
	AkUInt32			sequenceID;
	bool				bTargetFeedback;
	bool				bIsFirst;
};

class CAkTransportAware
{
public:
	CAkTransportAware() : m_uRegisteredNotif( 0 ) {}
	virtual void _Stop( AkPBIStopMode in_eStopMode = AkPBIStopMode_Normal, bool in_bIsFromTransition = false, bool in_bHasNotStarted = false ) = 0;
	virtual ~CAkTransportAware() {}

	AkUInt32 m_uRegisteredNotif;
};

class CAkPBI : public CAkTransportAware, public ITransitionable
{
public:
	CAkPBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, UserParams & in_rUserparams, PlayHistory & in_rPlayHistory, AkUniqueID in_SeqID, const PriorityInfoCurrent & in_rPriority, bool in_bTargetFeedback, AkUInt32 in_uSourceOffset, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter );

	virtual void _Stop( AkPBIStopMode in_eStopMode = AkPBIStopMode_Normal, bool in_bIsFromTransition = false, bool in_bHasNotStarted = false );
	virtual ~CAkPBI();
	virtual AKRESULT Init( AkPathInfo * in_pPathInfo );
	virtual void Term( bool in_bFailedToInit );
	virtual void SetEstimatedLength( AkReal32 in_fEstimatedLength ) {}
	virtual void _Pause( bool in_bIsFromTransition = false );
	virtual void _Resume();
	virtual void PlayToEnd( CAkParameterNodeBase * in_pNode );
	virtual void SeekTimeAbsolute( AkTimeMs in_iPosition, bool in_bSnapToMarker );
	virtual void SeekPercent( AkReal32 in_fPercent, bool in_bSnapToMarker );
	virtual void SetPauseStateForContinuous( bool in_bIsPaused ) {}
	virtual void PrepareSampleAccurateTransition() {}
	virtual void CalcEffectiveParams();
	virtual AkUInt32 GetStopOffset() const { return (AkUInt32)-1; }
	virtual AkUInt32 GetAndClearStopOffset() { return (AkUInt32)-1; }
	virtual AkCtxVirtualHandlingResult NotifyVirtualOff( AkVirtualQueueBehavior in_eBehavior );
	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	void _InitPlay();
	void _Play( TransParams & in_transParams, bool in_bPaused, bool in_bForceIgnoreSync );
	void _Stop( const TransParams & in_transParams, bool in_bUseMinTransTime );
	void _Pause( TransParams & in_transParams );
	void _Resume( TransParams & in_transParams, bool in_bIsMasterResume );

	void Play( AkReal32 in_fDuration );
	void Stop();
	void Pause();
	void Resume();
	void NotifAddedAsSA();
	void Destroy( AkCtxDestroyReason in_eReason );
	void ProcessContextNotif( AkCtxState in_eState, AkCtxDestroyReason in_eDestroyReason, AkReal32 in_fEstimatedLength );

	void Kick( KickFrom in_eIsForMemoryThreshold );
	void ForceVirtualize( KickFrom in_eReason );
	void Virtualize();
	void Devirtualize( bool in_bAllowKick );
	AkBelowThresholdBehavior GetVirtualBehavior( AkVirtualQueueBehavior & out_Behavior );
	void UpdatePriority( AkReal32 in_NewPriority );
	void RecalcNotification();
	void CalculateMutedEffectiveVolume();
	void CalculateEffectiveLPF();
	void MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems );
	AKRESULT SetMuteMapEntry( AkMutedMapItem & in_key, AkReal32 in_fFadeRatio );
	void NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	bool IsUsingThisSlot( const CAkUsageSlot * in_pSlotToCheck );
	bool IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest );
	AK::CAkBusCtx GetBusContext();
	void GetEnvironmentValues( AkEnvironmentValue * io_paEnvVal );
	AkUniqueID GetSoundID() const;

	CAkSoundBase * GetSound() const { return m_pSound; }
	CAkSource * GetSource() const { return m_pSource; }
	CAkRegisteredObj * GetGameObjectPtr() const { return m_pGameObj; }
	AkPlayingID GetPlayingID() const { return m_UserParams.m_PlayingID; }
	AkUInt32 GetRegisteredNotif() const { return m_uRegisteredNotif; }
	const AkSoundParams & GetEffectiveParams() const { return m_EffectiveParams; }
	AkAudioFormat & GetMediaFormat() { return m_sMediaFormat; }
	void SetMediaFormat( const AkAudioFormat & in_rFormat );
	void InvalidateFeedbackParameters();
	AkInt16 GetLooping() const { return m_LoopCount; }
	AkInt32 GetFrameOffset() const { return m_iFrameOffset; }
	void SetFrameOffset( AkInt32 in_iFrameOffset ) { m_iFrameOffset = in_iFrameOffset; }
	void ConsumeFrameOffset( AkInt32 in_iConsume ) { m_iFrameOffset -= in_iConsume; }
	CAkVPLSrcCbxNode * GetCbx() const { return m_pCbx; }
	void SetCbx( CAkVPLSrcCbxNode * in_pCbx ) { m_pCbx = in_pCbx; }
	AkUInt8 * GetDataPtr() const { return m_pDataPtr; }
	AkUInt32 GetDataSize() const { return m_uDataSize; }
	bool WasStopped() const { return m_bWasStopped; }
	bool WasPaused() const { return m_bWasPaused; }
	bool IsVirtual() const { return m_bIsVirtual; }
	bool IsForcedToVirtualize() const { return m_bIsForcedToVirtualize; }
	bool IsPrefetched() const;
	bool RequiresPreBuffering() const { return m_bRequiresPreBuffering; }
	bool IsSeekDirty() const { return m_bSeekDirty; }
	bool IsSeekRelativeToDuration() const { return m_bSeekRelativeToDuration; }
	bool SnapSeekToMarker() const { return m_bSnapSeekToMarker; }
	void SetSeekDirty( bool in_bDirty ) { m_bSeekDirty = in_bDirty; }
	AkUInt32 GetSeekPosition( AkReal32 in_fSourceDuration, bool & out_bSnapToMarker );
	AkUInt32 GetSourceOffsetRemainder();
	void SetSourceOffsetRemainder( AkUInt32 in_uSourceOffset )
	{
		m_bSeekDirty = 0;
		m_bSeekRelativeToDuration = 0;
		m_bSnapSeekToMarker = 0;
		m_uSeekPosition = in_uSourceOffset;
	}
	bool GetBypassAllFX() const { return m_bBypassAllFX; }
	bool IsAudible() const;
	AkReal32 GetMaxDistance() const { return m_fMaxDistance; }
	PriorityInfoCurrent & GetPriorityInfoCurrent() { return m_PriorityInfoCurrent; }
	AkReal32 GetPriority() const { return m_PriorityInfoCurrent.currentPriority.priority; }
	AkUInt32 GetPBIID() const { return m_PriorityInfoCurrent.currentPriority.PBIID; }
	BaseGenParams & GetBasePosParams() { return m_BasePosParams; }
	Prev2DParams & GetPrev2DParams() { return m_Prev2DParams; }
	CAkGen3DParams * Get3DSound() const { return m_p3DSound; }
	AkPositionKeeper & GetLastComputedPosition() { return m_LastComputedPosition; }

	static AkUInt32 ms_PBIIDGenerator;

	CAkPBI *						pNextItem;
	CAkPBI *						pNextLightItem;
	CAkUsageSlot *					m_pUsageSlot;
	CAkKeyArray<AkMutedMapItem, AkReal32, 5>	m_mapMutedNodes;
	AkPBIModValues					m_Ranges;
	CAkGen3DParams *				m_p3DSound;
	UserParams						m_UserParams;
	PlaybackTransition				m_PBTrans;
	AkUniqueID						m_SeqID;
	CAkSoundBase *					m_pSound;
	CAkSource *						m_pSource;
	CAkRegisteredObj *				m_pGameObj;
	CAkVPLSrcCbxNode *				m_pCbx;
	AkAudioFormat					m_sMediaFormat;
	AkSoundParams					m_EffectiveParams;
	AkReal32						m_Volume;
	AkReal32						m_LPF;
	AkReal32						m_LPFAutomationOffset;
	AkReal32						m_fPlayStopFadeRatio;
	AkReal32						m_fPauseResumeFadeRatio;
	CAkCntrHist						m_CntrHistArray;
	union
	{
		AkUInt32					m_uSeekPosition;
		AkReal32					m_fSeekPercent;
	};
	AkInt16							m_LoopCount;
	AkUInt8							m_eInitialState : 3;
	AkUInt8							m_State : 4;
	AkUInt8							m_eCachedVirtualQueueBehavior : 3;
	AkUInt8							m_eCachedBelowThresholdBehavior : 4;
	AkUInt8							m_bVirtualBehaviorCached : 1;
	AkUInt8							m_bAreParametersValid : 1;
	AkUInt8							m_b3DPositionDone : 1;
	AkUInt8							m_bGetAudioParamsCalled : 1;
	AkUInt8							m_bNeedNotifyEndReached : 1;
	AkUInt8							m_bIsNotifyEndReachedContinuous : 1;
	AkUInt8							m_bTerminatedByStop : 1;
	AkUInt8							m_bPlayFailed : 1;
	AkUInt8							m_bWasStopped : 1;
	AkUInt8							m_bWasPreStopped : 1;
	AkUInt8							m_bWasPaused : 1;
	AkUInt8							m_bInitPlayWasCalled : 1;
	AkUInt8							m_bWasKicked : 1;
	AkUInt8							m_eWasKickedForMemory : 3;
	AkUInt8							m_bWasPlayCountDecremented : 1;
	AkUInt8							m_bRequiresPreBuffering : 1;
	AkUInt8							m_bFeedbackParametersValid : 1;
	AkUInt8							m_bTargetIsFeedback : 1;
	AkUInt8							m_bBypassAllFX : 1;
	AkUInt8							m_bSeekDirty : 1;
	AkUInt8							m_bSeekRelativeToDuration : 1;
	AkUInt8							m_bSnapSeekToMarker : 1;
	AkUInt8							m_bIsForcedToVirtualize : 1;
	AkUInt8							m_bWasForcedToVirtualize : 1;
	AkUInt8							m_bIsVirtual : 1;
	AkUInt8							m_bNeedsFadeIn : 1;
	PriorityInfoCurrent				m_PriorityInfoCurrent;
	AkUInt32						m_ulPauseCount;
	AkInt32							m_iFrameOffset;
	AkUInt8 *						m_pDataPtr;
	AkUInt32						m_uDataSize;
	AkReal32						m_fMaxDistance;
	AkPositionKeeper				m_LastComputedPosition;
	BaseGenParams					m_BasePosParams;
	Prev2DParams					m_Prev2DParams;
	AkFeedbackParams *				m_pFeedbackInfo;
	AkPathInfo						m_PathInfo;
	CAkLimiter *					m_pAMLimiter;
	CAkLimiter *					m_pBusLimiter;

protected:
	void StopWithMinTransTime();
	void DecrementPlayCount();
	void RemoveAllVolatileMuteItems();

private:
	void RefreshParameters();
};
