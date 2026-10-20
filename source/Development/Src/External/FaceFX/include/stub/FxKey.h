// SHIM (agent FX2, wave 26) - FX1 owns FxKey.h (FxAnimKey); delete with FxCoreStub.h when FX1's core merges.
#ifndef FxKey_H__
#define FxKey_H__

#include "FxCoreStub.h"

namespace OC3Ent
{
namespace Face
{

class FxAnimKey
{
public:
	FxAnimKey();
	FxAnimKey( FxReal iTime, FxReal iValue, FxReal iInSlope = 0.0f, FxReal iOutSlope = 0.0f );
	FxAnimKey( const FxAnimKey& other );
	FxAnimKey& operator=( const FxAnimKey& other );
	~FxAnimKey();

	/** 2012 0xb09080. */
	FxReal GetTime() const { return _time; }
	/** 2012 0xb09090. */
	FxReal GetValue() const { return _value; }
	/** 2012 0xb090b0. */
	FxReal GetSlopeIn() const { return _slopeIn; }
	/** 2012 0xb090d0. */
	FxReal GetSlopeOut() const { return _slopeOut; }
	void SetTime( FxReal iTime );
	void SetValue( FxReal iValue );
	void SetSlopeIn( FxReal iInSlope );
	void SetSlopeOut( FxReal iOutSlope );
	void* GetUserData();
	void SetUserData( void* userData );

	friend FxArchive& operator<<( FxArchive& arc, FxAnimKey& key );

private:
	FxReal _time;
	FxReal _value;
	FxReal _slopeIn;
	FxReal _slopeOut;
	void* _userData;
};

FxArchive& operator<<( FxArchive& arc, FxAnimKey& key );

} // namespace Face
} // namespace OC3Ent

#endif
