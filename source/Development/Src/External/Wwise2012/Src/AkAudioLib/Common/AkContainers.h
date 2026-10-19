/*=============================================================================
	AkContainers.h - the containers of the node hierarchy (random / sequence, switch, layer and their multi-play base)
	with their playlists, per-game-object selection state and the continuation lists continuous playback walks.

	DISHONORED(port): agent IH1 (wave 26) declared these classes' data and loading half in AkHierarchy.h; agent IH3,
	fork C, moved them here (they now hold PBI-side types by value: PlayHistory, UserParams, ContParams) and added the
	playback half. Layouts are the 2012 PDB's: CAkRanSeqCntr gains m_mapObjectCntrInfo / m_pGlobalContainerInfo,
	CAkMultiPlayNode m_listContParameters, CAkSwitchCntr its CAkSwitchAware / CAkPreparationAware bases and
	m_listSwitchContPlayback; CAkPlayList is the PDB's 11-slot interface (IH1's members moved to the subclasses:
	Random holds {id, weight} x grow 8, Sequence ids x grow 16). CAkSmartPtr<T> members are raw pointers with the
	same AddRef / Release discipline (same 4 bytes).
	Include this header (not AkHierarchy.h) where a container's members are used.
	Deviation (named): CAkContinueListItem has a copy constructor (default state + retail's operator=) because this
	port's AkArray::GrowArray copy-constructs; retail's grows by default construction + operator=, same result.
=============================================================================*/
#pragma once

#include "AkPBI.h"
#include "AkSwitchAware.h"
#include "AkStateMgr.h"

class CAkContinuousPBI;
class CAkContinuationList;
class CAkActionPlayAndContinue;

enum AkTransitionMode
{
	Transition_Disabled			= 0,
	Transition_CrossFadeAmp		= 1,
	Transition_CrossFadePower	= 2,
	Transition_Delay			= 3,
	Transition_SampleAccurate	= 4,
	Transition_TriggerRate		= 5
};

enum AkRandomMode
{
	RandomMode_Normal	= 0,
	RandomMode_Shuffle	= 1
};

/** AkLoop (2012 PDB, 4 bytes): the loop count of a continuous pass through one container. */
struct AkLoop
{
	AkInt16	lLoopCount;
	AkUInt8	bIsEnabled : 1;
	AkUInt8	bIsInfinite : 1;
};

class CAkContainerBase : public CAkParentNode<CAkParameterNode>
{
public:
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );

protected:
	explicit CAkContainerBase( AkUniqueID in_ulID ) : CAkParentNode<CAkParameterNode>( in_ulID ) {}
};

/** CAkPlayList (akplaylist.h): the playlist interface, no data. */
class CAkPlayList
{
public:
	virtual ~CAkPlayList() {}
	virtual AKRESULT Init() { return AK_Success; }
	virtual void Destroy() = 0;
	virtual AkUInt32 Length() = 0;
	virtual AkUniqueID ID( AkUInt16 in_wPosition ) = 0;
	virtual AKRESULT Add( AkUniqueID in_ulID, AkUInt32 in_weight ) = 0;
	virtual void Remove( AkUniqueID in_ID ) = 0;
	virtual void RemoveAll() = 0;
	virtual bool Exists( AkUniqueID in_ID ) = 0;
	virtual bool GetPosition( AkUniqueID in_ID, AkUInt16 & out_rwPosition ) = 0;
	virtual AkUInt32 GetWeight( AkUInt16 in_wPosition ) = 0;
};

struct AkPlaylistItem
{
	AkUniqueID	ulID;
	AkUInt32	weight;
};

