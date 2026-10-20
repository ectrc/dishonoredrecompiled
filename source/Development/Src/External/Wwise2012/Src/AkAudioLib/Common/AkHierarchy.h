/*=============================================================================
	AkHierarchy.h - the objects a bank's HIRC chunk creates: the audio node hierarchy (CAkParameterNodeBase and
	its sound / container / bus subclasses), states, events, actions, layers, attenuations and FX.

	DISHONORED(port): agent IH1 (wave 26). Each class keeps the 2012 PDB's data members (names and order from
	resources/docs/types/all_types.h) and the loading half of its behaviour: Create, the ctor/dtor, AddRef /
	Release, AddChild / RemoveChild / CanAddChild, and SetInitialValues with every helper it calls
	(SetNodeBaseParams, SetInitialFxParams, SetInitialParams, SetPositioningParams, SetAdvSettingsParams,
	ReadStateChunk, SetInitialRTPC, ReadFeedbackInfo, SetChildren, the per-type readers). Rva of each is on its
	definition in AkHierarchy.cpp.

	Deviations (named):
	  * The playback half of every class (Play, PBI creation, activity chunks, SIS, PositioningChangeNotification
	    and the rest of the 100-slot CAkParameterNodeBase vtable) is IH2's; the notifications SetInitialValues
	    fires (RecalcNotification, NotifyStateParametersModified, NotifyRTPCChanged, UpdateFx) are empty
	    virtuals here because nothing is playing to be notified.
	  * CAkFxBase::SetFX(id, block, size) asks CAkEffectsMgr::AllocParams for the plug-in's parameter object;
	    the glue registers every plug-in with NULL factories (AkAudio akaudiodevice.cpp), so AllocParams
	    fails exactly as retail does for an unregistered plug-in and SetFX returns AK_Success without a param
	    (retail 0x94ab90's own early-out). The FX id and block are therefore not kept, as in retail.
	  * The music classes (HIRC 10..13) are AkMusicEngine's (IH4); CAkFeedbackBus / CAkFeedbackNode /
	    CAkDialogueEvent are not ported because no Dishonored bank contains one (wwise_bank_dump.py --all:
	    0 of 42548 objects); the bank manager skips their sections and counts them under -akdiag.
=============================================================================*/
#pragma once

#include "AkAudioLibIndex.h"
#include "AkAudioLibGlobals.h"
#include "AkGameSyncs.h"
#include "AkLEngineStructs.h"
#include "AkTransitionManager.h"
#include <AK/SoundEngine/Common/IAkPlugin.h>

class CAkUsageSlot;
class CAkLayerCntr;
class CAkParameterNodeBase;
class CAkRegisteredObj;
class CAkPBI;
class CAkLimiter;
class CAkSoundBase;
class CAkBus;
class CAkSource;
class CAkAttenuation;
class AkPendingAction;
struct AkActivityChunk;
struct AkPBIParams;
struct PriorityInfoCurrent;
struct PriorityInfo;
struct CounterParameters;
struct AkSoundParams;
struct AkPBIModValues;
struct AkMutedMapItem;
struct BaseGenParams;
struct SeekActionParams;
struct SeekActionParamsExcept;
struct AkFXDesc;
class AkFeedbackParams;
class CAkFeedbackBus;
class CAkLayer;

/** The property tables of akparameters.cpp (2012 .rdata 0xd8e0b0 / 0xd8e0f4, .data 0xe37100), indexed by AkPropID. */
extern const AkRTPC_ParameterID g_AkPropRTPCID[ 17 ];
extern const bool g_AkPropDecibel[ 17 ];
extern const AkPropValue g_AkPropDefault[ 17 ];

/** What an RTPC / SIS / duck / state change pushes down the hierarchy to the PBIs (akparameternodebase.h). */
struct NotifParams
{
	AkRTPC_ParameterID	eType;
	CAkRegisteredObj *	pGameObj;
	bool				bIsFromBus;
	void *				pExceptObjects;
	AkReal32			fValue;
};

/** A state, SIS or duck value with the transition driving it (8 bytes in the AkPropBundles). */
struct AkStateValue
{
	AkReal32		fValue;
	CAkTransition *	pTransition;
};

typedef AkStateValue AkSISValue;

/** CAkSIS (aksis.cpp, 0x14 bytes): the values a SetValue / Mute action set on a node, globally or for one game object. */
class CAkSIS : public ITransitionable
{
public:
	CAkSIS( CAkParameterNodeBase * in_pParamNode, CAkRegisteredObj * in_pGameObj, AkUInt8 in_bitsFXBypass ) : m_pParamNode( in_pParamNode ), m_pGameObj( in_pGameObj ), m_bitsFXBypass( in_bitsFXBypass ) {}
	~CAkSIS();
	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	CAkParameterNodeBase *		m_pParamNode;
	CAkRegisteredObj *			m_pGameObj;
	AkPropBundle<AkSISValue>	m_values;
	AkUInt8						m_bitsFXBypass;
};

/** CAkDuckItem (akduckitem.cpp, 0x10 bytes): one bus's duck of this bus. */
class CAkDuckItem : public ITransitionable
{
public:
	AKRESULT Init( CAkBus * in_pBusNode );
	AKRESULT Term();
	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	CAkBus *		m_pBusNode;
	CAkTransition *	m_pvVolumeTransition;
	AkReal32		m_EffectiveVolumeOffset;
};



/** TransParams (akparameters.h): a fade time and curve. */
struct TransParams
{
	AkTimeMs				TransitionTime;
	AkCurveInterpolation	eFadeCurve;
};

enum ActionParamType
{
	ActionParamType_Stop	= 0,
	ActionParamType_Pause	= 1,
	ActionParamType_Resume	= 2,
	ActionParamType_Break	= 3,
	ActionParamType_Seek	= 4
};

/** ActionParams / ActionParamsExcept (akactions.h): what an Execute pushes down the node tree. */
struct ActionParams
{
	ActionParamType		eType;
	CAkRegisteredObj *	pGameObj;
	AkPlayingID			playingID;
	TransParams			transParams;
	bool				bIsFromBus;
	bool				bIsMasterCall;
	bool				bIsMasterResume;
	AkUniqueID			targetNodeID;
};

struct ActionParamsExcept
{
	ActionParamType											eType;
	AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> *	pExeceptionList;
	CAkRegisteredObj *										pGameObj;
	AkPlayingID												playingID;
	TransParams												transParams;
	bool													bIsFromBus;
	bool													bIsMasterResume;
};

/** SeekActionParams(Except) (akactions.h): a seek action's position, absolute ms or a fraction of the duration. */
struct SeekActionParams : public ActionParams
{
	union
	{
		AkTimeMs	iSeekTime;
		AkReal32	fSeekPercent;
	};
	AkUInt8		bIsSeekRelativeToDuration : 1;
	AkUInt8		bSnapToNearestMarker : 1;
};

