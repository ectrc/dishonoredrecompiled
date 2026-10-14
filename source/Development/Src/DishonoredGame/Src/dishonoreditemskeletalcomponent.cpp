// DishonoredGame/src/dishonoreditemskeletalcomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x861d70  public: static void __cdecl UDishonoredItemSkeletalComponent::InitializePrivateStaticClassUDishonoredItemSkeletalComponent(void)
//   0x861d90  public: void __thiscall UDishonoredItemSkeletalComponent::SetInventoryItem(class UDishonoredInventoryItem *)
//   0x861da0  public: class UDishonoredInventoryItem * __thiscall UDishonoredItemSkeletalComponent::GetOwningInventoryItem(void)
//   0x8828e0  public: static class UClass * __cdecl UDishonoredItemSkeletalComponent::GetPrivateStaticClassUDishonoredItemSkeletalComponent(wchar_t const *)
//   0x882ef0  public: static class UClass * __cdecl UDishonoredItemSkeletalComponent::StaticClassNoInline(void)

// Agent GJ (PHASE20 GJ): the two accessors are inline in CppText/UDishonoredItemSkeletalComponent.h; the unit is compiled so the
// class's vtable is the component's own (UDishonoredPlayerSkeletalComponent's scene proxy and tick-end phase, agent GF).

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGJ_ItemSkeletalComponent()
{
}
