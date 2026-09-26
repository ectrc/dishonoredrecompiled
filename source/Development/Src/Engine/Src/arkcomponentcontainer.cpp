// Engine/src/arkcomponentcontainer.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (14):
//   0x574800  public: static void __cdecl UArkComponentContainer::InitializePrivateStaticClassUArkComponentContainer(void)
//   0x575980  private: void __thiscall UArkComponentContainer::AddNewComponent_Internal(class FArkComponentBase *)
//   0x5759a0  public: class FArkComponentBase * __thiscall UArkComponentContainer::AddNewComponentByID(int)
//   0x575ab0  public: virtual void __thiscall UArkComponentContainer::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x577550  public: void __thiscall UArkComponentContainer::RemoveComponent(class FArkComponentBase *)
//   0x5775a0  public: void __thiscall UArkComponentContainer::StartAllComponents(void)
//   0x5775d0  public: void __thiscall UArkComponentContainer::StopAllComponents(void)
//   0x577720  public: virtual void __thiscall UArkComponentContainer::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x577770  public: virtual void __thiscall UArkComponentContainer::Serialize(class FArchive &)
//   0x5777f0  public: virtual void __thiscall UArkComponentContainer::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x579440  public: static class UClass * __cdecl UArkComponentContainer::GetPrivateStaticClassUArkComponentContainer(wchar_t const *)
//   0x5794d0  public: void __thiscall UArkComponentContainer::RemoveAllComponents(void)
//   0x5795c0  public: virtual void __thiscall UArkComponentContainer::BeginDestroy(void)
//   0x57aa60  public: static class UClass * __cdecl UArkComponentContainer::StaticClassNoInline(void)

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UArkComponentContainer);

// DISHONORED(port): 2013 rva 0x5364b0 (2012 0x577770, arkcomponentcontainer.cpp:151): after the UObject data retail hands every
// FArkComponentBase of m_Components an FGCHelper holding the archive (vtable slot 4) so the component can serialize its own object
// references, and counts their memory for a counting archive (slot 6).
// DISHONORED(bringup): FArkComponentBase is not ported (Engine/Inc/arkcomponentbase.h is still a stub; the Dis* components are agent
// AJ's), so the elements stay opaque pointers and neither pass runs. m_Components is never saved (native TArray<FPointer>).
void UArkComponentContainer::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
}

// DISHONORED(port): 2013 rva 0x538a80 (2012 0x5795c0)
void UArkComponentContainer::BeginDestroy()
{
	RemoveAllComponents();
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x536460 (2012 0x577720): the same FGCHelper walk as Serialize, with the object array instead of an archive
// (DISHONORED(bringup): see Serialize)
void UArkComponentContainer::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
}

// DISHONORED(port): 2013 rva 0x538990 (2012 0x5794d0): StopAllComponents, then every component is deleted through its own vtable
// (slot 0, the scalar deleting destructor) from the back and the array is emptied.
// DISHONORED(bringup): the components are unported opaque pointers, so only the array is emptied (nothing has ever been added).
void UArkComponentContainer::RemoveAllComponents()
{
	m_Components.Empty();
}
