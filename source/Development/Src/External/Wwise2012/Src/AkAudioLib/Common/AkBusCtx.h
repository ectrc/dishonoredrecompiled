/*=============================================================================
	AkBusCtx.h - AK::CAkBusCtx (akbusctx.cpp): the lower engine's handle on a mixing bus (NULL = the final mix).

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list (the mix bus nodes cannot run without it).
	g_MasterBusCtx follows CAkBus::s_pMasterBus (set in CAkBus::Create 0x93cf50, cleared in the dtor 0x93d870).
=============================================================================*/
#pragma once

#include "AkHierarchy.h"

enum BusVolumeType
{
	BusVolumeType_ToNextBusWithEffect	= 0,
	BusVolumeType_IncludeEntireBusTree	= 1
};

namespace AK
{
	class CAkBusCtx
	{
	public:
		CAkBusCtx() : m_pBus( NULL ) {}

		CAkBusCtx GetParentCtx() const;
		AkUniqueID ID() const;
		AkReal32 GetVolume( BusVolumeType in_VolumeType ) const;
		bool IsMasterBus() const;
		AkUInt32 GetChannelConfig() const;
		bool GetBypassAllFX() const;
		bool HasEffect() const;
		bool IsEnvironmental() const;
		void GetFX( AkUniqueID in_envID, AkUInt32 in_uFXIndex, AkFXDesc & out_rFXInfo ) const;
		void GetFXDataID( AkUInt32 in_uFXIndex, AkUInt32 in_uDataIndex, AkUInt32 & out_rDataID ) const;

		CAkBus * m_pBus;
	};
}

extern AK::CAkBusCtx g_MasterBusCtx;
