/*=============================================================================
	DelayLines.h - the plug-ins' delay elements (plugins\effects\common): DSP::DelayLine (delayline.cpp),
	DSP::CDelayLight (delaylinelight.cpp, a delay of a multiple of 4 frames moved 4 floats at a time) and
	DSP::AllpassFilter (allpassfilter.cpp, the Schroeder allpass whose line interleaves x and y).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (12, 12, 16 bytes). Deviation named in
	OnePoleFilter.h (no 4x unroll; CDelayLight's 16-byte moves are 4 float moves).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

namespace DSP
{
	class DelayLine
	{
	public:
		DelayLine() : uDelayLineLength( 0 ), pfDelay( NULL ), uCurOffset( 0 ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		void ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames );
		void ProcessBuffer( AkReal32 * in_pfInBuffer, AkReal32 * out_pfOutBuffer, AkUInt32 in_uNumFrames );

		AkUInt32	uDelayLineLength;
		AkReal32 *	pfDelay;
		AkUInt32	uCurOffset;
	};

	class CDelayLight
	{
	public:
		CDelayLight() : m_uDelayLineLength( 0 ), m_pfDelay( NULL ), m_uCurrOffset( 0 ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		void ProcessBuffer( AkReal32 * io_pfInOutBuf, AkUInt32 in_uNumFrames );
		void ProcessBuffer( AkReal32 * in_pfInBuf, AkReal32 * out_pfOutBuf, AkUInt32 in_uNumFrames );

		AkUInt32	m_uDelayLineLength;
		AkReal32 *	m_pfDelay;
		AkUInt32	m_uCurrOffset;
	};

	class AllpassFilter
	{
	public:
		AllpassFilter() : uDelayLineLength( 0 ), pfDelay( NULL ), uCurOffset( 0 ), fG( 0.f ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength, AkReal32 in_fG );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		void ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames );
		void ProcessBuffer( AkReal32 * in_pfInBuffer, AkReal32 * out_pfOutBuffer, AkUInt32 in_uNumFrames );

		AkUInt32	uDelayLineLength;
		AkReal32 *	pfDelay;
		AkUInt32	uCurOffset;
		AkReal32	fG;
	};
}

static_assert( sizeof( DSP::DelayLine ) == 12, "DSP::DelayLine: 2012 PDB sizeof 12" );
static_assert( sizeof( DSP::CDelayLight ) == 12, "DSP::CDelayLight: 2012 PDB sizeof 12" );
static_assert( sizeof( DSP::AllpassFilter ) == 16, "DSP::AllpassFilter: 2012 PDB sizeof 16" );
