/*=============================================================================
	AkSwitchAware.h - CAkSwitchAware (akswitchaware.cpp): what follows a switch or state group (the switch container,
	the music switch container); it subscribes to the RTPC manager (switch) or the state manager (state).

	DISHONORED(port): agent IH3 (wave 26), fork R. The PDB class is the vtable alone (dtor, SetSwitch).
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

class CAkRegisteredObj;

class CAkSwitchAware
{
public:
	CAkSwitchAware() {}
	virtual ~CAkSwitchAware() {}

	virtual void SetSwitch( AkUInt32 in_Switch, CAkRegisteredObj * in_pGameObj = NULL ) = 0;

	AkUInt32 GetSwitchToUse( CAkRegisteredObj * in_pGameObj, AkUInt32 in_ulGroup, AkGroupType in_eGroupType );

protected:
	AKRESULT SubscribeSwitch( AkUInt32 in_ulGroup, AkGroupType in_eGroupType );
	void UnsubscribeSwitches();
};
