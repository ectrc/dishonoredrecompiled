/*=============================================================================
	OLACircularBuffer.h - DSP::CAkOLACircularBuffer (Wwise 2012.1, plugins\common\olacircularbuffer.h). Agent IH4 (fork F2).
=============================================================================*/
#ifndef _AK_OLACIRCULARBUFFER_H_
#define _AK_OLACIRCULARBUFFER_H_

#include "CircularBuffer.h"

namespace DSP
{
	class CAkOLACircularBuffer : public CAkCircularBuffer
	{
	public:
		CAkOLACircularBuffer() : m_uWindowSize( 0 ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uSize, AkUInt32 in_uWindowSize );
		bool PushOverlappedWindow( AkReal32 * in_pfBuffer, AkUInt32 in_uHopSize );
		AkUInt32 PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, bool in_bNoMoredata );
		bool IsDoneTail();

	protected:
		bool PushOverlappedWindow( AkReal32 * in_pfBuffer, AkUInt32 in_uHopSize, AkReal32 * io_pfData );
		AkUInt32 PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, bool in_bNoMoredata, AkReal32 * io_pfData );

		AkUInt32	m_uWindowSize;
	};

}

static_assert( sizeof( DSP::CAkOLACircularBuffer ) == 24, "DSP::CAkOLACircularBuffer: 2012 PDB sizeof 24" );

#endif
