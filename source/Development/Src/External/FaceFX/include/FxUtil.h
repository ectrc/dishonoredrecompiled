/*=============================================================================
	FxUtil.h - FaceFX 1.7.3.1 small templates (fxutil.h), math helpers (fxmath.h, fxmathansi.h) and the
	type traits the containers dispatch on (FxTypeTraits, PDB: is_arithmetic<bool/char/short/int/float/...>).

	DISHONORED(port): agent FX1 (wave 26). Out-of-line copies retail emitted:
	  FxByteSwap 0xb06e40 (FxName.obj), FxMax<unsigned int> 0xb234c0, FxSwap<unsigned int> 0x22ad40,
	  FxSquare<float> 0x331d40, FxSqrt 0x330070, FxRealEquality 0x330080, FxRealEqualityRelaxed, FxClamp<float>.
=============================================================================*/
#ifndef FxUtil_H__
#define FxUtil_H__

#include "FxPlatform.h"

namespace OC3Ent
{
namespace Face
{

static const FxReal FX_REAL_EPSILON = 1.1920929e-07f;
static const FxReal FX_REAL_RELAXED_EPSILON = 0.001f;

template<typename T> inline T FxMin(const T& a, const T& b)
{
	return b < a ? b : a;
}

template<typename T> inline T FxMax(const T& a, const T& b)
{
	return a < b ? b : a;
}

template<typename T> inline T FxClamp(const T& valMin, const T& val, const T& valMax)
{
	T a = FxMin(val, valMax);
	return FxMax(a, valMin);
}

template<typename T> inline T FxSquare(const T& a)
{
	return a * a;
}

template<typename T> inline T FxAbs(const T& a)
{
	return a < 0 ? -a : a;
}

template<typename T> inline void FxSwap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

inline void FxByteSwap(FxByte* bytes, FxSize size)
{
	FxSize i = 0;
	for( FxSize j = size - 1; i < j; --j )
	{
		bytes[i] ^= bytes[j];
		bytes[j] ^= bytes[i];
		bytes[i++] ^= bytes[j];
	}
}

inline FxReal FxSqrt(FxReal x)
{
	return sqrtf(x);
}

inline FxBool FxRealEquality(FxReal first, FxReal second)
{
	FxReal difference = first - second;
	return difference < FX_REAL_EPSILON && difference > -FX_REAL_EPSILON;
}

/** 2012 0xb0ba90 (fxmath.h:62, emitted in FxActorInstance.obj). */
inline FxBool FxDRealEquality(FxDReal first, FxDReal second)
{
	FxDReal difference = first - second;
	return difference < static_cast<FxDReal>(FX_REAL_EPSILON) && difference > -static_cast<FxDReal>(FX_REAL_EPSILON);
}

inline FxBool FxRealEqualityRelaxed(FxReal first, FxReal second)
{
	FxReal difference = first - second;
	return difference < FX_REAL_RELAXED_EPSILON && difference > -FX_REAL_RELAXED_EPSILON;
}

namespace FxTypeTraits
{
template<typename T, T v> struct constant_type
{
	enum { value = v };
};

typedef constant_type<bool, true> true_type;
typedef constant_type<bool, false> false_type;

template<typename T> struct is_arithmetic : public false_type {};
template<> struct is_arithmetic<bool> : public true_type {};
template<> struct is_arithmetic<char> : public true_type {};
template<> struct is_arithmetic<signed char> : public true_type {};
template<> struct is_arithmetic<unsigned char> : public true_type {};
template<> struct is_arithmetic<wchar_t> : public true_type {};
template<> struct is_arithmetic<short> : public true_type {};
template<> struct is_arithmetic<unsigned short> : public true_type {};
template<> struct is_arithmetic<int> : public true_type {};
template<> struct is_arithmetic<unsigned int> : public true_type {};
template<> struct is_arithmetic<long> : public true_type {};
template<> struct is_arithmetic<unsigned long> : public true_type {};
template<> struct is_arithmetic<float> : public true_type {};
template<> struct is_arithmetic<double> : public true_type {};

template<typename T> struct is_void : public false_type {};
template<> struct is_void<void> : public true_type {};
} // namespace FxTypeTraits

} // namespace Face
} // namespace OC3Ent

#endif
