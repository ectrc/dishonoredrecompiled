/*=============================================================================
	AkAudioMgr.h - CAkAudioMgr (akaudiomgr.cpp/.h): the message queue every AK::SoundEngine call writes into, the
	audio thread that drains it ("AK::EventManager", akaudiothread.cpp), and the pending (delayed / paused) actions.

	DISHONORED(port): agent IH2 (wave 26). Layouts are the 2012 PDB's: AkQueuedMsg is { u16 size, u16 type } followed
	by an 8-aligned union of the AkQueuedMsg_* records (so a message body starts at +8), CAkAudioMgr 0x94 bytes,
	AkPendingAction 0x40 bytes.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include "AkChunkRing.h"
#include "AkTransitionManager.h"
#include "AkHierarchy.h"

class CAkRegisteredObj;
class CAkUsageSlot;
class CAkDynamicSequence;
class CAkContinuousPBI;


/** AkExternalSourceArray (akaudiolib.cpp): the PostEvent external sources, ref-counted by every message and PBI using them. */
struct AkExternalSourceArray
{
	static AkExternalSourceArray * Create( AkUInt32 in_nCount, AkExternalSourceInfo * in_pSrcs );
	void AddRef() { ++m_cRefCount; }
	void Release();

	AkUInt32				m_cRefCount;
	AkUInt32				m_nCount;
	AkExternalSourceInfo	m_pSrcs[ 1 ];
};

/** UserParams (akpbi.h): the PostEvent custom parameter and playing id a PBI and a pending action carry. */
class UserParams
{
public:
	UserParams() : m_PlayingID( AK_INVALID_PLAYING_ID )
	{
		m_CustomParam.customParam = 0;
		m_CustomParam.ui32Reserved = 0;
		m_CustomParam.pExternalSrcs = NULL;
	}
	UserParams( const UserParams & in_rCopy ) : m_PlayingID( in_rCopy.m_PlayingID )
	{
		m_CustomParam = in_rCopy.m_CustomParam;
		if( m_CustomParam.pExternalSrcs )
		{
			m_CustomParam.pExternalSrcs->AddRef();
		}
	}
	~UserParams()
	{
		if( m_CustomParam.pExternalSrcs )
		{
			m_CustomParam.pExternalSrcs->Release();
		}
	}
	UserParams & operator=( const UserParams & in_rCopy )
	{
		Init( in_rCopy.m_PlayingID, in_rCopy.m_CustomParam );
		return *this;
	}
	void Init( AkPlayingID in_playingID, const AkCustomParamType & in_customParam )
	{
		m_PlayingID = in_playingID;
		m_CustomParam.customParam = in_customParam.customParam;
		m_CustomParam.ui32Reserved = in_customParam.ui32Reserved;
		if( m_CustomParam.pExternalSrcs )
		{
			m_CustomParam.pExternalSrcs->Release();
		}
		m_CustomParam.pExternalSrcs = in_customParam.pExternalSrcs;
		if( m_CustomParam.pExternalSrcs )
		{
			m_CustomParam.pExternalSrcs->AddRef();
		}
	}

	AkCustomParamType	m_CustomParam;
	AkPlayingID			m_PlayingID;
};

