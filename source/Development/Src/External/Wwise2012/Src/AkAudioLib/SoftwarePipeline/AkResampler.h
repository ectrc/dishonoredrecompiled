/*=============================================================================
	AkResampler.h - CAkResampler (akresampler.cpp) and its DSP routines (akresamplercommon.cpp, akresamplersimd.cpp): the
	pitch / sample-rate conversion of one voice, run by CAkVPLPitchNode. Input is 16-bit interleaved or float
	deinterleaved (the source's format), output is always float deinterleaved in pipeline channel order (LFE last).

	DISHONORED(port): agent IH2 (wave 26). Layouts of the 2012 PDB: AkInternalPitchState 56 bytes, CAkResampler 72 bytes.
	Frame skip is 16.16 fixed point (0x10000 = no pitch change); m_DSPFunctionIndex 0..3 = 16-bit 1 / 2 / 3-4 / 5-6
	channels, 4..7 the same for float; PitchDSPFuncTable[ mode ][ index ] with mode Bypass / Fixed / Interpolating.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

struct AkInternalPitchState
{
	union
	{
		AkReal32	fLastValue[ AK_VOICE_MAX_NUM_CHANNELS ];
		AkInt16		iLastValue[ AK_VOICE_MAX_NUM_CHANNELS ];
		AkUInt8		uLastValue[ AK_VOICE_MAX_NUM_CHANNELS ];
	};
	AkUInt32 uInFrameOffset;
	AkUInt32 uOutFrameOffset;
	AkUInt32 uFloatIndex;
	AkUInt32 uCurrentFrameSkip;
	AkUInt32 uTargetFrameSkip;
	AkUInt32 uInterpolationRampCount;
	AkUInt32 uInterpolationRampInc;
	AkUInt32 uRequestedFrames;
};
static_assert( sizeof( AkInternalPitchState ) == 56, "AkInternalPitchState: 2012 PDB sizeof 56" );

enum PitchOperatingMode
{
	PitchOperatingMode_Bypass			= 0,
	PitchOperatingMode_Fixed			= 1,
	PitchOperatingMode_Interpolating	= 2,
	NumPitchOperatingMode				= 3
};

#define PITCHRAMPLENGTH			1024
#define FPBITS					16
#define FPMUL					( 1 << FPBITS )
#define FPMASK					( FPMUL - 1 )
#define NUM_PITCH_DSP_FUNCTIONS	8

typedef AKRESULT ( *PitchDSPFuncPtr )( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );

class CAkResampler
{
public:
	static void InitDSPFunctTable();

	AkReal32 GetLastRate();
	void Init( AkAudioFormat * io_pFormat, AkUInt32 in_uSampleRate );
	AKRESULT Execute( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer );
	bool IsPostDeInterleaveRequired();
	AKRESULT DeinterleaveAndSwapOutput( AkPipelineBufferBase * io_pIOBuffer );
	AKRESULT InterleaveAndSwapOutput( AkPipelineBufferBase * io_pIOBuffer );
	void TimeInputToOutput( AkUInt32 & io_uFrames );
	void TimeOutputToInput( AkUInt32 & io_uFrames );
	void ResetOffsets();
	void SetPitch( AkReal32 in_fPitchVal );
	void SetPitchForTimeSkip( AkReal32 in_fPitchVal );
	void SwitchTo( const AkAudioFormat & in_fmt, AkReal32 in_fPitch, AkPipelineBufferBase * io_pIOBuffer, AkUInt32 in_uSampleRate );

	void SetRequestedFrames( AkUInt32 in_uRequestedFrames ) { m_InternalPitchState.uRequestedFrames = in_uRequestedFrames; }
	AkUInt32 GetInputFrameOffset() const { return m_InternalPitchState.uInFrameOffset; }
	AkUInt32 GetOutputFrameOffset() const { return m_InternalPitchState.uOutFrameOffset; }
	void SetInputFrameOffset( AkUInt32 in_uInputFrameOffset ) { m_InternalPitchState.uInFrameOffset = in_uInputFrameOffset; }
	void SetOutputFrameOffset( AkUInt32 in_uOutputFrameOffset ) { m_InternalPitchState.uOutFrameOffset = in_uOutputFrameOffset; }
	PitchOperatingMode GetPitchOperatingMode() const { return m_PitchOperationMode; }

	AkInternalPitchState	m_InternalPitchState;
	PitchOperatingMode		m_PitchOperationMode;
	AkReal32				m_fSampleRateConvertRatio;
	AkReal32				m_fTargetPitchVal;
	AkUInt8					m_DSPFunctionIndex;
	AkUInt8					m_uNumChannels;
	AkUInt8					m_uInputBlockAlign;
	bool					m_bFirstSetPitch;

private:
	void SetLastValues( AkReal32 * in_pfLastValues );
	void GetLastValues( AkReal32 * out_pfLastValues );
	AkUInt8 GetDSPFunctionIndex( const AkAudioFormat & in_fmt ) const;
};
static_assert( sizeof( CAkResampler ) == 72, "CAkResampler: 2012 PDB sizeof 72" );

extern PitchDSPFuncPtr PitchDSPFuncTable[ NumPitchOperatingMode ][ NUM_PITCH_DSP_FUNCTIONS ];

AKRESULT Bypass_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_I16_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_I16_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
void Deinterleave_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer );
void Interleave_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer );

AKRESULT Bypass_I16_NChanVec( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Bypass_I16_NChanVecSSE2( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Bypass_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Bypass_I16_2ChanSSE2( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_I16_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_Native_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Fixed_Native_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_I16_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_Native_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
AKRESULT Interpolating_Native_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState );
