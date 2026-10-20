/*=============================================================================
	FxNamedObject.h - FaceFX 1.7.3.1 object with an FxName (fxnamedobject.h). 8 bytes.

	DISHONORED(port): agent FX1 (wave 26). GetNameAsCstr is the header inline retail emitted at 0x3457a0.
=============================================================================*/
#ifndef FxNamedObject_H__
#define FxNamedObject_H__

#include "FxObject.h"

namespace OC3Ent
{
namespace Face
{

class FxNamedObject : public FxObject
{
	FX_DECLARE_CLASS(FxNamedObject, FxObject)
public:
	FxNamedObject();
	FxNamedObject(const FxNamedObject& other);
	FxNamedObject(const FxName& name);
	virtual ~FxNamedObject();
	FxNamedObject& operator=(const FxNamedObject& other);

	void SetName(const FxName& name);
	void Rename(const FxChar* name);

	const FxName& GetName() const
	{
		return _name;
	}

	const FxString& GetNameAsString() const
	{
		return _name.GetAsString();
	}

	const FxChar* GetNameAsCstr() const
	{
		return _name.GetAsString().GetData();
	}

	virtual void Serialize(FxArchive& arc);

protected:
	FxName _name;
};

} // namespace Face
} // namespace OC3Ent

#endif
