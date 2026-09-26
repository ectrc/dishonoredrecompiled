/*=============================================================================
	NxMath.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(written): header-only helper. The PDB reports NxMath as a 1-byte type with no members
	and no virtuals, i.e. a class of static inline functions; nothing in it crosses the DLL boundary,
	so only the semantics of the names the engine uses matter. Engine uses (grep of
	source/Development/Src): sqrt, pow, sin, cos, floor, ceil, clamp, degToRad.
=============================================================================*/

#ifndef NX_MATH_H
#define NX_MATH_H

#include "Nx.h"

// DISHONORED(written): the SDK's named constants; Engine/Src/NxForceField*.cpp uses NxPi.
static const NxReal NxPi = 3.141592653589793f;
static const NxReal NxHalfPi = 1.57079632679489661923f;
static const NxReal NxTwoPi = 6.28318530717958647692f;
static const NxReal NxInvPi = 0.31830988618379067154f;

class NxMath
{
public:
	static NX_INLINE bool equals(NxF32 a, NxF32 b, NxF32 eps) { return fabsf(a - b) < eps; }
	static NX_INLINE bool equals(NxF64 a, NxF64 b, NxF64 eps) { return fabs(a - b) < eps; }
	static NX_INLINE NxF32 sign(NxF32 a) { return a >= 0.0f ? 1.0f : -1.0f; }
	static NX_INLINE NxI32 sign(NxI32 a) { return a >= 0 ? 1 : -1; }
	static NX_INLINE NxF32 abs(NxF32 a) { return fabsf(a); }
	static NX_INLINE NxF64 abs(NxF64 a) { return fabs(a); }
	static NX_INLINE NxI32 abs(NxI32 a) { return a < 0 ? -a : a; }
	static NX_INLINE NxF32 mod(NxF32 x, NxF32 y) { return fmodf(x, y); }
	static NX_INLINE NxF32 sqrt(NxF32 a) { return sqrtf(a); }
	static NX_INLINE NxF64 sqrt(NxF64 a) { return ::sqrt(a); }
	static NX_INLINE NxF32 recipSqrt(NxF32 a) { return 1.0f / sqrtf(a); }
	static NX_INLINE NxF32 square(NxF32 a) { return a * a; }
	static NX_INLINE NxF32 pow(NxF32 x, NxF32 y) { return powf(x, y); }
	static NX_INLINE NxF32 exp(NxF32 a) { return expf(a); }
	static NX_INLINE NxF32 logE(NxF32 a) { return logf(a); }
	static NX_INLINE NxF32 log2(NxF32 a) { return logf(a) / 0.693147180559945f; }
	static NX_INLINE NxF32 log10(NxF32 a) { return log10f(a); }
	static NX_INLINE NxF32 sin(NxF32 a) { return sinf(a); }
	static NX_INLINE NxF32 cos(NxF32 a) { return cosf(a); }
	static NX_INLINE NxF32 tan(NxF32 a) { return tanf(a); }
	static NX_INLINE NxF32 asin(NxF32 a) { return asinf(a < -1.0f ? -1.0f : (a > 1.0f ? 1.0f : a)); }
	static NX_INLINE NxF32 acos(NxF32 a) { return acosf(a < -1.0f ? -1.0f : (a > 1.0f ? 1.0f : a)); }
	static NX_INLINE NxF32 atan(NxF32 a) { return atanf(a); }
	static NX_INLINE NxF32 atan2(NxF32 x, NxF32 y) { return atan2f(x, y); }
	static NX_INLINE NxF32 floor(NxF32 a) { return floorf(a); }
	static NX_INLINE NxF32 ceil(NxF32 a) { return ceilf(a); }
	static NX_INLINE NxI32 trunc(NxF32 a) { return (NxI32)a; }
	static NX_INLINE NxF32 degToRad(NxF32 a) { return a * (NX_PI / 180.0f); }
	static NX_INLINE NxF32 radToDeg(NxF32 a) { return a * (180.0f / NX_PI); }
	static NX_INLINE bool isFinite(NxF32 a) { return _finite((double)a) != 0; }
	static NX_INLINE bool isFinite(NxF64 a) { return _finite(a) != 0; }
	static NX_INLINE NxF32 rand(NxF32 a, NxF32 b) { return a + (b - a) * (::rand() / (NxF32)RAND_MAX); }
	static NX_INLINE NxI32 rand(NxI32 a, NxI32 b) { return a + (::rand() % (b - a + 1)); }
	static NX_INLINE NxU32 hash(NxU32 key)
	{
		key += ~(key << 15); key ^= (key >> 10); key += (key << 3);
		key ^= (key >> 6);   key += ~(key << 11); key ^= (key >> 16);
		return key;
	}
	template<class T> static NX_INLINE T min(T a, T b) { return a < b ? a : b; }
	template<class T> static NX_INLINE T max(T a, T b) { return a > b ? a : b; }
	template<class T> static NX_INLINE T clamp(T v, T hi, T lo) { return v > hi ? hi : (v < lo ? lo : v); }
};

#endif // NX_MATH_H
