/*=============================================================================
	AkSink.cpp - CAkSink (0x947fe0..0x948220), CAkSinkDirectSound (0x95b6b0..0x95c0e0), CAkSinkDummy (0x9480d0,
	0x948170); 2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26). The dummy's Start / Play / Term / PassData / PassSilence share the empty
	bodies of the retail vtable (identical-code-folded to return-success stubs); its GetThreadWaitTime folds onto
	CAkSinkDirectSound::GetThreadWaitTime (0x948160).
=============================================================================*/
#include "AkSink.h"
#include <ks.h>
#include <ksmedia.h>
#include "AkLEngine.h"
#include "AkSinkConvert.h"
#include "AkRuntimeEnvironmentMgr.h"
#include "AkAudioLibGlobals.h"

CAkSink * g_pAkSink = NULL;
IDirectSound8 * CAkSinkDirectSound::m_pDirectSound = NULL;

/** 2012 0x9481c0. */
CAkSink * CAkSink::Create( AkPlatformInitSettings & in_settings )
{
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSinkDirectSound ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSinkDirectSound * pSink = AkPlacementNew( pMem ) CAkSinkDirectSound();
	if( pSink->Init( in_settings ) == AK_Success )
	{
		return pSink;
	}
	pSink->Term();
	pSink->~CAkSinkDirectSound();
	AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pSink );
	return NULL;
}

/** 2012 0x948220. */
CAkSink * CAkSink::CreateDummy()
{
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSinkDummy ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSinkDummy * pSink = AkPlacementNew( pMem ) CAkSinkDummy();
	pSink->Init();
	return pSink;
}

/** 2012 0x948000: 16-bit PCM at the pipeline rate; 5.1 as WAVE_FORMAT_EXTENSIBLE. */
void CAkSink::PrepareFormat( WAVEFORMATEXTENSIBLE & out_wfExt )
{
	memset( &out_wfExt, 0, sizeof( out_wfExt ) );
	const AkUInt16 uNumChannels = (AkUInt16)AkGetNumChannels( m_SpeakersConfig );
	out_wfExt.Format.nChannels = uNumChannels;
	out_wfExt.Format.nSamplesPerSec = AkAudioLibSettings::g_pipelineCoreFrequency;
	out_wfExt.Format.wBitsPerSample = 16;
	out_wfExt.Format.nBlockAlign = 2 * uNumChannels;
	out_wfExt.Format.nAvgBytesPerSec = AkAudioLibSettings::g_pipelineCoreFrequency * out_wfExt.Format.nBlockAlign;
	if( m_SpeakersConfig == AK_SPEAKER_SETUP_STEREO )
	{
		out_wfExt.Format.cbSize = 0;
		out_wfExt.Format.wFormatTag = WAVE_FORMAT_PCM;
	}
	else
	{
		out_wfExt.Format.cbSize = 22;
		out_wfExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		out_wfExt.dwChannelMask = AK_SPEAKER_SETUP_5POINT1;
		out_wfExt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		out_wfExt.Samples.wValidBitsPerSample = 16;
	}
}

/** 2012 0x95b6b0. */
CAkSinkDirectSound::CAkSinkDirectSound()
	: m_pPrimaryBuffer( NULL )
	, m_pDSBuffer( NULL )
	, m_ulBufferSize( 0 )
	, m_ulRefillOffset( 0 )
	, m_lRefillToEnd( 0 )
	, m_uFreeRefillFrames( 0 )
	, m_uPlay( 0 )
	, m_uWrite( 0 )
	, m_usBlockAlign( 0 )
	, m_bStarved( false )
{
	m_pDirectSound = NULL;
}

/** 2012 0x95bf00: DirectSound8 at DSSCL_PRIORITY on the settings window, a primary buffer in the speaker format
    (5.1 falls back to stereo when the device refuses it). */
