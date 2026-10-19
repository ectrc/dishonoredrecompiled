/*=============================================================================
	AkGameSyncs.h - the data side of CAkStateMgr, CAkRTPCMgr and CAkEnvironmentsMgr: what a bank's STMG / ENVS
	chunks and every object's RTPC and state chunks register.

	DISHONORED(port): agent IH1 (wave 26). These three managers belong to IH3 (game syncs and their
	evaluation); the bank manager only calls their registration entry points while it loads, so IH1 ports
	those and keeps what they store:
	  CAkStateMgr::AddStateGroup / SetdefaultTransitionTime / AddStateTransition / AddStateGroupMember /
	    RemoveStateGroupMember / GetState / RemoveAllStateGroups      (akstatemgr.cpp)
	  CAkRTPCMgr::SubscribeRTPC / UnSubscribeRTPC / AddSwitchRTPC / SetDefaultParamValue / RegisterLayer /
	    UnregisterLayer                                                 (akrtpcmgr.cpp)
	  CAkEnvironmentsMgr::SetObsOccCurve, m_bCurveEnabled, m_bEnvironmentalBypass (akenvironmentsmgr.cpp)
	Deviation (named): the containers are this port's own (a group / subscription list per manager, not the
	PDB's hash-of-lists), nothing is ever notified (no PBI exists before IH2), and the curves are stored but
	never evaluated. IH3 replaces this file with the real managers; the call signatures are the retail ones.
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

class AkStateGroupChunk;
class CAkLayer;

/** One state group: the members (node chunks) that listen to it, its default and custom transitions. */
struct AkStateGroupInfo
{
	struct Transition
	{
		AkStateID	StateFrom;
		AkStateID	StateTo;
		AkTimeMs	TransitionTime;
	};

	AkStateGroupInfo() : ActualState( 0 ), lDefaultTransitionTime( 0 ) {}
	~AkStateGroupInfo() { mapTransitions.Term(); members.Term(); }

	AkStateID								ActualState;
	AkTimeMs								lDefaultTransitionTime;
	AkArray<Transition, const Transition &>	mapTransitions;
	AkArray<AkStateGroupChunk *, AkStateGroupChunk *>	members;
};

class CAkStateMgr
{
public:
	AKRESULT Init();
	void Term();

	AkStateGroupInfo * AddStateGroup( AkStateGroupID in_ulStateGroupID );
	AKRESULT SetdefaultTransitionTime( AkStateGroupID in_ulStateGroupID, AkTimeMs in_lTransitionTime );
	AKRESULT AddStateTransition( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID1, AkStateID in_ulStateID2, AkTimeMs in_lTransitionTime, bool in_bIsShared );
	AKRESULT AddStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember );
	void RemoveStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember );
	AkStateID GetState( AkStateGroupID in_ulStateGroupID );
	void RemoveAllStateGroups( bool in_bIsFromClearBanks );

	AkUInt32 NumStateGroups() const { return m_StateGroups.Length(); }

	AkHashList<AkStateGroupID, AkStateGroupInfo, 31> m_StateGroups;
};

enum AkRTPC_ParameterID
{
	RTPC_Volume = 0
};

class CAkRTPCMgr
{
public:
	enum SubscriberType
	{
		SubscriberType_CAkParameterNodeBase	= 0,
		SubscriberType_IAkRTPCSubscriber	= 1,
		SubscriberType_CAkLayer				= 2,
		SubscriberType_PBI					= 3,
		SubscriberType_CAkCrossfadingLayer	= 4
	};

	struct AkRTPCSubscription
	{
		AkRtpcID										RTPCID;
		AkUInt32										ParamID;
		AkUniqueID										RTPCCurveID;
		SubscriberType									eType;
		CAkConversionTable<AkRTPCGraphPoint, AkReal32>	Curve;
	};

	/** Every subscription of one subscriber (node, layer, ...), keyed by the subscriber's address. */
	struct AkRTPCSubscriberEntry
	{
		~AkRTPCSubscriberEntry()
		{
			for( AkRTPCSubscription * p = subscriptions.m_pItems; p != subscriptions.m_pItemsEnd; ++p )
			{
				p->Curve.Unset();
			}
			subscriptions.Term();
		}
		AkArray<AkRTPCSubscription, const AkRTPCSubscription &, ArrayPoolDefault, 2> subscriptions;
	};

	struct AkSwitchRTPC
	{
		AkSwitchGroupID									SwitchGroup;
		AkRtpcID										RTPCID;
		CAkConversionTable<AkRTPCGraphPointBase<AkUInt32>, AkUInt32>	Curve;
	};

	struct AkLayerRegistration
	{
		CAkLayer *	pLayer;
		AkRtpcID	RTPCID;
	};

	AKRESULT Init();
	void Term();

	AKRESULT SubscribeRTPC( void * in_pSubscriber, AkRtpcID in_RTPC_ID, AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID, AkCurveScaling in_eScaling, AkRTPCGraphPoint * in_pArrayConversion, AkUInt32 in_ulConversionArraySize, void * in_TargetGameObject, SubscriberType in_eType );
	void UnSubscribeRTPC( void * in_pSubscriber, AkUInt32 in_ParamID );
	AKRESULT AddSwitchRTPC( AkSwitchGroupID in_switchGroup, AkRtpcID in_rtpcID, AkRTPCGraphPointBase<AkUInt32> * in_pArrayConversion, AkUInt32 in_ulConversionArraySize );
	void SetDefaultParamValue( AkRtpcID in_RTPCid, AkReal32 in_fValue );
	AKRESULT RegisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID );
	void UnregisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID );

	AkUInt32 NumSubscriptions() const { return m_uNumSubscriptions; }

	AkHashList<AkUIntPtr, AkRTPCSubscriberEntry, 193>		m_Subscribers;
	AkUInt32												m_uNumSubscriptions;
	AkHashList<AkSwitchGroupID, AkSwitchRTPC, 31>			m_SwitchRTPC;
	AkHashList<AkRtpcID, AkReal32, 31>						m_DefaultParamValues;
	AkArray<AkLayerRegistration, const AkLayerRegistration &>	m_Layers;
};

class CAkEnvironmentsMgr
{
public:
	enum eCurveXType { CurveObs = 0, CurveOcc = 1, MAX_CURVE_X_TYPES = 2 };
	enum eCurveYType { CurveVol = 0, CurveLPF = 1, MAX_CURVE_Y_TYPES = 2 };

	CAkEnvironmentsMgr() : m_bEnvironmentalBypass( false ) { memset( m_bCurveEnabled, 0, sizeof( m_bCurveEnabled ) ); }

	AKRESULT Init();
	void Term();
	AKRESULT SetObsOccCurve( eCurveXType in_x, eCurveYType in_y, AkUInt32 in_ulNbPoints, AkRTPCGraphPoint * in_paPoints, AkCurveScaling in_eScaling );

	CAkConversionTable<AkRTPCGraphPoint, AkReal32>	m_ObsOccCurves[ MAX_CURVE_X_TYPES ][ MAX_CURVE_Y_TYPES ];
	bool											m_bCurveEnabled[ MAX_CURVE_X_TYPES ][ MAX_CURVE_Y_TYPES ];
	bool											m_bEnvironmentalBypass;
};