enum QueuedMsgType
{
	QueuedMsgType_EndOfList						= 0,
	QueuedMsgType_Event							= 1,
	QueuedMsgType_RTPC							= 2,
	QueuedMsgType_RTPCWithTransition			= 3,
	QueuedMsgType_ResetRTPC						= 4,
	QueuedMsgType_ResetRTPCValue				= 5,
	QueuedMsgType_ResetRTPCValueWithTransition	= 6,
	QueuedMsgType_State							= 7,
	QueuedMsgType_Switch						= 8,
	QueuedMsgType_ResetSwitches					= 9,
	QueuedMsgType_Trigger						= 10,
	QueuedMsgType_RegisterGameObj				= 11,
	QueuedMsgType_UnregisterGameObj				= 12,
	QueuedMsgType_GameObjPosition				= 13,
	QueuedMsgType_GameObjActiveListeners		= 14,
	QueuedMsgType_GameObjActiveControllers		= 15,
	QueuedMsgType_ListenerPosition				= 16,
	QueuedMsgType_ListenerSpatialization		= 17,
	QueuedMsgType_GameObjEnvValues				= 18,
	QueuedMsgType_GameObjDryLevel				= 19,
	QueuedMsgType_EnvVolume						= 20,
	QueuedMsgType_EnvBypass						= 21,
	QueuedMsgType_GameObjObstruction			= 22,
	QueuedMsgType_ControllerVolume				= 23,
	QueuedMsgType_OpenDynamicSequence			= 24,
	QueuedMsgType_DynamicSequenceCmd			= 25,
	QueuedMsgType_KillBank						= 26,
	QueuedMsgType_StopAll						= 27,
	QueuedMsgType_ListenerPipeline				= 28,
	QueuedMsgType_SetPlayerListener				= 29,
	QueuedMsgType_AddRemovePlayerDevice			= 30,
	QueuedMsgType_SetPlayerVolume				= 31,
	QueuedMsgType_StopPlayingID					= 32,
	QueuedMsgType_EventAction					= 33,
	QueuedMsgType_GameObjScalingFactor			= 34,
	QueuedMsgType_GameObjMultiPosition			= 35,
	QueuedMsgType_ListenerScalingFactor			= 36,
	QueuedMsgType_Seek							= 37,
	QueuedMsgType_SourcePluginAction			= 38,
	QueuedMsgType_StartStopOutputCapture		= 39,
	QueuedMsgType_SetEffect						= 40,
	QueuedMsgType_SetPanningRule				= 41
};

struct AkQueuedMsg_EventBase
{
	AkGameObjectID		GameObjID;
	AkPlayingID			PlayingID;
	AkPlayingID			TargetPlayingID;
	AkCustomParamType	CustomParam;
};

struct AkQueuedMsg_Event : public AkQueuedMsg_EventBase
{
	CAkEvent *	Event;
};

/** The game-sync records (IH3 fork R; PDB layouts). */
struct AkQueuedMsg_Rtpc
{
	AkRtpcID		ID;
	AkRtpcValue		Value;
	AkGameObjectID	GameObjID;
};

struct AkQueuedMsg_RtpcWithTransition : public AkQueuedMsg_Rtpc
{
	TransParams		transParams;
};

struct AkQueuedMsg_ResetRTPCValue
{
	AkRtpcID		ParamID;
	AkGameObjectID	GameObjID;
};

struct AkQueuedMsg_ResetRtpcValueWithTransition : public AkQueuedMsg_ResetRTPCValue
{
	TransParams		transParams;
};

struct AkQueuedMsg_State
{
	AkStateGroupID	StateGroupID;
	AkStateID		StateID;
	bool			bSkipTransition;
	bool			bSkipExtension;
};

struct AkQueuedMsg_Switch
{
	AkGameObjectID	GameObjID;
	AkSwitchGroupID	SwitchGroupID;
	AkSwitchStateID	SwitchStateID;
};

struct AkQueuedMsg_Trigger
{
	AkGameObjectID	GameObjID;
	AkTriggerID		TriggerID;
};

struct AkQueuedMsg_RegisterGameObj
{
	AkGameObjectID	GameObjID;
	void *			pMonitorData;
};

struct AkQueuedMsg_UnregisterGameObj
{
	AkGameObjectID	GameObjID;
};

struct AkQueuedMsg_GameObjPosition
{
	AkGameObjectID	GameObjID;
	AkSoundPosition	Position;
	AkUInt32		uListenerIndex;
};

struct AkQueuedMsg_GameObjMultiplePosition
{
	AkGameObjectID						GameObjID;
	AkUInt16							uNumPositions;
	AK::SoundEngine::MultiPositionType	eMultiPositionType;
	AkSoundPosition						aMultiPosition[ 1 ];
};

struct AkQueuedMsg_GameObjActiveListeners
{
	AkGameObjectID	GameObjID;
	AkUInt32		uListenerMask;
};

struct AkQueuedMsg_ListenerPosition
{
	AkUInt32			uListenerIndex;
	AkListenerPosition	Position;
};

