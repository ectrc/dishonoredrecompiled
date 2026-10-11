// GFxUI/Src/gfxuikismet.cpp
// DISHONORED(port): agent FP (PHASE16 FP). The two Kismet actions that open and close a GFx movie player.
//
// Retail's bodies are the reference ones in Src/ScaleformInteraction.cpp, line for line, and that unit is excluded
// by GFxUI/Sources.cmake because the rest of it is the GFx-4 glue this tree does not build; its two action bodies
// also sit under WITH_GFx, which cmake/DishonoredDefines.cmake sets to 0 in every module. So until wave 20 neither
// class had an Activated override at all: the op was counted as activated by the Kismet census, dispatched into
// USequenceAction::Activated, and opened nothing - which is the measured gap behind agent DC's -gfxuimenu bring-up
// ("a run at -startmap=Dishonored_MainMenu ... never constructs a UGFxMoviePlayer"). This is where retail opens the
// MAIN MENU: Dishonored_MainMenu's Main_Sequence.DisGFxActionOpenMovie_0 (a UDisGFxActionOpenMovie, whose only
// addition is m_pTweaks, which UDisGFxMoviePlayerBase::PreLoad reads) runs on LevelLoaded and constructs the player
// retail's object dump shows as ...Main_Sequence.DisGFxActionOpenMovie.DisGFxMoviePlayerMainMenu (agent FB's ue.log).
//
//   UGFxAction_OpenMovie::Activated    2013 rva 0x59d3a0 (0x47b bytes)   2012 0x5e51a0 (0x48e)
//   UGFxAction_CloseMovie::Activated   2013 rva 0x5874f0 (0x128 bytes)   2012 0x5e5630 (0x128)
//
// The 2013 open is pinned by its call to UGFxMoviePlayer::AddCaptureKey (0x59ad80, named) and its three UTF-16
// variable names; the 2012 one is named in the PDB. Differences from the reference body, both retail's: the
// player is constructed whenever MoviePlayerClass is set (the movie may be NULL and come from the tweaks), and
// RenderTextureMode is copied. The three bodies IMPLEMENT_CLASS'd in ScaleformInteraction.cpp are registered by
// GFxUIRegistrants.cpp, so nothing here registers.

#include "GFxUI.h"
#include "EngineSequenceClasses.h"
#include "GFxUIUISequenceClasses.h"

void UGFxAction_OpenMovie::Activated()
{
	if( GEngine == NULL || GEngine->GameViewport == NULL )
	{
		return;
	}
	Super::Activated();

	if( MoviePlayerClass != NULL )
	{
		MoviePlayer = Cast<UGFxMoviePlayer>( StaticConstructObject( MoviePlayerClass, GetOuter() ) );
		MoviePlayer->MovieInfo = Movie;
	}

	TArray<UObject**> ObjVars;
	GetObjectVars( ObjVars, TEXT("Movie Player") );
	for( INT Index = 0; Index < ObjVars.Num(); Index++ )
	{
		*ObjVars( Index ) = MoviePlayer;
	}

	if( MoviePlayer == NULL )
	{
		if( OutputLinks.Num() > 1 )
		{
			OutputLinks( 1 ).ActivateOutputLink();
		}
		return;
	}

	TArray<UObject**> ObjectVars;
	GetObjectVars( ObjectVars, TEXT("External Interface") );
	if( ObjectVars.Num() > 0 )
	{
		MoviePlayer->ExternalInterface = *ObjectVars( 0 );
	}
	ObjectVars.Empty();
	GetObjectVars( ObjectVars, TEXT("Player Owner") );
	if( ObjectVars.Num() > 0 )
	{
		APlayerController* Owner = Cast<APlayerController>( *ObjectVars( 0 ) );
		MoviePlayer->LocalPlayerOwnerIndex = GEngine->GamePlayers.FindItemIndex( Cast<ULocalPlayer>( Owner->Player ) );
	}

	MoviePlayer->bDisplayWithHudOff = bDisplayWithHudOff;
	MoviePlayer->RenderTextureMode = RenderTextureMode;
	MoviePlayer->RenderTexture = RenderTexture;
	for( INT Index = 0; Index < CaptureKeys.Num(); Index++ )
	{
		if( CaptureKeys( Index ) != NAME_None )
		{
			MoviePlayer->AddCaptureKey( CaptureKeys( Index ) );
		}
	}
	// retail reads CaptureKeys here too (a reference-engine slip retail kept)
	for( INT Index = 0; Index < FocusIgnoreKeys.Num(); Index++ )
	{
		if( FocusIgnoreKeys( Index ) != NAME_None )
		{
			MoviePlayer->AddFocusIgnoreKey( CaptureKeys( Index ) );
		}
	}

	const UBOOL bStarted = MoviePlayer->eventStart( bStartPaused );
	debugf( TEXT("DISHONORED(bringup): GFxAction_OpenMovie %s: %s %s through %s (focus %d input %d capture %d hudoff %d)"),
		*GetPathName(), bStarted ? TEXT("opened") : TEXT("FAILED to open"),
		MoviePlayer->MovieInfo ? *MoviePlayer->MovieInfo->GetPathName() : TEXT("(no MovieInfo)"),
		*MoviePlayer->GetClass()->GetName(), MoviePlayer->bAllowFocus ? 1 : 0, MoviePlayer->bAllowInput ? 1 : 0,
		MoviePlayer->bCaptureInput ? 1 : 0, MoviePlayer->bDisplayWithHudOff ? 1 : 0 );
	if( OutputLinks.Num() > 0 )
	{
		if( bStarted )
		{
			OutputLinks( 0 ).ActivateOutputLink();
		}
		else if( OutputLinks.Num() > 1 )
		{
			OutputLinks( 1 ).ActivateOutputLink();
		}
	}
}

void UGFxAction_CloseMovie::Activated()
{
	Super::Activated();
	TArray<UObject**> MovieVars;
	GetObjectVars( MovieVars, TEXT("Movie Player") );
	Movie = MovieVars.Num() > 0 ? Cast<UGFxMoviePlayer>( *MovieVars( 0 ) ) : NULL;
	if( Movie != NULL )
	{
		Movie->Close( bUnload );
		if( OutputLinks.Num() > 0 )
		{
			OutputLinks( 0 ).ActivateOutputLink();
		}
	}
	else if( OutputLinks.Num() > 1 )
	{
		OutputLinks( 1 ).ActivateOutputLink();
	}
}