struct SeekActionParamsExcept : public ActionParamsExcept
{
	union
	{
		AkTimeMs	iSeekTime;
		AkReal32	fSeekPercent;
	};
	AkUInt8		bIsSeekRelativeToDuration : 1;
	AkUInt8		bSnapToNearestMarker : 1;
};

/**
 * DISHONORED(port, IH1 diagnostic): where the last top-level SetInitialValues stopped reading. Retail never
 * checks that an object consumed its whole HIRC section; the bank manager compares this with the section end
 * so a misread field shows up as a per-type "misparsed" count in -akdiag and CoreSmoke instead of as
 * silently wrong data. Written only by the bank thread.
 */
extern const AkUInt8 * g_pHircParseEnd;

enum AkNodeCategory
{
	AkNodeCategory_Bus				= 0,
	AkNodeCategory_ActorMixer		= 1,
	AkNodeCategory_RanSeqCntr		= 2,
	AkNodeCategory_Sound			= 3,
	AkNodeCategory_SwitchCntr		= 4,
	AkNodeCategory_LayerCntr		= 5,
	AkNodeCategory_MusicTrack		= 6,
	AkNodeCategory_MusicSegment		= 7,
	AkNodeCategory_MusicRanSeqCntr	= 8,
	AkNodeCategory_MusicSwitchCntr	= 9,
	AkNodeCategory_FeedbackBus		= 10,
	AkNodeCategory_FeedbackNode		= 11,
	AkNodeCategory_None				= 1000
};

enum AkActionType
{
	AkActionType_None				= 0x0000,
	AkActionType_Play				= 0x0403,
	AkActionType_PlayAndContinue	= 0x0503,
	AkActionType_Duck				= 0x1820
};

enum AkPropID
{
	AkPropID_Volume					= 0,
	AkPropID_LFE					= 1,
	AkPropID_Pitch					= 2,
	AkPropID_LPF					= 3,
	AkPropID_Priority				= 4,
	AkPropID_PriorityDistanceOffset	= 5,
	AkPropID_Loop					= 6,
	AkPropID_FeedbackVolume			= 7,
	AkPropID_FeedbackLPF			= 8,
	AkPropID_MuteRatio				= 9,
	AkPropID_PAN_LR					= 10,
	AkPropID_PAN_FR					= 11,
	AkPropID_CenterPCT				= 12,
	AkPropID_DelayTime				= 13,
	AkPropID_TransitionTime			= 14,
	AkPropID_Probability			= 15,
	AkPropID_DialogueMode			= 16
};

enum AkContainerMode
{
	ContainerMode_Random	= 0,
	ContainerMode_Sequence	= 1
};

/** PDB values; bit 0 = random, bit 1 = continuous, bit 2 = "pick new path when the sound plays" (an authoring flag
    the 2012 enum does not name; CAkPBI::Init 0x940b80 tests m_ePathMode & 4). */
enum AkPathMode
{
	AkStepSequence			= 0,
	AkStepRandom			= 1,
	AkContinuousSequence	= 2,
	AkContinuousRandom		= 3,
	AkPathRandom			= 1,
	AkPathContinuous		= 2,
	AkPathPickNewPath		= 4
};

/** 20 bytes: AkMediaInformation with the PDB's bitfields. */
struct AkMediaInformation
{
	AkUniqueID	sourceID;
	AkFileID	uFileID;
	AkUInt32	uFileOffset;
	AkUInt32	uInMemoryMediaSize;
	AkUInt32	bIsLanguageSpecific : 1;
	AkUInt32	bPrefetch : 1;
	AkUInt32	Type : 5;
	AkUInt32	bHasSource : 1;
	AkUInt32	bExternallySupplied : 1;
	AkUInt32	bIsFromRSX : 1;
};

struct AkBankSourceData
{
	AkUniqueID			m_srcID;
	AkUInt32			m_PluginID;
	AkMediaInformation	m_MediaInfo;
	void *				m_pParam;
	AkUInt32			m_uSize;
};

struct AkSrcTypeInfo
{
	AkMediaInformation	mediaInfo;
	void *				pInMemoryMedia;
	AkUInt32			dwID;
};

class CAkSource
{
public:
	CAkSource();
	virtual ~CAkSource() { FreeSource(); }

	void FreeSource();
	void SetSource( AkUniqueID in_sourceID );
	void SetSource( AkUInt32 in_PluginID, const AkMediaInformation & in_MediaInfo );
	void LockDataPtr( void *& out_ppvBuffer, AkUInt32 & out_ulSize, CAkPBI * in_pPBI );
	void UnLockDataPtr();
	AKRESULT PrepareData();
	void UnPrepareData();

	AkSrcTypeInfo m_sSrcTypeInfo;
};

struct AkPathVertex
{
	AkVector	Vertex;
	AkInt32		Duration;
};

struct AkPathListItem
{
	AkPathVertex *	pVertices;
	AkInt32			iNumVertices;
	AkReal32		fRangeX;
	AkReal32		fRangeY;
};

struct AkPathListItemOffset
{
	AkUInt32	ulVerticesOffset;
	AkInt32		iNumVertices;
};

struct Gen3DParams
{
	AkUniqueID			m_ID;
	AkPositioningType	m_eType;
	AkUniqueID			m_uAttenuationID;
	AkReal32			m_fConeOutsideVolume;
	AkReal32			m_fConeLoPass;
	AkVector			m_Position;
	AkUInt32			m_ePathMode;
	AkTimeMs			m_TransitionTime;
	AkPathVertex *		m_pArrayVertex;
	AkUInt32			m_ulNumVertices;
	AkPathListItem *	m_pArrayPlaylist;
	AkUInt32			m_ulNumPlaylistItem;
	AkUInt8				m_bIsSpatialized : 1;
	AkUInt8				m_bIsConeEnabled : 1;
	AkUInt8				m_bIsPanningFromRTPC : 1;
	AkUInt8				m_bIsDynamic : 1;
	AkUInt8				m_bFollowOrientation : 1;
	AkUInt8				m_bIsLooping : 1;
	CAkAttenuation *	m_pAttenuation;

	CAkAttenuation * GetAttenuation();
};

class CAkPath;

/** AkPathState (PDB): a node's step / random path progress, shared by every PBI of the node (akpath.h). */
struct AkPathState
{
	AkUInt32	ulCurrentListIndex;
	bool *		pbPlayed;
};

class CAkGen3DParams
{
public:
	CAkGen3DParams();
	virtual ~CAkGen3DParams();

