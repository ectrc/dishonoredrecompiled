/*=============================================================================
	TimeWindow.cpp - DSP::CAkTimeWindow, Wwise 2012.1 (plugins/common/timewindow.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. The windows are evaluated in
	double and stored as float; m_fCummulativeSum is twice the half-window's energy (weighted OLA: twice the sum of the
	squared, i.e. unrooted, values), as retail. The rectangular window does not honour the weighted flag (retail neither).
=============================================================================*/
#include "TimeWindow.h"
#include <math.h>
#include <string.h>

namespace DSP
{

/** 2012 0x994460, 2013 0x991750. */
AKRESULT CAkTimeWindow::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uWindowSize, WindowType in_eWindowType, bool in_bWeighthedOLAWin, bool in_bZeroPhase )
{
	m_uWindowSize = in_uWindowSize;
	m_pfWindowData = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * ( in_uWindowSize / 2 ) );
	if( !m_pfWindowData )
	{
		return AK_InsufficientMemory;
	}
	switch( in_eWindowType )
	{
	case WINDOWTYPE_HAMMING:
		HammingWindow( in_bWeighthedOLAWin, in_bZeroPhase );
		break;
	case WINDOWTYPE_HANN:
		HannWindow( in_bWeighthedOLAWin, in_bZeroPhase );
		break;
	case WINDOWTYPE_BLACKMAN:
		BlackmanWindow( in_bWeighthedOLAWin, in_bZeroPhase );
		break;
	default:
		RectangularWindow( in_bWeighthedOLAWin, in_bZeroPhase );
		break;
	}
	return AK_Success;
}

/** 2012 0x993ec0, 2013 0x9911b0. */
void CAkTimeWindow::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pfWindowData )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pfWindowData );
		m_pfWindowData = NULL;
	}
}

/** 2012 0x993ee0, 2013 0x9911d0. */
void CAkTimeWindow::RectangularWindow( bool /*in_bWeighthedOLAWin*/, bool /*in_bZeroPhase*/ )
{
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	for( AkUInt32 i = 0; i < uHalfWindowSize; ++i )
	{
		m_pfWindowData[ i ] = 1.f;
	}
	m_fCummulativeSum = (AkReal32)m_uWindowSize;
}

/** 2012 0x993f20, 2013 0x991210. */
void CAkTimeWindow::HammingWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase )
{
	double dSum = 0.0;
	double dPhase = in_bZeroPhase ? 3.141592653589793 : 0.0;
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	const double dPhaseInc = 6.283185307179586 / ( (double)m_uWindowSize - 1.0 );
	for( AkUInt32 i = 0; i < uHalfWindowSize; ++i )
	{
		const double dWin = 0.54 - cos( dPhase ) * 0.46;
		if( in_bWeighthedOLAWin )
		{
			m_pfWindowData[ i ] = (AkReal32)sqrt( dWin );
			dSum = dSum + dWin;
		}
		else
		{
			m_pfWindowData[ i ] = (AkReal32)dWin;
			dSum = dSum + dWin * dWin;
		}
		dPhase = dPhase + dPhaseInc;
	}
	m_fCummulativeSum = (AkReal32)( dSum + dSum );
}

/** 2012 0x993ff0, 2013 0x9912e0. */
void CAkTimeWindow::HannWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase )
{
	double dSum = 0.0;
	double dPhase = in_bZeroPhase ? 3.141592653589793 : 0.0;
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	const double dPhaseInc = 6.283185307179586 / ( (double)m_uWindowSize - 1.0 );
	for( AkUInt32 i = 0; i < uHalfWindowSize; ++i )
	{
		const double dWin = ( 1.0 - cos( dPhase ) ) * 0.5;
		if( in_bWeighthedOLAWin )
		{
			m_pfWindowData[ i ] = (AkReal32)sqrt( dWin );
			dSum = dSum + dWin;
		}
		else
		{
			m_pfWindowData[ i ] = (AkReal32)dWin;
			dSum = dSum + dWin * dWin;
		}
		dPhase = dPhase + dPhaseInc;
	}
	m_fCummulativeSum = (AkReal32)( dSum + dSum );
}

/** 2012 0x9940a0, 2013 0x991390. */
void CAkTimeWindow::BlackmanWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase )
{
	double dSum = 0.0;
	double dPhase = in_bZeroPhase ? 3.141592653589793 : 0.0;
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	const double dPhaseInc = 6.283185307179586 / ( (double)m_uWindowSize - 1.0 );
	for( AkUInt32 i = 0; i < uHalfWindowSize; ++i )
	{
		const double dWin = 0.42 - cos( dPhase ) * 0.5 + cos( dPhase + dPhase ) * 0.08;
		if( in_bWeighthedOLAWin )
		{
			m_pfWindowData[ i ] = (AkReal32)sqrt( dWin );
			dSum = dSum + dWin;
		}
		else
		{
			m_pfWindowData[ i ] = (AkReal32)dWin;
			dSum = dSum + dWin * dWin;
		}
		dPhase = dPhase + dPhaseInc;
	}
	m_fCummulativeSum = (AkReal32)( dSum + dSum );
}

/** 2012 0x994190, 2013 0x991480: the first half forward, the second half mirrored. */
void CAkTimeWindow::Apply( AkReal32 * io_pfBuf, AkUInt32 /*in_uNumFrames*/, AkReal32 * in_pfWindow )
{
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	AkUInt32 i = 0;
	for( ; i < uHalfWindowSize; ++i )
	{
		io_pfBuf[ i ] = in_pfWindow[ i ] * io_pfBuf[ i ];
	}
	for( ; i < m_uWindowSize; ++i )
	{
		io_pfBuf[ i ] = io_pfBuf[ i ] * in_pfWindow[ m_uWindowSize - i - 1 ];
	}
}

/** 2012 0x9942e0, 2013 0x9915d0. */
void CAkTimeWindow::Apply( AkReal32 * io_pfBuf, AkUInt32 /*in_uNumFrames*/, AkReal32 in_fWinGain, AkReal32 * in_pfWindow )
{
	const AkUInt32 uHalfWindowSize = m_uWindowSize / 2;
	AkUInt32 i = 0;
	for( ; i < uHalfWindowSize; ++i )
	{
		io_pfBuf[ i ] = ( in_pfWindow[ i ] * io_pfBuf[ i ] ) * in_fWinGain;
	}
	for( ; i < m_uWindowSize; ++i )
	{
		io_pfBuf[ i ] = ( in_pfWindow[ m_uWindowSize - i - 1 ] * io_pfBuf[ i ] ) * in_fWinGain;
	}
}

/** 2012 0x994510, 2013 0x991800. */
void CAkTimeWindow::Apply( AkReal32 * io_pfBuf, AkUInt32 in_uNumFrames, AkReal32 in_fWinGain )
{
	if( in_fWinGain == 1.f )
	{
		Apply( io_pfBuf, in_uNumFrames, m_pfWindowData );
	}
	else
	{
		Apply( io_pfBuf, in_uNumFrames, in_fWinGain, m_pfWindowData );
	}
}

}