struct AkQueuedMsg_ListenerSpatialization
{
	AkUInt32			uListenerIndex;
	AkSpeakerVolumes	Volumes;
	bool				bSpatialized;
	bool				bSetVolumes;
};

struct AkQueuedMsg_ListenerScalingFactor
{
	AkUInt32	uListenerIndex;
	AkReal32	fValue;
};

struct AkQueuedMsg_ListenerPipeline
{
	AkUInt32	uListenerIndex;
	bool		bAudio;
	bool		bFeedback;
};

struct AkQueuedMsg_GameObjEnvValues
{
	AkGameObjectID		GameObjID;
	AkUInt32			uNumValues;
	AkEnvironmentValue	EnvValues[ 4 ];
};

struct AkQueuedMsg_GameObjDryLevel
{
	AkGameObjectID	GameObjID;
	AkReal32		fValue;
};

struct AkQueuedMsg_GameObjScalingFactor
{
	AkGameObjectID	GameObjID;
	AkReal32		fValue;
};

struct AkQueuedMsg_EnvVolume
{
	AkAuxBusID			EnvID;
	AkSpeakerVolumes	sVolumes;
};

struct AkQueuedMsg_EnvBypass
{
	AkAuxBusID	EnvID;
	bool		bBypass;
};

struct AkQueuedMsg_GameObjObstruction
{
	AkGameObjectID	GameObjID;
	AkUInt32		uListenerIndex;
	AkReal32		fObstructionLevel;
	AkReal32		fOcclusionLevel;
};

struct AkQueuedMsg_KillBank
{
	CAkUsageSlot *	pUsageSlot;
};

struct AkQueuedMsg_StopAll
{
	AkGameObjectID	GameObjID;
};

struct AkQueuedMsg_StopPlayingID
{
	AkPlayingID				playingID;
	AkTimeMs				uTransitionDuration;
	AkCurveInterpolation	eFadeCurve;
};

struct AkQueuedMsg_EventAction
{
	CAkEvent *								Event;
	AkGameObjectID							GameObjID;
	AK::SoundEngine::AkActionOnEventType	eActionToExecute;
	AkTimeMs								uTransitionDuration;
	AkCurveInterpolation					eFadeCurve;
	AkPlayingID								TargetPlayingID;
};

struct AkQueuedMsg_Seek
{
	CAkEvent *		Event;
	AkGameObjectID	GameObjID;
	union
	{
		AkTimeMs	iPosition;
		AkReal32	fPercent;
	};
	bool			bIsSeekRelativeToDuration;
	bool			bSnapToMarker;
};

struct AkQueuedMsg_SetEffect
{
	AkUniqueID	audioNodeID;
	AkUInt32	uFXIndex;
	AkUniqueID	shareSetID;
};

struct AkQueuedMsg_SetPanningRule
{
	AkPanningRule	panRule;
};

/** IH3 fork C: DynamicSequence::Open's record - the event layout with the sequence where the event goes (+40). */
struct AkQueuedMsg_OpenDynamicSequence : public AkQueuedMsg_EventBase
{
	CAkDynamicSequence *	pDynamicSequence;
};

/** IH3 fork C: the other DynamicSequence calls (sequence +8, command +12, duration +16, curve +20). */
struct AkQueuedMsg_DynamicSequenceCmd
{
	enum Command
	{
		Play			= 0,
		Pause			= 1,
		Resume			= 2,
		Close			= 3,
		Stop			= 4,
		Break			= 5,
		ResumeWaiting	= 6
	};

	CAkDynamicSequence *	pDynamicSequence;
	Command					eCommand;
	AkTimeMs				uTransitionDuration;
	AkCurveInterpolation	eFadeCurve;
};

