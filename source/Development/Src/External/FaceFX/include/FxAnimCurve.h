// FaceFX 1.7.3.1: FxAnimCurve (fxanimcurve.h) and the key interpolators of fxkey.h. Agent FX2, wave 26.
// FxAnimKey itself is FX1's (FxKey.h); FxHermiteInterpolate / FxLinearInterpolate live here until FX1's FxKey.h has them.
#ifndef FxAnimCurve_H__
#define FxAnimCurve_H__

#include "FxFaceGraphPrerequisites.h"
#include "FxKey.h"

namespace OC3Ent
{
namespace Face
{

enum FxInterpolationType
{
	IT_First = 0,
	IT_Hermite = IT_First,
	IT_Linear,
	IT_Last = IT_Linear,
	IT_NumInterps
};


/** 2012 0xb16c20 (fxkey.h:105): the cubic Hermite between two keys with their slopes scaled by the key spacing. */
inline FxReal FxHermiteInterpolate( const FxAnimKey& first, const FxAnimKey& second, FxReal currentTime )
{
	FxReal time1 = first.GetTime();
	FxReal time2 = second.GetTime();
	FxReal deltaTime = time2 - time1;
	FxReal parametricTime = (currentTime - time1) / deltaTime;
	FxReal p0 = first.GetValue();
	FxReal p1 = second.GetValue();
	FxReal m0 = first.GetSlopeOut() * deltaTime;
	FxReal m1 = second.GetSlopeIn() * deltaTime;
	return (((p0 * 2.0f - p1 * 2.0f + m0 + m1) * parametricTime + p0 * -3.0f + p1 * 3.0f - m0 * 2.0f - m1) * parametricTime + m0) * parametricTime + p0;
}

/** 2012 0xb16ce0 (fxkey.h:125). */
inline FxReal FxLinearInterpolate( const FxAnimKey& first, const FxAnimKey& second, FxReal currentTime )
{
	FxReal time1 = first.GetTime();
	FxReal time2 = second.GetTime();
	FxReal parametricTime = (currentTime - time1) / (time2 - time1);
	FxReal p0 = first.GetValue();
	FxReal p1 = second.GetValue();
	return (p1 - p0) * parametricTime + p0;
}

class FxAnimCurve : public FxNamedObject, public FxDataContainer
{
	FX_DECLARE_CLASS(FxAnimCurve, FxNamedObject)
public:
	FxAnimCurve();
	FxAnimCurve( const FxAnimCurve& other );
	FxAnimCurve& operator=( const FxAnimCurve& other );
	virtual ~FxAnimCurve();

	FxInterpolationType GetInterpolator() const { return _interp; }
	void SetInterpolator( const FxInterpolationType& interp );
	FxSize GetNumKeys() const { return _keys.Length(); }
	/** 2012 0xb090f0. */
	const FxAnimKey& GetKey( FxSize index ) const { return _keys[index]; }
	FxSize InsertKey( const FxAnimKey& key, FxBool autocomputeSlope = FxFalse );
	void SetKeys( const FxReal* pKeys, FxSize numKeys );
	void RemoveKey( FxSize index );
	void RemoveAllKeys();
	FxSize FindKey( FxReal time ) const;
	FxSize ModifyKey( FxSize index, const FxAnimKey& key );
	FxReal EvaluateAt( FxReal time ) const;
	void FindTimeExtents( FxReal& minTime, FxReal& maxTime ) const;
	void FindValueExtents( FxReal& minValue, FxReal& maxValue ) const;

	virtual void Serialize( FxArchive& arc );

private:
	void _findBoundingKeys( FxReal time, FxAnimKey& first, FxAnimKey& second ) const;

	FxArray<FxAnimKey> _keys;
	mutable FxSize _cachedPos;
	FxInterpolationType _interp;
};

} // namespace Face
} // namespace OC3Ent

#endif