	AKRESULT SetPath( AkPathVertex * in_pArrayVertex, AkUInt32 in_ulNumVertices, AkPathListItemOffset * in_pArrayPlaylist, AkUInt32 in_ulNumPlaylistItem );
	AKRESULT SetPathPlayList( CAkPath * in_pPath, AkPathState * in_pState );
	void SetTransition( AkTimeMs in_TransitionTime );
	void Term();

	Gen3DParams m_Params;

private:
	void UpdateTransitionTimeInVertex();
};

class CAkGen3DParamsEx : public CAkGen3DParams
{
public:
	CAkGen3DParamsEx() { m_PathState.ulCurrentListIndex = 0; m_PathState.pbPlayed = NULL; }
	virtual ~CAkGen3DParamsEx() { FreePathInfo(); }
	void FreePathInfo();

	AkPathState m_PathState;
};

/** One (state group, node) link: the node's state values per state id of the group. */
struct AkStateLink
{
	class CAkState *	pState;
	AkUniqueID			ulStateID;
};

class AkStateGroupChunk : public ITransitionable
{
public:
	AkStateGroupChunk( CAkParameterNodeBase * in_pOwner, AkStateGroupID in_ulStateGroup );
	~AkStateGroupChunk();

	AKRESULT AddState( AkUniqueID in_ulStateInstanceID, AkStateID in_ulStateID, bool in_bNotify );
	class CAkState * GetState( AkStateID in_StateTypeID );
	void FlushStateTransitions();
	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	AkStateGroupChunk *						m_pNextInGroup;
	AkStateGroupChunk *						m_pNextInNode;
	CAkParameterNodeBase *					m_pOwner;
	AkPropBundle<AkStateValue>				m_values;
	CAkKeyArray<AkStateID, AkStateLink, 5>	m_mapStates;
	const AkStateGroupID					m_ulStateGroup;
	AkStateID								m_ulActualState;
	AkUInt8									m_eStateSyncType;
};

struct AkListBareLightNextInNode
{
	static AkStateGroupChunk *& Get( AkStateGroupChunk * in_pItem ) { return in_pItem->m_pNextInNode; }
};

/** CAkPBIAware (akpbiaware.cpp): what can instantiate a PBI - every node, and the dynamic sequence. */
class CAkPBIAware : public CAkIndexable
{
public:
	explicit CAkPBIAware( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ) {}
	virtual CAkPBI * CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const;
};

class CAkParameterNodeBase : public CAkPBIAware
{
public:
	struct FXStruct
	{
		bool		bRendered;
		bool		bShareSet;
		AkUniqueID	id;
	};

	struct FXChunk
	{
		FXChunk();
		FXStruct	aFX[ 4 ];
		AkUInt8		bitsMainFXBypass;
	};

	struct AkFeedbackInfo
	{
		CAkParameterNodeBase * m_pFeedbackBus;
	};

