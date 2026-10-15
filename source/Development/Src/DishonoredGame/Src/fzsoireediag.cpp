/*=============================================================================
	fzsoireediag.cpp: agent FZ (PHASE18 FZ) - the -fzdiag switch. Once a second, from ADishonoredPlayerCamera::
	UpdateViewTarget: the view the camera produced (FOV, location, rotation), the core influence group's weights, the
	AnimDriven influence's player-look limits and offsets, the player master FSM's current state and the body mode.
	Nothing here changes behaviour.
=============================================================================*/
#include "DishonoredGame.h"
#include "fzsoireediag.h"

// DISHONORED(bringup): agent HD (PHASE22 HD) -pcamdiag - the view yaw around the soiree exit. Once a second, and every frame for two
// seconds after FDisSoireeControlState::ExitSoireeMode (with the eight frames before it replayed from a ring): the controller, pawn,
// player mesh, camera bone and final view rotations, the pawn's base, and the PlayerControl / AnimDriven reset state.
static INT GDisPCamDiag = -1;
static FLOAT GDisPCamExitTime = -1.f;
static FString GDisPCamRing[8];
static INT GDisPCamRingNext = 0;

UBOOL DisPCamDiagEnabled()
{
	if( GDisPCamDiag < 0 )
	{
		GDisPCamDiag = ( appStrfind( appCmdLine(), TEXT("-pcamdiag") ) != NULL ) ? 1 : 0;
	}
	return GDisPCamDiag == 1;
}

void DisPCamNoteSoireeExit()
{
	if( !DisPCamDiagEnabled() )
	{
		return;
	}
	GDisPCamExitTime = GWorld ? GWorld->GetTimeSeconds() : 0.f;
	for( INT Step = 0; Step < ARRAY_COUNT( GDisPCamRing ); Step++ )
	{
		const FString& Line = GDisPCamRing[( GDisPCamRingNext + Step ) % ARRAY_COUNT( GDisPCamRing )];
		if( Line.Len() )
		{
			debugf( TEXT("DISHONORED(bringup): pcamdiag pre-exit %s"), *Line );
		}
	}
}

void DisPCamReport( ADishonoredPlayerCamera* Camera, const FTViewTarget& OutVT, APawn* Pawn )
{
	if( !DisPCamDiagEnabled() || !Camera )
	{
		return;
	}
	const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
	FRotator ControllerRot( 0, 0, 0 );
	if( Pawn && Pawn->Controller )
	{
		ControllerRot = Pawn->Controller->Rotation;
	}
	FRotator MeshRot( 0, 0, 0 );
	if( Pawn && Pawn->Mesh )
	{
		MeshRot = Pawn->Mesh->LocalToWorld.Rotator();
	}
	FRotator BoneRot( 0, 0, 0 );
	ADishonoredPlayerPawn* Player = Cast<ADishonoredPlayerPawn>( Pawn );
	if( Player )
	{
		Player->GetCameraBone( NULL, &BoneRot );
	}
	UDishonoredCamera_PlayerControl* PlayerControl = Camera->GetCamera_PlayerControl();
	UDishonoredCamera_AnimDriven* AnimDriven = Camera->GetCamera_AnimDriven();
	const FString Line = FString::Printf( TEXT("t=%.3f view %d/%d controller %d/%d pawn %d/%d mesh %d/%d bone %d/%d base %s bone %s physics %d body %d"),
		Now, OutVT.POV.Rotation.Pitch, OutVT.POV.Rotation.Yaw, ControllerRot.Pitch, ControllerRot.Yaw,
		Pawn ? Pawn->Rotation.Pitch : 0, Pawn ? Pawn->Rotation.Yaw : 0, MeshRot.Pitch, MeshRot.Yaw, BoneRot.Pitch, BoneRot.Yaw,
		Pawn && Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"), Pawn ? *Pawn->BaseBoneName.ToString() : TEXT("none"),
		Pawn ? (INT)Pawn->Physics : -1, Player ? (INT)Player->m_BodyMode : -1 ) + FString::Printf( TEXT(" PlayerControl w %.2f->%.2f active %d reset %d goto %d first %d target %d/%d AnimDriven w %.2f->%.2f active %d flags %d"),
		PlayerControl ? PlayerControl->m_Weight : -1.f, PlayerControl ? PlayerControl->m_TargetWeight : -1.f, PlayerControl ? (INT)PlayerControl->m_bActive : -1,
		PlayerControl ? (INT)PlayerControl->m_bResetControllerRot : -1, PlayerControl ? (INT)PlayerControl->m_bGoToTargetOnControllerReset : -1,
		PlayerControl ? (INT)PlayerControl->m_bFirstRotation : -1,
		PlayerControl ? PlayerControl->m_TargetOnControllerReset.Pitch : 0, PlayerControl ? PlayerControl->m_TargetOnControllerReset.Yaw : 0,
		AnimDriven ? AnimDriven->m_Weight : -1.f, AnimDriven ? AnimDriven->m_TargetWeight : -1.f, AnimDriven ? (INT)AnimDriven->m_bActive : -1,
		AnimDriven ? AnimDriven->m_PlayerControlledLimitFlags : -1 );
	GDisPCamRing[GDisPCamRingNext] = Line;
	GDisPCamRingNext = ( GDisPCamRingNext + 1 ) % ARRAY_COUNT( GDisPCamRing );
	static FLOAT LastReport = -1.f;
	const UBOOL bAfterExit = GDisPCamExitTime >= 0.f && Now - GDisPCamExitTime <= 2.f;
	if( bAfterExit || LastReport < 0.f || Now - LastReport >= 1.f )
	{
		LastReport = Now;
		debugf( TEXT("DISHONORED(bringup): pcamdiag %s %s"), bAfterExit ? TEXT("exit") : TEXT("camera"), *Line );
	}
}

