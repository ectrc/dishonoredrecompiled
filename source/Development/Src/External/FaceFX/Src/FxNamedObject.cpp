/*=============================================================================
	FxNamedObject.cpp - FaceFX 1.7.3.1 named object (fxsdk\src\fxnamedobject.cpp, FxNamedObject.obj).

	DISHONORED(port): agent FX1 (wave 26). AllocateStaticClass 0xb151d0 (version 0, base FxObject, 8 bytes),
	ConstructObject 0xb154e0, GetClassDesc 0xb15560, GetCurrentVersion 0xb15590.
=============================================================================*/
#include "FxNamedObject.h"

namespace OC3Ent
{
namespace Face
{

FX_IMPLEMENT_CLASS(FxNamedObject, 0, FxObject)

static_assert(sizeof(FxNamedObject) == 8, "FxNamedObject: 2012 PDB sizeof 8");

/** 2012 0xb15260. */
FxNamedObject::FxNamedObject()
	: _name(FxName::NullName)
{
}

/** 2012 0xb152d0. */
FxNamedObject::FxNamedObject(const FxNamedObject& other)
	: FxObject()
	, _name()
{
	_name = other.GetName();
}

/** 2012 0xb15350. */
FxNamedObject::FxNamedObject(const FxName& name)
	: _name(name)
{
}

/** 2012 0xb153c0. */
FxNamedObject::~FxNamedObject()
{
}

/** 2012 0xb15420. */
FxNamedObject& FxNamedObject::operator=(const FxNamedObject& other)
{
	if( this != &other )
	{
		_name = other.GetName();
	}
	return *this;
}

/** 2012 0xb15460. */
void FxNamedObject::SetName(const FxName& name)
{
	_name = name;
}

/** 2012 0xb15480. */
void FxNamedObject::Rename(const FxChar* name)
{
	_name.Rename(name);
}

/** 2012 0xb154a0. */
void FxNamedObject::Serialize(FxArchive& arc)
{
	Super::Serialize(arc);
	arc.SerializeClassVersion("FxNamedObject");
	arc << _name;
}

} // namespace Face
} // namespace OC3Ent
