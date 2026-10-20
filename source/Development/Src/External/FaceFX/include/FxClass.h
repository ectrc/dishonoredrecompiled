/*=============================================================================
	FxClass.h - FaceFX 1.7.3.1 run-time class descriptors (fxclass.h).

	DISHONORED(port): agent FX1 (wave 26). 24 bytes: base descriptor, construct function, name, number of
	direct children, object size, current serialisation version. Every descriptor registers itself in the
	global class table (FxArray<FxClass*>, reserved to 64 by Startup); FindClassDesc is a linear name search.
=============================================================================*/
#ifndef FxClass_H__
#define FxClass_H__

#include "FxName.h"

namespace OC3Ent
{
namespace Face
{

class FxObject;

class FxClass : public FxUseAllocator
{
public:
	FxClass(const FxChar* className, FxUInt16 currentVersion, const FxClass* pBaseClassDesc, FxSize classSize, FxObject* (*constructObjectFn)());
	~FxClass();

	static void Startup();
	static void Shutdown();

	const FxName& GetName() const;
	FxUInt16 GetCurrentVersion() const;
	const FxClass* GetBaseClassDesc() const;
	FxSize GetSize() const;
	FxSize GetNumChildren() const;
	FxObject* ConstructObject() const;

	static const FxClass* FindClassDesc(const FxName& className);
	static FxSize GetNumClasses();
	static const FxClass* GetClass(FxSize index);

	FxBool IsExactKindOf(const FxClass* classDesc) const;
	FxBool IsKindOf(const FxClass* classDesc) const;
	FxBool IsA(const FxClass* classDesc) const;

private:
	FxClass(const FxClass&);
	FxClass& operator=(const FxClass&);

	const FxClass* _pBaseClassDesc;
	FxObject* (*_constructObjectFn)();
	FxName _name;
	mutable FxSize _numChildren;
	FxSize _size;
	FxUInt16 _currentVersion;

	static FxArray<FxClass*>* _classTable;
};

} // namespace Face
} // namespace OC3Ent

#endif
