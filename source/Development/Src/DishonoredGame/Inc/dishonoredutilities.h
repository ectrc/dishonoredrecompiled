#pragma once
// DishonoredGame/inc/dishonoredutilities.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x75e010  unsigned int __cdecl DisRefreshArrayClasses<class UDisTweaks_AISubState>(class TArray<class UDisTweaks_AISubState *, class FDefaultAllocator> const &, class TArray<class UDisTweaks_AISubState *, class FDefaultAllocator> &, class UObject *, unsigned int)
//   0x7b5300  struct FDisBodyMesh const * __cdecl DisChooseRandomMesh<struct FDisBodyMesh>(class TArray<struct FDisBodyMesh, class FDefaultAllocator> const &, int &)
//   0x7b5420  struct FDisPawnAccessoryMesh const * __cdecl DisChooseRandomMesh<struct FDisPawnAccessoryMesh>(class TArray<struct FDisPawnAccessoryMesh, class FDefaultAllocator> const &, int &)
//   0x7b8bb0  class UDisNPCDistractionComponent * __cdecl DisGetActorComponent<class UDisNPCDistractionComponent>(class AActor const *)
//   0x8355d0  class UDisGrenadeComponent * __cdecl DisGetActorComponent<class UDisGrenadeComponent>(class AActor const *)
//   0x8647e0  void __cdecl DisShuffle<struct FCheckResult const *, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>>(class TArray<struct FCheckResult const *, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &)
//   0x884f70  unsigned int __cdecl DisRefreshObjectClass<class UDisTweaks_AISubProcess>(class UDisTweaks_AISubProcess const *, class UDisTweaks_AISubProcess * &, class UObject *)
//   0x8bff30  class USkeletalMeshComponent * __cdecl DisGetActorComponent<class USkeletalMeshComponent>(class AActor const *)
//   0x915780  class UArkComponentContainer * __cdecl DisGetActorComponent<class UArkComponentContainer>(class AActor const *)

// DISHONORED(written): accessors of dishonoredutilities_accessors.cpp (2013 rvas in that unit)
class UDishonoredMapInfo* DishonoredGetMapInfo();
class ADishonoredGameInfo* DisGetGameInfo();
class UDisGlobalUIManager* DisGetGlobalUIManager();
class UDisLocalPlayer* DisGetLocalPlayer();
class UDishonoredAudioSystem* DisGetAudioSystem();
class UArkPpNode* DisGetArkPpNode( const FName& EffectName );
class UArkPpNodeMaterial* DisGetArkPpNodeMaterial( const FName& EffectName, UBOOL bMakeUnique );
class UDisPostProcessManager* DisGetPpManager();

// DISHONORED(bringup): agent EI. Retail reaches the one message-box movie as DisGetGlobalUIManager()->m_pGlobal;
// the -gfxuimenu bring-up constructs that movie player itself and no UDisGlobalUIManager is ever created, so this
// answers with the manager's when there is one and with the open UDisGFxMoviePlayerGlobal otherwise.
class UDisGFxMoviePlayerGlobal* DisGetGlobalMoviePlayer();

// DISHONORED(port): agent EI. The parameter block of DisGameEventType_MessageBoxResult: retail's
// UDisGFxMoviePlayerGlobal::OnMessageBoxConfirm (2013 0x7abbd0) builds the two words on its own stack as
// { the box's id, the button the player chose } and UDisGFxMoviePlayerBase::OnMessageBoxResult (0x793c70) reads
// them straight back out of FArkGameEvent::m_pEventParams. It is native-only, so neither PDB names the struct.
struct FDisMsgBoxResult
{
	INT m_ID;
	INT m_SelectedIndex;
};

/** DISHONORED(port): agent EI. The event id OnMessageBoxConfirm raises and every UDisGFxMoviePlayerBase listens for;
    the literal 31 at 2013 0x7abbd0, 0x7a4550 and 0x7a45c0. */
enum { DisGameEventType_MessageBoxResult = 31 };

// DISHONORED(written): agent AU (PHASE7 AU). The utilities the pickup collection path calls; 2013 rvas with the bodies.
class UDisGFxMoviePlayerHUD* DisGetGFxHUD();                                                   // 0x7bf730
UBOOL DisIsBendTimeOn();                                                                       // 0x7bf2c0
UBOOL DisIsBendTimeFrozen();                                                                   // 0x7bf2f0
void DisPullFromBendTime( class AActor* Actor, class AActor* Cause, UBOOL bRecursive, UBOOL bOnlyIfStaticOrTickDisabled ); // 0x7bec00
class ADishonoredPawn* DisGetPawnInstigator( const class AController* Controller );             // 0x7bf060
class ADishonoredPawn* DisGetValidInstigator( const class AActor& Actor );                      // 0x7bf010
class UDisParticleSystemComponent* DishonoredSpawnEmitter( class UParticleSystem* EmitterTemplate, const FVector& SpawnLocation, const FRotator& SpawnRotation, class AActor* TimeBoundActor, UBOOL bAttachToActor ); // 0x7cf970
void DisDestroyActorNextTick( class AActor& Actor );                                            // 0x7bafa0
void DisFireKismetEvent( class AActor* const Actor, class UClass* const EventClass, class AActor* const Instigator, class AActor* const Originator, UBOOL bExactClass, TArray<INT>* ActivateIndices ); // 0x7c7b30
UBOOL DisIsObjectStateGoingToBeRestored( class UObject* Object );                                // 0x7ec6a0
void DisAddUseMessage( const FString& Message );                                                 // 0x7b2420

