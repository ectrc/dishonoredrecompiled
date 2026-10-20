/*=============================================================================
	FxKey.h - FaceFX 1.7.3.1 animation key (fxkey.h). 20 bytes: time, value, slope in, slope out, user data.

	DISHONORED(port): agent FX1 (wave 26). The curve that owns the keys (FxAnimCurve) is FX3's.
=============================================================================*/
#ifndef FxKey_H__
#define FxKey_H__

#include "FxPlatform.h"

namespace OC3Ent
{
namespace Face
{

class FxArchive;

class FxAnimKey
{
public:
	FxAnimKey();
	FxAnimKey(FxReal iTime, FxReal iValue, FxReal iInSlope = 0.0f, FxReal iOutSlope = 0.0f);
	FxAnimKey(const FxAnimKey& other);
	FxAnimKey& operator=(const FxAnimKey& other);
	~FxAnimKey();

	FxReal GetTime() const
	{
		return _time;
	}

	FxReal GetValue() const
	{
		return _value;
	}

	FxReal GetSlopeIn() const
	{
		return _slopeIn;
	}

	FxReal GetSlopeOut() const
	{
		return _slopeOut;
	}

	void SetTime(FxReal iTime);
	void SetValue(FxReal iValue);
	void SetSlopeIn(FxReal iInSlope);
	void SetSlopeOut(FxReal iOutSlope);
	void* GetUserData();
	void SetUserData(void* userData);

	friend FxArchive& operator<<(FxArchive& arc, FxAnimKey& key);

protected:
	FxReal _time;
	FxReal _value;
	FxReal _slopeIn;
	FxReal _slopeOut;
	void* _userData;
};

FxArchive& operator<<(FxArchive& arc, FxAnimKey& key);

} // namespace Face
} // namespace OC3Ent

#endif
