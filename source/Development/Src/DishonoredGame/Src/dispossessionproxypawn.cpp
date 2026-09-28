// DishonoredGame/src/dispossessionproxypawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12):
//   0x849200  public: static void __cdecl ADisPossessionProxyPawn::InitializePrivateStaticClassADisPossessionProxyPawn(void)
//   0x849220  public: virtual void __thiscall ADisPossessionProxyPawn::PostBeginPlay_Body(void)
//   0x84af70  public: virtual void __thiscall ADisPossessionProxyPawn::PreBeginPlay_Actions(void)
//   0x84af90  public: virtual unsigned int __thiscall ADisPossessionProxyPawn::IgnoreBlockingBy(class AActor const *)const
//   0x84afd0  public: virtual void __thiscall ADisPossessionProxyPawn::TermPossessionOnPawn(void)
//   0x84b000  private: virtual unsigned int __thiscall ADisPossessionProxyPawn::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x84b040  private: class FVector __thiscall ADisPossessionProxyPawn::ConstrainMoveToVolume(class FVector)
//   0x84b1a0  private: virtual void __thiscall ADisPossessionProxyPawn::physSwimming(float, int)
//   0x84fef0  public: virtual void __thiscall ADisPossessionProxyPawn::PlayDying_Native(class AController *, class UClass *, class FVector)
//   0x8571d0  public: virtual unsigned int __thiscall ADisPossessionProxyPawn::InitPossessionOnPawn(class UDisTweaks_Possessable const *, class FVector const &)
//   0x860bd0  public: static class UClass * __cdecl ADisPossessionProxyPawn::GetPrivateStaticClassADisPossessionProxyPawn(wchar_t const *)
//   0x861340  public: static class UClass * __cdecl ADisPossessionProxyPawn::StaticClassNoInline(void)

// agentDO:possessionproxy
#include "DishonoredGame.h"

/**
 * DISHONORED(port): 2013 rva 0x7e7910 (2012 0x849220), 19 bytes.
 *
 * DISHONORED(retail): the proxy calls ADishonoredPawn::PostBeginPlay_Body, NOT its own parent's
 * (ADishonoredNPCPawn::PostBeginPlay_Body, 0x77f8a0) - it skips the head, the accessories and the material variations
 * of the NPC pass, because the proxy is never drawn. Then it hides itself.
 */
void ADisPossessionProxyPawn::PostBeginPlay_Body()
{
	ADishonoredPawn::PostBeginPlay_Body();
	// DISHONORED(bringup): ADishonoredPawn::SetIsVisible(FALSE) (2013 rva 0x74fd10, 2012 0x790d40) closes
	// retail's body. It is not about rendering: it stops FDisComponentObservable, the ark component the AI
	// sight query walks, so the proxy is not a target. ADishonoredPawn::m_pCpntObservable is an opaque
	// FPointer in this tree and FDisComponentObservable has no declaration at all, so there is nothing to
	// stop yet. That component is the AI package's.
}
