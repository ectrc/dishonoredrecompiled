// DishonoredGame/src/distallboynpcpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (33):
//   0x7c9990  public: static void __cdecl ADisTallboyNPCPawn::InitializePrivateStaticClassADisTallboyNPCPawn(void)
//   0x7c99b0  private: void __thiscall ADisTallboyNPCPawn::CreateLightParticleSystem(class UParticleSystem *)
//   0x7c9a10  public: virtual unsigned int __thiscall ADisTallboyNPCPawn::StartRagdolling(void)
//   0x7c9a40  protected: virtual enum eDisNPCLookAtMode __thiscall ADisTallboyNPCPawn::ChooseLookAt(void)
//   0x7c9a80  public: virtual void __thiscall ADisTallboyNPCPawn::DoPostDeathHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class UClass *, class AController *)
//   0x7cc4a0  public: virtual class FVector __thiscall ADisTallboyNPCPawn::GetDamageCenter(void)const
//   0x7cc4c0  private: virtual unsigned int __thiscall ADisTallboyNPCPawn::RequestNPCMasterAimState(void)
//   0x7ce880  public: void __thiscall ADisTallboyNPCPawn::OnAttachmentTookDamage(class AActor * const, int, class AController * const, class UClass * const, class AActor * const)
//   0x7ce900  private: virtual void __thiscall ADisTallboyNPCPawn::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7da470  public: virtual void __thiscall ADisTallboyNPCPawn::PreBeginPlay_Actions(void)
//   0x7da730  public: virtual void __thiscall ADisTallboyNPCPawn::GetSpringRazorDamageTestPoints(struct TMemStackArray<class FVector> &)const
//   0x7de090  public: virtual void __thiscall ADisTallboyNPCPawn::GetSpringRazorTriggerTestPoints(struct TMemStackArray<class FVector> &)const
//   0x7f00e0  public: static class UClass * __cdecl ADisTallboyNPCPawn::GetPrivateStaticClassADisTallboyNPCPawn(wchar_t const *)
//   0x7f0bf0  public: static class UClass * __cdecl ADisTallboyNPCPawn::StaticClassNoInline(void)
//   0x7f0f40  public: static class UClass * __cdecl UDisTweaks_TallboyNPCPawn::GetPrivateStaticClassUDisTweaks_TallboyNPCPawn(wchar_t const *)
//   0x7f11f0  public: static void __cdecl UDisTweaks_TallboyNPCPawn::InitializePrivateStaticClassUDisTweaks_TallboyNPCPawn(void)
//   0x7f1d40  public: static class UClass * __cdecl UDisTweaks_TallboyNPCPawn::StaticClassNoInline(void)
//   0x7f2440  private: virtual void __thiscall ADisTallboyNPCPawn::ApplyTweakChanges_Derived(void)
//   0x7f24d0  public: virtual void __thiscall ADisTallboyNPCPawn::PostBeginPlay_Body(void)
//   0x7f2600  private: void __thiscall ADisTallboyNPCPawn::SwitchToDyingContent(void)
//   0x7f2690  private: void __thiscall ADisTallboyNPCPawn::SwitchToRagdollContent(void)
//   0x7f2700  public: virtual void __thiscall ADisTallboyNPCPawn::ShutPawnDown_Derived(void)
//   0x7f27a0  public: virtual unsigned int __thiscall ADisTallboyNPCPawn::IsAllowedFinisher(struct FImpactInfo const &)const
//   0x7f27f0  private: void __thiscall ADisTallboyNPCPawn::ComponentTickLight(float)
//   0x7f3070  private: virtual void __thiscall ADisTallboyNPCPawn::SucceededRagdoll(void)
//   0x7f3370  private: virtual void __thiscall ADisTallboyNPCPawn::PostInitRagdoll(void)
//   0x7f34a0  public: virtual void __thiscall ADisTallboyNPCPawn::TakeDamage_Native(int &, class AController *, class FVector, class FVector &, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x7f3620  public: virtual void __thiscall ADisTallboyNPCPawn::DoWeakHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class UClass *)
//   0x7f3690  public: virtual unsigned int __thiscall ADisTallboyNPCPawn::CanBeShreddedBySpringRazor(struct FTraceHitInfo const &)const
//   0x7f3700  protected: virtual enum eDisPawnHitReactionType __thiscall ADisTallboyNPCPawn::DetermineHitReactionType_Derived(class UClass *, int, class FVector const &, struct FTraceHitInfo const *, class AActor *, class ADishonoredPawn *)const
//   0x7f3840  protected: virtual void __thiscall ADisTallboyNPCPawn::PlayDying_Native_Derived(class AController *, class UClass *)
//   0x7f3910  private: virtual void __thiscall ADisTallboyNPCPawn::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x7f3a80  private: virtual void __thiscall ADisTallboyNPCPawn::PostGameLoad(enum ESaveLoadLocation)