class CAkPlayListRandom : public CAkPlayList
{
public:
	virtual ~CAkPlayListRandom() { m_PlayList.Term(); }
	virtual void Destroy();
	virtual AkUInt32 Length();
	virtual AkUniqueID ID( AkUInt16 in_wPosition );
	virtual AKRESULT Add( AkUniqueID in_ulID, AkUInt32 in_weight );
	virtual void Remove( AkUniqueID in_ID );
	virtual void RemoveAll();
	virtual bool Exists( AkUniqueID in_ID );
	virtual bool GetPosition( AkUniqueID in_ID, AkUInt16 & out_rwPosition );
	virtual AkUInt32 GetWeight( AkUInt16 in_wPosition );
	virtual void SetWeight( AkUInt16 in_wPosition, AkUInt32 in_weight );
	virtual AkUInt32 CalculateTotalWeight();

	AkArray<AkPlaylistItem, const AkPlaylistItem &, ArrayPoolDefault, 8> m_PlayList;
};

class CAkPlayListSequence : public CAkPlayList
{
public:
	virtual ~CAkPlayListSequence() { m_PlayList.Term(); }
	virtual void Destroy();
	virtual AkUInt32 Length();
	virtual AkUniqueID ID( AkUInt16 in_wPosition );
	virtual AKRESULT Add( AkUniqueID in_ulID, AkUInt32 in_weight );
	virtual void Remove( AkUniqueID in_ID );
	virtual void RemoveAll();
	virtual bool Exists( AkUniqueID in_ID );
	virtual bool GetPosition( AkUniqueID in_ID, AkUInt16 & out_rwPosition );
	virtual AkUInt32 GetWeight( AkUInt16 in_wPosition );

	AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 16> m_PlayList;
};

/** CAkContainerBaseInfo (akranseqbaseinfo.h): a container's selection state for one game object (or global). */
class CAkContainerBaseInfo
{
public:
	CAkContainerBaseInfo() : key( NULL ), pNextItem( NULL ) {}
	virtual ~CAkContainerBaseInfo() {}
	virtual void Destroy() = 0;
	virtual AkContainerMode Type() = 0;
	virtual CAkContainerBaseInfo * Clone( AkUInt16 in_wItemCount ) = 0;

	CAkRegisteredObj *		key;
	CAkContainerBaseInfo *	pNextItem;
};

/** CAkRandomInfo (0x2C): played / blocked bit flags and the avoid-repeat list. */
class CAkRandomInfo : public CAkContainerBaseInfo
{
public:
	explicit CAkRandomInfo( AkUInt16 in_wItemCount );
	virtual ~CAkRandomInfo();
	virtual void Destroy();
	virtual AkContainerMode Type() { return ContainerMode_Random; }
	virtual CAkContainerBaseInfo * Clone( AkUInt16 in_wItemCount );

	AKRESULT Init( AkUInt16 in_wAvoidRepeatCount );
	void FlagSetPlayed( AkUInt16 in_wPosition );
	bool IsFlagSetPlayed( AkUInt16 in_wPosition ) const;
	void ResetFlagsPlayed( AkUInt32 in_PlaylistSize );
	void FlagAsBlocked( AkUInt16 in_wPosition );
	void FlagAsUnBlocked( AkUInt16 in_wPosition );
	bool IsFlagBlocked( AkUInt16 in_wPosition ) const;
	AkInt32 GetRandomValue();

	AkUInt32										m_ulTotalWeight;
	AkUInt32										m_ulRemainingWeight;
	AkUInt16										m_wRemainingItemsToPlay;
	AkUInt16										m_wCounter;
	AkArray<AkUInt16, AkUInt16, ArrayPoolDefault, 1>	m_listAvoid;
	char *											m_pcArrayBeenPlayedFlag;
	char *											m_pcArrayBlockedFlag;
};

/** CAkSequenceInfo (0x10). */
class CAkSequenceInfo : public CAkContainerBaseInfo
{
public:
	CAkSequenceInfo() : m_bIsForward( true ), m_i16LastPositionChosen( -1 ) {}
	virtual void Destroy();
	virtual AkContainerMode Type() { return ContainerMode_Sequence; }
	virtual CAkContainerBaseInfo * Clone( AkUInt16 in_wItemCount );

