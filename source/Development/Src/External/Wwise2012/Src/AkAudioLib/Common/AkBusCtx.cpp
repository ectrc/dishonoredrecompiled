/*=============================================================================
	AkBusCtx.cpp - AK::CAkBusCtx (akbusctx.cpp, 2012 0x93de70..0x93df40) and CAkBus::GetBusEffectiveVolume (0x93c890).

	DISHONORED(port): agent IH2 (wave 26).
	IH3 fork N: GetBusEffectiveVolume with the bus's state values, RTPC, SIS and ducking.
	Agent IH4 fork F1: GetFX 0x93df60 (an environmental bus's slot 0 is the environment's share set) and GetFXDataID 0x93def0.
=============================================================================*/
#include "AkBusCtx.h"
#include "AkPBI.h"

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

	/** 2012 0x93df60, 2013 0x93a980: an environmental (aux) bus runs the environment's own share set in slot 0 and nothing
	    else, unbypassed; any other bus answers its own slots. */
	void CAkBusCtx::GetFX( AkUniqueID in_envID, AkUInt32 in_uFXIndex, AkFXDesc & out_rFXInfo ) const
	{
		if( !m_pBus )
		{
			out_rFXInfo.SetFx( NULL );
			out_rFXInfo.bIsBypassed = false;
			return;
		}
		if( !m_pBus->m_bIsEnvironmental )
		{
			m_pBus->GetFX( in_uFXIndex, out_rFXInfo, NULL );
			return;
		}
		out_rFXInfo.SetFx( in_uFXIndex == 0 ? AkGetFxAndAddRef( in_envID, true ) : NULL );
		out_rFXInfo.bIsBypassed = false;
	}

	/** 2012 0x93def0. */
	void CAkBusCtx::GetFXDataID( AkUInt32 in_uFXIndex, AkUInt32 in_uDataIndex, AkUInt32 & out_rDataID ) const
	{
		if( m_pBus )
		{
			m_pBus->GetFXDataID( in_uFXIndex, in_uDataIndex, out_rDataID );
		}
		else
		{
			out_rDataID = (AkUInt32)-1;
		}
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

/** 2012 0x93c890 (2013 0x939230): the bus's states, property (or its RTPC), SIS value and ducking, plus the output bus's
    up to the next bus with an effect (or the whole tree). */
AkReal32 CAkBus::GetBusEffectiveVolume( AkUInt32 in_eVolumeType, AkPropID in_eProp )
{
	AkSoundParams soundParams;
	soundParams.Volume = 0.f;
	soundParams.Pitch = 0.f;
	soundParams.LPF = 0.f;
	GetAudioStateParams( soundParams, 1 );
	const AkPropValue * pValue = m_props.FindProp( (AkUInt8)in_eProp );
	AkReal32 fVolume = ( pValue ? pValue->fValue : 0.f ) + soundParams.Volume;
	const AkRTPC_ParameterID eRTPC = g_AkPropRTPCID[ in_eProp ];
	if( m_RTPCBitArrayMax32.IsSet( eRTPC ) )
	{
		fVolume += g_pRTPCMgr->GetRTPCConvertedValue( this, eRTPC, NULL );
	}
	if( m_pGlobalSIS )
	{
		const AkSISValue * pSISValue = m_pGlobalSIS->m_values.FindProp( (AkUInt8)in_eProp );
		if( pSISValue )
		{
			fVolume += pSISValue->fValue;
		}
	}
	fVolume += GetDuckedVolume();
	CAkBus * pParent = static_cast<CAkBus *>( m_pBusOutputNode );
	if( pParent && ( in_eVolumeType == BusVolumeType_IncludeEntireBusTree || !pParent->HasEffect() ) )
	{
		return pParent->GetBusEffectiveVolume( in_eVolumeType, in_eProp ) + fVolume;
	}
	return fVolume;
}
