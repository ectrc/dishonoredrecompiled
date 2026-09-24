// DishonoredGame/src/dishonoredpawn_combat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20):
//   0x7aa7e0  public: void __thiscall FDisLineProbeResult::InitForLineProbe(void)
//   0x7aa7f0  public: virtual unsigned int __thiscall ADishonoredPawn::DoesActorCauseHitRebound(int, class UClass *)const
//   0x7aa810  public: virtual void __thiscall ADishonoredPawn::Tick_Combat(float, enum ELevelTick)
//   0x7aa840  public: virtual void __thiscall ADishonoredPawn::GetMeleeCombatInfo(struct FDisMeleeInfo &, class UDisItemContext const *)const
//   0x7aa860  public: virtual unsigned int __thiscall ADishonoredPawn::IsStunned(unsigned int &)const
//   0x7aa880  public: unsigned int __thiscall ADishonoredPawn::IsVulnerable(enum eDisVulnerabilityType)const
//   0x7aa8f0  protected: virtual unsigned int __thiscall ADishonoredPawn::IsVulnerable_Derived(enum eDisVulnerabilityType, unsigned int)const
//   0x7aa930  public: void __thiscall ADishonoredPawn::MakeVulnerable(enum eDisVulnerabilityType)
//   0x7aa970  public: void __thiscall ADishonoredPawn::StopVulnerable(enum eDisVulnerabilityType, float)
//   0x7aa9c0  public: void __thiscall ADishonoredPawn::OnMeleeOutgoing(enum eDisMeleeZone, class UDisItemContext_MeleeAttack *)
//   0x7aaa10  public: enum eDisMeleeResponse __thiscall ADishonoredPawn::OnMeleeIncoming(enum eDisMeleeZone, class UDisItemContext_MeleeAttack *)
//   0x7aaa90  public: enum eDisPawnHitReactionType __thiscall ADishonoredPawn::GetLastHitReactionType(void)const
//   0x7aaaa0  public: unsigned int __thiscall ADishonoredPawn::IsSeenBy(class AActor const *)const
//   0x7ac960  public: float __thiscall ADishonoredPawn::CalcMeleeHitQuality(void)const
//   0x7ac9b0  public: virtual class FVector __thiscall ADishonoredPawn::GetDamageCenter(void)const
//   0x7aca10  public: virtual enum eDisMeleeResponse __thiscall ADishonoredPawn::OnMeleeIncoming_Versusable(class UDisItemContext_MeleeAttack *)
//   0x7c2e00  public: virtual void __thiscall ADishonoredPawn::PostBeginPlay_Combat(void)
//   0x7c2e80  public: virtual enum eDisMeleeResponse __thiscall ADishonoredPawn::OnMeleeIncoming_Dodgeable(class UDisItemContext_MeleeAttack *)
//   0x7c2f70  public: enum eDisPawnHitReactionType __thiscall ADishonoredPawn::DetermineHitReactionType(class UClass *, int, class FVector const &, struct FTraceHitInfo const *, class AActor *, class ADishonoredPawn *)const
//   0x7c2ff0  protected: virtual enum eDisPawnHitReactionType __thiscall ADishonoredPawn::DetermineHitReactionType_Derived(class UClass *, int, class FVector const &, struct FTraceHitInfo const *, class AActor *, class ADishonoredPawn *)const
