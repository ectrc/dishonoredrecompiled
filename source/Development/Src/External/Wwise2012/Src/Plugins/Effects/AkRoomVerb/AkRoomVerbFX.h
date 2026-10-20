/*=============================================================================
	AkRoomVerbFX.h - CAkRoomVerbFX (plugins\effects\akroomverb\akroomverbfx.cpp/.h, Wwise 2012.1): the RoomVerb, an
	in-place reverb of early reflections (ERUnitDual pattern taps) and a late tail of N four-line FDN units behind four
	diffusion allpasses, with three optional tone-control biquads; the Init.bnk environments are RoomVerb share sets.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (CAkRoomVerbFX 388 bytes: CreateRoomVerbFX;
	ReverbState, ReverbUnitState 108, ToneControlsState 40).
=============================================================================*/
#pragma once

#include "AkRoomVerbFXParams.h"
#include "ERUnitDual.h"
#include "BiquadFilter.h"
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>

struct ReverbState
{
	DSP::AllpassFilter			DiffusionFilters[ 4 ];
	DSP::OnePoleZeroHPFilter	DCFilter[ 6 ];
	DSP::DelayLine				ERDelay;
	DSP::DelayLine				ReverbDelay;
	DSP::DelayLine				ERFrontBackDelay[ 2 ];
	AkFXTailHandler				FXTailHandler;
	AkUInt32					uTailLength;
	AkReal32					fReverbUnitsMixGain;
	AkUInt32					uNumReverbUnits;
	AkUInt32					uSampleRate;
	AkUInt8						uTCFilterIndex[ 4 ];
	AkUInt8						uNumERSignals;
	bool						bIsSentMode;
};

struct ReverbUnitState
{
	DSP::FDN4		ReverbUnits;
	DSP::DelayLine	RUInputDelay;
};

struct ToneControlsState
{
	DSP::BiquadFilter	Filter;
	FilterInsertType	FilterPos;
};

class CAkRoomVerbFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkRoomVerbFX();
	virtual ~CAkRoomVerbFX() {}

	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames ) { return AK_DataReady; }

private:
	AKRESULT SetupERDelay( AK::IAkPluginMemAlloc * in_pAllocator );
	AKRESULT SetupReverbDelay( AK::IAkPluginMemAlloc * in_pAllocator );
	AKRESULT SetupERFrontBackDelay( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uChannelMask );
	AKRESULT SetupERUnit( AK::IAkPluginMemAlloc * in_pAllocator );
	AKRESULT SetupToneControlFilters();
	AKRESULT SetupDiffusionAPF( AK::IAkPluginMemAlloc * in_pAllocator );
	AKRESULT SetupFDNs( AK::IAkPluginMemAlloc * in_pAllocator );
	void SetupDCFilters();
	void ComputeTCCoefs( AkUInt32 in_uFilter );
	void ComputeTCCoefs1() { ComputeTCCoefs( 0 ); }
	void ComputeTCCoefs2() { ComputeTCCoefs( 1 ); }
	void ComputeTCCoefs3() { ComputeTCCoefs( 2 ); }
	void TermToneControlFilters( AK::IAkPluginMemAlloc * in_pAllocator );
	void TermFDNs( AK::IAkPluginMemAlloc * in_pAllocator );
	void TermDiffusionAPF( AK::IAkPluginMemAlloc * in_pAllocator );
	void TermERUnit( AK::IAkPluginMemAlloc * in_pAllocator );
	void ResetFDNs();
	void ResetDiffusionAPF();
	void ResetDCFilters();
	void ResetToneControlFilters();
	void ResetERUnit();
	void ChangeDiffusion();
	void ChangeDecay();
	bool LiveParametersUpdate( AkAudioBuffer * io_pBuffer );
	void RTPCParametersUpdate();
	void WetPreProcess( AkAudioBuffer * in_pBuffer, AkReal32 * out_pfWetIn, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset );
	void ReverbPreProcess( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames );
	void ReverbPostProcess( AkReal32 ** out_ppfReverb, AkUInt32 in_uNumOutSignals, AkReal32 in_fGain, AkUInt32 in_uNumFrames );
	void ProcessFront( AkAudioBuffer * io_pBuffer, AkReal32 * in_pfTemp, AkReal32 ** io_ppfEROut, bool in_bFrontBack, AkReal32 ** out_ppfReverbOut, AkUInt32 in_uNumReverbOut, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset );
	AKRESULT ProcessSpread1Out( AkAudioBuffer * io_pBuffer );
	AKRESULT ProcessSpread2Out( AkAudioBuffer * io_pBuffer );
	AKRESULT ProcessSpread3Out( AkAudioBuffer * io_pBuffer );

	ReverbState				m_Reverb;
	ReverbUnitState *		m_pReverbUnitsState;
	ToneControlsState *		m_pTCFiltersState;
	DSP::ERUnitDual *		m_pERUnit;
	CAkRoomVerbFXParams *	m_pParams;
	AK::IAkPluginMemAlloc *	m_pAllocator;
	RTPCParams				m_PrevRTPCParams;
	InvariantParams			m_PrevInvariantParams;
};

static_assert( sizeof( ReverbUnitState ) == 108, "ReverbUnitState: 2012 sizeof 108 (SetupFDNs)" );
static_assert( sizeof( ToneControlsState ) == 40, "ToneControlsState: 2012 sizeof 40 (SetupToneControlFilters)" );
static_assert( sizeof( CAkRoomVerbFX ) == 388, "CAkRoomVerbFX: 2012 sizeof 388 (CreateRoomVerbFX)" );
