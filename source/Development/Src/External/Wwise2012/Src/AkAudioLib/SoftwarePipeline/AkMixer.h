/*=============================================================================
	AkMixer.h - CAkMixer (akmixer.cpp, akmixersimd.cpp): adds a voice or bus buffer into a bus buffer with a per-channel
	volume ramp from AudioMix.Previous to AudioMix.Next over the frame, and the final mix into the interleaved sink format.

	DISHONORED(port): agent IH2 (wave 26). 8 bytes as the 2012 PDB (m_usMaxFrames, m_fOneOverNumFrames). Buffers are
	planar in pipeline order FL FR C RL RR LFE (the AkSpeakerVolumes order); AddVolume works on blocks of 8 frames, so
	m_usMaxFrames must be a multiple of 8 (it is LE_MAX_FRAMES_PER_BUFFER).
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

struct AkSpeakerVolumesStereo3D
{
	AkSpeakerVolumesStereo3D( AkAudioMix & AudioMix, AkReal32 fOneOverNumSamples );

	AkReal32 fLeft;
	AkReal32 fLeftDelta;
	AkReal32 fRight;
	AkReal32 fRightDelta;
};

class CAkMixer
{
public:
	CAkMixer();

	void Init( AkUInt16 in_uMaxFrames );
	void MixStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void Mix51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void Mix3D( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixFinalStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixFinal51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );

	AkUInt16 m_usMaxFrames;
	AkReal32 m_fOneOverNumFrames;

private:
	void AddVolume( AkReal32 * in_pSourceData, AkReal32 * in_pDestData, AkReal32 in_fVolume, AkReal32 in_fVolumeDelta );
	void MixNStereoPrev( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixN51Prev( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixN3DFiveOnePrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixN3DFourPrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixN3DStereoPrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixAndInterleaveStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
	void MixAndInterleave51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer );
};
static_assert( sizeof( CAkMixer ) == 8, "CAkMixer: 2012 PDB sizeof 8" );
