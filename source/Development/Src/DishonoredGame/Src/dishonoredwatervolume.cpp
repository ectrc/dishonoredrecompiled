// DishonoredGame/src/dishonoredwatervolume.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x630a90  public: static void __cdecl ADishonoredWaterVolume::InitializePrivateStaticClassADishonoredWaterVolume(void)
//   0x630ab0  public: unsigned int __thiscall ADishonoredWaterVolume::CheckPoint(class FVector const &, class FVector const &, unsigned long)
//   0x630b50  public: float __thiscall ADishonoredWaterVolume::GetSurfaceZ(void)const
//   0x630b70  public: virtual void __thiscall ADishonoredWaterVolume::OnTriggerNotify(struct FTriggerNotifyInfo const *, int)
//   0x630be0  public: virtual class UClass * __thiscall ADishonoredWaterVolume::GetContactTypeOverride(void)const
//   0x630bf0  public: virtual class UClass * __thiscall ADishonoredKActor::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x649d30  public: virtual void __thiscall ADishonoredWaterVolume::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x651e80  public: static class UClass * __cdecl ADishonoredWaterVolume::GetPrivateStaticClassADishonoredWaterVolume(wchar_t const *)
//   0x653ac0  public: static class UClass * __cdecl ADishonoredWaterVolume::StaticClassNoInline(void)
//   0x6568c0  public: virtual void __thiscall ADishonoredWaterVolume::UnTouch(class AActor *)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): the water volume's queries. Ported: CheckPoint (2012 0x630ab0 / 2013 0x61b9f0) and GetSurfaceZ
	(0x630b50 / 0x617d20), which the pawn's Tick_Water uses to tell whether the camera is under water.
	DISHONORED(bringup), named and not ported:
	  Touch (0x649d30 / 0x6202a0) and UnTouch (0x6568c0 / 0x624b10) - the entry/exit splash (UDishonoredWaterVolumeInfo::
	    DetermineEffect, DishonoredSpawnEmitter, PostAkEventAtPoint) and the pawn's OnEnterWaterVolume / OnExitWaterVolume
	    (2013 pawn vtable +1420 / +1424). Retail reaches them through the native AActor Touch/UnTouch virtuals (AVolume::Touch
	    2012 0x2bf090, APhysicsVolume::Touch 0x2c64e0, called from AActor's touch dispatch). This tree's engine has no native
	    Touch layer - TouchTo/EndTouch raise the script events only (Engine/Src/UnActor.cpp) - so there is nothing for them to
	    override, and m_CurrentWaterVolumes stays empty: the pawn's Tick_Water runs but never finds a camera under water. The
	    swimming itself does not depend on it: the engine's zone change (APhysicsVolume::bWaterVolume) starts PHYS_Swimming.
	  OnTriggerNotify (0x630b70 / 0x617d40) - the rigid bodies' buoyancy through UDisWaterVolumeManager::SetBodyInWaterVolume
	    (0x8b82e0, 1268 B; the manager's Tick 0x8c0460, 2714 B): the manager is not reached by the player and stays deferred.
	  GetContactTypeOverride (0x630be0) - the contact system, unported (agent AU's root 4).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_DishonoredWaterVolume()
{
}

// 2013 0x61b9f0 (151 B): TRUE when the brush does not contain the point (no brush: TRUE)
UBOOL ADishonoredWaterVolume::CheckPoint( const FVector& Point, const FVector& Extent, DWORD TraceFlags )
{
	if( BrushComponent == NULL )
	{
		return TRUE;
	}
	FCheckResult Result( 1.f );
	return BrushComponent->PointCheck( Result, Point, Extent, TraceFlags ) != 0;
}

// 2013 0x617d20 (19 B): the top of the collision bounds
FLOAT ADishonoredWaterVolume::GetSurfaceZ() const
{
	return CollisionComponent->Bounds.Origin.Z + CollisionComponent->Bounds.BoxExtent.Z;
}
