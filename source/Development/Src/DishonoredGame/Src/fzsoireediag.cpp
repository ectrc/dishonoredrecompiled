/*=============================================================================
	fzsoireediag.cpp: agent FZ (PHASE18 FZ) - the -fzdiag switch. Once a second, from ADishonoredPlayerCamera::
	UpdateViewTarget: the view the camera produced (FOV, location, rotation), the core influence group's weights, the
	AnimDriven influence's player-look limits and offsets, the player master FSM's current state and the body mode.
	Nothing here changes behaviour.
=============================================================================*/
#include "DishonoredGame.h"
#include "fzsoireediag.h"

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
