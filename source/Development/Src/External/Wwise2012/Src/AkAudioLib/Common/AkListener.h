/*=============================================================================
	AkListener.h - CAkListener (ak3dlistener.cpp): the eight listeners' positions, rotation matrices, spatialization
	offsets, scaling factors and the audio / feedback pipeline masks.

	DISHONORED(port): agent IH2 (wave 26). One listener is 144 bytes (the decompile strides 36 dwords): the position,
	the AkListenerData volume offsets and spatialization flag, the 3x3 rotation matrix, the scaling factor.
	The 3D half (Get3DVolumes 0x935400, GetListenerVolume 0x935270, ComputeConeEffect 0x935180,
	IsInitiallyUnderThreshold 0x936d30, DoSpeakerVolumeMatrixCallback 0x934ff0) is agent IH3's (wave 26).
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"
#include "AkRegistryMgr.h"

class CAkPBI;
class CAkGen3DParams;

/** AkSoundPositioningParams (PDB, 20 bytes): what CAkPBI::Get3DPositioningParams hands the 3D volume computation. */
struct AkSoundPositioningParams
{
	AkPositioningType			ePosType;
	const AkSoundPositionRef *	pPosRef;
	AkReal32					fDivergenceCenter;
	AkReal32					fPAN_RL;
	AkReal32					fPAN_FR;
};

struct AkListenerData
{
	AkListenerPosition		Pos;
	AkSIMDSpeakerVolumes	VolumeOffset;
	bool					bSpatialized;
};
static_assert( sizeof( AkListenerData ) == 96, "AkListenerData: 2012 PDB align 16" );

struct AkListener
{
	AkListenerData	data;
	AkReal32		Matrix[ 3 ][ 3 ];
	AkReal32		fScalingFactor;
};
static_assert( sizeof( AkListener ) == 144, "AkListener: the 2012 decompile strides 144 bytes" );

class CAkListener
{
public:
	static AKRESULT Init();
	static void ResetListenerData();
	static AKRESULT SetListenerPosition( AkUInt32 in_uListener, const AkListenerPosition & in_Position );
	static AKRESULT GetListenerPosition( AkUInt32 in_uListener, AkListenerPosition & out_rPosition );
	static void SetScalingFactor( AkUInt32 in_uListener, AkReal32 in_fScalingFactor );
	static AkReal32 GetScalingFactor( AkUInt32 in_uListener );
	static AKRESULT SetListenerSpatialization( AkUInt32 in_uListener, bool in_bSpatialized, AkSpeakerVolumes * in_pVolumeOffsets );
	static AKRESULT GetListenerSpatialization( AkUInt32 in_uListener, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets );
	static void SetListenerPipeline( AkUInt32 in_uListener, bool in_bAudio, bool in_bFeedback );
	static AkReal32 * GetDefaultRotationMatrix();
	static AkUInt32 GetAudioMask() { return m_uAudioMask; }

	static void Get3DVolumes( CAkPBI * in_pContext, const AkSoundPositioningParams & in_posParams, AkChannelMask in_uChannelMask, AkAudioMix * out_pAudioMixWet, AkAudioMix * out_pAudioMixDry, AkReal32 * out_pLPF, AkReal32 * out_pObsLPF, bool in_bEnvBus );
	static bool IsInitiallyUnderThreshold( CAkPBI * in_pCtx );
	static void DoSpeakerVolumeMatrixCallback( AkPlayingID in_playingID, AkUInt32 in_uNumChannels, AkChannelMask in_uChannelMask, bool in_bEnvBus, AkAudioMix * io_pAudioMixWet, AkAudioMix * io_pAudioMixDry );

	static AkListener	m_listeners[ 8 ];
	static AkReal32		m_matRot[ 3 ][ 3 ];
	static AkReal32		m_matIdentity[ 3 ][ 3 ];
	static AkUInt32		m_uAudioMask;
	static AkUInt32		m_uFeedbackMask;

private:
	static void ComputeConeEffect( CAkGen3DParams * in_pGen3DParams, const AkSoundPosition & in_soundPos, const AkVector & in_direction, AkReal32 in_fDistance, AkReal32 & io_fMinCone, AkReal32 & io_fListenerVolumeDry );
	static void GetListenerVolume( CAkGen3DParams * in_pGen3DParams, const AkSoundPosition & in_soundPos, const AkVector & in_vecListenerPos, AkReal32 & io_fMinCone, AkReal32 & io_fDistance, AkReal32 & io_fListenerVolumeDry, AkReal32 & io_fListenerVolumeWet, AkReal32 in_fScalingFactor );
};