	bool		m_bIsForward;
	AkInt16		m_i16LastPositionChosen;
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
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual CAkPBI * CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const;
	virtual AKRESULT SetParamComplexFromRTPCManager( void * in_pToken, AkUInt32 in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray );
	virtual void Unregister( CAkRegisteredObj * in_pGameObj );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT Mode( AkContainerMode in_eMode );
	AKRESULT SetPlaylistWithoutCheck( AkUInt8 *& io_pData, AkUInt32 & io_ulDataSize );

	CAkParameterNodeBase * GetNextToPlayContinuous( CAkRegisteredObj * in_pGameObj, AkUInt16 & out_rwPositionSelected, AkUniqueID & out_uSelectedNodeID, CAkContainerBaseInfo *& io_pContainerInfo, AkLoop & io_rLoopInfo );
	AkTransitionMode TransitionMode();
	AkReal32 TransitionTime( CAkRegisteredObj * in_GameObj );
	AkUInt32 GetPlaylistLength();
	void ResetSpecificInfo();

	CAkPlayList *	m_pPlayList;
	AkHashListBare<CAkRegisteredObj *, CAkContainerBaseInfo, 31>	m_mapObjectCntrInfo;
	CAkContainerBaseInfo *	m_pGlobalContainerInfo;
	CoreRanSeqCntr	m_RanSeqParams;
	AkInt16			m_Loop;
	bool			m_bContainerBeenPlayed;

protected:
	CAkRanSeqCntr( AkUniqueID in_ulID, AkContainerMode in_ContainerMode );
	AKRESULT Init();
	CAkPlayList * NewPlayList();
	CAkParameterNodeBase * GetNextToPlay( CAkRegisteredObj * in_pGameObj, AkUInt16 & out_rwPositionSelected, AkUniqueID & out_uSelectedNodeID );
	CAkSequenceInfo * CreateSequenceInfo();
	CAkRandomInfo * CreateRandomInfo( AkUInt16 in_uPlaylistSize );
	CAkSequenceInfo * GetExistingSequenceInfo( CAkRegisteredObj * in_pGameObj );
	CAkRandomInfo * GetExistingRandomInfo( AkUInt16 in_uPlaylistSize, CAkRegisteredObj * in_pGameObj );

private:
	AKRESULT _Play( AkPBIParams & in_rPBIParams );
	AKRESULT _PlayContinuous( AkPBIParams & in_rPBIParams );
	AkUInt16 SelectRandomly( CAkRandomInfo * in_pRandomInfo, bool & out_bIsAnswerValid, AkLoop * io_pLoopCount );
	AkUInt16 SelectSequentially( CAkSequenceInfo * in_pSeqInfo, bool & out_bIsAnswerValid, AkLoop * io_pLoopCount );
	bool CanPlayPosition( const CAkRandomInfo * in_pRandomInfo, AkUInt16 in_wPosition ) const;
	bool CanContinueAfterCompleteLoop( AkLoop * io_pLoopingInfo );
	AKRESULT UpdateNormalAvoidRepeat( CAkRandomInfo * in_pRandomInfo, AkUInt16 in_uPosition );
	void UpdateResetPlayListSetup( CAkSequenceInfo * in_pSeqInfo, CAkRegisteredObj * in_pGameObj );
	void DestroySpecificInfo();
};

class CAkMultiPlayNode;

/** CAkContinueListItem (0x18): one container of a continuous pass, or a multi-play node pointing back at the list it
    branched from. */
class CAkContinueListItem
{
public:
	CAkContinueListItem();
	CAkContinueListItem( const CAkContinueListItem & in_listItem );
	~CAkContinueListItem();
	const CAkContinueListItem & operator=( const CAkContinueListItem & in_listItem );

	CAkRanSeqCntr *			m_pContainer;
	CAkContainerBaseInfo *	m_pContainerInfo;
	AkLoop					m_LoopingInfo;
	CAkMultiPlayNode *		m_pMultiPlayNode;
	CAkContinuationList *	m_pAlternateContList;
};

