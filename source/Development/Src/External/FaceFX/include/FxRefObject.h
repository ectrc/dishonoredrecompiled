/*=============================================================================
	FxRefObject.h - FaceFX 1.7.3.1 reference-counted object (fxrefobject.h) and the string the name table
	shares (fxrefstring.h).

	DISHONORED(port): agent FX1 (wave 26). RemoveReference (0xb05b60) destroys the object through its virtual
	destructor and frees GetClassDesc()->GetSize() bytes when the count reaches 0. Sizes: FxRefObject 8,
	FxRefString 20.
=============================================================================*/
#ifndef FxRefObject_H__
#define FxRefObject_H__

#include "FxObject.h"

namespace OC3Ent
{
namespace Face
{

class FxRefObject : public FxObject
{
	FX_DECLARE_CLASS(FxRefObject, FxObject)
public:
	FxRefObject();
	virtual ~FxRefObject();

	FxSize AddReference()
	{
		return ++_referenceCount;
	}

	FxSize RemoveReference()
	{
		if( --_referenceCount )
		{
			return _referenceCount;
		}
		FxSize size = GetClassDesc()->GetSize();
		this->~FxRefObject();
		FxFree(this, size);
		return 0;
	}

	FxSize GetReferenceCount() const
	{
		return _referenceCount;
	}

	virtual void Serialize(FxArchive& arc);

protected:
	FxSize _referenceCount;
};

class FxRefString : public FxRefObject
{
	FX_DECLARE_CLASS(FxRefString, FxRefObject)
public:
	FxRefString();
	FxRefString(const FxChar* string);
	virtual ~FxRefString();

	void SetString(const FxChar* string);

	const FxString& GetString() const
	{
		return _string;
	}

	virtual void Serialize(FxArchive& arc);

protected:
	FxString _string;
};

} // namespace Face
} // namespace OC3Ent

#endif