	explicit CAkParameterNodeBase( AkUniqueID in_ulID );
	virtual ~CAkParameterNodeBase();

	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_ActorMixer; }

	virtual void OnPreRelease() {}
	virtual void Parent( CAkParameterNodeBase * in_pParent ) { m_pParentNode = in_pParent; }
	virtual void ParentBus( CAkParameterNodeBase * in_pParentBus ) { m_pBusOutputNode = in_pParentBus; }
	virtual AKRESULT AddChild( AkUniqueID ) { return AK_NotImplemented; }
	virtual void RemoveChild( CAkParameterNodeBase * ) {}
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * ) { return AK_NotCompatible; }
	virtual void GetChildren( AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos, AkUInt32 & index_out, AkUInt32 iDepth ) {}
	virtual AkNodeCategory NodeCategory() = 0;
	virtual void RecalcNotification() {}
	virtual void NotifyStateParametersModified();
	virtual void NotifyRTPCChanged() {}
	virtual void UpdateFx( AkUInt32 in_uFXIndex ) {}
	virtual CAkRTPCMgr::SubscriberType GetRTPCSubscriberType() const { return CAkRTPCMgr::SubscriberType_CAkParameterNodeBase; }

	// DISHONORED(port, IH3 node graph): the notification, SIS and RTPC half of the 2012 vtable (akparameternodebase.h).
	virtual void ParamNotification( NotifParams & in_rParams ) {}
	virtual void Notification( AkRTPC_ParameterID in_ParamID, AkReal32 in_fValue, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL );
	virtual void PriorityNotification( NotifParams & in_rParams );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus ) {}
	virtual void MuteNotification( AkReal32 in_fMuteRatio, CAkRegisteredObj * in_pGameObj, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems = false ) {}
	virtual void ForAllPBI( void ( *in_funcForAll )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj * in_pGameObj, void * in_pCookie ) {}
	virtual void PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray ) {}
	virtual void NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL ) {}
	virtual bool Has3DParams() { return false; }
	virtual AKRESULT PlayToEnd( CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, AkPlayingID in_PlayingID ) { return AK_Success; }
	virtual void SetAkProp( AkPropID in_eProp, CAkRegisteredObj * in_GameObjPtr, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue = 0.f, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 ) {}
	virtual void SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax );
	virtual void SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax );
	virtual void ResetAkProp( AkPropID in_eProp, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime ) {}
	virtual void Mute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 ) {}
	virtual void Unmute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 ) {}
	virtual void UnmuteAll( AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 ) {}
	virtual void Unregister( CAkRegisteredObj * in_GameObjPtr ) {}
	virtual void ResetFXBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask = 0xFFFFFFFF ) {}
	virtual void SetFeedbackVolume( CAkRegisteredObj * in_GameObjPtr, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue = 0.f, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void ApplyMaxNumInstances( AkUInt16 in_u16MaxNumInstance, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL, bool in_bFromRTPC = false ) {}
	virtual AKRESULT SetParamComplexFromRTPCManager( void * in_pToken, AkUInt32 in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray );
	virtual void UnsetRTPC( AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID );
	virtual AKRESULT ExecuteActionExceptParentCheck( ActionParamsExcept & in_rAction ) { return ExecuteActionExcept( in_rAction ); }
	virtual CAkBus * GetLimitingBus();
	virtual void FeedbackParentBus( CAkFeedbackBus * in_pParent );
	virtual CAkFeedbackBus * FeedbackParentBus();

	struct CAkStateSyncArray
	{
		AkArray<AkUInt32, AkUInt32, ArrayPoolDefault, 16> m_StateSyncArray;
	};

	AKRESULT Stop( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
	AKRESULT Pause( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
	AKRESULT Resume( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
	void PriorityNotification( AkReal32 in_Priority, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray );
	bool IsPlaying() const;
	static bool IsException( AkUniqueID in_ID, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> & in_rExceptionList );
	void UseState( bool in_bUseState );
	AKRESULT RemoveFX( AkUInt32 in_uFXIndex );
	void BypassFX( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL, bool in_bIsFromReset = false );
	void ResetBypassFX( AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL );
	void StartSISTransition( CAkSIS * in_pSIS, AkPropID in_ePropID, AkReal32 in_fTargetValue, AkValueMeaning in_eValueMeaning, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	void StartSisMuteTransitions( CAkSIS * in_pSIS, AkReal32 in_fTargetValue, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	bool CheckSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes );
	bool GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes, bool in_bBusChecked = false );

	// The playback half (DISHONORED(port, IH2): the members the play / stop / pause path reaches).
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams ) { return AK_NotImplemented; }
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction ) { return AK_Success; }
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction ) { return AK_Success; }
	virtual AKRESULT IncrementPlayCount( CounterParameters & io_params ) = 0;
	virtual void DecrementPlayCount( CounterParameters & io_params ) = 0;
	virtual void IncrementVirtualCount( CounterParameters & io_params ) {}
	virtual void DecrementVirtualCount( CounterParameters & io_params ) {}
	virtual void PauseTransitions( bool in_bPause );
	virtual bool GetBypassAllFX( CAkRegisteredObj * in_GameObjPtr ) { return false; }
	/** DISHONORED(IH4): the insert effect of a slot (AddRef'd share set or custom) and its media ids; the node base answers
	    none, CAkParameterNode (0x93a2b0 / 0x93a370) and CAkBus (0x93d6b0 / 0x93d750) override. */
	virtual void GetFX( AkUInt32 in_uFXIndex, AkFXDesc & out_rFXInfo, CAkRegisteredObj * in_GameObj );
	virtual void GetFXDataID( AkUInt32 in_uFXIndex, AkUInt32 in_uDataIndex, AkUInt32 & out_rDataID ) { out_rDataID = (AkUInt32)-1; }
	virtual CAkBus * GetMixingBus();
	virtual AKRESULT GetAudioParameters( AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr, bool in_bIncludeRange, AkPBIModValues & io_Ranges, bool in_bDoBusCheck ) { return AK_Success; }
	virtual bool IsPlayable() { return true; }
	virtual bool IncrementActivityCount( AkUInt16 in_flagForwardToBus );
	virtual void DecrementActivityCount( AkUInt16 in_flagForwardToBus );
	virtual AKRESULT PrepareData() { return AK_Success; }
	virtual void UnPrepareData() {}

	static AKRESULT PrepareNodeData( AkUniqueID in_NodeID );
	static void UnPrepareNodeData( AkUniqueID in_NodeID );

	PriorityInfo GetPriority( CAkRegisteredObj * in_GameObjPtr );
	AkUInt16 GetMaxNumInstances( CAkRegisteredObj * in_GameObjPtr );

	virtual AKRESULT SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize ) { return AK_Success; }
	virtual AKRESULT SetInitialFxParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly ) { return AK_Success; }
	virtual AKRESULT SetPositioningParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize ) { return AK_Success; }
	virtual AKRESULT SetAdvSettingsParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize ) { return AK_Success; }

	void AddToIndex();
	AKRESULT SetFX( AkUInt32 in_uFXIndex, AkUniqueID in_uID, bool in_bShareSet );
	void RenderedFX( AkUInt32 in_uFXIndex, bool in_bRendered );
	void MainBypassFX( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	AkStateGroupChunk * AddStateGroup( AkStateGroupID in_ulStateGroupID, bool in_bNotify );
	void RemoveStateGroups( bool in_bNotify );

	void SetMaxReachedBehavior( bool in_bKillNewest );
	void SetOverLimitBehavior( bool in_bUseVirtualBehavior );
	void SetMaxNumInstOverrideParent( bool in_bOverride ) { m_bIsMaxNumInstOverrideParent = in_bOverride; }
	void SetVVoicesOptOverrideParent( bool in_bOverride ) { m_bIsVVoicesOptOverrideParent = in_bOverride; }

	AkListBareLight<AkStateGroupChunk, AkListBareLightNextInNode>	m_states;
	CAkSIS *					m_pGlobalSIS;
	FXChunk *					m_pFXChunk;
	AkActivityChunk *			m_pActivityChunk;
	CAkParameterNodeBase *		m_pParentNode;
	CAkParameterNodeBase *		m_pBusOutputNode;
	AkPropBundle<AkPropValue>	m_props;
	AkUInt16					m_u16MaxNumInstance;
	AkUInt8						m_bKillNewest : 1;
	AkUInt8						m_bUseVirtualBehavior : 1;
	AkUInt8						m_bIsVVoicesOptOverrideParent : 1;
	AkUInt8						m_bIsMaxNumInstOverrideParent : 1;
	AkUInt8						m_bIsGlobalLimit : 1;
	AkUInt8						m_bPriorityApplyDistFactor : 1;
	AkUInt8						m_bPriorityOverrideParent : 1;
	AkUInt8						m_bUseState : 1;
	AkUInt8						m_bIsInDestructor : 1;
	AkUInt8						m_bPositioningInfoOverrideParent : 1;
	AkUInt8						m_bPositioningEnablePanner : 1;
	AkUInt8						m_bIsFXOverrideParent : 1;
	AkUInt8						m_bIsContinuousValidation : 1;
	CAkBitArrayMax32			m_RTPCBitArrayMax32;
	AkFeedbackInfo *			m_pFeedbackInfo;

	bool EnableActivityChunk();
	void DeleteActivityChunk();

protected:
	AKRESULT IncrementPlayCountValue();
	void DecrementPlayCountValue();
	AKRESULT IncrementPlayCountGlobal( AkReal32 in_fPriority, AkUInt16 & io_ui16NumKickedOrRevived, CAkLimiter *& io_pLimiter );
	void DecrementPlayCountGlobal();
	AKRESULT IncrementPlayCountGameObject( AkReal32 in_fPriority, AkUInt16 & io_ui16NumKickedOrRevived, CAkRegisteredObj * in_pGameObj, CAkLimiter *& io_pLimiter );
	void DecrementPlayCountGameObject( CAkRegisteredObj * in_pGameObj );
	void IncrementVirtualCountGameObject( CAkRegisteredObj * in_pGameObj );
	void DecrementVirtualCountGlobal( AkUInt16 & io_ui16NumKickedOrRevived, bool in_bAllowKick );
	void DecrementVirtualCountGameObject( AkUInt16 & io_ui16NumKickedOrRevived, bool in_bAllowKick, CAkRegisteredObj * in_pGameObj );
	void GetAudioStateParams( AkSoundParams & io_Parameters, AkUInt32 in_uParamSelect );
	void FlushStateTransitions();
	virtual CAkSIS * GetSIS( CAkRegisteredObj * in_GameObjPtr ) { return NULL; }

	AKRESULT SetNodeBaseParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly );
	AKRESULT SetInitialRTPC( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	AKRESULT ReadStateChunk( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	AKRESULT ReadFeedbackInfo( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
};

class CAkParameterNode : public CAkParameterNodeBase
{
public:
	explicit CAkParameterNode( AkUniqueID in_ulID );
	virtual ~CAkParameterNode();

	virtual AKRESULT SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT SetInitialFxParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly );
	virtual AKRESULT SetPositioningParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT SetAdvSettingsParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

	AKRESULT GetStatic3DParams( AkPositioningInfo & out_rPosInfo );
	bool GetMaxRadius( AkReal32 & out_fRadius );

	virtual AKRESULT IncrementPlayCount( CounterParameters & io_params );
	virtual void DecrementPlayCount( CounterParameters & io_params );
	virtual void GetFX( AkUInt32 in_uFXIndex, AkFXDesc & out_rFXInfo, CAkRegisteredObj * in_GameObj );
	virtual void GetFXDataID( AkUInt32 in_uFXIndex, AkUInt32 in_uDataIndex, AkUInt32 & out_rDataID );
	virtual bool GetBypassAllFX( CAkRegisteredObj * in_GameObjPtr );
	virtual AKRESULT GetAudioParameters( AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr, bool in_bIncludeRange, AkPBIModValues & io_Ranges, bool in_bDoBusCheck );
	AkBelowThresholdBehavior GetVirtualBehavior( AkVirtualQueueBehavior & out_Behavior ) const;
	bool IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest, bool in_bIsBusChecked, bool in_bIgnoreNonBus );
	void Get3DParams( CAkGen3DParams *& io_p3DParams, CAkRegisteredObj * in_GameObj, bool in_bUpdateOnly, BaseGenParams * io_pBasePosParams );
	bool UpdateBaseParams( CAkRegisteredObj * in_GameObj, BaseGenParams * io_pBasePosParams );
	bool UpdateBaseParamsFromRTPC( CAkRegisteredObj * in_GameObj, BaseGenParams * io_pBasePosParams );
	AkPathState * GetPathState();

	using CAkParameterNodeBase::SetAkProp;
	virtual void SetAkProp( AkPropID in_eProp, CAkRegisteredObj * in_GameObjPtr, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue = 0.f, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax );
	virtual void SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax );
	virtual void ResetAkProp( AkPropID in_eProp, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	virtual void Mute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void Unmute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void UnmuteAll( AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void UnmuteAllObj( AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	virtual void Unregister( CAkRegisteredObj * in_GameObjPtr );
	virtual void ResetFXBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask = 0xFFFFFFFF );
	virtual AKRESULT ExecuteActionExceptParentCheck( ActionParamsExcept & in_rAction );
	virtual bool Has3DParams() { return m_p3DParameters != NULL; }
	virtual void IncrementVirtualCount( CounterParameters & io_params );
	virtual void DecrementVirtualCount( CounterParameters & io_params );
	virtual void ApplyMaxNumInstances( AkUInt16 in_u16MaxNumInstance, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL, bool in_bFromRTPC = false );
	virtual void UnsetRTPC( AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID );
	virtual void NotifyRTPCChanged();
	bool IsOrIsChildOf( AkUniqueID in_NodeIDToTest, bool in_bIsBusChecked );
	bool GetBypassFX( AkUInt32 in_uFXIndex, CAkRegisteredObj * in_GameObjPtr );
	AKRESULT AssociateLayer( CAkLayer * in_pLayer );
	AKRESULT DissociateLayer( CAkLayer * in_pLayer );
	virtual AKRESULT PlayAndContinueAlternate( AkPBIParams & in_rPBIParams );

	CAkKeyArray<CAkRegisteredObj *, CAkSIS *, 8> *	m_pMapSIS;
	CAkGen3DParamsEx *								m_p3DParameters;
	AkPropBundle< RANGED_MODIFIERS<AkPropValue> >	m_ranges;
	AkUInt16										m_eVirtualQueueBehavior;
	AkUInt16										m_eBelowThresholdBehavior;
	AkArray<class CAkLayer *, class CAkLayer *, ArrayPoolDefault, 8> *	m_pAssociatedLayers;

protected:
	AKRESULT Enable3DPosParams();
	virtual CAkSIS * GetSIS( CAkRegisteredObj * in_GameObjPtr );
};

struct AkChildIDValueGetKey
{
	static AkUniqueID Get( CAkParameterNodeBase * const & in_item ) { return in_item->key; }
};

typedef AkSortedKeyArray<AkUniqueID, CAkParameterNodeBase *, ArrayPoolDefault, AkChildIDValueGetKey, 1> AkMapChildID;

/** CAkParentNode<T>: the sorted child map plus AddChild / RemoveChild / SetChildren / GetChildren. */
template <class T>
class CAkParentNode : public T
{
public:
	explicit CAkParentNode( AkUniqueID in_ulID ) : T( in_ulID ) {}
	virtual ~CAkParentNode() { m_mapChildId.Term(); }

	virtual AKRESULT AddChild( AkUniqueID in_ulID );
	virtual void RemoveChild( CAkParameterNodeBase * in_pChild );
	virtual void GetChildren( AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos, AkUInt32 & index_out, AkUInt32 iDepth );

	AKRESULT SetChildren( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

	/** CAkActiveParent<T>::ExecuteAction / ExecuteActionExcept (akactiveparent.h, 2012 0x94de50 / 0x94dee0): every child. */
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );

	/** CAkActiveParent<T>::PrepareData / UnPrepareData (2012 0x94e240 / 0x94e2a0, 0x93ccf0 / 0x93cd50): every child. */
	virtual AKRESULT PrepareData();
	virtual void UnPrepareData();

	/** CAkActiveParent<T>'s notification forwarding (akactiveparent.h): every child that is playing. */
	virtual void ParamNotification( NotifParams & in_rParams );
	virtual void PriorityNotification( NotifParams & in_rParams );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, CAkRegisteredObj * in_pGameObj, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems = false );
	virtual void ForAllPBI( void ( *in_funcForAll )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj * in_pGameObj, void * in_pCookie );
	virtual void PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray );
	virtual void RecalcNotification();
	virtual void NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL );
	virtual void UpdateFx( AkUInt32 in_uFXIndex );
	virtual AKRESULT PlayToEnd( CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, AkPlayingID in_PlayingID );

	AkMapChildID m_mapChildId;
};

