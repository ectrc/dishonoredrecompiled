/*=============================================================================
	FxClass.cpp - FaceFX 1.7.3.1 class descriptors (fxsdk\src\fxclass.cpp, compiland FxClass.obj).

	DISHONORED(port): agent FX1 (wave 26). The constructor looks the name up first (the result fed an assert
	the retail build compiles out) and registers unconditionally.
=============================================================================*/
#include "FxObject.h"

namespace OC3Ent
{
namespace Face
{

FxArray<FxClass*>* FxClass::_classTable = NULL;

static_assert(sizeof(FxClass) == 24, "FxClass: 2012 PDB sizeof 24");

/** 2012 0xb17a00. */
FxClass::FxClass(const FxChar* className, FxUInt16 currentVersion, const FxClass* pBaseClassDesc, FxSize classSize, FxObject* (*constructObjectFn)())
	: _pBaseClassDesc(pBaseClassDesc)
	, _constructObjectFn(constructObjectFn)
	, _name(className)
	, _numChildren(0)
	, _size(classSize)
	, _currentVersion(currentVersion)
{
	const FxClass* duplicateClass = FindClassDesc(FxName(className));
	(void)duplicateClass;
	if( _pBaseClassDesc )
	{
		++_pBaseClassDesc->_numChildren;
	}
	_classTable->PushBack(this);
}

/** 2012 0xb17af0. */
FxClass::~FxClass()
{
}

/** 2012 0xb17b10. */
void FxClass::Startup()
{
	if( !_classTable )
	{
		_classTable = static_cast<FxArray<FxClass*>*>(FxAllocate(sizeof(FxArray<FxClass*>)));
		FxDefaultConstruct(_classTable);
		_classTable->Reserve(64);
	}
}

/** 2012 0xb17b50. */
void FxClass::Shutdown()
{
	if( _classTable )
	{
		FxSize numClasses = _classTable->Length();
		for( FxSize i = 0; i < numClasses; ++i )
		{
			FxDelete((*_classTable)[i]);
		}
		_classTable->Clear();
		FxDelete(_classTable);
	}
}

/** 2012 0xb17bc0. */
const FxName& FxClass::GetName() const
{
	return _name;
}

/** 2012 0xb17be0. */
FxUInt16 FxClass::GetCurrentVersion() const
{
	return _currentVersion;
}

/** 2012 0xb17c00. */
const FxClass* FxClass::GetBaseClassDesc() const
{
	return _pBaseClassDesc;
}

/** 2012 0xb17c10. */
FxSize FxClass::GetSize() const
{
	return _size;
}

/** 2012 0xb17c30. */
FxSize FxClass::GetNumChildren() const
{
	return _numChildren;
}

/** 2012 0xb17c50. */
FxObject* FxClass::ConstructObject() const
{
	if( _constructObjectFn )
	{
		return _constructObjectFn();
	}
	return NULL;
}

/** 2012 0xb17c70. */
const FxClass* FxClass::FindClassDesc(const FxName& className)
{
	if( _classTable )
	{
		FxSize numClasses = _classTable->Length();
		for( FxSize i = 0; i < numClasses; ++i )
		{
			if( (*_classTable)[i]->GetName() == className )
			{
				return (*_classTable)[i];
			}
		}
	}
	return NULL;
}

/** 2012 0xb17cf0. */
FxSize FxClass::GetNumClasses()
{
	if( _classTable )
	{
		return _classTable->Length();
	}
	return 0;
}

/** 2012 0xb17d10. */
const FxClass* FxClass::GetClass(FxSize index)
{
	if( _classTable )
	{
		return (*_classTable)[index];
	}
	return NULL;
}

/** 2012 0xb17d40. */
FxBool FxClass::IsExactKindOf(const FxClass* classDesc) const
{
	return IsA(classDesc);
}

/** 2012 0xb17d60. */
FxBool FxClass::IsKindOf(const FxClass* classDesc) const
{
	for( const FxClass* pClass = this; pClass; pClass = pClass->GetBaseClassDesc() )
	{
		if( pClass == classDesc )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

/** 2012 0xb17da0. */
FxBool FxClass::IsA(const FxClass* classDesc) const
{
	return this == classDesc;
}

} // namespace Face
} // namespace OC3Ent