/** { size, type } then the record; the union is 8-aligned (AkQueuedMsg_Event holds an __int64), so a body starts at +8. */
struct AkQueuedMsg
{
	AkUInt16	size;
	AkUInt16	type;
	union
	{
		AkQueuedMsg_Event						event;
		AkQueuedMsg_Rtpc						rtpc;
		AkQueuedMsg_RtpcWithTransition			rtpcWithTransition;
		AkQueuedMsg_State						setstate;
		AkQueuedMsg_Switch						setswitch;
		AkQueuedMsg_Trigger						trigger;
		AkQueuedMsg_ResetRTPCValue				resetrtpcvalue;
		AkQueuedMsg_ResetRtpcValueWithTransition	resetrtpcvalueWithTransition;
		AkQueuedMsg_RegisterGameObj				reggameobj;
		AkQueuedMsg_UnregisterGameObj			unreggameobj;
		AkQueuedMsg_GameObjPosition				gameobjpos;
		AkQueuedMsg_GameObjMultiplePosition		gameObjMultiPos;
		AkQueuedMsg_GameObjActiveListeners		gameobjactlist;
		AkQueuedMsg_ListenerPosition			listpos;
		AkQueuedMsg_ListenerSpatialization		listspat;
		AkQueuedMsg_GameObjEnvValues			gameobjenvvalues;
		AkQueuedMsg_GameObjDryLevel				gameobjdrylevel;
		AkQueuedMsg_EnvVolume					envvolume;
		AkQueuedMsg_EnvBypass					envbypass;
		AkQueuedMsg_GameObjObstruction			gameobjobstr;
		AkQueuedMsg_KillBank					killbank;
		AkQueuedMsg_ListenerPipeline			listpipe;
		AkQueuedMsg_StopAll						stopAll;
		AkQueuedMsg_StopPlayingID				stopEvent;
		AkQueuedMsg_EventAction					eventAction;
		AkQueuedMsg_GameObjScalingFactor		gameobjscalingfactor;
		AkQueuedMsg_ListenerScalingFactor		listenerscalingfactor;
		AkQueuedMsg_Seek						seek;
		AkQueuedMsg_SetEffect					setEffect;
		AkQueuedMsg_SetPanningRule				setPanningRule;
		AkQueuedMsg_OpenDynamicSequence			opendynamicsequence;
		AkQueuedMsg_DynamicSequenceCmd			dynamicsequencecmd;
	};

	/** The Sizeof_* helpers retail inlines (2012 folds them into a dozen identical 2-instruction functions). */
	static AkUInt16 Sizeof_EndOfList() { return (AkUInt16)offsetof( AkQueuedMsg, event ); }
	template <class T> static AkUInt16 SizeofBody() { return (AkUInt16)( offsetof( AkQueuedMsg, event ) + sizeof( T ) ); }
	static AkUInt16 Sizeof_GameObjMultiPosition( AkUInt16 in_uNumPositions ) { return (AkUInt16)( offsetof( AkQueuedMsg, gameObjMultiPos.aMultiPosition ) + in_uNumPositions * sizeof( AkSoundPosition ) ); }
};
static_assert( offsetof( AkQueuedMsg, event ) == 8, "AkQueuedMsg: the body starts at +8 in the 2012 decompile" );
static_assert( offsetof( AkQueuedMsg, event.Event ) == 40, "AkQueuedMsg_Event::Event at +40 (ProcessMsgQueue reads v5 + 10 dwords)" );

/** One action waiting for its delay to elapse (m_mmapPending, keyed by launch tick) or paused (m_mmapPausedPending, keyed by element). */
class AkPendingAction : public ITransitionable
{
public:
	explicit AkPendingAction( CAkRegisteredObj * in_pGameObj );
	virtual ~AkPendingAction();
	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	CAkAction *			pAction;
	AkUInt32			LaunchTick;
	AkUInt32			LaunchFrameOffset;
	AkUInt32			PausedTick;
	UserParams			UserParam;
	AkUInt32			ulPauseCount;
	AkPlayingID			TargetPlayingID;
	CAkRegisteredObj *	pGameObj;
};

typedef CAkMultiKeyList<AkUInt32, AkPendingAction *, 2> AkMultimapPending;

class CAkAudioThread
{
public:
	CAkAudioThread() : m_eventProcess( NULL ), m_bStopThread( false ) {}

	AKRESULT Start();
	void Stop();
	void WakeupEventsConsumer();

