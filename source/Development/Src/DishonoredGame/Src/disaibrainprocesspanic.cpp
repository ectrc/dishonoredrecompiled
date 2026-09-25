// DishonoredGame/src/disaibrainprocesspanic.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (25):
//   0x78e090  private: unsigned int __thiscall UDisAIBrainProcessPanic::OwnerMaySeePawn(class ADishonoredPawn *)
//   0x78e0c0  public: virtual unsigned char const * __thiscall UDisAIBrainProcessPanic::BuildFilterStimMask(void)const
//   0x793f10  private: class UDisTweaks_AIBehavior_Panic const * __thiscall UDisAIBrainProcessPanic::GetPanicParameters(void)const
//   0x79b320  public: static void __cdecl UDisAIBrainProcessPanic::InitializePrivateStaticClassUDisAIBrainProcessPanic(void)
//   0x79ec00  public: static class UClass * __cdecl UDisAIBrainProcessPanic::GetPrivateStaticClassUDisAIBrainProcessPanic(wchar_t const *)
//   0x79ec90  private: int __thiscall UDisAIBrainProcessPanic::GetPanickingNPCCount(void)const
//   0x7a0ac0  public: static class UClass * __cdecl UDisAIBrainProcessPanic::StaticClassNoInline(void)
//   0x7a0af0  private: unsigned int __thiscall UDisAIBrainProcessPanic::IsPanicInhibited(class AActor *, enum EDisPanicReason, unsigned int)
//   0x7a2510  private: void __thiscall UDisAIBrainProcessPanic::TriggerPanicFromActor(class AActor *, enum EDisPanicReason, unsigned int)
//   0x7a2600  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterIncomingDamage(struct FAIStimStruct_IncomingDamage const &)
//   0x7a26a0  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterAllyBusted(struct FAIStimStruct_AllyBusted const &)
//   0x7a27b0  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterHelp(struct FAIStimStruct_Help const &)
//   0x7a2830  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterAlarm(struct FAIStimStruct_Alarm const &)
//   0x7a28d0  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterEnemyBusted(struct FAIStimStruct_EnemyBusted const &)
//   0x7a2930  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterHeardSomething(struct FAIStimStruct_HeardSomething const &)
//   0x7a29e0  private: void __thiscall UDisAIBrainProcessPanic::OnNpcDbgForcedPanic(class FArkGameEvent const &)
//   0x7a2a10  private: void __thiscall UDisAIBrainProcessPanic::OnNpcDeath(class FArkGameEvent const &)
//   0x7a2be0  private: void __thiscall UDisAIBrainProcessPanic::OnActorSighted(class FArkGameEvent const &)
//   0x7a4550  public: void __thiscall UDisAIBrainProcessPanic::SetupPanicProcess(class UDisBehaviorPanic *)
//   0x7a4620  public: virtual void __thiscall UDisAIBrainProcessPanic::TermBrainProcess_Derived(void)
//   0x7a46d0  private: void __thiscall UDisAIBrainProcessPanic::TriggerPanicFromCorpse(class IDisCorpseInterface *, class ADishonoredPawn *)
//   0x7a4810  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterDiscoveredCorpse(struct FAIStimStruct_DiscoveredCorpse const &)
//   0x7a4890  private: unsigned int __thiscall UDisAIBrainProcessPanic::FilterWitnessDeath(struct FAIStimStruct_WitnessDeath const &)
//   0x7a5700  private: class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAIBrainProcessPanic::GetFilterStimDelegate_Civilians(enum EAIStimID)
//   0x7a5c80  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAIBrainProcessPanic::GetFilterStimDelegate_BrainProcess(enum EAIStimID)
