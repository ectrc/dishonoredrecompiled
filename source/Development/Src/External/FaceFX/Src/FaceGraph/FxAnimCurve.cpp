// FaceFX 1.7.3.1: FxAnimCurve (fxanimcurve.cpp). Agent FX2, wave 26, ported from the 2012 decompile.
// Since class version 1 the keys are not in the curve: FxAnim::Serialize packs every curve's keys into one block and
// hands them back through SetKeys.
#include "FxAnimCurve.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb15f40 (AllocateStaticClassFxAnimCurve). */
FX_IMPLEMENT_CLASS(FxAnimCurve, 1, FxNamedObject)

/** 2012 0xb15fd0. */
FxAnimCurve::FxAnimCurve()
	: _cachedPos(FxInvalidIndex)
	, _interp(IT_First)
{
}

/** 2012 0xb16060. */
FxAnimCurve::FxAnimCurve( const FxAnimCurve& other )
	: FxNamedObject(other)
	, FxDataContainer(other)
	, _keys(other._keys)
	, _cachedPos(FxInvalidIndex)
	, _interp(other._interp)
{
}

/** 2012 0xb16110. */
FxAnimCurve& FxAnimCurve::operator=( const FxAnimCurve& other )
{
	if( this == &other )
	{
		return *this;
	}
	FxNamedObject::operator=(other);
	FxDataContainer::operator=(other);
	_keys = other._keys;
	_cachedPos = FxInvalidIndex;
	_interp = other._interp;
	_userData = other._userData;
	return *this;
}

/** 2012 0xb161a0. */
FxAnimCurve::~FxAnimCurve()
{
}

/** 2012 0xb16200. */
void FxAnimCurve::SetInterpolator( const FxInterpolationType& interp )
{
	_interp = interp;
}

/** 2012 0xb16220: a key between two others gets the Hermite derivative there when asked. */
FxSize FxAnimCurve::InsertKey( const FxAnimKey& key, FxBool autocomputeSlope )
{
	FxSize numKeys = _keys.Length();
	FxSize index = 0;
	for( ; index < numKeys; ++index )
	{
		if( key.GetTime() < _keys[index].GetTime() )
		{
			break;
		}
	}
	if( autocomputeSlope && index && index != numKeys )
	{
		const FxAnimKey& prev = _keys[index - 1];
		const FxAnimKey& next = _keys[index];
		FxReal deltaTime = next.GetTime() - prev.GetTime();
		if( deltaTime > 0.0f )
		{
			FxReal t = (key.GetTime() - prev.GetTime()) / deltaTime;
			FxReal s0 = prev.GetSlopeOut() * deltaTime;
			FxReal s1 = next.GetSlopeIn() * deltaTime;
			FxReal slope = (prev.GetValue() * 6.0f - next.GetValue() * 6.0f + s0 * 3.0f + s1 * 3.0f) * (t * t);
			slope = (next.GetValue() * 6.0f + prev.GetValue() * -6.0f - s0 * 4.0f - s1 * 2.0f) * t + slope + s0;
			slope = slope / deltaTime;
			FxAnimKey toInsert(key);
			toInsert.SetSlopeIn(slope);
			toInsert.SetSlopeOut(slope);
			_keys.Insert(toInsert, index);
		}
	}
	else
	{
		_keys.Insert(key, index);
	}
	_cachedPos = FxInvalidIndex;
	return index;
}

/** 2012 0xb16470: numKeys keys from (time, value, slopeIn, slopeOut) quadruples. */
void FxAnimCurve::SetKeys( const FxReal* pKeys, FxSize numKeys )
{
	if( !pKeys || !numKeys )
	{
		return;
	}
	FxArray<FxAnimKey> keys(numKeys);
	FxSize keyIndex = 0;
	for( FxSize i = 0; i < numKeys; ++i )
	{
		FxReal time = pKeys[keyIndex++];
		FxReal value = pKeys[keyIndex++];
		FxReal slopeIn = pKeys[keyIndex++];
		FxReal slopeOut = pKeys[keyIndex++];
		keys.PushBack(FxAnimKey(time, value, slopeIn, slopeOut));
	}
	_keys.Swap(keys);
	_cachedPos = FxInvalidIndex;
}

/** 2012 0xb165e0. */
void FxAnimCurve::RemoveKey( FxSize index )
{
	_keys.Remove(index);
	_cachedPos = FxInvalidIndex;
}

