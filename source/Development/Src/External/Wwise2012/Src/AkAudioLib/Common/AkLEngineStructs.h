/*=============================================================================
	AkLEngineStructs.h - the lower engine's buffer and state records (aklengine*, akvpl*, akcommon.h, akpbi.h).

	DISHONORED(port): agent IH2 (wave 26). Every record is the 2012 PDB's (resources/docs/types/all_types.h), field for
	field; sizes are static_asserted where the decompile indexes into them. Audio buffers are non-interleaved float:
	channel i of an AkAudioBuffer starts at pData + i * uMaxFrames (the SDK's AkAudioBuffer::GetChannel).
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <xmmintrin.h>

class CAkPBI;

/** The pipeline runs at 48 kHz in blocks of 1024 frames (AK_NUM_VOICE_REFILL_FRAMES, LE_MAX_FRAMES_PER_BUFFER). */
#define AK_CORE_SAMPLERATE			48000
#define LE_MAX_FRAMES_PER_BUFFER	1024
#define AK_NUM_VOICE_REFILL_FRAMES	1024
#define AK_VOICE_MAX_NUM_CHANNELS	6

#define AK_INT						0
#define AK_FLOAT					1
#define AK_INTERLEAVED				0
#define AK_NONINTERLEAVED			1

#define AKCODECID_BANK				0
#define AKCODECID_PCM				1
#define AKCODECID_ADPCM				2
#define AKCODECID_XMA				3
#define AKCODECID_VORBIS			4
#define AKCODECID_WIIADPCM			5
#define AKCODECID_PCMEX				7
#define AKCODECID_EXTERNAL_SOURCE	8

/** AkPluginID: (company << 16) | (id << 4) | type; the codec of a source is (id >> 16) & 0xFFF. */
#define CODECID_FROM_PLUGINID( _pluginid_ )	( ( (_pluginid_) >> 16 ) & 0xFFF )
#define AkPluginTypeCodec_Value		1

extern AkMemPoolId g_LEngineDefaultPoolId;

struct ArrayPoolLEngineDefault
{
	static AkMemPoolId Get() { return g_LEngineDefaultPoolId; }
};

inline AkUInt32 AkGetNumChannels( AkUInt32 in_uChannelMask )
{
	AkUInt32 num = 0;
	while( in_uChannelMask )
	{
		++num;
		in_uChannelMask &= in_uChannelMask - 1;
	}
	return num;
}

/** 8 bytes: the 2012 PDB's bitfields. */
struct AkAudioFormat
{
	AkUInt32	uSampleRate;
	AkUInt32	uChannelMask : 18;
	AkUInt32	uBitsPerSample : 6;
	AkUInt32	uBlockAlign : 5;
	AkUInt32	uTypeID : 2;
	AkUInt32	uInterleaveID : 1;

	AkUInt32 GetNumChannels() const { return AkGetNumChannels( uChannelMask ); }
	void SetAll( AkUInt32 in_uSampleRate, AkUInt32 in_uChannelMask, AkUInt32 in_uBitsPerSample, AkUInt32 in_uBlockAlign, AkUInt32 in_uTypeID, AkUInt32 in_uInterleaveID )
	{
		uSampleRate = in_uSampleRate;
		uChannelMask = in_uChannelMask;
		uBitsPerSample = in_uBitsPerSample;
		uBlockAlign = in_uBlockAlign;
		uTypeID = in_uTypeID;
		uInterleaveID = in_uInterleaveID;
	}
};
static_assert( sizeof( AkAudioFormat ) == 8, "AkAudioFormat: 2012 PDB sizeof 8" );

struct AkAudioMarker
{
	AkUInt32	dwIdentifier;
	AkUInt32	dwPosition;
	char *		strLabel;
};

struct AkBufferMarker
{
	CAkPBI *		pContext;
	AkUInt32		dwPositionInBuffer;
	AkAudioMarker	marker;
};

struct AkBufferPosInformation
{
	AkUInt32	uStartPos;
	AkReal32	fLastRate;
	AkUInt32	uFileEnd;
	AkUInt32	uSampleRate;

	void Clear()
	{
		uStartPos = (AkUInt32)-1;
		fLastRate = 1.f;
		uFileEnd = (AkUInt32)-1;
		uSampleRate = 1;
	}
};

/** 16 bytes. */
struct AkAudioBuffer
{
	void *		pData;
	AkUInt32	uChannelMask;
	AKRESULT	eState;
	AkUInt16	uMaxFrames;
	AkUInt16	uValidFrames;

