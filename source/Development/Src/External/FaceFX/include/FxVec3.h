/*=============================================================================
	FxVec3.h / FxQuat - FaceFX 1.7.3.1 vector and quaternion (fxvec3.h, fxquat.h).

	DISHONORED(port): agent FX1 (wave 26). FxVec3 {x,y,z} 12 bytes, FxQuat {w,x,y,z} 16 bytes, identity
	quaternion by default (0x330100). Out of line in retail: FxVec3::SetLength 0xb2b310, Length 0xb2b3b0,
	operator<<(FxArchive&, FxVec3&) 0xb2b380, operator<<(FxArchive&, FxQuat&) 0xb24f70 (w, x, y, z),
	FxQuat::Normalize 0x334560. The math the face graph and the bone blending use is added by FX2/FX3 next to
	these.
=============================================================================*/
#ifndef FxVec3_H__
#define FxVec3_H__

#include "FxUtil.h"

namespace OC3Ent
{
namespace Face
{

class FxArchive;

class FxVec3
{
public:
	FxVec3()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
	{
	}

	FxVec3(FxReal iX, FxReal iY, FxReal iZ)
		: x(iX)
		, y(iY)
		, z(iZ)
	{
	}

	FxReal Length() const;
	void SetLength(FxReal length);

	FxReal x;
	FxReal y;
	FxReal z;
};

FxArchive& operator<<(FxArchive& arc, FxVec3& vec);

class FxQuat
{
public:
	FxQuat()
		: w(1.0f)
		, x(0.0f)
		, y(0.0f)
		, z(0.0f)
	{
	}

	FxQuat(FxReal iW, FxReal iX, FxReal iY, FxReal iZ)
		: w(iW)
		, x(iX)
		, y(iY)
		, z(iZ)
	{
	}

	const FxQuat& Normalize()
	{
		FxReal norm = x * x + w * w + y * y + z * z;
		if( FxRealEquality(norm, 1.0f) )
		{
			return *this;
		}
		FxReal length = FxSqrt(norm);
		if( length >= FX_REAL_EPSILON )
		{
			FxReal inverse = 1.0f / length;
			x = inverse * x;
			y = y * inverse;
			z = z * inverse;
			w = w * inverse;
		}
		else
		{
			w = 1.0f;
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		return *this;
	}

	FxReal w;
	FxReal x;
	FxReal y;
	FxReal z;
};

FxArchive& operator<<(FxArchive& arc, FxQuat& quat);

} // namespace Face
} // namespace OC3Ent

#endif