class CAkSoundBase : public CAkParameterNode
{
public:
	explicit CAkSoundBase( AkUniqueID in_ulID ) : CAkParameterNode( in_ulID ) {}

	AkInt16 Loop();
	virtual AKRESULT PlayToEnd( CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, AkPlayingID in_PlayingID );
	virtual void ParamNotification( NotifParams & in_rParams );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, CAkRegisteredObj * in_pGameObj, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems = false );
	virtual void ForAllPBI( void ( *in_funcForAll )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj * in_pGameObj, void * in_pCookie );
	virtual void PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray );
	virtual void RecalcNotification();
	virtual void NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL );
	virtual void UpdateFx( AkUInt32 in_uFXIndex );
};

class CAkSound : public CAkSoundBase
{
public:
	static CAkSound * Create( AkUniqueID in_ulID );
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_Sound; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize, CAkUsageSlot * in_pUsageSlot, bool in_bIsPartialLoadOnly );
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual bool IsPlayable();
	virtual AKRESULT PrepareData() { return m_Source.PrepareData(); }
	virtual void UnPrepareData() { m_Source.UnPrepareData(); }
	bool HasBankSource() const { return m_Source.m_sSrcTypeInfo.mediaInfo.Type != 0 && m_Source.m_sSrcTypeInfo.mediaInfo.bHasSource; }
	void SeekSound( CAkRegisteredObj * in_pGameObj, const SeekActionParams & in_rSeekParams );
	void SeekSound( CAkRegisteredObj * in_pGameObj, const SeekActionParamsExcept & in_rSeekParams );

	CAkSource m_Source;

