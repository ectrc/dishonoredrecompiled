// DishonoredGame/src/disparticlesystemcomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x630010  public: static void __cdecl UDisParticleSystemComponent::InitializePrivateStaticClassUDisParticleSystemComponent(void)
//   0x630030  public: virtual void __thiscall UDisParticleSystemComponent::Tick(float)
//   0x630080  public: void __thiscall UDisParticleSystemComponent::SetTimeToDisable(float)
//   0x6407d0  public: virtual void __thiscall UDisParticleSystemComponent::InitializeForPool(void)
//   0x663270  public: static class UClass * __cdecl UDisParticleSystemComponent::GetPrivateStaticClassUDisParticleSystemComponent(wchar_t const *)
//   0x663300  public: static class UClass * __cdecl UDisParticleSystemComponent::StaticClassNoInline(void)
// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x5fa8a0 (2012 0x6407d0, same bytes): a pooled emitter starts with no disable countdown.
void UDisParticleSystemComponent::InitializeForPool()
{
	m_fTimeToDisable = -1.f;
}

void UDisParticleSystemComponent::execInitializeForPool( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	InitializeForPool();
}
