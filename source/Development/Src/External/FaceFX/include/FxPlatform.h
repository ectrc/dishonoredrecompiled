/*=============================================================================
	FxPlatform.h - FaceFX 1.7.3.1 base types and the ANSI platform layer.

	DISHONORED(port): agent FX1 (wave 26). Retail compiles the OC3 FaceFX SDK from source
	(v:\dishonored\unrealengine3qatest\development\external\facefx\fxsdk\inc\fxplatform.h, fxplatformansi.h);
	the types are the 2012 PDB's (FxSize = unsigned int, FxReal = float, FxBool = bool). The out-of-line copies
	retail emitted of the inline helpers:
	  FxStrcpy 0xb15070, FxItoa 0xb14f90 (FxSDK.obj), FxStrncmp 0xb06230 (FxName.obj).
	The SDK is built with the engine's /Zp4 like every UE3 module (FxActorInstance::FxAnimPlaybackInfo has its
	double at offset 12 in the PDB), so cmake/FaceFX.cmake compiles the library with /Zp4 as well.
=============================================================================*/
#ifndef FxPlatform_H__
#define FxPlatform_H__

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cwchar>
#include <new>

namespace OC3Ent
{
namespace Face
{

typedef char FxChar;
typedef unsigned char FxUChar;
typedef wchar_t FxWChar;
typedef signed char FxInt8;
typedef unsigned char FxUInt8;
typedef unsigned char FxByte;
typedef short FxInt16;
typedef unsigned short FxUInt16;
typedef int FxInt32;
typedef unsigned int FxUInt32;
typedef float FxReal;
typedef double FxDReal;
typedef bool FxBool;
typedef unsigned int FxSize;

#define FxTrue true
#define FxFalse false

static const FxSize FxInvalidIndex = static_cast<FxSize>(-1);
static const FxSize FxInvalidValue = static_cast<FxSize>(-1);

#define FxAssert(condition) ((void)0)

inline FxChar* FxStrcpy(FxChar* dest, FxSize destLen, const FxChar* src)
{
	strcpy_s(dest, destLen, src);
	return dest;
}

inline FxInt32 FxStrncmp(const FxChar* str1, const FxChar* str2, FxSize cnt)
{
	return strncmp(str1, str2, cnt);
}

inline void* FxMemcpy(void* dest, const void* src, FxSize numBytes)
{
	return memcpy(dest, src, numBytes);
}

inline void* FxMemmove(void* dest, const void* src, FxSize numBytes)
{
	return memmove(dest, src, numBytes);
}

inline void* FxMemset(void* dest, FxInt32 value, FxSize numBytes)
{
	return memset(dest, value, numBytes);
}

/** 2012 0xb14f90: the digit count comes from log10 of the magnitude as a float, as retail. */
inline FxChar* FxItoa(FxInt32 val, FxChar* buf, FxSize bufLen)
{
	FxChar buffer[68];
	memset(buffer, 0, 64);
	if( val )
	{
		FxBool needsNegative = val < 0;
		FxInt32 value = abs(val);
		FxInt32 numLen = needsNegative + static_cast<FxInt32>(log10(static_cast<FxReal>(value)));
		FxInt32 cnt = 0;
		while( value )
		{
			buffer[numLen - cnt] = static_cast<FxChar>(value % 10 + '0');
			value /= 10;
			++cnt;
		}
		if( needsNegative )
		{
			buffer[0] = '-';
		}
	}
	else
	{
		buffer[0] = '0';
	}
	return FxStrcpy(buf, bufLen, buffer);
}

} // namespace Face
} // namespace OC3Ent

#endif
