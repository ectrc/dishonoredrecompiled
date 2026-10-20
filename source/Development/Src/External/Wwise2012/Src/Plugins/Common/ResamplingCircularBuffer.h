/*=============================================================================
	ResamplingCircularBuffer.h - DSP::CAkResamplingCircularBuffer (Wwise 2012.1, plugins/common/resamplingcircularbuffer.h). Agent IH4 (fork F2).
=============================================================================*/
#ifndef _AK_RESAMPLINGCIRCULARBUFFER_H_
#define _AK_RESAMPLINGCIRCULARBUFFER_H_

#include "CircularBuffer.h"

namespace DSP
{
	class CAkResamplingCircularBuffer : public CAkCircularBuffer
	{
	public:
		CAkResamplingCircularBuffer() : m_fPastVal( 0.f ), m_fInterpLoc( 0.f ) {}

		void Reset();
		AkUInt32 PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 in_fResamplingFactor );

	protected:
		AkUInt32 PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 * io_pfData, AkReal32 in_fResamplingFactor );

		AkReal32	m_fPastVal;
		AkReal32	m_fInterpLoc;
	};
}

static_assert( sizeof( DSP::CAkResamplingCircularBuffer ) == 28, "DSP::CAkResamplingCircularBuffer: 2012 PDB sizeof 28" );

#endif
