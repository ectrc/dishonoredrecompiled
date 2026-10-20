// SHIM (agent FX2, wave 26) - FX1 owns FxVec3.h / FxQuat.h / FxBone.h; delete with FxCoreStub.h when FX1's core merges.
// FaceFX 1.7.3.1 FxVec3, FxQuat, FxBone ported from the 2012 decompile (FxQuat::Normalize 2012 0x334560 and the
// operators are the engine-compiled header copies), one header where the SDK has three.
#ifndef FxBone_H__
#define FxBone_H__

#include "FxFaceGraphPrerequisites.h"

namespace OC3Ent
{
namespace Face
{

class FxVec3
{
public:
	/** 2012 0x330160. */
	FxVec3() : x(0.0f), y(0.0f), z(0.0f) {}
	/** 2012 0x330180. */
	FxVec3( FxReal iX, FxReal iY, FxReal iZ ) : x(iX), y(iY), z(iZ) {}

	/** 2012 0xb0f3c0. */
	FxVec3 operator+( const FxVec3& rhs ) const { return FxVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	/** 2012 0xb0f120. */
	FxVec3 operator-( const FxVec3& rhs ) const { return FxVec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	/** 2012 0xb2b3b0. */
	FxReal Length() const { return FxSqrt(FxSquare(x) + FxSquare(y) + FxSquare(z)); }
	void SetLength( FxReal length );

	FxReal x;
	FxReal y;
	FxReal z;
};

FxArchive& operator<<( FxArchive& arc, FxVec3& vec );

class FxQuat
{
public:
	/** 2012 0x330100. */
	FxQuat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
	/** 2012 0x330130. */
	FxQuat( FxReal iW, FxReal iX, FxReal iY, FxReal iZ ) : w(iW), x(iX), y(iY), z(iZ) {}

	/** 2012 0xb0efa0. */
	FxQuat GetInverse() const { return FxQuat(w, -x, -y, -z); }

	/** 2012 0x334560. */
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
			x *= inverse;
			y *= inverse;
			z *= inverse;
			w *= inverse;
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

FxArchive& operator<<( FxArchive& arc, FxQuat& quat );

class FxBone : public FxNamedObject
{
	FX_DECLARE_CLASS(FxBone, FxNamedObject)
public:
	FxBone();
	FxBone( const FxName& name, const FxVec3& iPos, const FxQuat& iRot, const FxVec3& iScale );
	/** 2012 0xb0eff0. */
	FxBone( const FxBone& other ) : FxNamedObject(other), _pos(other._pos), _rot(other._rot), _scale(other._scale) {}
	/** 2012 0xb0f470. */
	FxBone& operator=( const FxBone& other )
	{
		FxNamedObject::operator=(other);
		_pos = other._pos;
		_rot = other._rot;
		_scale = other._scale;
		return *this;
	}
	virtual ~FxBone();

	/** 2012 0xb0f5e0. */
	FxBool operator==( const FxBone& other ) const
	{
		return GetName() == other.GetName()
			&& FxRealEqualityRelaxed(_pos.x, other._pos.x) && FxRealEqualityRelaxed(_pos.y, other._pos.y) && FxRealEqualityRelaxed(_pos.z, other._pos.z)
			&& FxRealEqualityRelaxed(_rot.w, other._rot.w) && FxRealEqualityRelaxed(_rot.x, other._rot.x)
			&& FxRealEqualityRelaxed(_rot.y, other._rot.y) && FxRealEqualityRelaxed(_rot.z, other._rot.z)
			&& FxRealEqualityRelaxed(_scale.x, other._scale.x) && FxRealEqualityRelaxed(_scale.y, other._scale.y) && FxRealEqualityRelaxed(_scale.z, other._scale.z);
	}

	const FxVec3& GetPos() const { return _pos; }
	const FxQuat& GetRot() const { return _rot; }
	const FxVec3& GetScale() const { return _scale; }
	void SetPos( const FxVec3& iPos );
	void SetRot( const FxQuat& iRot );
	void SetScale( const FxVec3& iScale );

	virtual void Serialize( FxArchive& arc );

protected:
	FxVec3 _pos;
	FxQuat _rot;
	FxVec3 _scale;
};

} // namespace Face
} // namespace OC3Ent

#endif
