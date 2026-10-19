// DishonoredGame/src/dishonoredpawn_water.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x7b60b0  public: virtual void __thiscall ADishonoredPawn::OnEnterWaterVolume(class ADishonoredWaterVolume *)
//   0x7b60e0  public: virtual void __thiscall ADishonoredPawn::Tick_Water(float, enum ELevelTick)
//   0x7b98d0  public: virtual void __thiscall ADishonoredPawn::OnExitWaterVolume(class ADishonoredWaterVolume *)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): the water half every Dishonored pawn carries. 2012 / 2013 rvas: OnEnterWaterVolume 0x7b60b0 / 0x758bc0,
	OnExitWaterVolume 0x7b98d0 / 0x75d670, Tick_Water 0x7b60e0 / 0x758bf0 (the second tick phase, ADishonoredPawn::Tick).
	The enter/exit notifications come from ADishonoredWaterVolume::Touch / UnTouch, which this engine cannot reach yet
	(dishonoredwatervolume.cpp): until it can, m_CurrentWaterVolumes is empty and Tick_Water only ages the post-process timer.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_DishonoredPawnWater()
{
}

/**
 * 2012 0x7b60b0 (41 B): the volume joins the pawn's list and the mesh's attached emitters are switched off.
 * DISHONORED(bringup): DisDeactivateAllAttachedEmitters (dishonoredutilities) is not ported; named.
 */
void ADishonoredPawn::OnEnterWaterVolume( ADishonoredWaterVolume* Volume )
{
	m_CurrentWaterVolumes.AddUniqueItem( Volume );
	IFDIAG( TEXT("water %s enters %s (%d volumes)"), *GetName(), *Volume->GetName(), m_CurrentWaterVolumes.Num() );
	IFDIAG_ONCE( TEXT("ADishonoredPawn::OnEnterWaterVolume: DisDeactivateAllAttachedEmitters is not ported") );
}

// 2012 0x7b98d0 (22 B)
void ADishonoredPawn::OnExitWaterVolume( ADishonoredWaterVolume* Volume )
{
	m_CurrentWaterVolumes.RemoveItem( Volume );
	IFDIAG( TEXT("water %s exits %s (%d volumes)"), *GetName(), *Volume->GetName(), m_CurrentWaterVolumes.Num() );
}

/**
 * 2012 0x7b60e0 / 2013 0x758bf0 (303 B both). The post-process blend timer runs down; with a controller, the first touched volume
 * (that has its info object) containing the controller's view point is the active one, and when that changes OnChangeWaterVolume
 * hears it and the blend restarts over the entered volume's blend time (the left one's when leaving; 0.5 s without either).
 */
void ADishonoredPawn::Tick_Water( FLOAT DeltaTime, ELevelTick TickType )
{
	if( m_fCurrentWaterPostProcessTimer > 0.f )
	{
		m_fCurrentWaterPostProcessTimer -= m_fCurrentWaterPostProcessRate * DeltaTime;
	}
	if( Controller == NULL )
	{
		return;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint( CameraLocation, CameraRotation );
	ADishonoredWaterVolume* ActiveVolume = NULL;
	for( INT Index = 0; Index < m_CurrentWaterVolumes.Num(); Index++ )
	{
		ADishonoredWaterVolume* Volume = m_CurrentWaterVolumes(Index);
		if( Volume && Volume->m_pWaterVolumeInfo && !Volume->CheckPoint( CameraLocation, FVector( 0.f, 0.f, 0.f ), 0 ) )
		{
			ActiveVolume = Volume;
			break;
		}
	}
	if( ActiveVolume == m_pActiveWaterVolume )
	{
		return;
	}
	FLOAT BlendTime = 0.5f;
	UDishonoredWaterVolumeInfo* Info = ActiveVolume ? ActiveVolume->m_pWaterVolumeInfo : m_pActiveWaterVolume->m_pWaterVolumeInfo;
	if( Info )
	{
		BlendTime = Info->m_fBlendTime;
	}
	OnChangeWaterVolume( m_pActiveWaterVolume, ActiveVolume );
	m_fCurrentWaterPostProcessTimer = 1.f;
	m_fCurrentWaterPostProcessRate = 1.f / BlendTime;
	m_pActiveWaterVolume = ActiveVolume;
}