/** CAkContinuationList (0x10, refcounted): the stack of containers a continuous pass is inside. */
class CAkContinuationList
{
public:
	static CAkContinuationList * Create();
	void AddRef();
	void Release();
	void RemoveLast();

	AkArray<CAkContinueListItem, const CAkContinueListItem &, ArrayPoolDefault, 2>	m_listItems;
	AkInt32																		m_iRefCount;

private:
	CAkContinuationList() : m_iRefCount( 1 ) {}
	void Term();
};

/** AkContParamsAndPath (akmultiplaynode.h): a child's own continuous params, holding a path user for its lifetime. */
class AkContParamsAndPath
{
public:
	explicit AkContParamsAndPath( ContParams * in_pFrom );
	~AkContParamsAndPath();
	ContParams & Get() { return m_continuousParams; }

	ContParams m_continuousParams;
};

/** SafeContinuationList (akmultiplaynode.h). */
class SafeContinuationList
{
public:
	SafeContinuationList( AkPBIParams & in_rPBIParams, CAkMultiPlayNode * in_pMultiPlayNode );
	~SafeContinuationList();
	CAkContinuationList * Get() { return m_spBackupContinuationList; }

	CAkContinuationList * m_spBackupContinuationList;
};

class CAkMultiPlayNode : public CAkContainerBase
{
public:
	struct ContParam
	{
		CAkContinuationList *	pContList;
		AkUInt32				uRefCount;
	};

	struct ContParamGetKey
	{
		static CAkContinuationList * Get( const ContParam & in_item ) { return in_item.pContList; }
	};

	void ContGetList( CAkContinuationList * in_pList, CAkContinuationList *& io_spList );
	AKRESULT ContRefList( CAkContinuationList * in_pList );
	AKRESULT ContUnrefList( CAkContinuationList * in_pList );

	AkSortedKeyArray<CAkContinuationList *, ContParam, ArrayPoolDefault, ContParamGetKey, 1> m_listContParameters;

protected:
	explicit CAkMultiPlayNode( AkUniqueID in_ulID );
	virtual ~CAkMultiPlayNode();
	virtual bool IsContinuousPlayback();
	AKRESULT Init();
	void Term();
	AKRESULT AddMultiplayItem( AkContParamsAndPath & in_rContParams, AkPBIParams & in_rParams, SafeContinuationList & in_rSafeContList );
	AKRESULT PlayAndContinueAlternateMultiPlay( AkPBIParams & in_rPBIParams );
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

class CAkSwitchCntr : public CAkMultiPlayNode, public CAkSwitchAware, public CAkPreparationAware
{
public:
	/** One remembered play of a continuous switch container (a switch change restarts it). */
	struct SwitchContPlaybackItem
	{
		UserParams			UserParameters;
		CAkRegisteredObj *	GameObject;
		PlayHistory			PlayHist;
		AkPlaybackState		ePlaybackState;
	};

	typedef CAkKeyList<AkUInt32, CAkSwitchPackage, 2, ArrayPoolDefault> SwitchList;
	typedef CAkList2<SwitchContPlaybackItem, const SwitchContPlaybackItem &, 2, ArrayPoolDefault> SwitchContPlaybackList;

	static CAkSwitchCntr * Create( AkUniqueID in_ulID );
	virtual ~CAkSwitchCntr();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_SwitchCntr; }
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual void RemoveChild( CAkParameterNodeBase * in_pChild );
	virtual void OnPreRelease();
	virtual AKRESULT PrepareData();
	virtual void UnPrepareData();
	virtual void SetSwitch( AkUInt32 in_Switch, CAkRegisteredObj * in_GameObj );
	virtual AKRESULT ModifyActiveState( AkUInt32 in_stateID, bool in_bSupported );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );
	AKRESULT AddNodeInSwitch( CAkSwitchPackage * in_pSwitchPack, AkUniqueID in_NodeID );
	void ClearSwitches();

	AkGroupType														m_eGroupType;
	AkUInt32														m_ulGroupID;
	AkUInt32														m_ulDefaultSwitch;
	SwitchList														m_SwitchList;
	CAkKeyList<AkUniqueID, AkSwitchNodeParams, 2, ArrayPoolDefault>	m_listParameters;
	SwitchContPlaybackList											m_listSwitchContPlayback;

