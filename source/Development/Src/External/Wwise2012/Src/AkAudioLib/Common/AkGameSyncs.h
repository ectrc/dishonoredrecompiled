/*=============================================================================
	AkGameSyncs.h - the game-sync managers in one include: CAkRTPCMgr (AkRTPCMgr.h), CAkStateMgr (AkStateMgr.h),
	CAkEnvironmentsMgr (AkEnvironmentsMgr.h) and CAkSwitchAware (AkSwitchAware.h).

	DISHONORED(port): agent IH1 (wave 26) kept data stores of these managers here; agent IH3 (fork R) replaced them
	with the ported managers in their retail files and keeps this header so the hierarchy includes one name.
=============================================================================*/
#pragma once

#include "AkRTPCMgr.h"
#include "AkStateMgr.h"
#include "AkEnvironmentsMgr.h"
#include "AkSwitchAware.h"
