/*=============================================================================
	ERPatterns.h - AkRoomVerb::TapInfo / ERPatternInfo and the 32-entry g_ERPatternsList (2012 .rdata 0xda9348),
	the early-reflection tap patterns of the RoomVerb (data in ERPatterns.cpp). Agent IH4 fork F1 (wave 26).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <stddef.h>

namespace AkRoomVerb
{
	/** 8 bytes: time in ms, linear gain. */
	struct TapInfo
	{
		AkReal32 fTapTime;
		AkReal32 fTapGain;
	};

	/** 20 bytes. */
	struct ERPatternInfo
	{
		const wchar_t *	wszName;
		AkUInt32		uNumTapsLeft;
		const TapInfo *	TapInfoLeft;
		AkUInt32		uNumTapsRight;
		const TapInfo *	TapInfoRight;
	};

	extern const ERPatternInfo g_ERPatternsList[ 32 ];
}

static_assert( sizeof( AkRoomVerb::ERPatternInfo ) == 20, "AkRoomVerb::ERPatternInfo: 2012 PDB sizeof 20" );