protected:
	explicit CAkSound( AkUniqueID in_ulID ) : CAkSoundBase( in_ulID ) {}
};

class CAkActorMixer : public CAkParentNode<CAkParameterNode>
{
public:
	static CAkActorMixer * Create( AkUniqueID in_ulID );
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_ActorMixer; }
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

protected:
	explicit CAkActorMixer( AkUniqueID in_ulID ) : CAkParentNode<CAkParameterNode>( in_ulID ) {}
};

/* The containers (CAkContainerBase, CAkRanSeqCntr and its playlists, CAkMultiPlayNode, CAkSwitchCntr, CAkLayer,
   CAkLayerCntr) are declared in AkContainers.h (agent IH3 fork C): they hold PBI-side types by value. */

class CAkBus : public CAkParentNode<CAkParameterNodeBase>
{
public:
	struct AkDuckInfo
	{
		AkReal32				DuckVolume;
		AkTimeMs				FadeOutTime;
		AkTimeMs				FadeInTime;
		AkCurveInterpolation	FadeCurve;
	};

	static CAkBus * Create( AkUniqueID in_ulID );
	static void ClearMasterBus();
	static CAkBus * s_pMasterBus;

	virtual ~CAkBus();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_Bus; }
	virtual AKRESULT AddChild( AkUniqueID in_ulID );
	virtual void RemoveChild( CAkParameterNodeBase * in_pChild );
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );
	virtual AKRESULT SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT SetInitialFxParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT AddDuck( AkUniqueID in_BusID, AkReal32 in_DuckVolume, AkTimeMs in_FadeOutTime, AkTimeMs in_FadeInTime, AkCurveInterpolation in_eFadeCurve );
	bool IsMasterBus() const { return this == s_pMasterBus; }
	static CAkBus * GetMasterBusAndAddRef();
	bool IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest );
	AkReal32 GetBusEffectiveVolume( AkUInt32 in_eVolumeType, AkPropID in_eProp );

	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual AKRESULT IncrementPlayCount( CounterParameters & io_params );
	virtual void DecrementPlayCount( CounterParameters & io_params );
	virtual void GetFX( AkUInt32 in_uFXIndex, AkFXDesc & out_rFXInfo, CAkRegisteredObj * in_GameObj );
	virtual void GetFXDataID( AkUInt32 in_uFXIndex, AkUInt32 in_uDataIndex, AkUInt32 & out_rDataID );
	virtual bool GetBypassAllFX( CAkRegisteredObj * in_GameObjPtr );
	virtual CAkBus * GetMixingBus();
	virtual AKRESULT GetAudioParameters( AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr, bool in_bIncludeRange, AkPBIModValues & io_Ranges, bool in_bDoBusCheck );
	virtual bool IncrementActivityCount( AkUInt16 in_flagForwardToBus );
	virtual void DecrementActivityCount( AkUInt16 in_flagForwardToBus );
	bool HasEffect() const;

	enum DuckState
	{
		DuckState_OFF		= 0,
		DuckState_ON		= 1,
		DuckState_PENDING	= 2
	};

	virtual CAkRTPCMgr::SubscriberType GetRTPCSubscriberType() const { return CAkRTPCMgr::SubscriberType_CAkBus; }
	using CAkParameterNodeBase::SetAkProp;
	virtual void SetAkProp( AkPropID in_eProp, CAkRegisteredObj * in_pGameObj, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue = 0.f, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax );
	virtual void ResetAkProp( AkPropID in_eProp, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	virtual void Mute( CAkRegisteredObj * in_pGameObj, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void Unmute( CAkRegisteredObj * in_pGameObj, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void UnmuteAll( AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkTimeMs in_lTransitionTime = 0 );
	virtual void ResetFXBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask = 0xFFFFFFFF );
	virtual void ParamNotification( NotifParams & in_rParams );
	virtual void MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus );
	virtual void NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL );
	virtual void RecalcNotification();
	virtual void UpdateFx( AkUInt32 in_uFXIndex );
	virtual void ApplyMaxNumInstances( AkUInt16 in_u16MaxNumInstance, CAkRegisteredObj * in_pGameObj = NULL, void * in_pExceptArray = NULL, bool in_bFromRTPC = false );
	virtual void IncrementVirtualCount( CounterParameters & io_params );
	virtual void DecrementVirtualCount( CounterParameters & io_params );
	virtual AKRESULT ExecuteActionExceptParentCheck( ActionParamsExcept & in_rAction ) { return ExecuteActionExcept( in_rAction ); }
	virtual CAkBus * GetLimitingBus();
	bool IsOrIsChildOf( AkUniqueID in_NodeIDToTest );
	bool GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes );
	bool GetBypassFX( AkUInt32 in_uFXIndex );
	void CheckDuck();
	AkReal32 GetDuckedVolume();
	AKRESULT Duck( AkUniqueID in_BusID, AkReal32 in_DuckVolume, AkTimeMs in_FadeOutTime, AkCurveInterpolation in_eFadeCurve );
	AKRESULT Unduck( AkUniqueID in_BusID, AkTimeMs in_FadeInTime, AkCurveInterpolation in_eFadeCurve );
	AKRESULT PauseDuck( AkUniqueID in_BusID );
	void DuckNotif();

	AkUInt32												m_RecoveryTime;
	AkUInt32												m_uChannelConfig;
	AkReal32												m_fMaxDuckVolume;
	CAkKeyList<AkUniqueID, AkDuckInfo, 1, ArrayPoolDefault>	m_ToDuckList;
	CAkKeyList<AkUniqueID, CAkDuckItem, 1, ArrayPoolDefault>	m_DuckedVolumeList;
	AkUInt8													m_eDuckingState : 3;
	AkUInt8													m_bIsEnvironmental : 1;

