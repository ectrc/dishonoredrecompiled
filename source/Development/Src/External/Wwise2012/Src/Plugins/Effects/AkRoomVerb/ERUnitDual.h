/*=============================================================================
	ERUnitDual.h - DSP::ERUnitDual (plugins\effects\akroomverb\erunitdual.cpp/.h): the RoomVerb's early reflections, one
	delay line read by up to 64 left and 64 right taps of a pattern (AkRoomVerb::g_ERPatternsList), scaled by room size.
	DSP::FDN4 (fdn4.cpp) and FDNLPFilter (fdnlpfilter.h): a 4-line feedback delay network with a Householder matrix and
	one-pole damping per line.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (ERUnitDual 788 bytes, FDN4 96, FDNLPFilter
	12). Deviation (named): ERUnitDual::ProcessBuffer reads its taps one frame at a time where retail reads 4 frames per
	SSE step; with every tap offset a multiple of 4 (Init rounds them) the sums are the same, in the same tap order.
=============================================================================*/
#pragma once

#include "DelayLines.h"
#include "OnePoleFilter.h"
#include "ERPatterns.h"

class FDNLPFilter : public DSP::OnePoleFilter
{
public:
	void ComputeFDNLPCoefs( AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate, AkUInt32 in_uFDNDelayLength );
};

namespace DSP
{
	class ERUnitDual
	{
	public:
		ERUnitDual();

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 in_fRoomSizeScale, const AkRoomVerb::TapInfo * in_pTapInfoLeft, const AkRoomVerb::TapInfo * in_pTapInfoRight,
			AkUInt32 in_uNumTapsL, AkUInt32 in_uNumTapsR, AkUInt32 in_uSampleRate );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		void ProcessBuffer( AkReal32 * in_pfInput, AkReal32 * out_pfEROutputL, AkReal32 * out_pfEROutputR, AkUInt32 in_uNumFrames );

		AkUInt32	uDelayLineLength;
		AkReal32 *	pfDelay;
		AkUInt32	uWriteOffset;
		AkReal32	fTapGainsL[ 64 ];
		AkReal32	fTapGainsR[ 64 ];
		AkUInt16	uTapOffsetsL[ 64 ];
		AkUInt16	uTapOffsetsR[ 64 ];
		AkUInt16	uIndexToNextWrappingTapL;
		AkUInt16	uIndexToNextWrappingTapR;
		AkUInt16	uNumTapsL;
		AkUInt16	uNumTapsR;
	};

	class FDN4
	{
	public:
		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 * in_uDelayLineLength, AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		void ChangeDecay( AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate );
		void ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer, AkUInt32 in_uNumFrames );
		void ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer1, AkReal32 * io_pfOutBuffer2, AkUInt32 in_uNumFrames );
		void ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer1, AkReal32 * io_pfOutBuffer2, AkReal32 * io_pfOutBuffer3, AkUInt32 in_uNumFrames );

		DelayLine	FDNDelayLine[ 4 ];
		FDNLPFilter	delayLowPassFilter[ 4 ];

	protected:
		static AkReal32 ComputeMaxStableHFRatio( AkUInt32 in_uDelayLineLength, AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate );
		void ReadLines( AkReal32 out_fLines[ 4 ] );
		void WriteLines( const AkReal32 in_fLines[ 4 ], AkReal32 in_fIn );
	};
}

static_assert( sizeof( DSP::ERUnitDual ) == 788, "DSP::ERUnitDual: 2012 sizeof 788 (CAkRoomVerbFX::SetupERUnit)" );
static_assert( sizeof( DSP::FDN4 ) == 96, "DSP::FDN4: 2012 PDB sizeof 96" );