AKRESULT CAkSinkDirectSound::Init( AkPlatformInitSettings & in_settings )
{
	if( FAILED( CoCreateInstance( CLSID_DirectSound8, NULL, CLSCTX_INPROC_SERVER, IID_IDirectSound8, (LPVOID *)&m_pDirectSound ) ) )
	{
		return AK_Fail;
	}
	if( FAILED( m_pDirectSound->Initialize( NULL ) ) )
	{
		return AK_Fail;
	}
	if( FAILED( m_pDirectSound->SetCooperativeLevel( in_settings.hWnd, DSSCL_PRIORITY ) ) )
	{
		return AK_Fail;
	}
	DSBUFFERDESC dsbd;
	memset( &dsbd, 0, sizeof( dsbd ) );
	dsbd.dwSize = sizeof( DSBUFFERDESC );
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if( FAILED( m_pDirectSound->CreateSoundBuffer( &dsbd, &m_pPrimaryBuffer, NULL ) ) )
	{
		return AK_Fail;
	}
	m_SpeakersConfig = in_settings.uChannelMask;
	if( !m_SpeakersConfig )
	{
		m_SpeakersConfig = GetDSSpeakerConfig();
	}
	if( m_SpeakersConfig != AK_SPEAKER_SETUP_5POINT1 && m_SpeakersConfig != AK_SPEAKER_SETUP_STEREO )
	{
		m_SpeakersConfig = AK_SPEAKER_SETUP_STEREO;
	}
	const AKRESULT eResult = SetPrimaryBufferFormat();
	if( m_SpeakersConfig != AK_SPEAKER_SETUP_STEREO && eResult != AK_Success )
	{
		m_SpeakersConfig = AK_SPEAKER_SETUP_STEREO;
		const AKRESULT eStereo = SetPrimaryBufferFormat();
		if( eStereo != AK_Success )
		{
			return eStereo;
		}
	}
	m_IsHardwareDevicePresent = true;
	return AK_Success;
}

/** 2012 0x95b700: 5.1 for DSSPEAKER_5POINT1 and the 7.1 / surround layouts DirectSound reports above quad. */
AkChannelMask CAkSinkDirectSound::GetDSSpeakerConfig()
{
	DWORD dwSpeakerConfig = 0;
	if( FAILED( m_pDirectSound->GetSpeakerConfig( &dwSpeakerConfig ) ) )
	{
		return AK_SPEAKER_SETUP_STEREO;
	}
	const AkUInt8 uConfig = (AkUInt8)dwSpeakerConfig;
	if( uConfig == 3 || ( uConfig > 4 && uConfig <= 9 ) )
	{
		return AK_SPEAKER_SETUP_5POINT1;
	}
	return AK_SPEAKER_SETUP_STEREO;
}

/** 2012 0x95b990: the primary format is accepted when a secondary buffer of that format can be created. */
AKRESULT CAkSinkDirectSound::SetPrimaryBufferFormat()
{
	DSBUFFERDESC desc;
	memset( &desc, 0, sizeof( desc ) );
	desc.dwBufferBytes = ( AkGetNumChannels( m_SpeakersConfig ) * g_PDSettings.uNumRefillsInVoice ) << 11;
	m_pPrimaryBuffer->Stop();
	WAVEFORMATEXTENSIBLE wfExt;
	PrepareFormat( wfExt );
	if( FAILED( m_pPrimaryBuffer->SetFormat( (const WAVEFORMATEX *)&wfExt ) ) )
	{
		return AK_Fail;
	}
	while( SUCCEEDED( m_pPrimaryBuffer->Play( 0, 0, DSBPLAY_LOOPING ) ) )
	{
		desc.dwFlags |= DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
		desc.dwSize = sizeof( DSBUFFERDESC );
		if( g_PDSettings.bGlobalFocus )
		{
			desc.dwFlags |= DSBCAPS_GLOBALFOCUS;
		}
		desc.lpwfxFormat = (WAVEFORMATEX *)&wfExt;
		IDirectSoundBuffer * pDSBuffer = NULL;
		if( SUCCEEDED( m_pDirectSound->CreateSoundBuffer( &desc, &pDSBuffer, NULL ) ) )
		{
			pDSBuffer->Release();
			return AK_Success;
		}
		if( m_SpeakersConfig != AK_SPEAKER_SETUP_5POINT1 )
		{
			return AK_NotCompatible;
		}
		m_SpeakersConfig = AK_SPEAKER_SETUP_STEREO;
		desc.dwBufferBytes = g_PDSettings.uNumRefillsInVoice << 12;
		m_pPrimaryBuffer->Stop();
		PrepareFormat( wfExt );
		if( FAILED( m_pPrimaryBuffer->SetFormat( (const WAVEFORMATEX *)&wfExt ) ) )
		{
			return AK_Fail;
		}
	}
	return AK_Fail;
}