protected:
	explicit CAkBus( AkUniqueID in_ulID );
	virtual CAkSIS * GetSIS( CAkRegisteredObj * in_pGameObj );
	void StartDuckTransitions( CAkDuckItem * in_pDuckItem, AkReal32 in_fTargetValue, AkValueMeaning in_eValueMeaning, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime );
	void UpdateDuckedBus();
	AKRESULT RequestDuckNotif();
};

class CAkState : public CAkIndexable
{
public:
	static CAkState * Create( AkUniqueID in_ulID );
	virtual ~CAkState() { m_props.RemoveAll(); }
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_State; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	void SetParentToNotify( CAkParameterNodeBase * in_pParent ) { m_pParentToNotify = in_pParent; }
	void TermNotificationSystem() { m_pParentToNotify = NULL; }

	AkPropBundle<AkReal32>	m_props;
	CAkParameterNodeBase *	m_pParentToNotify;

protected:
	explicit CAkState( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_pParentToNotify( NULL ) {}
};

class CAkAttenuation : public CAkIndexable
{
public:
	struct ConeParams
	{
		AkReal32 fInsideAngle;
		AkReal32 fOutsideAngle;
		AkReal32 fOutsideVolume;
		AkReal32 LoPass;
	};

	struct RTPCSubs
	{
		AkRtpcID										RTPCID;
		AkUInt32										ParamID;
		AkUniqueID										RTPCCurveID;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	ConversionTable;
	};

	enum AttenuationCurveID
	{
		AttenuationCurveID_VolumeDry		= 0,
		AttenuationCurveID_VolumeWet		= 1,
		AttenuationCurveID_LowPassFilter	= 2,
		AttenuationCurveID_Spread			= 3,
		AttenuationCurveID_None				= 0xFF
	};

	static CAkAttenuation * Create( AkUniqueID in_ulID );
	virtual ~CAkAttenuation();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Attenuation; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * GetCurve( AkUInt8 in_parameterIndex );
	void ClearRTPCs();

	ConeParams											m_ConeParams;
	CAkConversionTable<AkRTPCGraphPoint, AkReal32>		m_curves[ 4 ];
	AkUInt8												m_curveToUse[ 4 ];
	AkUInt8												m_bIsConeEnabled : 1;
	AkArray<RTPCSubs, const RTPCSubs &, ArrayPoolDefault, 2>	m_rtpcsubs;

protected:
	explicit CAkAttenuation( AkUniqueID in_ulID );
	void UnsetRTPC( AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID );
};

class CAkFxBase : public CAkIndexable
{
public:
	/** DISHONORED(IH4): the PDB order (RTPCID, RTPCCurveID, ParamID, ConversionTable). */
	struct RTPCSubs
	{
		AkRtpcID										RTPCID;
		AkUniqueID										RTPCCurveID;
		AkUInt32										ParamID;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	ConversionTable;
	};

	virtual ~CAkFxBase();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT SetFX( AkPluginID in_FXID, void * in_pParamBlock, AkUInt32 in_uParamBlockSize );
	void SetFX( AkPluginID in_FXID, AK::IAkPluginParam * in_pParam );
	void SubscribeRTPC( AK::IAkRTPCSubscriber * in_pSubscriber, CAkRegisteredObj * in_pGameObj );
	AkUniqueID GetMediaID( AkUInt32 in_uIdx );

	AkPluginID											m_FXID;
	AK::IAkPluginParam *								m_pParam;
	CAkKeyArray<AkUInt32, AkUniqueID, 8>				m_media;
	AkArray<RTPCSubs, const RTPCSubs &, ArrayPoolDefault, 2>	m_rtpcsubs;

protected:
	explicit CAkFxBase( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_FXID( AK_INVALID_PLUGINID ), m_pParam( NULL ) {}
	void UnsetRTPC( AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID );
	virtual CAkIndexItemBase & Index() = 0;
};

/** 8 bytes: the 2012 PDB's AkFXDesc { CAkSmartPtr<CAkFxBase> pFx; bool bIsBypassed; }. DISHONORED(IH4): the smart
    pointer's behaviour (release on reassignment and destruction) is written out on the struct. */
struct AkFXDesc
{
	AkFXDesc() : pFx( NULL ), bIsBypassed( false ) {}
	~AkFXDesc() { SetFx( NULL ); }
	void SetFx( CAkFxBase * in_pFx )
	{
		CAkFxBase * pOld = pFx;
		pFx = in_pFx;
		if( pOld )
		{
			pOld->Release();
		}
	}

	CAkFxBase *	pFx;
	bool		bIsBypassed;

private:
	AkFXDesc( const AkFXDesc & );
	AkFXDesc & operator=( const AkFXDesc & );
};

CAkFxBase * AkGetFxAndAddRef( AkUniqueID in_id, bool in_bShareSet );

class CAkFxShareSet : public CAkFxBase
{
public:
	static CAkFxShareSet * Create( AkUniqueID in_ulID );
	virtual AkObjectCategory Category() { return ObjCategory_FxShareSet; }

	AkSpeakerVolumes	m_sVolumes;
	bool				m_bBypassed;

protected:
	explicit CAkFxShareSet( AkUniqueID in_ulID );
	virtual CAkIndexItemBase & Index();
};

class CAkFxCustom : public CAkFxBase
{
public:
	static CAkFxCustom * Create( AkUniqueID in_ulID );
	virtual AkObjectCategory Category() { return ObjCategory_FxCustom; }

protected:
	explicit CAkFxCustom( AkUniqueID in_ulID ) : CAkFxBase( in_ulID ) {}
	virtual CAkIndexItemBase & Index();
};

class CAkAction : public CAkIndexable
{
public:
	static CAkAction * Create( AkUInt32 in_eActionType, AkUniqueID in_ulID );
	virtual ~CAkAction();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Action; }
	virtual void SetElementID( AkUniqueID in_ulElementID ) { m_ulElementID = in_ulElementID; }
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize ) { return AK_Success; }
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize ) { return AK_Success; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	void AddToIndex();

	/** DISHONORED(port, IH2): retail's base Execute is pure; the action types IH2 does not run (the SetValue family,
	    states, switches, triggers, bypass, seek, use-state - IH3/IH4) keep this default, which -akdiag logs. */
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	virtual AKRESULT SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax );
	virtual AKRESULT SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax );
	AkTimeMs GetTransitionTime();
	AkInt32 GetDelayTime();

	CAkAction *										pNextLightItem;
	AkPropBundle<AkPropValue>						m_props;
	AkPropBundle< RANGED_MODIFIERS<AkPropValue> >	m_ranges;
	AkUniqueID										m_ulElementID;
	AkUInt16										m_eActionType;
	AkUInt8											m_eFadeCurve : 5;
	AkUInt8											m_bWasLoadedFromBank : 1;

