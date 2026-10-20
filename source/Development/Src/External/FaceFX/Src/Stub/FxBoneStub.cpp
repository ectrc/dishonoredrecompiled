// SHIM (agent FX2, wave 26) - FX1 owns these (its Src/FxBone.cpp); delete with FxCoreStub.h when FX1's core merges.
// FaceFX 1.7.3.1: FxVec3 (fxvec3.cpp), FxQuat (fxquat.cpp), FxBone (fxbone.cpp), FxAnimKey (fxkey.cpp), ported from the
// 2012 decompile.
#include "FxBone.h"
#include "FxKey.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb2b310. */
void FxVec3::SetLength( FxReal length )
{
	FxReal tempLength = Length();
	if( length != tempLength && tempLength > 0.0f )
	{
		FxReal r = length / tempLength;
		x *= r;
		y *= r;
		z *= r;
	}
}

/** 2012 0xb2b380. */
FxArchive& operator<<( FxArchive& arc, FxVec3& vec )
{
	return arc << vec.x << vec.y << vec.z;
}

/** 2012 0xb24f70. */
FxArchive& operator<<( FxArchive& arc, FxQuat& quat )
{
	return arc << quat.w << quat.x << quat.y << quat.z;
}

/** 2012 0xb234f0 (AllocateStaticClassFxBone), 0xb23790 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxBone, 0, FxNamedObject)

/** 2012 0xb23580. */
FxBone::FxBone()
{
}

/** 2012 0xb23600. */
FxBone::FxBone( const FxName& name, const FxVec3& iPos, const FxQuat& iRot, const FxVec3& iScale )
	: FxNamedObject(name)
	, _pos(iPos)
	, _rot(iRot)
	, _scale(iScale)
{
}

/** 2012 0xb23680. */
FxBone::~FxBone()
{
}

/** 2012 0xb236a0. */
void FxBone::SetPos( const FxVec3& iPos )
{
	_pos = iPos;
}

/** 2012 0xb236d0. */
void FxBone::SetRot( const FxQuat& iRot )
{
	_rot = iRot;
}

/** 2012 0xb23700. */
void FxBone::SetScale( const FxVec3& iScale )
{
	_scale = iScale;
}

/** 2012 0xb23730. */
void FxBone::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	arc.SerializeClassVersion("FxBone");
	arc << _pos << _rot << _scale;
}

/** 2012 0xb2ace0. */
FxAnimKey::FxAnimKey()
	: _time(0.0f)
	, _value(0.0f)
	, _slopeIn(0.0f)
	, _slopeOut(0.0f)
	, _userData(0)
{
}

/** 2012 0xb2ad20. */
FxAnimKey::FxAnimKey( FxReal iTime, FxReal iValue, FxReal iInSlope, FxReal iOutSlope )
	: _time(iTime)
	, _value(iValue)
	, _slopeIn(iInSlope)
	, _slopeOut(iOutSlope)
	, _userData(0)
{
}

/** 2012 0xb2ad60. */
FxAnimKey::FxAnimKey( const FxAnimKey& other )
	: _time(other._time)
	, _value(other._value)
	, _slopeIn(other._slopeIn)
	, _slopeOut(other._slopeOut)
	, _userData(other._userData)
{
}

/** 2012 0xb2adb0. */
FxAnimKey& FxAnimKey::operator=( const FxAnimKey& other )
{
	if( this != &other )
	{
		_time = other._time;
		_value = other._value;
		_slopeIn = other._slopeIn;
		_slopeOut = other._slopeOut;
		_userData = other._userData;
	}
	return *this;
}

/** 2012 0xb2ae10. */
FxAnimKey::~FxAnimKey()
{
}

/** 2012 0xb2ae20. */
void FxAnimKey::SetTime( FxReal iTime )
{
	_time = iTime;
}

/** 2012 0xb2ae40. */
void FxAnimKey::SetValue( FxReal iValue )
{
	_value = iValue;
}

/** 2012 0xb2ae60. */
void FxAnimKey::SetSlopeIn( FxReal iInSlope )
{
	_slopeIn = iInSlope;
}

/** 2012 0xb2ae80. */
void FxAnimKey::SetSlopeOut( FxReal iOutSlope )
{
	_slopeOut = iOutSlope;
}

/** 2012 0xb2aea0. */
void* FxAnimKey::GetUserData()
{
	return _userData;
}

/** 2012 0xb2aec0. */
void FxAnimKey::SetUserData( void* userData )
{
	_userData = userData;
}

/** 2012 0xb2aee0: a 16-bit version word, then time, value and the slopes. */
FxArchive& operator<<( FxArchive& arc, FxAnimKey& key )
{
	FxUInt16 version = 0;
	arc << version;
	arc << key._time << key._value << key._slopeIn << key._slopeOut;
	return arc;
}

} // namespace Face
} // namespace OC3Ent