static INT GDisFZDiag = -1;

UBOOL DisFZDiagEnabled()
{
	if( GDisFZDiag < 0 )
	{
		GDisFZDiag = ( appStrfind( appCmdLine(), TEXT("-fzdiag") ) != NULL ) ? 1 : 0;
	}
	return GDisFZDiag == 1;
}

void DisFZReportCamera( ADishonoredPlayerCamera* Camera, const FTViewTarget& OutVT, APawn* Pawn, FLOAT DeltaTime )
{
	DisPCamReport( Camera, OutVT, Pawn );
	if( !DisFZDiagEnabled() || !Camera )
	{
		return;
	}
	static FLOAT LastReport = -1.f;
	const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
	if( LastReport >= 0.f && Now - LastReport < 1.f )
	{
		return;
	}
	LastReport = Now;

	FString Weights;
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& Groups = Camera->InfluenceGroups();
	if( Groups.Num() > 0 && Groups(0) )
	{
		for( INT Index = 0; Index < Groups(0)->m_Influences.Num(); Index++ )
		{
			UDishonoredCameraInfluence* Influence = Groups(0)->m_Influences(Index);
			if( Influence && ( Influence->m_Weight > 0.001f || Influence->m_TargetWeight > 0.001f ) )
			{
				Weights += FString::Printf( TEXT("%s %.2f->%.2f%s "), *Influence->GetClass()->GetName(), Influence->m_Weight, Influence->m_TargetWeight, Influence->IsActive() ? TEXT("*") : TEXT("") );
			}
		}
	}
	UDishonoredCamera_AnimDriven* AnimDriven = Camera->GetCamera_AnimDriven();
	FString Limits;
	if( AnimDriven )
	{
		const FLOAT ToDeg = 360.f / 65536.f;
		Limits = FString::Printf( TEXT("limits yaw %.1f..%.1f pitch %.1f..%.1f deg flags %d, look yaw %.1f pitch %.1f (blended %.1f/%.1f)"),
			AnimDriven->m_CurMaxPlayerControlledYaw_Neg * ToDeg, AnimDriven->m_CurMaxPlayerControlledYaw_Pos * ToDeg,
			AnimDriven->m_CurMaxPlayerControlledPitch_Neg * ToDeg, AnimDriven->m_CurMaxPlayerControlledPitch_Pos * ToDeg,
			AnimDriven->m_PlayerControlledLimitFlags, AnimDriven->m_CurPlayerControlledYaw * ToDeg, AnimDriven->m_CurPlayerControlledPitch * ToDeg,
			AnimDriven->m_CurPlayerControlledYaw_Blended * ToDeg, AnimDriven->m_CurPlayerControlledPitch_Blended * ToDeg );
	}
	ADishonoredPlayerPawn* Player = Cast<ADishonoredPlayerPawn>( Pawn );
	FString StateName( TEXT("no FSM") );
	if( Player && Player->GetPlayerMasterFSM() )
	{
		StateName = Player->GetPlayerMasterFSM()->GetCurrentlyActiveState() ? Player->GetPlayerMasterFSM()->GetCurrentlyActiveState()->GetClass()->GetName() : FString(TEXT("NULL"));
	}
	debugf( TEXT("DISHONORED(bringup): fzdiag camera t=%.2f FOV %.2f (cur %.2f default %.2f) loc %.1f/%.1f/%.1f rot %d/%d/%d groups %d [%s] %s state %s body %d physics %d"),
		Now, OutVT.POV.FOV, Camera->GetCurFOV(), Camera->DefaultFOV,
		OutVT.POV.Location.X, OutVT.POV.Location.Y, OutVT.POV.Location.Z, OutVT.POV.Rotation.Pitch, OutVT.POV.Rotation.Yaw, OutVT.POV.Rotation.Roll,
		Groups.Num(), *Weights, *Limits, *StateName, Player ? (INT)Player->m_BodyMode : -1, Pawn ? (INT)Pawn->Physics : -1 );
}
