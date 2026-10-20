/*=============================================================================
	CircularBuffer.h - DSP::CAkCircularBuffer (Wwise 2012.1, plugins\common\circularbuffer.h). Agent IH4 (fork F2).
	20 bytes as the 2012 PDB. The protected overloads take the data pointer explicitly, as retail's public wrappers
	pass m_pfData.
=============================================================================*/
#ifndef _AK_CIRCULARBUFFER_H_
#define _AK_CIRCULARBUFFER_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

namespace DSP
{
	class CAkCircularBuffer
	{
	public:
		CAkCircularBuffer() : m_uSize( 0 ), m_uWriteOffset( 0 ), m_uReadOffset( 0 ), m_uFramesReady( 0 ), m_pfData( NULL ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uSize );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Reset();
		AkUInt32 AdvanceFrames( AkUInt32 in_uNumFrames );
		AkUInt32 PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames );
		AkUInt32 PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames );
		bool ReadFrameBlock( AkReal32 * out_pfBuffer, AkUInt32 in_uBlockSize, bool in_bNoMoreInputData );
		AkUInt32 FramesReady() const { return m_uFramesReady; }
		AkUInt32 FramesEmpty() const { return m_uSize - m_uFramesReady; }
		/** The resampling vocoder's ResetInputFill writes m_uFramesReady = m_uSize inline (0x9921e0). */
		void MarkFull() { m_uFramesReady = m_uSize; }

	protected:
		AkUInt32 PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 * io_pfData );
		AkUInt32 PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, AkReal32 * io_pfData );
		bool ReadFrameBlock( AkReal32 * out_pfBuffer, AkUInt32 in_uBlockSize, bool in_bNoMoreInputData, AkReal32 * io_pfData );

		AkUInt32	m_uSize;
		AkUInt32	m_uWriteOffset;
		AkUInt32	m_uReadOffset;
		AkUInt32	m_uFramesReady;
		AkReal32 *	m_pfData;
	};

}

static_assert( sizeof( DSP::CAkCircularBuffer ) == 20, "DSP::CAkCircularBuffer: 2012 PDB sizeof 20" );

#endif
