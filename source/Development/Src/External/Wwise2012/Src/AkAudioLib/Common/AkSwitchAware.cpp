/*=============================================================================
	AkSwitchAware.cpp - CAkSwitchAware (akswitchaware.cpp, 2012 0x95e970..0x95ea20; 2013 0x95b160..0x95b210, by bytes).

	DISHONORED(port): agent IH3 (wave 26), fork R. The ctor / dtor (0x95e970 / 0x95e980) only set the vtable and are
	the inline ones in AkSwitchAware.h.
=============================================================================*/
#include "AkSwitchAware.h"
#include "AkRTPCMgr.h"
#include "AkStateMgr.h"

/** 2012 0x95e990. */
void CAkSwitchAware::UnsubscribeSwitches()
{
	g_pRTPCMgr->UnSubscribeSwitch( this );
	g_pStateMgr->UnregisterSwitch( this );
}

/** 2012 0x95e9b0: group 0 means "follow nothing". */
AKRESULT CAkSwitchAware::SubscribeSwitch( AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	UnsubscribeSwitches();
	if( !in_ulGroup )
	{
		return AK_Success;
	}
	if( in_eGroupType == AkGroupType_Switch )
	{
		return g_pRTPCMgr->SubscribeSwitch( this, in_ulGroup );
	}
	if( in_eGroupType == AkGroupType_State )
	{
		return g_pStateMgr->RegisterSwitch( this, in_ulGroup );
	}
	return AK_InvalidParameter;
}

/** 2012 0x95ea20. */
AkUInt32 CAkSwitchAware::GetSwitchToUse( CAkRegisteredObj * in_pGameObj, AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	if( in_eGroupType == AkGroupType_Switch )
	{
		return g_pRTPCMgr->GetSwitch( in_ulGroup, in_pGameObj );
	}
	if( in_eGroupType == AkGroupType_State )
	{
		return g_pStateMgr->GetState( in_ulGroup );
	}
	return 0;
}
