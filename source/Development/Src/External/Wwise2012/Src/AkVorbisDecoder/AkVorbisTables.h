/*=============================================================================
	AkVorbisTables.h - the decoder's constant tables, defined (generated from the retail exe) in AkVorbisTables.cpp and
	AkVorbisPackedCodebooks.cpp. Retail names and sizes from the 2012 PDB (resources/docs/symbols/globals.csv).

	DISHONORED(port): agent IH2 (wave 26).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <xmmintrin.h>

union AkVorbisFloatBits
{
	AkUInt32 u;
	float f;
};

union __declspec(align(16)) AkVorbisVectorBits
{
	AkUInt32 u[ 4 ];
	__m128 v;
};

extern const AkUInt8 g_packedCodebookData[ 71991 ];
extern const AkUInt8 * const g_packedCodebooks[ 599 ];

extern const AkUInt32 mask[ 33 ];
extern const int quant_look[ 4 ];
extern const AkUInt8 bitrev[ 16 ];
extern const float * const fFLOOR_fromdB_LOOKUP;
extern const float * const fsincos_lookup0;
extern const float * const fsincos_lookup1;
extern const float * const vwin256;
extern const float * const vwin512;
extern const float * const vwin1024;
extern const float * const vwin2048;
extern const float * const vwin4096;
extern const __m128 & vfsign1;
extern const __m128 & vfsign2;
extern const __m128 & vfsign3;
extern const __m128 & vfsign4;
extern const __m128 & vfcPI2_8;
extern const __m128 & vfcPI_3131;
extern const __m128 & vfcPI_m313m1;
extern const __m128 & vfcPI_1313;
extern const __m128 & vfcPI_m131m3;