// agentDO:tallboy
#include "DishonoredGame.h"
#include "EngineMaterialClasses.h"
#include "dishonoredutilities.h"
#include "dispowercensus.h"

/**
 * DISHONORED(port): 2013 rva 0x840860 (2012 0x88b3b0) - FDisSpotlightManager::Init. Spelled as a file-local free
 * function, as agent DI spelled FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh: FDisSpotlightManager is a
 * generated struct, and a method on one costs a new Inc/CppText/<Struct>.h plus a full DishonoredGame regeneration.
 * The body is retail's.
 *
 * Two halves. From the light it caches the cone angle, the brightness and the inner/outer ratio, which is what the
 * per-frame spotlight drive interpolates against. From a mesh (NULL for the tallboy, which passes no mesh and
 * material index -1) it makes the material slot its own UMaterialInstanceConstant and remembers the named scalar's
 * starting value.
 */
static void DisSpotlightManagerInit( FDisSpotlightManager& Manager, USkeletalMeshComponent* Mesh, INT MaterialIndex, const FName& OpacityParamName, ALight* Light )
{
	if( Light && Light->LightComponent )
	{
		USpotLightComponent* SpotComponent = Cast<USpotLightComponent>( Light->LightComponent );
		const FLOAT OuterConeAngle = SpotComponent ? SpotComponent->OuterConeAngle : 0.0f;
		Manager.m_fOriginalLightBrightness = Light->LightComponent->Brightness;
		Manager.m_fCurrentLightAngle = OuterConeAngle;
		Manager.m_fOriginalInnerToOuterRatio = ( Abs(OuterConeAngle) >= SMALL_NUMBER && SpotComponent )
			? SpotComponent->InnerConeAngle / OuterConeAngle
			: 1.0f;
	}
	if( Mesh )
	{
		UMaterialInterface* SlotMaterial = Mesh->GetMaterial( MaterialIndex );
		if( SlotMaterial )
		{
			Manager.m_pLight_MIC = ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass(), UObject::GetTransientPackage() );
			if( Manager.m_pLight_MIC )
			{
				Manager.m_pLight_MIC->SetParent( SlotMaterial );
				Manager.m_pLight_MIC->GetScalarParameterValue( OpacityParamName, Manager.m_fOriginalMaterialOpacity );
				Mesh->SetMaterial( MaterialIndex, Manager.m_pLight_MIC );
			}
		}
	}
}

/**
 * DISHONORED(port): 2013 rva 0x77daa0 (2012 0x7f2440) - THE STILTS. The tallboy is two skeletal meshes below the waist
 * the same way an NPC is two above the neck: the body on APawn::Mesh and the stilts on m_pStiltsMesh, and this is the
 * only function in the executable that ever puts a USkeletalMesh on that component. Agent DI's hand-over 1 attributed
 * the stilts to PostBeginPlay_Body; PostBeginPlay_Body is the searchlight.
 *
 * Retail takes the attached branch only in game with the component already attached, because SetSkeletalMesh on an
 * unattached component would not reattach it; out of game it writes the member and lets the attach carry it.
 */