protected:
	explicit CAkSwitchCntr( AkUniqueID in_ulID );
	AKRESULT Init();
	virtual bool IsContinuousPlayback();

private:
	void ResumeContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID );
	void PauseContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID );
	void StopContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID );
	void CleanSwitchContPlayback();
	AKRESULT PrepareNodeList( const AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 1> & in_rNodeList );
	void UnPrepareNodeList( const AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 1> & in_rNodeList );
	void GetAllParams( AkUniqueID in_NodeID, AkSwitchNodeParams & out_rParams );
	AKRESULT PlayOnSwitch( AkUniqueID in_ID, SwitchContPlaybackItem & in_rContItem );
	AKRESULT StopOnSwitch( AkUniqueID in_ID, AkSwitchNodeParams & in_rSwitchNodeParams, CAkRegisteredObj * in_GameObj );
	AKRESULT StopPrevious( CAkSwitchPackage * in_pPreviousSwitchPack, CAkSwitchPackage * in_pNextSwitchPack, CAkRegisteredObj * in_GameObj );
	AKRESULT PerformSwitchChangeContPerObject( AkSwitchStateID in_SwitchTo, CAkRegisteredObj * in_GameObj );
	AKRESULT PerformSwitchChange( AkSwitchStateID in_SwitchTo, CAkRegisteredObj * in_GameObj );
};

class CAkLayer : public CAkIndexable
{
public:
	struct CAssociatedChildData
	{
		CAssociatedChildData() : m_ulChildID( 0 ), m_pChild( NULL ) {}
		AKRESULT UpdateChildPtr( CAkLayer * in_pLayer );
		AKRESULT Term( CAkLayer * in_pLayer );

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
	AKRESULT UnsetChildAssoc( AkUniqueID in_ChildID );
	void UpdateChildPtr( AkUniqueID in_ChildID );
	void RecalcNotification();
	bool IsPlaying() const;
	void OnRTPCChanged( CAkRegisteredObj * in_GameObjPtr, AkReal32 in_fValue );
	AKRESULT SetParamComplexFromRTPCManager( void * in_pToken, AkRTPC_ParameterID in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray );
	AKRESULT GetAudioParameters( CAkParameterNode * in_pAssociatedChild, AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr );

	CAkKeyArray<AkUniqueID, CAssociatedChildData, 2>	m_assocs;
	CAkBitArrayMax32									m_RTPCBitArrayMax32;
	CAkLayerCntr *										m_pOwner;
	AkRtpcID											m_crossfadingRTPCID;

protected:
	explicit CAkLayer( AkUniqueID in_ulID ) : CAkIndexable( in_ulID ), m_pOwner( NULL ), m_crossfadingRTPCID( 0 ) {}
	AKRESULT SetInitialRTPC( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize );

private:
	void ParamNotification( NotifParams & in_rParams );
};

class CAkLayerCntr : public CAkMultiPlayNode
{
public:
	static CAkLayerCntr * Create( AkUniqueID in_ulID );
	virtual ~CAkLayerCntr();
	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_LayerCntr; }
	virtual AKRESULT AddChild( AkUniqueID in_ulID );
	virtual void RemoveChild( CAkParameterNodeBase * in_pChild );
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

	AkArray<CAkLayer *, CAkLayer *, ArrayPoolDefault, 8> m_layers;

protected:
	explicit CAkLayerCntr( AkUniqueID in_ulID ) : CAkMultiPlayNode( in_ulID ) {}
};