/** 2012 0xb16610. */
void FxAnimCurve::RemoveAllKeys()
{
	_keys.Clear();
	_cachedPos = FxInvalidIndex;
}

/** 2012 0xb16630. */
FxSize FxAnimCurve::FindKey( FxReal time ) const
{
	FxSize numKeys = _keys.Length();
	for( FxSize i = 0; i < numKeys; ++i )
	{
		FxReal currTime = _keys[i].GetTime();
		if( FxRealEquality(currTime, time) )
		{
			return i;
		}
		if( time < currTime )
		{
			return FxInvalidIndex;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb166d0. */
FxSize FxAnimCurve::ModifyKey( FxSize index, const FxAnimKey& key )
{
	RemoveKey(index);
	return InsertKey(key);
}

/** 2012 0xb16700: the first key's value before it, the last key's after it, the interpolator between. */
FxReal FxAnimCurve::EvaluateAt( FxReal time ) const
{
	FxReal value = 0.0f;
	if( !_keys.Length() )
	{
		return value;
	}
	if( time <= _keys[0].GetTime() )
	{
		return _keys[0].GetValue();
	}
	if( time >= _keys.Back().GetTime() )
	{
		return _keys.Back().GetValue();
	}
	if( _keys.Length() == 1 )
	{
		return _keys[0].GetValue();
	}
	FxAnimKey thisKey;
	FxAnimKey nextKey;
	_findBoundingKeys(time, thisKey, nextKey);
	if( _interp == IT_Hermite )
	{
		value = FxHermiteInterpolate(thisKey, nextKey, time);
	}
	else if( _interp == IT_Linear )
	{
		value = FxLinearInterpolate(thisKey, nextKey, time);
	}
	return value;
}

/** 2012 0xb16890. */
void FxAnimCurve::FindTimeExtents( FxReal& minTime, FxReal& maxTime ) const
{
	minTime = FX_REAL_MAX;
	maxTime = FX_REAL_MIN;
	FxSize numKeys = GetNumKeys();
	if( numKeys )
	{
		maxTime = GetKey(numKeys - 1).GetTime();
		minTime = GetKey(0).GetTime();
	}
	if( minTime == FX_REAL_MAX && maxTime == FX_REAL_MIN )
	{
		minTime = 0.0f;
		maxTime = 0.0f;
	}
}

/** 2012 0xb16930: sampled at 60 Hz from three frames before the first key to three after the last. */
void FxAnimCurve::FindValueExtents( FxReal& minValue, FxReal& maxValue ) const
{
	minValue = FX_REAL_MAX;
	maxValue = FX_REAL_MIN;
	FxSize numKeys = GetNumKeys();
	if( numKeys )
	{
		const FxReal frame = 0.016666668f;
		FxReal startTime = GetKey(0).GetTime() - frame * 3.0f;
		FxReal endTime = GetKey(numKeys - 1).GetTime() + frame * 3.0f;
		for( FxReal time = startTime; endTime > time; time = time + frame )
		{
			FxReal value = EvaluateAt(time);
			minValue = minValue <= value ? minValue : value;
			maxValue = maxValue >= value ? maxValue : value;
		}
	}
	if( minValue == FX_REAL_MAX && maxValue == FX_REAL_MIN )
	{
		minValue = 0.0f;
		maxValue = 0.0f;
	}
}

/** 2012 0xb16a70. */
void FxAnimCurve::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxAnimCurve");
	FxInt32 tempInterp = _interp;
	arc << tempInterp;
	_interp = static_cast<FxInterpolationType>(tempInterp);
	if( arc.IsLoading() && !version )
	{
		arc << _keys;
	}
}

/** 2012 0xb16d40: scans forward from the cached key when the time has not gone back past it. */
void FxAnimCurve::_findBoundingKeys( FxReal time, FxAnimKey& first, FxAnimKey& second ) const
{
	FxSize numKeysM1 = _keys.Length() - 1;
	FxSize pos = 0;
	if( _cachedPos != FxInvalidIndex && time >= _keys[_cachedPos].GetTime() )
	{
		pos = _cachedPos;
	}
	while( pos < numKeysM1 )
	{
		if( time >= _keys[pos].GetTime() && time < _keys[pos + 1].GetTime() )
		{
			break;
		}
		++pos;
	}
	if( pos == numKeysM1 )
	{
		first = _keys[pos - 1];
		second = _keys[pos];
	}
	else
	{
		first = _keys[pos];
		second = _keys[pos + 1];
	}
	_cachedPos = pos;
}

} // namespace Face
} // namespace OC3Ent
