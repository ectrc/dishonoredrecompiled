// Engine/src/disfogcomponent.cpp
// DISHONORED(port): Arkane's fog component. PDB functions attributed to this file (7):
//   0xd7f10  public: static void __cdecl UDisFogComponent::InitializePrivateStaticClassUDisFogComponent(void)
//   0xd7f50  protected: virtual void __thiscall UDisFogComponent::Attach(void)
//   0xd7f70  protected: virtual void __thiscall UDisFogComponent::UpdateTransform(void)
//   0xd7fa0  protected: virtual void __thiscall UDisFogComponent::Detach(unsigned int)
//   0xdeae0  protected: virtual void __thiscall UDisFogComponent::SetParentToWorld(class FMatrix const &)
//   0xf5450  public: static class UClass * __cdecl UDisFogComponent::GetPrivateStaticClassUDisFogComponent(wchar_t const *)
//   0x1da8d0 public: void __thiscall UDisFogComponent::execSetEnabled(struct FFrame &, void * const)
// (2013 rvas: Attach 0xd5ef0, UpdateTransform 0xd5f10, Detach 0xd5f40, SetParentToWorld 0xdc9c0, execSetEnabled 0x1c8530.)

#include "EnginePrivate.h"
#include "EngineDisFogClasses.h"

IMPLEMENT_CLASS(UDisFogComponent);
IMPLEMENT_CLASS(UDisFogDisplayComponent);
IMPLEMENT_CLASS(ADisFog);

FNativeFunctionLookup GEngineUDisFogComponentNatives[] =
{
	MAP_NATIVE(UDisFogComponent, execSetEnabled)
	{NULL, NULL}
};

/** DISHONORED(port): 2013 rva 0xdc9c0 - only the height of the component's transform matters. */
void UDisFogComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	Origin = ParentToWorld.M[3][2];
}

/** DISHONORED(port): 2013 rva 0xd5ef0. */
void UDisFogComponent::Attach()
{
	UActorComponent::Attach();
	if (bEnabled)
	{
		Scene->AddDisFog(this);
	}
}

/** DISHONORED(port): 2013 rva 0xd5f10 - the base transform update, then re-add with the new origin. */
void UDisFogComponent::UpdateTransform()
{
	UActorComponent::UpdateTransform();
	Scene->RemoveDisFog(this);
	if (bEnabled)
	{
		Scene->AddDisFog(this);
	}
}

/** DISHONORED(port): 2013 rva 0xd5f40. */
void UDisFogComponent::Detach(UBOOL bWillReattach)
{
	UActorComponent::Detach(bWillReattach);
	Scene->RemoveDisFog(this);
}

/**
 * DISHONORED(port): the body of the SetEnabled native (2013 rva 0x1c8530, shared with
 * UWorldRainComponent::SetEnabled): the flag, then a re-attach so the scene array follows it.
 */
void UDisFogComponent::SetEnabled(UBOOL bSetEnabled)
{
	if (bEnabled != bSetEnabled)
	{
		bEnabled = bSetEnabled;
		if (IsAttached())
		{
			BeginDeferredReattach();
		}
	}
}
