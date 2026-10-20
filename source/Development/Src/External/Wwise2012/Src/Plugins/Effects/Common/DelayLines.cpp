/*=============================================================================
	DelayLines.cpp - DSP::DelayLine (2012 0x98db90..0x98dce0), DSP::CDelayLight (0x992840..0x9929b0),
	DSP::AllpassFilter (0x98ee90..0x98f0f0); 2013 matched by bytes. Agent IH4 fork F1 (wave 26).
=============================================================================*/
#include "DelayLines.h"
#include <string.h>

namespace DSP
{
	/** 2012 0x98db90: at least 4 frames. */
	AKRESULT DelayLine::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength )
	{
		uDelayLineLength = in_uDelayLineLength < 4 ? 4 : in_uDelayLineLength;
		pfDelay = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * uDelayLineLength );
		if( !pfDelay )
		{
			return AK_InsufficientMemory;
		}
		uCurOffset = 0;
		return AK_Success;
	}

	/** 2012 0x98dbe0. */
	void DelayLine::Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		if( pfDelay )
		{
			AK_PLUGIN_FREE( in_pAllocator, pfDelay );
			pfDelay = NULL;
		}
		uDelayLineLength = 0;
	}

	/** 2012 0x98dc10. */
	void DelayLine::Reset()
	{
		if( pfDelay )
		{
			memset( pfDelay, 0, sizeof( AkReal32 ) * uDelayLineLength );
		}
	}

	/** 2012 0x98dc30: in place, the line swaps with the buffer. */
	void DelayLine::ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames )
	{
		AkUInt32 uFramesLeft = in_uNumFrames;
		while( uFramesLeft )
		{
			AkUInt32 uFrames = uDelayLineLength - uCurOffset;
			if( uFrames > uFramesLeft )
			{
				uFrames = uFramesLeft;
			}
			AkReal32 * pfLine = pfDelay + uCurOffset;
			for( AkUInt32 i = 0; i < uFrames; ++i )
			{
				const AkReal32 fOut = pfLine[ i ];
				pfLine[ i ] = io_pfBuffer[ i ];
				io_pfBuffer[ i ] = fOut;
			}
			io_pfBuffer += uFrames;
			uCurOffset += uFrames;
			if( uCurOffset == uDelayLineLength )
			{
				uCurOffset = 0;
			}
			uFramesLeft -= uFrames;
		}
	}

	/** 2012 0x98dce0. */
	void DelayLine::ProcessBuffer( AkReal32 * in_pfInBuffer, AkReal32 * out_pfOutBuffer, AkUInt32 in_uNumFrames )
	{
		AkUInt32 uFramesLeft = in_uNumFrames;
		while( uFramesLeft )
		{
			AkUInt32 uFrames = uDelayLineLength - uCurOffset;
			if( uFrames > uFramesLeft )
			{
				uFrames = uFramesLeft;
			}
			AkReal32 * pfLine = pfDelay + uCurOffset;
			for( AkUInt32 i = 0; i < uFrames; ++i )
			{
				const AkReal32 fOut = pfLine[ i ];
				pfLine[ i ] = in_pfInBuffer[ i ];
				out_pfOutBuffer[ i ] = fOut;
			}
			in_pfInBuffer += uFrames;
			out_pfOutBuffer += uFrames;
			uCurOffset += uFrames;
			if( uCurOffset == uDelayLineLength )
			{
				uCurOffset = 0;
			}
			uFramesLeft -= uFrames;
		}
	}

	/** 2012 0x992840: the length is rounded down to a multiple of 4 (no line at all below 4). */
	AKRESULT CDelayLight::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength )
	{
		m_uDelayLineLength = in_uDelayLineLength & ~3u;
		if( m_uDelayLineLength )
		{
			m_pfDelay = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * m_uDelayLineLength );
			if( !m_pfDelay )
			{
				return AK_InsufficientMemory;
			}
		}
		m_uCurrOffset = 0;
		return AK_Success;
	}

	/** 2012 0x992890. */
	void CDelayLight::Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		if( m_pfDelay )
		{
			AK_PLUGIN_FREE( in_pAllocator, m_pfDelay );
			m_pfDelay = NULL;
		}
		m_uDelayLineLength = 0;
	}

	/** 2012 0x9928c0. */
	void CDelayLight::Reset()
	{
		if( m_pfDelay )
		{
			memset( m_pfDelay, 0, sizeof( AkReal32 ) * m_uDelayLineLength );
		}
		m_uCurrOffset = 0;
	}

	/** 2012 0x9928f0: whole quads only (the frame count is a multiple of 4 in every caller). */
	void CDelayLight::ProcessBuffer( AkReal32 * io_pfInOutBuf, AkUInt32 in_uNumFrames )
	{
		if( !m_pfDelay )
		{
			return;
		}
		AkUInt32 uQuadsLeft = in_uNumFrames >> 2;
		while( uQuadsLeft )
		{
			AkUInt32 uQuads = ( m_uDelayLineLength - m_uCurrOffset ) >> 2;
			if( uQuads > uQuadsLeft )
			{
				uQuads = uQuadsLeft;
			}
			AkReal32 * pfLine = m_pfDelay + m_uCurrOffset;
			for( AkUInt32 i = 0; i < 4 * uQuads; ++i )
			{
				const AkReal32 fOut = pfLine[ i ];
				pfLine[ i ] = io_pfInOutBuf[ i ];
				io_pfInOutBuf[ i ] = fOut;
			}
			io_pfInOutBuf += 4 * uQuads;
			m_uCurrOffset += 4 * uQuads;
			if( m_uCurrOffset == m_uDelayLineLength )
			{
				m_uCurrOffset = 0;
			}
			uQuadsLeft -= uQuads;
		}
	}

	/** 2012 0x9929b0. */
	void CDelayLight::ProcessBuffer( AkReal32 * in_pfInBuf, AkReal32 * out_pfOutBuf, AkUInt32 in_uNumFrames )
	{
		if( !m_pfDelay )
		{
			return;
		}
		AkUInt32 uQuadsLeft = in_uNumFrames >> 2;
		while( uQuadsLeft )
		{
			AkUInt32 uQuads = ( m_uDelayLineLength - m_uCurrOffset ) >> 2;
			if( uQuads > uQuadsLeft )
			{
				uQuads = uQuadsLeft;
			}
			AkReal32 * pfLine = m_pfDelay + m_uCurrOffset;
			for( AkUInt32 i = 0; i < 4 * uQuads; ++i )
			{
				const AkReal32 fOut = pfLine[ i ];
				pfLine[ i ] = in_pfInBuf[ i ];
				out_pfOutBuf[ i ] = fOut;
			}
			in_pfInBuf += 4 * uQuads;
			out_pfOutBuf += 4 * uQuads;
			m_uCurrOffset += 4 * uQuads;
			if( m_uCurrOffset == m_uDelayLineLength )
			{
				m_uCurrOffset = 0;
			}
			uQuadsLeft -= uQuads;
		}
	}

	/** 2012 0x98ee90: the line holds (x, y) pairs. */
	AKRESULT AllpassFilter::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength, AkReal32 in_fG )
	{
		uDelayLineLength = in_uDelayLineLength ? in_uDelayLineLength : 1;
		pfDelay = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, 2 * sizeof( AkReal32 ) * uDelayLineLength );
		if( !pfDelay )
		{
			return AK_InsufficientMemory;
		}
		uCurOffset = 0;
		fG = in_fG;
		return AK_Success;
	}

	/** 2012 0x98eee0. */
	void AllpassFilter::Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		if( pfDelay )
		{
			AK_PLUGIN_FREE( in_pAllocator, pfDelay );
			pfDelay = NULL;
		}
		uDelayLineLength = 0;
	}

	/** 2012 0x98ef10. */
	void AllpassFilter::Reset()
	{
		if( pfDelay )
		{
			memset( pfDelay, 0, 2 * sizeof( AkReal32 ) * uDelayLineLength );
		}
	}

	/** 2012 0x98ef30: y[n] = g (x[n] - y[n-D]) + x[n-D]. */
	void AllpassFilter::ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames )
	{
		ProcessBuffer( io_pfBuffer, io_pfBuffer, in_uNumFrames );
	}

	/** 2012 0x98f0f0. */
	void AllpassFilter::ProcessBuffer( AkReal32 * in_pfInBuffer, AkReal32 * out_pfOutBuffer, AkUInt32 in_uNumFrames )
	{
		const AkReal32 fLocalG = fG;
		AkUInt32 uFramesLeft = in_uNumFrames;
		while( uFramesLeft )
		{
			AkUInt32 uFrames = uDelayLineLength - uCurOffset;
			if( uFrames > uFramesLeft )
			{
				uFrames = uFramesLeft;
			}
			AkReal32 * pfLine = pfDelay + 2 * uCurOffset;
			for( AkUInt32 i = 0; i < uFrames; ++i )
			{
				const AkReal32 fXd = pfLine[ 0 ];
				const AkReal32 fX = in_pfInBuffer[ i ];
				pfLine[ 0 ] = fX;
				const AkReal32 fY = ( fX - pfLine[ 1 ] ) * fLocalG + fXd;
				pfLine[ 1 ] = fY;
				out_pfOutBuffer[ i ] = fY;
				pfLine += 2;
			}
			in_pfInBuffer += uFrames;
			out_pfOutBuffer += uFrames;
			uCurOffset += uFrames;
			if( uCurOffset == uDelayLineLength )
			{
				uCurOffset = 0;
			}
			uFramesLeft -= uFrames;
		}
	}
}