/** 2012 0x95b7a0: the looping secondary buffer of uNumRefillsInVoice refills, zeroed. */
AKRESULT CAkSinkDirectSound::Start()
{
	DSBUFFERDESC desc;
	memset( &desc, 0, sizeof( desc ) );
	desc.dwSize = sizeof( DSBUFFERDESC );
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
	if( g_PDSettings.bGlobalFocus )
	{
		desc.dwFlags |= DSBCAPS_GLOBALFOCUS;
	}
	IDirectSoundBuffer * pDSB = NULL;
	WAVEFORMATEXTENSIBLE wfExt;
	desc.lpwfxFormat = (WAVEFORMATEX *)&wfExt;
	if( m_SpeakersConfig == AK_SPEAKER_SETUP_STEREO || m_SpeakersConfig == AK_SPEAKER_SETUP_5POINT1 )
	{
		const AkUInt16 uNumChannels = (AkUInt16)AkGetNumChannels( m_SpeakersConfig );
		desc.dwBufferBytes = ( uNumChannels * g_PDSettings.uNumRefillsInVoice ) << 11;
		m_usBlockAlign = 2 * uNumChannels;
		PrepareFormat( wfExt );
		m_ulBufferSize = desc.dwBufferBytes;
		m_lRefillToEnd = desc.dwBufferBytes;
		if( SUCCEEDED( m_pDirectSound->CreateSoundBuffer( &desc, &pDSB, NULL ) ) )
		{
			void * pData1 = NULL;
			void * pData2 = NULL;
			DWORD ulData1Bytes;
			DWORD ulData2Bytes;
			if( SUCCEEDED( pDSB->QueryInterface( IID_IDirectSoundBuffer8, (void **)&m_pDSBuffer ) )
				&& SUCCEEDED( m_pDSBuffer->Lock( 0, m_ulBufferSize, &pData1, &ulData1Bytes, &pData2, &ulData2Bytes, DSBLOCK_FROMWRITECURSOR ) ) )
			{
				memset( pData1, 0, ulData1Bytes );
				if( SUCCEEDED( m_pDSBuffer->Unlock( pData1, ulData1Bytes, pData2, ulData2Bytes ) ) )
				{
					m_pDSBuffer->SetVolume( DSBVOLUME_MAX );
					pDSB->Release();
					return AK_Success;
				}
			}
		}
	}
	if( m_pDSBuffer )
	{
		m_pDSBuffer->Release();
		m_pDSBuffer = NULL;
	}
	if( pDSB )
	{
		pDSB->Release();
	}
	return AK_Fail;
}

/** 2012 0x95b970. */
AKRESULT CAkSinkDirectSound::Play()
{
	return m_pDSBuffer->Play( 0, 0, DSBPLAY_LOOPING ) != DS_OK ? AK_Fail : AK_Success;
}

/** 2012 0x95b750. */
void CAkSinkDirectSound::Term()
{
	if( m_pDSBuffer )
	{
		m_pDSBuffer->Stop();
		m_pDSBuffer->Release();
		m_pDSBuffer = NULL;
	}
	if( m_pDirectSound )
	{
		m_pDirectSound->Release();
		m_pDirectSound = NULL;
	}
}

/** 2012 0x948160. */
AkUInt32 CAkSinkDirectSound::GetThreadWaitTime()
{
	return AkAudioLibSettings::g_pcWaitTime;
}

/** 2012 0x95be60: free frames between the refill offset and the play cursor; starved when the write cursor passed the
    refill offset since the last call. */
