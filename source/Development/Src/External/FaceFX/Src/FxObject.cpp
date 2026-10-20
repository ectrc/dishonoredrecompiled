/*=============================================================================
	FxObject.cpp - FaceFX 1.7.3.1 object root and user-data mix-in (fxsdk\src\fxobject.cpp, FxObject.obj).

	DISHONORED(port): agent FX1 (wave 26). FX_IMPLEMENT_BASE_CLASS is AllocateStaticClassFxObject 0xb1bb70
	(FxObject, version 0, no base, 4 bytes); StaticClass 0xb14c60, ConstructObject 0xb1bcf0, GetClassDesc
	0xb1bd70, GetCurrentVersion 0xb1bda0 come from FX_DECLARE_BASE_CLASS.
=============================================================================*/
#include "FxObject.h"

namespace OC3Ent
{
namespace Face
{

FX_IMPLEMENT_BASE_CLASS(FxObject, 0)

static_assert(sizeof(FxObject) == 4, "FxObject: 2012 PDB sizeof 4");
static_assert(sizeof(FxDataContainer) == 4, "FxDataContainer: 2012 PDB sizeof 4");

/** 2012 0xb1bc00. */
FxObject::FxObject()
{
}

/** 2012 0xb1bc20. */
FxObject::~FxObject()
{
}

/** 2012 0xb1bc40. */
void FxObject::Serialize(FxArchive& arc)
{
	arc.SerializeClassVersion("FxObject");
}

/** 2012 0xb1bc60. */
FxDataContainer::FxDataContainer()
	: _userData(NULL)
{
}

/** 2012 0xb1bc80. */
FxDataContainer::FxDataContainer(const FxDataContainer& other)
	: _userData(other._userData)
{
}

/** 2012 0xb1bca0. */
FxDataContainer& FxDataContainer::operator=(const FxDataContainer& other)
{
	_userData = other._userData;
	return *this;
}

/** 2012 0xb1bcc0. */
void* FxDataContainer::GetUserData()
{
	return _userData;
}

/** 2012 0xb1bcd0. */
void FxDataContainer::SetUserData(void* userData)
{
	_userData = userData;
}

} // namespace Face
} // namespace OC3Ent