protected:
	CAkAction( AkUInt32 in_eActionType, AkUniqueID in_ulID );
};

class CAkActionPlay : public CAkAction
{
public:
	CAkActionPlay( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ), m_fileID( AK_INVALID_FILE_ID ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	AkFileID m_fileID;
};

class CAkActionExcept : public CAkAction
{
public:
	CAkActionExcept( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual ~CAkActionExcept() { m_listElementException.Term(); }
	AKRESULT SetExceptParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT AddException( AkUniqueID in_ulElementID );
	virtual void RemoveException( AkUniqueID in_ulElementID );
	virtual void ClearExceptions() { m_listElementException.m_pItemsEnd = m_listElementException.m_pItems; }

	AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> m_listElementException;
};

/** CAkActionActive::SetActionParams 0x95fdb0 is also the slot of CAkActionSetValue and its subclasses. */
class CAkActionActive : public CAkActionExcept
{
public:
	CAkActionActive( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ), m_bIsMasterResume( false ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	bool m_bIsMasterResume;

protected:
	AKRESULT Exec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
	void AllExec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
	void AllExecExcept( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
};

class CAkActionStop : public CAkActionActive
{
public:
	CAkActionStop( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionPause : public CAkActionActive
{
public:
	CAkActionPause( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ), m_bPausePendingResume( false ) {}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	bool m_bPausePendingResume;
};

class CAkActionResume : public CAkActionActive
{
public:
	CAkActionResume( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ) {}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionSetValue : public CAkActionExcept
{
public:
	CAkActionSetValue( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );

protected:
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode ) = 0;
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj ) = 0;
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode ) = 0;
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj ) = 0;
	virtual void ExecResetValueAll( CAkParameterNodeBase * in_pNode ) = 0;
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode ) = 0;
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj ) = 0;

private:
	void ResetAllHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID );
	void ResetAEHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID );
};

class CAkActionMute : public CAkActionSetValue
{
public:
	CAkActionMute( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionSetValue( in_eActionType, in_ulID ) {}

protected:
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValueAll( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
};

class CAkActionSetAkProp : public CAkActionSetValue
{
public:
	CAkActionSetAkProp( AkUInt32 in_eActionType, AkUniqueID in_ulID, AkPropID in_ePropID ) : CAkActionSetValue( in_eActionType, in_ulID ), m_eValueMeaning( 2 ), m_ePropID( (AkUInt8)in_ePropID )
	{
		m_TargetValue.m_base = 0.f;
		m_TargetValue.m_mod.m_min = 0.f;
		m_TargetValue.m_mod.m_max = 0.f;
	}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

	RANGED_PARAMETER<AkReal32>	m_TargetValue;
	AkUInt8						m_eValueMeaning;
	AkUInt8						m_ePropID;

protected:
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValueAll( CAkParameterNodeBase * in_pNode ) { ExecResetValue( in_pNode ); }
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
};

class CAkActionSetGameParameter : public CAkActionSetValue
{
public:
	CAkActionSetGameParameter( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionSetValue( in_eActionType, in_ulID ), m_eValueMeaning( 2 )
	{
		m_TargetValue.m_base = 0.f;
		m_TargetValue.m_mod.m_min = 0.f;
		m_TargetValue.m_mod.m_max = 0.f;
	}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

	RANGED_PARAMETER<AkReal32>	m_TargetValue;
	AkUInt8						m_eValueMeaning;

protected:
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode );
	virtual void ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj );
	virtual void ExecResetValueAll( CAkParameterNodeBase * in_pNode ) { ExecResetValue( in_pNode ); }
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode ) {}
	virtual void ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj ) {}
};

/** CAkActionSetSwitch::SetActionParams 0x95d120 is also CAkActionSetState's slot (two ids). */
class CAkActionSetSwitch : public CAkAction
{
public:
	CAkActionSetSwitch( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ), m_ulSwitchGroupID( 0 ), m_ulSwitchStateID( 0 ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	AkUInt32 m_ulSwitchGroupID;
	AkUInt32 m_ulSwitchStateID;
};

class CAkActionSetState : public CAkActionSetSwitch
{
public:
	CAkActionSetState( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionSetSwitch( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionUseState : public CAkAction
{
public:
	CAkActionUseState( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

/** CAkActionDuck (akactionduck.cpp): created by a bus whose last voice ended, to release its ducks after the recovery time. */
class CAkActionDuck : public CAkAction
{
public:
	CAkActionDuck( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionBreak : public CAkAction
{
public:
	CAkActionBreak( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionTrigger : public CAkAction
{
public:
	CAkActionTrigger( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
};

class CAkActionBypassFX : public CAkActionExcept
{
public:
	CAkActionBypassFX( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ), m_bIsBypass( false ), m_uTargetMask( 0 ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	bool		m_bIsBypass;
	AkUInt32	m_uTargetMask;

private:
	void ResetBypassFxAllHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID );
	void ResetBypassFXAEHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID );
};

class CAkActionSeek : public CAkActionExcept
{
public:
	CAkActionSeek( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ), m_bIsSeekRelativeToDuration( false ), m_bSnapToNearestMarker( false )
	{
		m_position.m_base = 0.f;
		m_position.m_mod.m_min = 0.f;
		m_position.m_mod.m_max = 0.f;
	}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	virtual AKRESULT Execute( AkPendingAction * in_pAction );
	RANGED_PARAMETER<AkReal32>	m_position;
	bool						m_bIsSeekRelativeToDuration;
	bool						m_bSnapToNearestMarker;

protected:
	AKRESULT Exec( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
	void AllExec( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
	void AllExecExcept( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID );
};

class CAkEvent : public CAkIndexable
{
public:
	static CAkEvent * CreateNoIndex( AkUniqueID in_ulID );
	virtual ~CAkEvent() { Clear(); }
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Event; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	void AddToIndex();
	void Clear();
	AKRESULT QueryAudioObjectIDs( AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos );

	AkListBareLight<CAkAction>	m_actions;
	AkUInt32					m_iPreparationCount;

protected:
	explicit CAkEvent( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_iPreparationCount( 0 ) {}
};
