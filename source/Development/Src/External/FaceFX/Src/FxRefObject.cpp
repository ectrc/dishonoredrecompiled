/*=============================================================================
	FxRefObject.cpp - FaceFX 1.7.3.1 reference-counted object and string (fxsdk\src\fxrefobject.cpp and
	fxrefstring.cpp, compilands FxRefObject.obj / FxRefString.obj).

	DISHONORED(port): agent FX1 (wave 26).
	  FxRefObject: AllocateStaticClass 0xb2aae0 (version 0, base FxObject, 8 bytes), ctor 0xb2ab70, dtor
	  0xb2aba0, Serialize 0xb2abc0 (writes nothing), ConstructObject 0xb2abd0, GetClassDesc 0xb2ac50,
	  GetCurrentVersion 0xb2ac80, AddReference 0xb059e0, RemoveReference 0xb05b60, GetReferenceCount 0xb07c80.
	  FxRefString: AllocateStaticClass 0xb176b0 (version 0, base FxRefObject, 20 bytes), ctors 0xb17740 /
	  0xb177b0, dtor 0xb17820, SetString 0xb17880, Serialize 0xb178a0 (writes nothing), ConstructObject 0xb178b0,
	  GetClassDesc 0xb17930, GetCurrentVersion 0xb17960, GetString 0xb05a00.
=============================================================================*/
#include "FxRefObject.h"

namespace OC3Ent
{
namespace Face
{

FX_IMPLEMENT_CLASS(FxRefObject, 0, FxObject)

static_assert(sizeof(FxRefObject) == 8, "FxRefObject: 2012 PDB sizeof 8");

/** 2012 0xb2ab70. */
FxRefObject::FxRefObject()
	: _referenceCount(0)
{
}

/** 2012 0xb2aba0. */
FxRefObject::~FxRefObject()
{
}

/** 2012 0xb2abc0. */
void FxRefObject::Serialize(FxArchive&)
{
}

FX_IMPLEMENT_CLASS(FxRefString, 0, FxRefObject)

static_assert(sizeof(FxRefString) == 20, "FxRefString: 2012 PDB sizeof 20");

/** 2012 0xb17740. */
FxRefString::FxRefString()
	: _string(static_cast<FxSize>(0))
{
}

/** 2012 0xb177b0. */
FxRefString::FxRefString(const FxChar* string)
	: _string(string)
{
}

/** 2012 0xb17820. */
FxRefString::~FxRefString()
{
}

/** 2012 0xb17880. */
void FxRefString::SetString(const FxChar* string)
{
	_string = string;
}

/** 2012 0xb178a0. */
void FxRefString::Serialize(FxArchive&)
{
}

} // namespace Face
} // namespace OC3Ent
