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

class CAkUsageSlot;
class CAkLayerCntr;
class CAkParameterNodeBase;

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
	AkActionType_PlayAndContinue	= 0x0503
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

enum AkPathMode
{
	AkStepSequence	= 0
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
	void *				m_pAttenuation;
};

class CAkGen3DParams
{
public:
	CAkGen3DParams();
	virtual ~CAkGen3DParams() { Term(); }

	AKRESULT SetPath( AkPathVertex * in_pArrayVertex, AkUInt32 in_ulNumVertices, AkPathListItemOffset * in_pArrayPlaylist, AkUInt32 in_ulNumPlaylistItem );
	void Term();

	Gen3DParams m_Params;

private:
	void UpdateTransitionTimeInVertex();
};

class CAkGen3DParamsEx : public CAkGen3DParams
{
public:
	struct PathState
	{
		AkUInt32	ulCurrentListIndex;
		bool *		pbPlayed;
	};

	CAkGen3DParamsEx() { m_PathState.ulCurrentListIndex = 0; m_PathState.pbPlayed = NULL; }
	virtual ~CAkGen3DParamsEx() { FreePathInfo(); }
	void FreePathInfo();

	PathState m_PathState;
};

/** One (state group, node) link: the node's state values per state id of the group. */
struct AkStateLink
{
	class CAkState *	pState;
	AkUniqueID			ulStateID;
};

class AkStateGroupChunk
{
public:
	AkStateGroupChunk( CAkParameterNodeBase * in_pOwner, AkStateGroupID in_ulStateGroup );
	virtual ~AkStateGroupChunk();

	AKRESULT AddState( AkUniqueID in_ulStateInstanceID, AkStateID in_ulStateID, bool in_bNotify );

	AkStateGroupChunk *						m_pNextInGroup;
	AkStateGroupChunk *						m_pNextInNode;
	CAkParameterNodeBase *					m_pOwner;
	AkPropBundle<AkReal32>					m_values;
	CAkKeyArray<AkStateID, AkStateLink, 5>	m_mapStates;
	const AkStateGroupID					m_ulStateGroup;
	AkStateID								m_ulActualState;
	AkUInt8									m_eStateSyncType;
};

struct AkListBareLightNextInNode
{
	static AkStateGroupChunk *& Get( AkStateGroupChunk * in_pItem ) { return in_pItem->m_pNextInNode; }
};

class CAkParameterNodeBase : public CAkIndexable
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
	virtual void NotifyStateParametersModified() {}
	virtual void NotifyRTPCChanged() {}
	virtual void UpdateFx( AkUInt32 in_uFXIndex ) {}
	virtual CAkRTPCMgr::SubscriberType GetRTPCSubscriberType() const { return CAkRTPCMgr::SubscriberType_CAkParameterNodeBase; }

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

	void SetMaxReachedBehavior( bool in_bKillNewest ) { m_bKillNewest = in_bKillNewest; }
	void SetOverLimitBehavior( bool in_bUseVirtualBehavior ) { m_bUseVirtualBehavior = in_bUseVirtualBehavior; }
	void SetMaxNumInstOverrideParent( bool in_bOverride ) { m_bIsMaxNumInstOverrideParent = in_bOverride; }
	void SetVVoicesOptOverrideParent( bool in_bOverride ) { m_bIsVVoicesOptOverrideParent = in_bOverride; }

	AkListBareLight<AkStateGroupChunk, AkListBareLightNextInNode>	m_states;
	void *						m_pGlobalSIS;
	FXChunk *					m_pFXChunk;
	void *						m_pActivityChunk;
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

protected:
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

	void *											m_pMapSIS;
	CAkGen3DParamsEx *								m_p3DParameters;
	AkPropBundle< RANGED_MODIFIERS<AkPropValue> >	m_ranges;
	AkUInt16										m_eVirtualQueueBehavior;
	AkUInt16										m_eBelowThresholdBehavior;
	AkArray<class CAkLayer *, class CAkLayer *, ArrayPoolDefault, 8> *	m_pAssociatedLayers;

protected:
	AKRESULT Enable3DPosParams();
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

	AkMapChildID m_mapChildId;
};

class CAkSoundBase : public CAkParameterNode
{
public:
	explicit CAkSoundBase( AkUniqueID in_ulID ) : CAkParameterNode( in_ulID ) {}
};

class CAkSound : public CAkSoundBase
{
public:
	static CAkSound * Create( AkUniqueID in_ulID );
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_Sound; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize, CAkUsageSlot * in_pUsageSlot, bool in_bIsPartialLoadOnly );
	bool HasBankSource() const { return m_Source.m_sSrcTypeInfo.mediaInfo.Type != 0 && m_Source.m_sSrcTypeInfo.mediaInfo.bHasSource; }

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

class CAkContainerBase : public CAkParentNode<CAkParameterNode>
{
public:
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );

protected:
	explicit CAkContainerBase( AkUniqueID in_ulID ) : CAkParentNode<CAkParameterNode>( in_ulID ) {}
};