	AkUInt32 NumChannels() const { return AkGetNumChannels( uChannelMask ); }
	AkReal32 * GetChannel( AkUInt32 in_uIndex ) { return (AkReal32 *)pData + in_uIndex * uMaxFrames; }
	/** 2012 0x97c5a0: the LFE is the last channel of a deinterleaved buffer. */
	AkReal32 * GetLFE()
	{
		if( !( uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) )
		{
			return NULL;
		}
		return (AkReal32 *)pData + ( NumChannels() - 1 ) * uMaxFrames;
	}
	void * GetInterleavedData() { return pData; }
	bool HasData() const { return pData != NULL; }
	AkUInt16 MaxFrames() const { return uMaxFrames; }
	void SetRequestSize( AkUInt16 in_uMaxFrames ) { uMaxFrames = in_uMaxFrames; }
	void Clear()
	{
		pData = NULL;
		uValidFrames = 0;
	}
	void ClearData()
	{
		if( pData )
		{
			memset( pData, 0, uMaxFrames * NumChannels() * sizeof( AkReal32 ) );
		}
	}
	void ZeroPadToMaxFrames()
	{
		const AkUInt32 uNumChannels = NumChannels();
		const AkUInt32 uNumZeroFrames = uMaxFrames - uValidFrames;
		if( uNumZeroFrames )
		{
			for( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				memset( GetChannel( i ) + uValidFrames, 0, uNumZeroFrames * sizeof( AkReal32 ) );
			}
			uValidFrames = uMaxFrames;
		}
	}
};
static_assert( sizeof( AkAudioBuffer ) == 16, "AkAudioBuffer: 2012 PDB sizeof 16" );

/**
 * AkPipelineBufferBase: AkAudioBuffer plus the cached-buffer helpers (inlined everywhere in retail: GetCachedBuffer
 * takes a block from CAkLEngine's cache keyed by size, ReleaseCachedBuffer returns it).
 */
class AkPipelineBufferBase : public AkAudioBuffer
{
public:
	AKRESULT GetCachedBuffer( AkUInt16 in_uMaxFrames, AkChannelMask in_uChannelMask );
	void ReleaseCachedBuffer();
	void SetChannelMask( AkChannelMask in_uChannelMask ) { uChannelMask = in_uChannelMask; }
	void AttachContiguousDeinterleavedData( void * in_pData, AkUInt16 in_uMaxFrames, AkUInt16 in_uValidFrames, AkChannelMask in_uChannelMask )
	{
		pData = in_pData;
		uMaxFrames = in_uMaxFrames;
		uValidFrames = in_uValidFrames;
		uChannelMask = in_uChannelMask;
	}
	void AttachInterleavedData( void * in_pData, AkUInt16 in_uMaxFrames, AkUInt16 in_uValidFrames, AkChannelMask in_uChannelMask )
	{
		pData = in_pData;
		uMaxFrames = in_uMaxFrames;
		uValidFrames = in_uValidFrames;
		uChannelMask = in_uChannelMask;
	}
	void * DetachContiguousDeinterleavedData()
	{
		void * pDataOld = pData;
		pData = NULL;
		return pDataOld;
	}
};

/** 40 bytes: + markers and the position information the pitch node forwards to the PBI. */
class AkPipelineBuffer : public AkPipelineBufferBase
{
public:
	AkPipelineBuffer()
	{
		pData = NULL;
		uChannelMask = 0;
		eState = AK_DataNeeded;
		uMaxFrames = 0;
		uValidFrames = 0;
		uNumMarkers = 0;
		pMarkers = NULL;
		posInfo.Clear();
	}

	void FreeMarkers();

	AkUInt16				uNumMarkers;
	AkBufferMarker *		pMarkers;
	AkBufferPosInformation	posInfo;
};
static_assert( sizeof( AkPipelineBuffer ) == 40, "AkPipelineBuffer: 2012 PDB sizeof 40" );

/** 32 bytes: the SIMD view of one speaker-volume set. */
struct AkSIMDSpeakerVolumes
{
	union
	{
		__m128				vector[ 2 ];
		AkSpeakerVolumes	volumes;
		AkReal32			aVolumes[ 8 ];
	};