	static AkThread		m_hEventMgrThread;
	static AkThreadID	m_hEventMgrThreadID;

	AkEvent	m_eventProcess;
	bool	m_bStopThread;

private:
	static DWORD WINAPI EventMgrThreadFunc( LPVOID lpParameter );
};

class CAkAudioMgr
{
public:
	CAkAudioMgr();
	~CAkAudioMgr();

	AKRESULT Init();
	void Term();
	AKRESULT Start();
	void Stop();

	AKRESULT Enqueue( AkQueuedMsg & in_rItem, AkUInt32 in_uSize );
	AKRESULT RenderAudio();
	void Perform();

	void EnqueueOrExecuteAction( AkPendingAction * in_pActionItem );
	void ProcessPendingList();
	void RemoveAllPreallocAndReferences();
	void RemoveAllPausedPendingAction();
	void RemoveAllPendingAction();
	void ClearPendingItems( AkPlayingID in_PlayingID );
	void ClearCrossFadeOccurence( CAkContinuousPBI * in_pPBIToCheck );
	void PausePendingItems( AkPlayingID in_PlayingID );
	void ResumePausedPendingItems( AkPlayingID in_playingID );

	AKRESULT StopAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID );
	AKRESULT PauseAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID );
	AKRESULT ResumeAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID );

	AKRESULT StopPendingAction( AkUniqueID in_TargetID, CAkRegisteredObj * in_GameObj, AkPlayingID in_PlayingID );
	AKRESULT StopPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, AkPlayingID in_PlayingID );
	void PausePendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	void PausePendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	void ResumePausedPendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	void ResumePausedPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	void ResumeNotPausedPendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	void ResumeNotPausedPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID );
	AKRESULT BreakPendingAction( AkUniqueID in_TargetID, CAkRegisteredObj * in_GameObj, AkPlayingID in_PlayingID );
	void RemovePendingAction( AkUniqueID in_ulElementID );
	void RemovePausedPendingAction( AkUniqueID in_ulElementID );
	AKRESULT PausePending( AkPendingAction * in_pPA );
	AKRESULT StopPending( AkPendingAction * in_pPA );

	void InsertAsPaused( AkUniqueID in_ElementID, AkPendingAction * in_pPendingAction, AkUInt32 in_ulPauseCount );
	void TransferToPending( AkPendingAction * in_pPendingAction );

	AkUInt32 GetBufferTick() const { return m_uBufferTick; }

	CAkLock				m_queueLock;
	AkSparseChunkRing	m_MsgQueue;
	bool				m_bDrainMsgQueue;
	AkUInt32			m_MsgQueueActualSize;
	AkMultimapPending	m_mmapPending;
	AkMultimapPending	m_mmapPausedPending;
	AkUInt32			m_uBufferTick;
	AkUInt32			m_ulWriterFlag;
	AkUInt32			m_ulReaderFlag;
	CAkAudioThread		m_audioThread;
	AkManualEvent		m_hEventMgrThreadDrainEvent;

private:
	AKRESULT LockedEnqueue( AkQueuedMsg & in_rItem, AkUInt32 in_uSize );
	AKRESULT ProcessMsgQueue();
	void ProcessAction( AkPendingAction * in_pAction );
	void ProcessCustomAction( AkUniqueID in_Target, CAkRegisteredObj * in_pGameObj, AK::SoundEngine::AkActionOnEventType in_ActionToExecute, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID );
	void NotifyDelayStarted( AkPendingAction * in_pPending );
	void NotifyDelayAborted( AkPendingAction * in_pPending, bool in_bWasPaused );
	bool IsAnException( CAkAction * in_pAction, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList );
	bool IsElementOf( AkUniqueID in_TargetID, AkUniqueID in_IDToCheck );
	AkMultimapPending::ListItem * FlushPendingItem( AkPendingAction * in_pPA, AkMultimapPending & in_List, AkMultimapPending::ListItem * in_pItem, AkMultimapPending::ListItem * in_pPrevItem );
	void ReleasePendingAction( AkPendingAction * in_pPA );
};

extern CAkAudioMgr * g_pAudioMgr;