AKRESULT CAkSinkDirectSound::GetRefillSize( AkUInt32 & out_uRefillFrames )
{
	const AkUInt32 uOldPlay = m_uPlay;
	if( m_pDSBuffer->GetCurrentPosition( (LPDWORD)&m_uPlay, (LPDWORD)&m_uWrite ) != DS_OK )
	{
		return AK_Fail;
	}
	const AkUInt32 uRefill = m_ulRefillOffset;
	m_lRefillToEnd = m_ulBufferSize - uRefill;
	AkUInt32 uFreeBytes = m_ulBufferSize;
	if( m_uWrite + m_uPlay )
	{
		m_bStarved = ( uOldPlay < uRefill && uRefill < m_uWrite ) || ( m_uWrite < uOldPlay && uOldPlay < uRefill ) || ( uRefill < m_uWrite && m_uWrite < uOldPlay );
		uFreeBytes = uRefill <= m_uPlay ? m_uPlay - uRefill : m_uPlay + m_ulBufferSize - uRefill;
	}
	out_uRefillFrames = uFreeBytes / m_usBlockAlign;
	return AK_Success;
}

/** 2012 0x95c010. */
AKRESULT CAkSinkDirectSound::IsDataNeeded( AkUInt32 & out_uBuffersNeeded )
{
	const AKRESULT eResult = GetRefillSize( m_uFreeRefillFrames );
	if( eResult == AK_Success )
	{
		out_uBuffersNeeded = m_uFreeRefillFrames / LE_MAX_FRAMES_PER_BUFFER;
	}
	return eResult;
}

/** 2012 0x95bb00. */
AKRESULT CAkSinkDirectSound::GetBuffer( BufferUpdateParams & io_Params )
{
	return FAILED( m_pDSBuffer->Lock( io_Params.ulOffset, io_Params.ulBytes, &io_Params.pvAudioPtr1, (LPDWORD)&io_Params.ulAudioBytes1, &io_Params.pvAudioPtr2, (LPDWORD)&io_Params.ulAudioBytes2, io_Params.ulFlags ) ) ? AK_Fail : AK_Success;
}

/** 2012 0x95bb40. */
AKRESULT CAkSinkDirectSound::ReleaseBuffer( const BufferUpdateParams & in_rParams )
{
	return FAILED( m_pDSBuffer->Unlock( in_rParams.pvAudioPtr1, in_rParams.ulAudioBytes1, in_rParams.pvAudioPtr2, in_rParams.ulAudioBytes2 ) ) ? AK_Fail : AK_Success;
}

/** 2012 0x95bb80: interleaved floats to 16-bit; a stereo mix into a 5.1 device fills L/R only, a 5.1 mix into a stereo
    device folds C and the surrounds in at 0.707. */
void CAkSinkDirectSound::DoRefill( AkPipelineBufferBase & in_AudioBuffer, const BufferUpdateParams & in_rParams )
{
	AkReal32 * pData = (AkReal32 *)in_AudioBuffer.pData;
	const AkUInt32 uNumSamples = AkGetNumChannels( m_SpeakersConfig ) * LE_MAX_FRAMES_PER_BUFFER;
	AkInt16 * pOut = (AkInt16 *)in_rParams.pvAudioPtr1;
	if( in_AudioBuffer.uChannelMask == m_SpeakersConfig )
	{
		if( AK::AkRuntimeEnvironmentMgr::Instance()->GetSIMDSupport( AK::AK_SIMD_SSE2 ) )
		{
			FloatsToShortsSSE2( pOut, pData, uNumSamples );
		}
		else
		{
			FloatsToShortsMMX( pOut, pData, uNumSamples );
		}
	}
	else if( in_AudioBuffer.uChannelMask == AK_SPEAKER_SETUP_STEREO )
	{
		memset( pOut, 0, uNumSamples * sizeof( AkInt16 ) );
		for( AkUInt32 uChannel = 0; uChannel < 2; ++uChannel )
		{
			for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
			{
				AkReal32 fSample = pData[ i * 2 + uChannel ];
				fSample = fSample > 1.f ? 1.f : ( fSample < -1.f ? -1.f : fSample );
				pOut[ i * 6 + uChannel ] = (AkInt16)( fSample * 32767.f );
			}
		}
	}
	else
	{
		for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
		{
			pData[ i * 6 ] += pData[ i * 6 + 2 ] * 0.707f;
		}
		for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
		{
			pData[ i * 6 + 1 ] += pData[ i * 6 + 2 ] * 0.707f;
		}
		for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
		{
			pData[ i * 6 ] += pData[ i * 6 + 4 ] * 0.707f;
		}
		for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
		{
			pData[ i * 6 + 1 ] += pData[ i * 6 + 5 ] * 0.707f;
		}
		for( AkUInt32 uChannel = 0; uChannel < 2; ++uChannel )
		{
			for( AkUInt32 i = 0; i < LE_MAX_FRAMES_PER_BUFFER; ++i )
			{
				AkReal32 fSample = pData[ i * 6 + uChannel ];
				fSample = fSample > 1.f ? 1.f : ( fSample < -1.f ? -1.f : fSample );
				pOut[ i * 2 + uChannel ] = (AkInt16)( fSample * 32767.f );
			}
		}
	}
	m_ulRefillOffset += uNumSamples * sizeof( AkInt16 );
	if( m_ulRefillOffset >= m_ulBufferSize )
	{
		m_ulRefillOffset = 0;
	}
}