/** CAkPlayList: the random / sequence playlist of a CAkRanSeqCntr (akplaylist.cpp). */
class CAkPlayList
{
public:
	struct AkPlaylistItem
	{
		AkUniqueID	ulID;
		AkUInt32	weight;
	};

	virtual ~CAkPlayList() { m_PlayList.Term(); }
	virtual bool Exists( AkUniqueID in_ulID );
	virtual AKRESULT Add( AkUniqueID in_ulID, AkUInt32 in_weight );

	AkArray<AkPlaylistItem, const AkPlaylistItem &, ArrayPoolDefault, 1> m_PlayList;
};

class CAkPlayListRandom : public CAkPlayList
{
public:
	CAkPlayListRandom() : m_ulTotalWeight( 0 ) {}
	virtual AKRESULT Add( AkUniqueID in_ulID, AkUInt32 in_weight );
	AkUInt32 m_ulTotalWeight;
};

class CAkPlayListSequence : public CAkPlayList
{
};

class CAkRanSeqCntr : public CAkContainerBase
{
public:
	/** CoreRanSeqCntr: 16 bytes, the PDB's bitfields at +14 / +15. */
	struct CoreRanSeqCntr
	{
		RANGED_PARAMETER<AkReal32>	m_TransitionTime;
		AkUInt16					m_wAvoidRepeatCount;
		AkUInt8						m_eTransitionMode : 4;
		AkUInt8						m_eRandomMode : 2;
		AkUInt8						m_eMode : 3;
		AkUInt8						m_bIsUsingWeight : 1;
		AkUInt8						m_bResetPlayListAtEachPlay : 1;
		AkUInt8						m_bIsRestartBackward : 1;
		AkUInt8						m_bIsContinuous : 1;
		AkUInt8						m_bIsGlobal : 1;
	};

	static CAkRanSeqCntr * Create( AkUniqueID in_ulID, AkContainerMode in_ContainerMode );
	virtual ~CAkRanSeqCntr();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_RanSeqCntr; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT Mode( AkContainerMode in_eMode );
	AKRESULT SetPlaylistWithoutCheck( AkUInt8 *& io_pData, AkUInt32 & io_ulDataSize );

	CAkPlayList *	m_pPlayList;
	CoreRanSeqCntr	m_RanSeqParams;
	AkInt16			m_Loop;
	bool			m_bContainerBeenPlayed;

protected:
	CAkRanSeqCntr( AkUniqueID in_ulID, AkContainerMode in_ContainerMode );
	AKRESULT Init();
	CAkPlayList * NewPlayList();
};

class CAkMultiPlayNode : public CAkContainerBase
{
protected:
	explicit CAkMultiPlayNode( AkUniqueID in_ulID ) : CAkContainerBase( in_ulID ) {}
};

struct AkSwitchNodeParams
{
	AkTimeMs	FadeOutTime;
	AkTimeMs	FadeInTime;
	AkUInt8		eOnSwitchMode : 3;
	AkUInt8		bIsFirstOnly : 1;
	AkUInt8		bContinuePlayback : 1;
};

class CAkSwitchPackage
{
public:
	~CAkSwitchPackage() { m_list.Term(); }
	AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 1> m_list;
};

class CAkSwitchCntr : public CAkMultiPlayNode
{
public:
	static CAkSwitchCntr * Create( AkUniqueID in_ulID );
	virtual ~CAkSwitchCntr();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_SwitchCntr; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT AddNodeInSwitch( CAkSwitchPackage * in_pSwitchPack, AkUniqueID in_NodeID );

	AkGroupType														m_eGroupType;
	AkUInt32														m_ulGroupID;
	AkUInt32														m_ulDefaultSwitch;
	CAkKeyList<AkUInt32, CAkSwitchPackage, 2, ArrayPoolDefault>		m_SwitchList;
	CAkKeyList<AkUniqueID, AkSwitchNodeParams, 2, ArrayPoolDefault>	m_listParameters;

protected:
	explicit CAkSwitchCntr( AkUniqueID in_ulID );
	AKRESULT Init();
};

class CAkLayer : public CAkIndexable
{
public:
	struct CAssociatedChildData
	{
		AkUniqueID										m_ulChildID;
		CAkParameterNode *								m_pChild;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	m_fadeCurve;
	};

