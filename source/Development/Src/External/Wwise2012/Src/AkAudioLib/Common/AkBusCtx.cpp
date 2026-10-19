/*=============================================================================
	AkBusCtx.cpp - AK::CAkBusCtx (akbusctx.cpp, 2012 0x93de70..0x93df40) and CAkBus::GetBusEffectiveVolume (0x93c890).

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * GetBusEffectiveVolume: no state values, RTPC, SIS or ducking (IH3 seams) - the bus's own property, summed
	    up the output chain to the next bus with an effect (or the whole tree).
	  * GetFX / GetFXDataID (the bus insert effects) are IH4's and not ported.
=============================================================================*/
#include "AkBusCtx.h"

AK::CAkBusCtx g_MasterBusCtx;

namespace AK
{
	/** 2012 0x93de70: the mixing bus above this one (NULL = the final mix). */
	CAkBusCtx CAkBusCtx::GetParentCtx() const
	{
		CAkBusCtx parentCtx;
		if( m_pBus && m_pBus->m_pBusOutputNode )
		{
			parentCtx.m_pBus = m_pBus->m_pBusOutputNode->GetMixingBus();
		}
		return parentCtx;
	}

	/** 2012 0x93dea0. */
	AkUniqueID CAkBusCtx::ID() const
	{
		return m_pBus ? m_pBus->key : AK_INVALID_UNIQUE_ID;
	}

	/** 2012 0x93deb0. */
	AkReal32 CAkBusCtx::GetVolume( BusVolumeType in_VolumeType ) const
	{
		return m_pBus ? m_pBus->GetBusEffectiveVolume( in_VolumeType, AkPropID_Volume ) : 0.f;
	}

	/** 2012 0x93ded0. */
	bool CAkBusCtx::IsMasterBus() const
	{
		return m_pBus && m_pBus->IsMasterBus();
	}

	/** 2012 0x93dee0: the final mix is 5.1-capable. */
	AkUInt32 CAkBusCtx::GetChannelConfig() const
	{
		return m_pBus ? m_pBus->m_uChannelConfig : AK_SPEAKER_SETUP_5POINT1;
	}

	/** 2012 0x93df10. */
	bool CAkBusCtx::GetBypassAllFX() const
	{
		return m_pBus && m_pBus->GetBypassAllFX( NULL );
	}

	/** 2012 0x93df30. */
	bool CAkBusCtx::HasEffect() const
	{
		return m_pBus && m_pBus->HasEffect();
	}

	/** 2012 0x93df40. */
	bool CAkBusCtx::IsEnvironmental() const
	{
		return m_pBus && m_pBus->m_bIsEnvironmental;
	}
}

/** 2012 0x93c890. */
AkReal32 CAkBus::GetBusEffectiveVolume( AkUInt32 in_eVolumeType, AkPropID in_eProp )
{
	const AkPropValue * pValue = m_props.FindProp( (AkUInt8)in_eProp );
	const AkReal32 fVolume = pValue ? pValue->fValue : 0.f;
	CAkBus * pParent = static_cast<CAkBus *>( m_pBusOutputNode );
	if( pParent && ( in_eVolumeType == BusVolumeType_IncludeEntireBusTree || !pParent->HasEffect() ) )
	{
		return pParent->GetBusEffectiveVolume( in_eVolumeType, in_eProp ) + fVolume;
	}
	return fVolume;
}
