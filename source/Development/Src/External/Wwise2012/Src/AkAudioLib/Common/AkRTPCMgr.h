/*=============================================================================
	AkRTPCMgr.h - CAkRTPCMgr (akrtpcmgr.cpp / akrtpcmgr.h): the game parameters (RTPC) per game object, their
	transitions, the subscriptions every node / bus / layer / PBI / effect holds on them with one curve per RTPC,
	the switches per game object and the RTPC-driven switch groups.

	DISHONORED(port): agent IH3 (wave 26), fork R; replaces IH1's AkGameSyncs data store. Layouts are the 2012 PDB's
	(CAkRTPCMgr::AkRTPCEntry 0x28, AkRTPCSubscription 0x20, RTPCCurve 0x14, CAkRTPCTransition 0x18).
	AK::IAkRTPCSubscriber now comes from the public IAkPlugin.h (agent IH4 moved it there, as the SDK has it).
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"
#include "AkTransitionManager.h"
#include <AK/SoundEngine/Common/IAkPlugin.h>

class CAkRegisteredObj;
class CAkSwitchAware;
class CAkLayer;
struct TransParams;

enum AkRTPC_ParameterID
{
	RTPC_Volume									= 0,
	RTPC_LFE									= 1,
	RTPC_Pitch									= 2,
	RTPC_LPF									= 3,
	RTPC_PlayMechanismSpecialTransitionsValue	= 4,
	RTPC_Priority								= 8,
	RTPC_MaxNumInstances						= 9,
	RTPC_Positioning_Radius_LPF					= 10,
	RTPC_Positioning_Divergence_Center_PCT		= 11,
	RTPC_Positioning_Cone_Attenuation_ON_OFF	= 12,
	RTPC_Positioning_Cone_Attenuation			= 13,
	RTPC_Positioning_Cone_LPF					= 14,
	RTPC_Position_PAN_RL						= 20,
	RTPC_Position_PAN_FR						= 21,
	RTPC_BypassFX0								= 24,
	RTPC_BypassFX1								= 25,
	RTPC_BypassFX2								= 26,
	RTPC_BypassFX3								= 27,
	RTPC_BypassAllFX							= 28,
	RTPC_FeedbackVolume							= 29,
	RTPC_FeedbackLowpass						= 30,
	RTPC_FeedbackPitch							= 31,
	RTPC_MaxNumRTPC								= 32
};

typedef AkArray<CAkRegisteredObj *, CAkRegisteredObj *, ArrayPoolDefault, 16> GameObjExceptArray;

struct AkRTPCSubscriptionKey
{
	void *				pSubscriber;
	AkRTPC_ParameterID	ParamID;

	bool operator==( const AkRTPCSubscriptionKey & in_rOther ) const { return pSubscriber == in_rOther.pSubscriber && ParamID == in_rOther.ParamID; }
	bool operator!=( const AkRTPCSubscriptionKey & in_rOther ) const { return !( *this == in_rOther ); }
};

inline AkUIntPtr AkHashListHash( const AkRTPCSubscriptionKey & in_key ) { return (AkUIntPtr)in_key.pSubscriber + in_key.ParamID; }

struct AkSwitchKey
{
	AkSwitchGroupID		m_SwitchGroup;
	CAkRegisteredObj *	m_pGameObj;

	bool operator==( const AkSwitchKey & in_rOther ) const { return m_SwitchGroup == in_rOther.m_SwitchGroup && m_pGameObj == in_rOther.m_pGameObj; }
	bool operator!=( const AkSwitchKey & in_rOther ) const { return !( *this == in_rOther ); }
};

inline AkUIntPtr AkHashListHash( const AkSwitchKey & in_key ) { return (AkUIntPtr)in_key.m_pGameObj + in_key.m_SwitchGroup; }

class CAkRTPCMgr
{
public:
	enum SubscriberType
	{
		SubscriberType_IAkRTPCSubscriber	= 0,
		SubscriberType_CAkBus				= 1,
		SubscriberType_CAkParameterNodeBase	= 2,
		SubscriberType_CAkLayer				= 3,
		SubscriberType_PBI					= 4
	};

	struct RTPCCurve
	{
		AkUniqueID										RTPCCurveID;
		AkRtpcID										RTPC_ID;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	ConversionTable;
	};

	struct AkRTPCSubscription
	{
		AkRTPCSubscriptionKey									key;
		AkRTPCSubscription *									pNextItem;
		CAkRegisteredObj *										TargetGameObject;
		SubscriberType											eType;
		AkArray<RTPCCurve, const RTPCCurve &, ArrayPoolDefault, 1>	Curves;
	};

	struct AkRTPCValue
	{
		CAkRegisteredObj *	pGameObj;
		AkReal32			fValue;
	};

	struct AkRTPCValueGetKey
	{
		static CAkRegisteredObj * Get( const AkRTPCValue & in_item ) { return in_item.pGameObj; }
	};

	struct AkSubsPtrGetKey
	{
		static AkRTPCSubscription * Get( AkRTPCSubscription * const & in_item ) { return in_item; }
	};

	struct AkRTPCEntry;

	class CAkRTPCTransition : public ITransitionable
	{
	public:
		CAkRTPCTransition( AkRTPCEntry * in_pOwner, CAkRegisteredObj * in_pGameObject ) : m_pTransition( NULL ), m_pOwner( in_pOwner ), m_pGameObject( in_pGameObject ), m_bRemoveEntryWhenDone( false ) {}
		~CAkRTPCTransition();

		AKRESULT Start( AkReal32 in_fStartValue, AkReal32 in_fTargetValue, TransParams & in_transParams, bool in_bRemoveEntryWhenDone );
		virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_unionValue, bool in_bIsTerminated );

		CAkRTPCTransition *	pNextLightItem;
		CAkTransition *		m_pTransition;
		AkRTPCEntry *		m_pOwner;
		CAkRegisteredObj *	m_pGameObject;
		bool				m_bRemoveEntryWhenDone;
	};

	typedef AkListBareLight<CAkRTPCTransition> RTPCTransitions;

	struct AkRTPCEntry
	{
		explicit AkRTPCEntry( AkRtpcID in_RTPCid ) : key( in_RTPCid ), pNextItem( NULL ), fDefaultValue( 0.f ) {}
		~AkRTPCEntry() { values.Term(); subscriptions.Term(); }

		AkRTPCValue * ValueExists( CAkRegisteredObj * in_pGameObj ) { return values.Exists( in_pGameObj ); }
		AkReal32 GetCurrentValue( AkRTPCValue * in_pValueEntry );
		AkReal32 GetCurrentTargetValue( AkRTPCValue * in_pValueEntry );
		AKRESULT SetRTPC( AkRTPCValue * in_pValueEntry, AkReal32 in_Value, CAkRegisteredObj * in_GameObj, TransParams & in_transParams, bool in_bUnsetWhenDone );
		void RemoveGameObject( CAkRegisteredObj * in_pGameObj );
		AKRESULT ApplyRTPCValue( AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj, bool in_bUnsetValue );
		bool CreateOrModifyTransition( CAkRegisteredObj * in_pGameObj, AkReal32 in_fStartValue, AkReal32 in_fTargetValue, TransParams & in_transParams, bool in_bRemoveEntryWhenDone );
		void DestroyTransition( CAkRTPCTransition * in_pPrevItem, CAkRTPCTransition * in_pItem );
		void NotifyRTPCChange( AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj );
		void GetRTPCExceptions( GameObjExceptArray & io_ExceptArrayObj );

		AkRtpcID																		key;
		AkRTPCEntry *																	pNextItem;
		AkReal32																		fDefaultValue;
		AkSortedKeyArray<CAkRegisteredObj *, AkRTPCValue, ArrayPoolDefault, AkRTPCValueGetKey, 8>		values;
		RTPCTransitions																	transitions;
		AkSortedKeyArray<AkRTPCSubscription *, AkRTPCSubscription *, ArrayPoolDefault, AkSubsPtrGetKey, 2>	subscriptions;
	};

	struct AkSwitchSubscription
	{
		CAkSwitchAware *	pSwitch;
		AkSwitchGroupID		switchGroup;
	};

	struct AkRTPCSwitchAssociation
	{
		AkSwitchGroupID													switchGroup;
		AkRtpcID														RTPC_ID;
		CAkConversionTable<AkRTPCGraphPointBase<AkUInt32>, AkUInt32>	ConversionTable;
		CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2>					listRTPCSwitchSubscribers;
	};

	CAkRTPCMgr();

	AKRESULT Init();
	AKRESULT Term();

	AKRESULT SubscribeRTPC( void * in_pSubscriber, AkRtpcID in_RTPC_ID, AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID, AkCurveScaling in_eScaling, AkRTPCGraphPoint * in_pArrayConversion, AkUInt32 in_ulConversionArraySize, CAkRegisteredObj * in_TargetGameObject, SubscriberType in_eType );
	void UnSubscribeRTPC( void * in_pSubscriber, AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID, bool * out_bMoreCurvesRemaining = NULL );
	void UnSubscribeRTPC( void * in_pSubscriber, AkRTPC_ParameterID in_ParamID );
	void UnSubscribeRTPC( void * in_pSubscriber );

	AKRESULT SetRTPCInternal( AkRtpcID in_RTPCid, AkReal32 in_Value, CAkRegisteredObj * in_GameObj, TransParams & in_transParams, AkValueMeaning in_eValueMeaning );
	void ResetRTPCValue( AkRtpcID in_RTPCid, CAkRegisteredObj * in_GameObj, TransParams & in_transParams );
	bool GetRTPCValue( AkRtpcID in_RTPC_ID, CAkRegisteredObj * in_GameObj, AkReal32 & out_value, bool & out_bGameObjectSpecificValue );
	AkReal32 GetRTPCConvertedValue( void * in_pToken, CAkRegisteredObj * in_GameObj, AkRtpcID in_RTPCid );
	AkReal32 GetRTPCConvertedValue( void * in_pSubscriber, AkUInt32 in_ParamID, CAkRegisteredObj * in_GameObj );
	AkReal32 GetDefaultValue( AkRtpcID in_RTPCid, bool * out_pbWasFound = NULL );
	void SetDefaultParamValue( AkRtpcID in_RTPCid, AkReal32 in_fValue );
	void UnregisterGameObject( CAkRegisteredObj * in_GameObj );

	AKRESULT SetSwitchInternal( AkSwitchGroupID in_SwitchGroup, AkSwitchStateID in_SwitchState, CAkRegisteredObj * in_GameObj );
	AkSwitchStateID GetSwitch( AkSwitchGroupID in_SwitchGroup, CAkRegisteredObj * in_GameObj );
	AKRESULT SubscribeSwitch( CAkSwitchAware * in_pSwitch, AkSwitchGroupID in_switchGroup, bool in_bForceNoRTPC = false );
	void UnSubscribeSwitch( CAkSwitchAware * in_pSwitch );
	AKRESULT AddSwitchRTPC( AkSwitchGroupID in_switchGroup, AkRtpcID in_RTPC_ID, AkRTPCGraphPointBase<AkUInt32> * in_pArrayConversion, AkUInt32 in_ulConversionArraySize );
	void RemoveSwitchRTPC( AkSwitchGroupID in_switchGroup );

	AKRESULT RegisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID );
	void UnregisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID );

	AkHashListBare<AkRtpcID, AkRTPCEntry, 193>											m_RTPCEntries;
	AkHashList<AkSwitchKey, AkSwitchStateID, 193>										m_SwitchEntries;
	AkHashListBare<AkRTPCSubscriptionKey, AkRTPCSubscription, 193>						m_RTPCSubscribers;
	CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>					m_listSwitchSubscribers;
	CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>				m_listRTPCSwitch;

private:
	AkRTPCEntry * GetRTPCEntry( AkRtpcID in_RTPCid );
	void NotifyClientRTPCChange( AkRtpcID in_idRTPC, AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj );
	void RemoveReferencesToSubscription( AkRTPCSubscription * in_pSubscription );
	void UpdateSubscription( AkRTPCSubscription & in_rSubscription );
	AKRESULT UpdateRTPCSubscriberInfo( void * in_pSubscriber );
	void DestroySubscription( AkRTPCSubscription * in_pSubscription );
};
