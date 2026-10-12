// DishonoredGame/src/dishonorednpcpawn_nativecomponents.cpp
// DISHONORED(port): agent GA (PHASE18 package GA). The NPC's PreBeginPlay and the native components it adds to its
// UArkComponentContainer before ADishonoredPawn::PreBeginPlay starts them all. Retail attributes both to
// dishonorednpcpawn.cpp; they are in a unit of their own because agent FV is in that unit this wave.
// This unit is also the external-linkage anchor that keeps Engine's FArkComponentMeshOffset registrant in the image.

#include "DishonoredGame.h"
#include "arkcomponentmeshoffset.h"

ARKCOMPONENT_LINK_TYPE( FArkComponentMeshOffset )

// DISHONORED(port): 2012 rva 0x7ab250 (19 B): the base, then the NPC ticks in the pre-async group
void ADishonoredNPCPawn::PreBeginPlay()
{
	Super::PreBeginPlay();
	SetTickGroup( TG_PreAsyncWork );
}

// DISHONORED(port): 2012 rva 0x7b0c10 (247 B). Retail adds four components out of the NPC tweaks (the class default
// tweaks when the pawn has none): FDisComponentHitReactionThrow when m_pComponentHitReactionThrowConfig is set,
// FArkComponentMeshOffset when m_pComponentMeshOffsetConfig is - its six config values copied straight into the
// component - then FDisComponentLODManager (m_pCpntLODManager) and FDisComponentRBDamages (m_pCpntRBDamages)
// unconditionally, and the base's.
// DISHONORED(bringup): the hit-reaction-throw, LOD-manager and RB-damages components have no bodies in this tree
// (FDisComponentLODManager::StartLOD is in agent FS's MAT_BeginAIGroup inventory), so only the mesh offset is added.
void ADishonoredNPCPawn::PreBeginPlay_NativeComponents()
{
	UDisTweaks_NPCPawn* NPCTweaks = Cast<UDisTweaks_NPCPawn>( GetTweaks_Derived() );
	if( NPCTweaks == NULL )
	{
		NPCTweaks = UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_NPCPawn>();
	}
	if( NPCTweaks != NULL && m_ComponentContainer != NULL && NPCTweaks->m_pComponentMeshOffsetConfig != NULL )
	{
		const UDisTweaks_ComponentMeshOffset* Config = NPCTweaks->m_pComponentMeshOffsetConfig;
		FArkComponentMeshOffset* MeshOffset = m_ComponentContainer->AddNewComponent<FArkComponentMeshOffset>();
		if( MeshOffset != NULL )
		{
			MeshOffset->m_bPitchOnGround = Config->m_bPitchOnGround ? TRUE : FALSE;
			MeshOffset->m_fMinHeightSpeed = Config->m_fMinHeightSpeed;
			MeshOffset->m_fMinHeightSpeedWithFootProps = Config->m_fMinHeightSpeedWithFootProps;
			MeshOffset->m_fOffsetDown = Config->m_fOffsetDown;
			MeshOffset->m_fGoDownMinZSpeed = Config->m_fGoDownMinZSpeed;
			MeshOffset->m_fPitchSpeed = Config->m_fPitchSpeed;
		}
	}
	Super::PreBeginPlay_NativeComponents();
}
