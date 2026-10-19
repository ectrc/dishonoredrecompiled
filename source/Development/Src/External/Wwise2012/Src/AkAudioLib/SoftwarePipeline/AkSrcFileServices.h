/*=============================================================================
	AkSrcFileServices.h - AK::SrcFileServices (aksrcfileservices.h): the prebuffering test every streamed source makes
	before its first buffer.

	DISHONORED(port): agent IH2 fork S (wave 26). Retail compiles IsPrebufferingReady out of line (2012 0x955bb0); it is
	defined in AkSrcFileBase.cpp.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkStreamMgr.h>

namespace AK
{
	namespace SrcFileServices
	{
		AKRESULT IsPrebufferingReady( AK::IAkAutoStream * in_pStream, AkUInt32 in_uSizeLeft );
	}
}
