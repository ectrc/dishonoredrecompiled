/*=============================================================================
	FxObject.h - FaceFX 1.7.3.1 object root, user-data mix-in and the class macros (fxobject.h).

	DISHONORED(port): agent FX1 (wave 26). The FxObject vtable (PDB FxObject_vtbl): GetClassDesc,
	GetCurrentVersion, the destructor, Serialize - FX_DECLARE_CLASS must therefore open every class body,
	before the destructor. What the macro generates matches the per-class functions retail emitted, e.g. for
	FxBone: StaticClass 0xb14cc0, AllocateStaticClassFxBone 0xb234f0 (FX_IMPLEMENT_CLASS), ConstructObject
	0xb23790, GetClassDesc / GetCurrentVersion (header inline), and the two archive operators
	(operator<<(FxArchive&, FxFaceGraph&) 0xb0d6a0 calls Serialize; operator<<(FxArchive&, FxFaceGraphNode*&)
	0xb21920 is the object-pointer protocol below).
	Object pointers are written as an FxSize index into the archive object table; the first time an object
	is written its class name (an FxName, i.e. an archive name index) and class size follow, then the object.
	Before sdk 0x640 the class name was a plain string.
	IsA / IsKindOf / IsExactKindOf and FxCast<T> are the fxobject.h inlines (no out-of-line copy in the PDB; FX2's
	graph casts with FxCast as retail's FxCompiledFaceGraph/FxFaceGraph do).
=============================================================================*/
#ifndef FxObject_H__
#define FxObject_H__

#include "FxClass.h"
#include "FxArchive.h"

namespace OC3Ent
{
namespace Face
{

#define FX_DECLARE_CLASS_COMMON(classname) \
public: \
	static FxClass* AllocateStaticClass##classname(); \
	static const FxClass* StaticClass() \
	{ \
		if( !_pClassDesc ) \
		{ \
			_pClassDesc = AllocateStaticClass##classname(); \
		} \
		return _pClassDesc; \
	} \
	virtual const FxClass* GetClassDesc() const \
	{ \
		return StaticClass(); \
	} \
	virtual FxUInt16 GetCurrentVersion() const \
	{ \
		return StaticClass()->GetCurrentVersion(); \
	} \
	friend FxArchive& operator<<(FxArchive& arc, classname& obj) \
	{ \
		obj.Serialize(arc); \
		return arc; \
	} \
	friend FxArchive& operator<<(FxArchive& arc, classname*& obj) \
	{ \
		return FxSerializeObjectPointer(arc, obj); \
	} \
private: \
	static FxClass* _pClassDesc;

#define FX_DECLARE_CLASS(classname, baseclassname) \
	FX_DECLARE_CLASS_COMMON(classname) \
public: \
	typedef baseclassname Super; \
	static FxObject* ConstructObject() \
	{ \
		return new classname(); \
	} \
private:

#define FX_DECLARE_ABSTRACT_CLASS(classname, baseclassname) \
	FX_DECLARE_CLASS_COMMON(classname) \
public: \
	typedef baseclassname Super; \
	static FxObject* ConstructObject() \
	{ \
		return NULL; \
	} \
private:

#define FX_DECLARE_BASE_CLASS(classname) \
	FX_DECLARE_CLASS_COMMON(classname) \
public: \
	static FxObject* ConstructObject() \
	{ \
		return new classname(); \
	} \
private:

#define FX_IMPLEMENT_CLASS(classname, version, baseclassname) \
	FxClass* classname::_pClassDesc = NULL; \
	FxClass* classname::AllocateStaticClass##classname() \
	{ \
		return new FxClass(#classname, version, baseclassname::StaticClass(), sizeof(classname), classname::ConstructObject); \
	}

#define FX_IMPLEMENT_BASE_CLASS(classname, version) \
	FxClass* classname::_pClassDesc = NULL; \
	FxClass* classname::AllocateStaticClass##classname() \
	{ \
		return new FxClass(#classname, version, NULL, sizeof(classname), classname::ConstructObject); \
	}

template<typename T> FxArchive& FxSerializeObjectPointer(FxArchive& arc, T*& obj);

class FxObject : public FxUseAllocator
{
	FX_DECLARE_BASE_CLASS(FxObject)
public:
	FxObject();
	virtual ~FxObject();
	virtual void Serialize(FxArchive& arc);

	FxBool IsExactKindOf(const FxClass* classDesc) const
	{
		return GetClassDesc()->IsExactKindOf(classDesc);
	}

	FxBool IsKindOf(const FxClass* classDesc) const
	{
		return GetClassDesc()->IsKindOf(classDesc);
	}

	FxBool IsA(const FxClass* classDesc) const
	{
		return GetClassDesc()->IsA(classDesc);
	}
};

template<typename T> inline T* FxCast(FxObject* pObj)
{
	return (pObj && pObj->IsKindOf(T::StaticClass())) ? static_cast<T*>(pObj) : NULL;
}

class FxDataContainer
{
public:
	FxDataContainer();
	FxDataContainer(const FxDataContainer& other);
	FxDataContainer& operator=(const FxDataContainer& other);

	void* GetUserData();
	void SetUserData(void* userData);

protected:
	void* _userData;
};

template<typename T> FxArchive& FxSerializeObjectPointer(FxArchive& arc, T*& obj)
{
	if( arc.IsLoading() )
	{
		FxSize index = FxInvalidIndex;
		arc << index;
		if( index == FxInvalidIndex )
		{
			obj = NULL;
		}
		else
		{
			FxObject* foundObject = arc.FindObject(index);
			if( foundObject )
			{
				obj = static_cast<T*>(foundObject);
			}
			else
			{
				const FxClass* objectClass = NULL;
				if( arc.GetSDKVersion() < 0x640 )
				{
					FxString classNameStr;
					arc << classNameStr;
					objectClass = FxClass::FindClassDesc(FxName(classNameStr.GetData()));
				}
				else
				{
					FxName className;
					arc << className;
					objectClass = FxClass::FindClassDesc(className);
				}
				FxSize classSize;
				arc << classSize;
				if( objectClass )
				{
					FxObject* newObject = objectClass->ConstructObject();
					if( newObject )
					{
						newObject->Serialize(arc);
						obj = static_cast<T*>(newObject);
						arc.AddObject(obj, index);
					}
				}
			}
		}
	}
	else if( obj )
	{
		FxBool needToSaveObject = arc.FindObject(obj) == FxInvalidIndex;
		FxSize objIndex = arc.AddObject(obj);
		arc << objIndex;
		if( needToSaveObject )
		{
			FxName name(obj->GetClassDesc()->GetName());
			FxSize classSize = obj->GetClassDesc()->GetSize();
			arc << name << classSize;
			obj->Serialize(arc);
		}
	}
	else
	{
		FxSize nullIndex = FxInvalidIndex;
		arc << nullIndex;
	}
	return arc;
}

} // namespace Face
} // namespace OC3Ent

#endif