/** 2012 0x95be20. */
void CAkSinkDirectSound::DoRefillSilence( AkUInt32 in_uNumFrames, const BufferUpdateParams & in_rParams )
{
	const AkUInt32 uBytes = in_uNumFrames * m_usBlockAlign;
	memset( in_rParams.pvAudioPtr1, 0, uBytes );
	m_ulRefillOffset += uBytes;
	if( m_ulRefillOffset >= m_ulBufferSize )
	{
		m_ulRefillOffset = 0;
	}
}

/** 2012 0x95c040. */
AKRESULT CAkSinkDirectSound::PassData( AkPipelineBufferBase & io_rXfer )
{
	if( !m_pDSBuffer )
	{
		return AK_Fail;
	}
	if( !io_rXfer.uValidFrames )
	{
		return AK_Success;
	}
	BufferUpdateParams Params;
	memset( &Params, 0, sizeof( Params ) );
	Params.ulOffset = m_ulRefillOffset;
	Params.ulBytes = AkGetNumChannels( m_SpeakersConfig ) << 11;
	AKRESULT eResult = GetBuffer( Params );
	if( eResult == AK_Success )
	{
		DoRefill( io_rXfer, Params );
		eResult = ReleaseBuffer( Params );
	}
	return eResult;
}

/** 2012 0x95c0e0. */
AKRESULT CAkSinkDirectSound::PassSilence( AkUInt32 uNumFrames )
{
	if( !m_pDSBuffer )
	{
		return AK_Fail;
	}
	if( !uNumFrames )
	{
		return AK_Success;
	}
	BufferUpdateParams Params;
	memset( &Params, 0, sizeof( Params ) );
	Params.ulBytes = uNumFrames * m_usBlockAlign;
	Params.ulOffset = m_ulRefillOffset;
	AKRESULT eResult = GetBuffer( Params );
	if( eResult == AK_Success )
	{
		DoRefillSilence( uNumFrames, Params );
		eResult = ReleaseBuffer( Params );
	}
	return eResult;
}

/** 2012 0x9480d0: stereo, one buffer per 1024 frames of wall-clock time. */
AKRESULT CAkSinkDummy::Init()
{
	m_Timer = GetTickCount();
	m_SpeakersConfig = AK_SPEAKER_SETUP_STEREO;
	m_dwMSPerBuffer = (AkUInt32)( 1000.0 / ( (double)AkAudioLibSettings::g_pipelineCoreFrequency * 0.0009765625 ) );
	m_usBlockAlign = 2 * (AkUInt16)AkGetNumChannels( m_SpeakersConfig );
	return AK_Success;
}

AkUInt32 CAkSinkDummy::GetThreadWaitTime()
{
	return AkAudioLibSettings::g_pcWaitTime;
}

/** 2012 0x948170. */
AKRESULT CAkSinkDummy::IsDataNeeded( AkUInt32 & out_uBuffersNeeded )
{
	const AkUInt32 uNow = GetTickCount();
	if( uNow < m_Timer )
	{
		out_uBuffersNeeded = 1;
		m_Timer = uNow;
		return AK_Success;
	}
	out_uBuffersNeeded = ( uNow - m_Timer ) / m_dwMSPerBuffer;
	if( out_uBuffersNeeded )
	{
		m_Timer = uNow;
	}
	return AK_Success;
}