// DISHONORED(written): agent AU. -dispickup, the pickup census and the -dispickupprobe walk-in; defined in
// dishonoredplayercontroller.cpp next to the ported HandleHeldButtons.
UBOOL DisPickupCensusEnabled();
void DisPickupReport( class UWorld* World, FLOAT DeltaSeconds );

// DISHONORED(written): agent AU. 2013 rva 0x7e88b0, body in dishonoredutilities_physics.cpp.
struct FDisPhysicsUtil
{
	static UBOOL PhysObjectShouldTraceCommon( const class AActor* PhysObject, DWORD TraceFlags );
};

// DISHONORED(written): agent DI (PHASE10 DI). The modular-character helpers of the NPC appearance path.
//
// DisChooseRandomMesh is a template in retail's own dishonoredutilities.h (line 661 of the PDB's copy); the two
// instantiations the PDB attributes to this header are FDisBodyMesh (2012 rva 0x7b5300, retail's own inlined into
// ADishonoredNPCPawn::PostBeginPlay_Body at 2013 rva 0x776390) and FDisPawnAccessoryMesh (2012 0x7b5420, 2013
// 0x7764b0). It is a weighted draw over m_fRandomChance with the array index reported back, because the pawn stores
// the index it drew (m_iRandomHeadMeshSel, m_RandomAccessoriesSel) so a save can restore the same appearance.
template<class MeshType>
const MeshType* DisChooseRandomMesh( const TArray<MeshType>& _rMeshes, INT& _rIndex )
{
	_rIndex = INDEX_NONE;
	FLOAT fRandomChanceTotal = 0.f;
	for( INT Idx = 0; Idx < _rMeshes.Num(); Idx++ )
	{
		fRandomChanceTotal += _rMeshes(Idx).m_fRandomChance;
	}
	// retail: rand() * (fRandomChanceTotal - 1e-8) * (1/32767), i.e. appFrand() scaled just inside the total, so the
	// last entry still wins when every chance is authored and the sum lands exactly on the draw
	const FLOAT fRandChance = appFrand() * ( fRandomChanceTotal - 0.00000001f );
	FLOAT fRunningChance = 0.f;
	for( INT Idx = 0; Idx < _rMeshes.Num(); Idx++ )
	{
		fRunningChance += _rMeshes(Idx).m_fRandomChance;
		if( fRunningChance > fRandChance )
		{
			_rIndex = Idx;
			return &_rMeshes(Idx);
		}
	}
	if( _rMeshes.Num() == 0 )
	{
		return NULL;
	}
	_rIndex = 0;
	return &_rMeshes(0);
}

// DISHONORED(written): agent DI. 2013 rva 0x7c7dd0 (2012 0x82ce70, dishonoredutilities.cpp:1657 neighbourhood): the
// sections of a body or head mesh that only exist to be revealed when a limb is cut off are hidden at spawn.
void DisHideGoreSections( class USkeletalMeshComponent& _rMeshComponent );

// DISHONORED(written): agent DI. 2013 rva 0x7c8640 (2012 0x82d600) and 0x7cfb50 (2012 0x8321e0): every element of the
// component whose mesh material matches is overridden; the index form looks the reference material up first.
void DisReplaceMatchingMaterialsInSkelMesh( class USkeletalMeshComponent* _pMeshComponent, const class UMaterialInterface* _pReferenceMaterial, class UMaterialInterface* _pMaterial );
void DisReplaceMatchingMaterialsInSkelMesh( class USkeletalMeshComponent* _pMeshComponent, INT _MaterialIndex, class UMaterialInterface* _pMaterial );

// DISHONORED(port): agent DN. 2013 rva 0x7baff0 (2012 0x8239b0, dishonoredutilities_accessors.cpp:528): a pawn's feet.
// Retail takes the cylinder's *bounds*, not its Location and CollisionHeight, so a pawn whose cylinder has been moved
// (a crouching or a possessed one) answers where its cylinder really is.
const FVector DisGetPawnFeet( const class APawn* _pPawn );