	void Zero()
	{
		vector[ 0 ] = _mm_setzero_ps();
		vector[ 1 ] = _mm_setzero_ps();
	}
	void Set( AkReal32 in_fValue )
	{
		vector[ 0 ] = _mm_set1_ps( in_fValue );
		vector[ 1 ] = _mm_set1_ps( in_fValue );
	}
	void Copy( const AkSIMDSpeakerVolumes & in_rOther )
	{
		vector[ 0 ] = in_rOther.vector[ 0 ];
		vector[ 1 ] = in_rOther.vector[ 1 ];
	}
	void Mul( AkReal32 in_fValue )
	{
		const __m128 v = _mm_set1_ps( in_fValue );
		vector[ 0 ] = _mm_mul_ps( vector[ 0 ], v );
		vector[ 1 ] = _mm_mul_ps( vector[ 1 ], v );
	}
	void Add( const AkSIMDSpeakerVolumes & in_rOther )
	{
		vector[ 0 ] = _mm_add_ps( vector[ 0 ], in_rOther.vector[ 0 ] );
		vector[ 1 ] = _mm_add_ps( vector[ 1 ], in_rOther.vector[ 1 ] );
	}
	bool IsLessOrEqual( const AkSIMDSpeakerVolumes & in_rOther ) const
	{
		for( int i = 0; i < 8; ++i )
		{
			if( aVolumes[ i ] > in_rOther.aVolumes[ i ] )
			{
				return false;
			}
		}
		return true;
	}
};
static_assert( sizeof( AkSIMDSpeakerVolumes ) == 32, "AkSIMDSpeakerVolumes: 2012 PDB sizeof 32" );

/** 64 bytes: the volumes a buffer is mixed with at the start (Previous) and end (Next) of the frame. */
struct AkAudioMix
{
	AkSIMDSpeakerVolumes Next;
	AkSIMDSpeakerVolumes Previous;
};
static_assert( sizeof( AkAudioMix ) == 64, "AkAudioMix: 2012 PDB sizeof 64" );

/** AkPipelineBuffer + one mix per input channel. */
class AkAudioBufferMix : public AkPipelineBuffer
{
public:
	AkAudioMix AudioMix[ AK_VOICE_MAX_NUM_CHANNELS ];
};

/** The voice's buffer as it leaves the source combiner: the dry mix and the environmental (aux) mix per channel. */
class AkAudioBufferCbx : public AkAudioBufferMix
{
public:
	AkAudioMix EnvMix[ AK_VOICE_MAX_NUM_CHANNELS ];
};

/** A bus's output buffer: one mix (the bus volume ramp) for the whole buffer. */
class AkAudioBufferBus : public AkPipelineBufferBase
{
public:
	AkAudioMix AudioMix;
};

struct AkMergedEnvironmentValue
{
	AkEnvironmentValue	envValue;
	AkReal32			fLastControlValue;
};

struct AkVPLMixState
{
	AkAudioBufferCbx	buffer;
	AKRESULT			result;
};

/** The state handed down a voice's pipeline for one frame. */
struct AkVPLState : public AkVPLMixState
{
	bool						bIsEnvironmentalBus;
	bool						bAudible;
	bool						bPause;
	bool						bStop;
	AkMergedEnvironmentValue	aMergedValues[ 4 ];
};

enum VPLNodeState
{
	NodeStateInit	= 0,
	NodeStatePlay	= 1,
	NodeStateStop	= 2,
	NodeStatePause	= 3,
	NodeStateIdle	= 4
};

enum LEState
{
	LEStatePlay			= 0,
	LEStatePlayPause	= 1,
	LEStateStop			= 2,
	LEStatePause		= 3,
	LEStateResume		= 4,
	LEStateStopLooping	= 5,
	LEStateSeek			= 6
};

enum AkCtxState
{
	CtxStateStop		= 0,
	CtxStatePause		= 1,
	CtxStateResume		= 2,
	CtxStatePlay		= 3,
	CtxStateToDestroy	= 4
};

enum AkCtxDestroyReason
{
	CtxDestroyReasonFinished	= 0,
	CtxDestroyReasonPlayFailed	= 1
};

enum AkVirtualQueueBehavior
{
	AkVirtualQueueBehavior_FromBeginning	= 0,
	AkVirtualQueueBehavior_FromElapsedTime	= 1,
	AkVirtualQueueBehavior_Resume			= 2
};

enum AkBelowThresholdBehavior
{
	AkBelowThresholdBehavior_ContinueToPlay		= 0,
	AkBelowThresholdBehavior_KillVoice			= 1,
	AkBelowThresholdBehavior_SetAsVirtualVoice	= 2
};

enum AkPBIStopMode
{
	AkPBIStopMode_Normal					= 0,
	AkPBIStopMode_StopAndContinue			= 1,
	AkPBIStopMode_StopAndContinueSequel		= 2
};

enum AkPlaybackState
{
	PB_Playing	= 0,
	PB_Paused	= 1
};

/** BufferUpdateParams: the DirectSound lock of one refill. */
struct BufferUpdateParams
{
	AkUInt32	ulOffset;
	AkUInt32	ulBytes;
	void *		pvAudioPtr1;
	AkUInt32	ulAudioBytes1;
	void *		pvAudioPtr2;
	AkUInt32	ulAudioBytes2;
	AkUInt32	ulFlags;
};
