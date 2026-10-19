/*=============================================================================
	AkSink.h - the audio output (aksink.cpp, aksinkdirectsound.cpp): CAkSink, CAkSinkDirectSound (the one retail creates)
	and CAkSinkDummy (the timer-paced fallback when no device opens). Layouts are the 2012 PDB's (DirectSound sink 0x30
	bytes, dummy 0x18).

	DISHONORED(port): agent IH2 (wave 26). No deviation in the sinks themselves; the cooperative-level window and the
	global-focus flag they read are the glue's (see akaudiodevice.cpp).
=============================================================================*/
#pragma once

#include <windows.h>
#include <mmreg.h>
#include <dsound.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include "AkLEngineStructs.h"

class CAkSink
{
public:
	static CAkSink * Create( AkPlatformInitSettings & in_settings );
	static CAkSink * CreateDummy();

	CAkSink() : m_SpeakersConfig( 0 ), m_IsHardwareDevicePresent( false ) {}
	virtual ~CAkSink() {}
	virtual AKRESULT Start() = 0;
	virtual AKRESULT Play() = 0;
	virtual void Term() = 0;
	virtual AkUInt32 GetThreadWaitTime() = 0;
	virtual bool IsStarved() = 0;
	virtual void ResetStarved() = 0;
	virtual AKRESULT IsDataNeeded( AkUInt32 & out_uBuffersNeeded ) = 0;
	virtual AKRESULT PassData( AkPipelineBufferBase & io_rXfer ) = 0;
	virtual AKRESULT PassSilence( AkUInt32 uNumFrames ) = 0;
	virtual IDirectSound8 * GetDirectSoundInstance() = 0;

	AkChannelMask	m_SpeakersConfig;
	bool			m_IsHardwareDevicePresent;

protected:
	void PrepareFormat( WAVEFORMATEXTENSIBLE & out_wfExt );
};

class CAkSinkDirectSound : public CAkSink
{
public:
	CAkSinkDirectSound();

	AKRESULT Init( AkPlatformInitSettings & in_settings );
	virtual AKRESULT Start();
	virtual AKRESULT Play();
	virtual void Term();
	virtual AkUInt32 GetThreadWaitTime();
	virtual bool IsStarved() { return m_bStarved; }
	virtual void ResetStarved() { m_bStarved = false; }
	virtual AKRESULT IsDataNeeded( AkUInt32 & out_uBuffersNeeded );
	virtual AKRESULT PassData( AkPipelineBufferBase & io_rXfer );
	virtual AKRESULT PassSilence( AkUInt32 uNumFrames );
	virtual IDirectSound8 * GetDirectSoundInstance() { return m_pDirectSound; }

	static IDirectSound8 * m_pDirectSound;

	IDirectSoundBuffer *	m_pPrimaryBuffer;
	IDirectSoundBuffer8 *	m_pDSBuffer;
	AkUInt32				m_ulBufferSize;
	AkUInt32				m_ulRefillOffset;
	AkInt32					m_lRefillToEnd;
	AkUInt32				m_uFreeRefillFrames;
	AkUInt32				m_uPlay;
	AkUInt32				m_uWrite;
	AkUInt16				m_usBlockAlign;
	bool					m_bStarved;

private:
	AkChannelMask GetDSSpeakerConfig();
	AKRESULT SetPrimaryBufferFormat();
	AKRESULT GetRefillSize( AkUInt32 & out_uRefillFrames );
	AKRESULT GetBuffer( BufferUpdateParams & io_Params );
	AKRESULT ReleaseBuffer( const BufferUpdateParams & in_rParams );
	void DoRefill( AkPipelineBufferBase & in_AudioBuffer, const BufferUpdateParams & in_rParams );
	void DoRefillSilence( AkUInt32 in_uNumFrames, const BufferUpdateParams & in_rParams );
};

class CAkSinkDummy : public CAkSink
{
public:
	AKRESULT Init();
	virtual AKRESULT Start() { return AK_Success; }
	virtual AKRESULT Play() { return AK_Success; }
	virtual void Term() {}
	virtual AkUInt32 GetThreadWaitTime();
	virtual bool IsStarved() { return false; }
	virtual void ResetStarved() {}
	virtual AKRESULT IsDataNeeded( AkUInt32 & out_uBuffersNeeded );
	virtual AKRESULT PassData( AkPipelineBufferBase & io_rXfer ) { return AK_Success; }
	virtual AKRESULT PassSilence( AkUInt32 uNumFrames ) { return AK_Success; }
	virtual IDirectSound8 * GetDirectSoundInstance() { return NULL; }

	AkUInt32	m_Timer;
	AkUInt32	m_dwMSPerBuffer;
	AkUInt16	m_usBlockAlign;
};

extern CAkSink * g_pAkSink;
