/*=============================================================================
	TimeWindow.h - DSP::CAkTimeWindow (Wwise 2012.1, plugins/common/timewindow.h). Agent IH4 (fork F2).
	12 bytes as the 2012 PDB. Only the first half of the symmetric window is stored (m_uWindowSize / 2 values);
	Apply mirrors it.
=============================================================================*/
#ifndef _AK_TIMEWINDOW_H_
#define _AK_TIMEWINDOW_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

namespace DSP
{
	class CAkTimeWindow
	{
	public:
		enum WindowType
		{
			WINDOWTYPE_RECTANGULAR = 0,
			WINDOWTYPE_HAMMING,
			WINDOWTYPE_HANN,
			WINDOWTYPE_BLACKMAN
		};

		CAkTimeWindow() : m_pfWindowData( NULL ), m_uWindowSize( 0 ), m_fCummulativeSum( 0.f ) {}

		AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uWindowSize, WindowType in_eWindowType, bool in_bWeighthedOLAWin, bool in_bZeroPhase );
		void Term( AK::IAkPluginMemAlloc * in_pAllocator );
		void Apply( AkReal32 * io_pfBuf, AkUInt32 in_uNumFrames, AkReal32 in_fWinGain );
		AkReal32 GetCummulativeSum() const { return m_fCummulativeSum; }

	protected:
		void Apply( AkReal32 * io_pfBuf, AkUInt32 in_uNumFrames, AkReal32 * in_pfWindow );
		void Apply( AkReal32 * io_pfBuf, AkUInt32 in_uNumFrames, AkReal32 in_fWinGain, AkReal32 * in_pfWindow );
		void RectangularWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase );
		void HammingWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase );
		void HannWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase );
		void BlackmanWindow( bool in_bWeighthedOLAWin, bool in_bZeroPhase );

		AkReal32 *	m_pfWindowData;
		AkUInt32	m_uWindowSize;
		AkReal32	m_fCummulativeSum;
	};
}

static_assert( sizeof( DSP::CAkTimeWindow ) == 12, "DSP::CAkTimeWindow: 2012 PDB sizeof 12" );

#endif
