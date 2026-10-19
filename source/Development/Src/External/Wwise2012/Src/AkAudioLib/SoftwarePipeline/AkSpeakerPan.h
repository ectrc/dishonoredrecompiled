/*=============================================================================
	AkSpeakerPan.h - CAkSpeakerPan (akspeakerpan.cpp): speaker volumes for a 2D pan position (the "2D panner" and the
	direct-speaker defaults per source channel layout) and for a 3D direction with spread / center divergence (dB).

	DISHONORED(port): agent IH2 (wave 26). m_fSin2 is retail's 2012 .data 0x1011e50 table (sin^2( i pi / 256 ), i = 0..255,
	filled by Init). The panning rule is CAkLEngine::m_panningRule (AkPanningRule_Speakers 0, _Headphones 1).
	GetSpeakerVolumes2DPan writes one AkSIMDSpeakerVolumes per input channel (out_pVolumes[ channel ]), except a mono
	source (mask 4), which writes one AkSpeakerVolumes.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

class CAkSpeakerPan
{
public:
	static void Init();
	static void AddSpeakerVolumesPower( AkInt32 in_iAngle, AkReal32 in_fDivergenceCenter, AkSpeakerVolumes * out_pVolumes );
	static void GetSpeakerVolumesdB( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fDivergenceCenter, AkSpeakerVolumes * out_pVolumes );
	static void GetSpeakerVolumes2DPan( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fCenterPct, bool in_bIsPannerEnabled, AkChannelMask in_uChannelMask, AkSIMDSpeakerVolumes * out_pVolumes );
	static void GetSpeakerVolumes( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fDivergenceCenter, AkReal32 in_fSpread, AkSIMDSpeakerVolumes * out_pVolumes, AkUInt32 in_uNumFullBandChannels );

	static AkReal32 m_fSin2[ 256 ];

private:
	static void _GetSpeakerVolumes2DPan( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fCenterPct, AkPanningRule in_panningRule, AkSpeakerVolumes * out_pVolumes );
	static void _GetSpeakerVolumes2DPanSourceHasCenter( AkReal32 in_fX, AkReal32 in_fY, AkSpeakerVolumes * out_pVolumes );
};