	static CAkLayer * Create( AkUniqueID in_ulID );
	virtual ~CAkLayer();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Layer; }

	void SetOwner( CAkLayerCntr * in_pOwner );
	AKRESULT SetInitialValues( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	AKRESULT SetCrossfadingRTPC( AkRtpcID in_rtpcID );
	AKRESULT SetChildAssoc( AkUniqueID in_ChildID, AkRTPCGraphPoint * in_pCrossfadingCurve, AkUInt32 in_ulCrossfadingCurveSize );
	void UpdateChildPtr( AkUniqueID in_ChildID );

	CAkKeyArray<AkUniqueID, CAssociatedChildData, 2>	m_assocs;
	CAkBitArrayMax32									m_RTPCBitArrayMax32;
	CAkLayerCntr *										m_pOwner;
	AkRtpcID											m_crossfadingRTPCID;

protected:
	explicit CAkLayer( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_pOwner( NULL ), m_crossfadingRTPCID( 0 ) {}
	AKRESULT SetInitialRTPC( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	AKRESULT UpdateAssociatedChild( CAssociatedChildData & io_rData );
};

class CAkLayerCntr : public CAkMultiPlayNode
{
public:
	static CAkLayerCntr * Create( AkUniqueID in_ulID );
	virtual ~CAkLayerCntr();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_LayerCntr; }
	virtual AKRESULT AddChild( AkUniqueID in_ulID );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

	AkArray<CAkLayer *, CAkLayer *, ArrayPoolDefault, 8> m_layers;

protected:
	explicit CAkLayerCntr( AkUniqueID in_ulID ) : CAkMultiPlayNode( in_ulID ) {}
};

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

	AkUInt32												m_RecoveryTime;
	AkUInt32												m_uChannelConfig;
	AkReal32												m_fMaxDuckVolume;
	CAkKeyList<AkUniqueID, AkDuckInfo, 1, ArrayPoolDefault>	m_ToDuckList;
	AkUInt8													m_eDuckingState : 3;
	AkUInt8													m_bIsEnvironmental : 1;

protected:
	explicit CAkBus( AkUniqueID in_ulID );
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

	enum { AttenuationCurveID_None = 0xFF };

	static CAkAttenuation * Create( AkUniqueID in_ulID );
	virtual ~CAkAttenuation();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Attenuation; }

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * GetCurve( AkUInt32 in_uIndex );

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
	struct RTPCSubs
	{
		AkRtpcID										RTPCID;
		AkUInt32										ParamID;
		AkUniqueID										RTPCCurveID;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	ConversionTable;
	};

	virtual ~CAkFxBase();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT SetFX( AkPluginID in_FXID, void * in_pParamBlock, AkUInt32 in_uParamBlockSize );

	AkPluginID											m_FXID;
	void *												m_pParam;
	CAkKeyArray<AkUInt32, AkUniqueID, 8>				m_media;
	AkArray<RTPCSubs, const RTPCSubs &, ArrayPoolDefault, 2>	m_rtpcsubs;

protected:
	explicit CAkFxBase( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_FXID( AK_INVALID_PLUGINID ), m_pParam( NULL ) {}
	void UnsetRTPC( AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID );
	virtual CAkIndexItemBase & Index() = 0;
};

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
	AkFileID m_fileID;
};

class CAkActionExcept : public CAkAction
{
public:
	CAkActionExcept( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
	virtual ~CAkActionExcept() { m_listElementException.Term(); }
	AKRESULT SetExceptParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

	AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> m_listElementException;
};

/** CAkActionActive::SetActionParams 0x95fdb0 is also the slot of CAkActionSetValue and its subclasses. */
class CAkActionActive : public CAkActionExcept
{
public:
	CAkActionActive( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ), m_bIsMasterResume( false ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	bool m_bIsMasterResume;
};

class CAkActionStop : public CAkActionActive
{
public:
	CAkActionStop( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ) {}
};

class CAkActionPause : public CAkActionActive
{
public:
	CAkActionPause( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ), m_bPausePendingResume( false ) {}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	bool m_bPausePendingResume;
};

class CAkActionResume : public CAkActionActive
{
public:
	CAkActionResume( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionActive( in_eActionType, in_ulID ) {}
	virtual AKRESULT SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
};

class CAkActionSetValue : public CAkActionExcept
{
public:
	CAkActionSetValue( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
};

class CAkActionMute : public CAkActionSetValue
{
public:
	CAkActionMute( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionSetValue( in_eActionType, in_ulID ) {}
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
};

/** CAkActionSetSwitch::SetActionParams 0x95d120 is also CAkActionSetState's slot (two ids). */
class CAkActionSetSwitch : public CAkAction
{
public:
	CAkActionSetSwitch( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ), m_ulSwitchGroupID( 0 ), m_ulSwitchStateID( 0 ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	AkUInt32 m_ulSwitchGroupID;
	AkUInt32 m_ulSwitchStateID;
};

class CAkActionSetState : public CAkActionSetSwitch
{
public:
	CAkActionSetState( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionSetSwitch( in_eActionType, in_ulID ) {}
};

class CAkActionUseState : public CAkAction
{
public:
	CAkActionUseState( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
};

class CAkActionBreak : public CAkAction
{
public:
	CAkActionBreak( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
};

class CAkActionTrigger : public CAkAction
{
public:
	CAkActionTrigger( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkAction( in_eActionType, in_ulID ) {}
};

class CAkActionBypassFX : public CAkActionExcept
{
public:
	CAkActionBypassFX( AkUInt32 in_eActionType, AkUniqueID in_ulID ) : CAkActionExcept( in_eActionType, in_ulID ), m_bIsBypass( false ), m_uTargetMask( 0 ) {}
	virtual AKRESULT SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );
	bool		m_bIsBypass;
	AkUInt32	m_uTargetMask;
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
	RANGED_PARAMETER<AkReal32>	m_position;
	bool						m_bIsSeekRelativeToDuration;
	bool						m_bSnapToNearestMarker;
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
