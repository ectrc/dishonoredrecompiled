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

// DISHONORED(port): 2013 rva 0x533f40 (2012 0x575980): the component's owner is the container's own Owner actor, then it
// joins m_Components. Nothing is started here.
void UArkComponentContainer::AddNewComponent_Internal( FArkComponentBase* _pNewComponent )
{
	if( _pNewComponent )
	{
		_pNewComponent->m_pOwner = Owner;
		m_Components.AddItem( (FPointer)_pNewComponent );
	}
}

// DISHONORED(port): 2013 rva 0x534100 (2012 0x5759a0): the creator table is searched for the id (retail scans the whole
// table and keeps the last match rather than breaking out), the component is created through the creator function, given
// the container's owner and appended. This is the path UArkComponentContainer::GameLoad rebuilds a saved component on.
// DISHONORED(written): retail calls through the creator pointer unconditionally, so an unknown id dereferences NULL. Ours
// names the id and returns NULL instead, which is the same outcome without taking the process down.
FArkComponentBase* UArkComponentContainer::AddNewComponentByID( INT _ID )
{
	const TArray<FArkComponentCreatorRegister*>& Creators = FArkComponentCreatorRegister::GetRegistry();
	FArkComponentCreatorRegister::CreatorFnType pCreatorFn = NULL;
	for( INT i = 0; i < Creators.Num(); ++i )
	{
		if( Creators(i)->m_ID == _ID )
		{
			pCreatorFn = Creators(i)->m_pCreatorFn;
		}
	}
	if( !pCreatorFn )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UArkComponentContainer::AddNewComponentByID: no component type is registered for id %d"), _ID );
		return NULL;
	}

	FArkComponentBase* pNewComponent = pCreatorFn();
	if( pNewComponent )
	{
		pNewComponent->m_pOwner = Owner;
		m_Components.AddItem( (FPointer)pNewComponent );
	}
	return pNewComponent;
}

// DISHONORED(port): 2013 rva 0x536290 (2012 0x577550): a started component is stopped before it leaves the array, and both
// of its flags are cleared. The component itself is not deleted - only RemoveAllComponents deletes.
void UArkComponentContainer::RemoveComponent( FArkComponentBase* _pComponent )
{
	for( INT i = 0; i < m_Components.Num(); ++i )
	{
		if( (FArkComponentBase*)m_Components(i) == _pComponent )
		{
			if( _pComponent->m_bStarted )
			{
				_pComponent->Stopping();
				_pComponent->m_bStarted = FALSE;
				_pComponent->m_bPendingStop = FALSE;
			}
			m_Components.Remove( i, 1 );
			return;
		}
	}
}

// DISHONORED(port): 2013 rva 0x5362e0 (2012 0x5775a0): m_bStarted is set BEFORE Starting() runs, so a component that looks
// at its own container from Starting sees itself started.
void UArkComponentContainer::StartAllComponents()
{
	for( INT i = 0; i < m_Components.Num(); ++i )
	{
		FArkComponentBase* pComponent = (FArkComponentBase*)m_Components(i);
		if( !pComponent->m_bStarted )
		{
			pComponent->m_bStarted = TRUE;
			pComponent->Starting();
		}
	}
}

// DISHONORED(port): 2013 rva 0x536310 (2012 0x5775d0): two passes. The first collects every started component and marks it
// m_bPendingStop, the second calls Stopping() and clears both flags. The split is what lets a component's Stopping look at
// the others and tell "already stopped" from "about to stop".
void UArkComponentContainer::StopAllComponents()
{
	TArray<FArkComponentBase*> ComponentsToStop;
	for( INT i = 0; i < m_Components.Num(); ++i )
	{
		FArkComponentBase* pComponent = (FArkComponentBase*)m_Components(i);
		if( pComponent->m_bStarted )
		{
			ComponentsToStop.AddItem( pComponent );
			pComponent->m_bPendingStop = TRUE;
		}
	}
	for( INT i = 0; i < ComponentsToStop.Num(); ++i )
	{
		FArkComponentBase* pComponent = ComponentsToStop(i);
		pComponent->Stopping();
		pComponent->m_bStarted = FALSE;
		pComponent->m_bPendingStop = FALSE;
	}
}

// DISHONORED(port): 2013 rva 0x5364b0 (2012 0x577770, arkcomponentcontainer.cpp:151): after the UObject data every
// component gets an FGCHelper holding the archive so it can serialize its own object references, and a counting archive
// additionally collects their memory footprints in one CountBytes call.
void UArkComponentContainer::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	FArkComponentBase::FGCHelper Helper( NULL, &Ar );
	for( INT i = 0; i < m_Components.Num(); ++i )
	{
		( (FArkComponentBase*)m_Components(i) )->ManageReferences( &Helper );
	}

	if( Ar.IsCountingMemory() )
	{
		DWORD Footprint = 0;
		for( INT i = 0; i < m_Components.Num(); ++i )
		{
			Footprint += ( (FArkComponentBase*)m_Components(i) )->GetMemoryFootprint();
		}
		Ar.CountBytes( Footprint, Footprint );
	}
}

// DISHONORED(port): 2013 rva 0x536460 (2012 0x577720): the same FGCHelper walk as Serialize, with the object array instead
// of an archive.
void UArkComponentContainer::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );

	FArkComponentBase::FGCHelper Helper( &ObjectArray, NULL );
	for( INT i = 0; i < m_Components.Num(); ++i )
	{
		( (FArkComponentBase*)m_Components(i) )->ManageReferences( &Helper );
	}
}

// DISHONORED(port): 2013 rva 0x538990 (2012 0x5794d0): StopAllComponents, then every component is deleted through its own
// virtual destructor from the back, and the array is emptied.
void UArkComponentContainer::RemoveAllComponents()
{
	StopAllComponents();

	for( INT i = m_Components.Num() - 1; i >= 0; --i )
	{
		FArkComponentBase* pComponent = (FArkComponentBase*)m_Components(i);
		if( pComponent )
		{
			delete pComponent;
		}
		m_Components.Remove( i, 1 );
	}
	m_Components.Empty();
}

// DISHONORED(port): 2013 rva 0x538a80 (2012 0x5795c0)
void UArkComponentContainer::BeginDestroy()
{
	RemoveAllComponents();
	Super::BeginDestroy();
}

/*-----------------------------------------------------------------------------
	DISHONORED(bringup): GameSave / GameLoad (2012 rvas 0x5777f0 / 0x575ab0) are NOT ported. They are two of the
	DisSaveLoad virtuals, and neither the ESaveLoadLocation enumeration nor the GameSave/GameLoad vtable slots exist in
	the tree yet (the save system is a package of its own). The retail bodies are three lines each and are recorded here
	so that landing them is a transcription:

	  GameSave( FArchive& Ar, ESaveLoadLocation Location )
	      INT NumComponents = m_Components.Num();
	      Ar << NumComponents;                                  // FArchive::ByteOrderSerialize, 4 bytes
	      for each component:  INT Type = pComponent->GetType(); Ar << Type; pComponent->Serialize( Ar );

	  GameLoad( FArchive& Ar, ESaveLoadLocation Location )
	      INT NumComponents; Ar << NumComponents;
	      for NumComponents times:  INT ComponentID; Ar << ComponentID;
	                                AddNewComponentByID( ComponentID )->Serialize( Ar );

	Note that GameLoad ADDS to whatever is already in m_Components rather than clearing it first, and that it trusts the
	id, which is the reason AddNewComponentByID has to cope with an id that has no creator.
-----------------------------------------------------------------------------*/