void ADisTallboyNPCPawn::ApplyTweakChanges_Derived()
{
	Super::ApplyTweakChanges_Derived();

	UDisTweaks_TallboyNPCPawn* Tweaks = Cast<UDisTweaks_TallboyNPCPawn>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_TallboyNPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_TallboyNPCPawn>();
	}
	if( !m_pStiltsMesh )
	{
		// retail dereferences m_pStiltsMesh unchecked; the archetype always carries it
		return;
	}
	if( GIsGame && m_pStiltsMesh->IsAttached() )
	{
		m_pStiltsMesh->SetSkeletalMesh( Tweaks->m_pStiltsSkeletalMesh, FALSE );
	}
	else
	{
		m_pStiltsMesh->SkeletalMesh = Tweaks->m_pStiltsSkeletalMesh;
	}
	m_pStiltsMesh->SetParentAnimComponent( Mesh );
	GDisTallboyStiltsSet++;
}

/**
 * DISHONORED(port): 2013 rva 0x781270 (2012 0x7f24d0) - the tallboy's searchlight. Spawns the light actor the tweaks
 * name at the pawn's own location with no rotation, pushes the tweaked radius, cone and brightness into its light
 * component, reattaches it, seeds the spotlight manager from it and starts the light's particle system; then the
 * ambient sound.
 *
 * DISHONORED(retail): retail 2013 puts AActor::PostAkEvent INSIDE the "state is not going to be restored" guard; the
 * 2012 build posts it unconditionally and tests the light class and the restore flag together. Ported as retail 2013
 * has it.
 */
void ADisTallboyNPCPawn::PostBeginPlay_Body()
{
	Super::PostBeginPlay_Body();

	UDisTweaks_TallboyNPCPawn* Tweaks = Cast<UDisTweaks_TallboyNPCPawn>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_TallboyNPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_TallboyNPCPawn>();
	}
	if( DisIsObjectStateGoingToBeRestored( this ) )
	{
		return;
	}

	if( Tweaks->m_pLightToSpawn )
	{
		m_pAttachedLight = Cast<ASpotLight>( GWorld->SpawnActor( Tweaks->m_pLightToSpawn, NAME_None, Location, FRotator(0,0,0) ) );
		if( m_pAttachedLight && m_pAttachedLight->LightComponent )
		{
			ULightComponent* LightComponent = m_pAttachedLight->LightComponent;
			UPointLightComponent* PointComponent = Cast<UPointLightComponent>( LightComponent );
			USpotLightComponent* SpotComponent = Cast<USpotLightComponent>( LightComponent );
			if( PointComponent )
			{
				PointComponent->Radius = Tweaks->m_fLightRadius;
			}
			if( SpotComponent )
			{
				SpotComponent->OuterConeAngle = Tweaks->m_fLightOuterConeAngle;
			}
			LightComponent->Brightness = Tweaks->m_fLightBrightness;
			LightComponent->BeginDeferredReattach();

			DisSpotlightManagerInit( m_SpotlightManager, NULL, INDEX_NONE, NAME_None, m_pAttachedLight );
			CreateLightParticleSystem( Tweaks->m_pLightParticleSystem );
			GDisTallboyLightsSpawned++;
		}
	}
	PostAkEvent( Tweaks->m_pAmbientStartSoundEvent );
}

/** DISHONORED(port): 2013 rva 0x76eff0 (2012 0x7c99b0) - one particle system at a time, at the pawn's transform. */
void ADisTallboyNPCPawn::CreateLightParticleSystem( UParticleSystem* ParticleSystem )
{
	if( m_pLightParticleSystem )
	{
		m_pLightParticleSystem->DeactivateSystem();
		m_pLightParticleSystem = NULL;
	}
	m_pLightParticleSystem = DishonoredSpawnEmitter( ParticleSystem, Location, Rotation, this, FALSE );
}
