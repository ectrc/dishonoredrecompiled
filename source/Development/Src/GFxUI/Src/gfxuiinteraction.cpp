// GFxUI/src/gfxuiinteraction.cpp
// PDB functions attributed to this file (61). This unit ports the eleven that make UGFxInteraction the
// interface's per-frame owner; the rest of the list is the six Kismet actions, the FSCommand event and
// the class boilerplate, which live in Src/gfxuinatives.cpp and the generated registrants.
//
// ---------------------------------------------------------------------------------------------
// DISHONORED(port): UGFxInteraction is where retail advances the interface and where the viewport's
// input enters it. Measured from the retail database rather than assumed (build/agentDC/xr.py):
//   UGFxInteraction::Init              2013 0x5a2c40  -> FGFxEngine::GetEngine, then registers for
//                                                       CALLBACK_ViewportResized
//   UGFxInteraction::Tick              2013 0x57b790  -> FGFxEngine::Tick
//   UGFxInteraction::SetRenderViewport 2013 0x57b770  -> FGFxEngine::SetRenderViewport
//   UGFxInteraction::Send              2013 0x57b860  -> the same, on a resize of the HUD viewport
//   UGFxInteraction::InputKey          2013 0x591f10  -> FGFxEngine::InputKey  (+ RefreshIsUsingGamepad)
//   UGFxInteraction::InputChar         2013 0x591fd0  -> FGFxEngine::InputChar
//   UGFxInteraction::InputAxis         2013 0x595140  -> FGFxEngine::InputAxis
//   UGFxInteraction::NotifyPlayerAdded/Removed        -> FGFxEngine::AddPlayerState / RemovePlayerState
//   UGFxInteraction::Exec              2012 0x5e4af0  -> the gfx console commands
// Every one of those bodies tests GRenderGFxUI first, which is retail's own runtime switch for the
// interface; this tree spells it GDrawGFx (Engine/Src/UnEngine.cpp's "DrawGFx" Exec) and the engine
// checks it in RenderUI, so the guard here is the engine pointer only.
#include "GFxUI.h"
#include "gfxui_gfx3.h"

#if DISHONORED_GFXUI_GFX3_RUNTIME

// DISHONORED(port): 2013 0x5a2c40 (2012 0x5e3530)
void UGFxInteraction::Init()
{
	Super::Init();
	if( GGFxEngine == NULL )
	{
		FGFxEngine::GetEngine();
	}
	GCallbackEvent->Register( CALLBACK_ViewportResized, this );
}

// DISHONORED(port): 2013 0x57b790 (2012 0x5bffa0)
void UGFxInteraction::Tick( FLOAT DeltaTime )
{
	Super::Tick( DeltaTime );
	if( GGFxEngine != NULL )
	{
		GGFxEngine->Tick( DeltaTime );
	}
}

// DISHONORED(port): 2013 0x57b770 (2012 0x5bff80)
void UGFxInteraction::SetRenderViewport( FViewport* InViewport )
{
	if( GGFxEngine != NULL )
	{
		GGFxEngine->SetRenderViewport( InViewport );
	}
}

// DISHONORED(port): 2013 0x57b860 (2012 0x5bffd0)
void UGFxInteraction::Send( ECallbackEventType InType, FViewport* InViewport, UINT InMessage )
{
	(void)InMessage;
	if( GGFxEngine != NULL && InType == CALLBACK_ViewportResized
		&& GGFxEngine->GetRenderViewport() == InViewport )
	{
		GGFxEngine->SetRenderViewport( InViewport );
	}
}

// DISHONORED(port): 2013 0x591f10 (2012 0x5d22d0). The modifier keys deliberately do not count as
// "the player used the gamepad", which is what the six FName comparisons in the retail body are.
UBOOL UGFxInteraction::InputKey( INT ControllerId, FName Key, EInputEvent Event,
	FLOAT AmountDepressed, UBOOL bGamepad )
{
	(void)AmountDepressed;
	(void)bGamepad;
	if( GGFxEngine == NULL )
	{
		return FALSE;
	}
	return GGFxEngine->InputKey( ControllerId, Key, Event );
}

// DISHONORED(port): 2013 0x591fd0 (2012 0x5d2390)
UBOOL UGFxInteraction::InputChar( INT ControllerId, TCHAR Character )
{
	if( GGFxEngine == NULL )
	{
		return FALSE;
	}
	return GGFxEngine->InputChar( ControllerId, Character );
}

// DISHONORED(port): 2013 0x595140 (2012 0x5d4f40)
UBOOL UGFxInteraction::InputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime,
	UBOOL bGamepad )
{
	if( GGFxEngine == NULL )
	{
		return FALSE;
	}
	return GGFxEngine->InputAxis( ControllerId, Key, Delta, DeltaTime, bGamepad );
}

// DISHONORED(port): 2012 0x5e4af0 - the console commands. Retail's list is longer (it dumps the GFx
// heaps and the resource report, which are GMemoryHeap's and GFxResourceReport's); these three are the
// ones this reconstruction can answer.
UBOOL UGFxInteraction::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( ParseCommand( &Cmd, TEXT("GFXUI") ) )
	{
		if( ParseCommand( &Cmd, TEXT("CENSUS") ) )
		{
			if( GGFxEngine != NULL )
			{
				GGFxEngine->LogCensus( TEXT("console") );
			}
			return TRUE;
		}
		if( ParseCommand( &Cmd, TEXT("MOVIES") ) )
		{
			if( GGFxEngine != NULL )
			{
				Ar.Logf( TEXT("%d open, %d all"), GGFxEngine->GetNumOpenMovies(),
					GGFxEngine->GetNumAllMovies() );
				for( INT Index = 0; Index < GGFxEngine->GetNumOpenMovies(); Index++ )
				{
					FGFxMovie* Movie = GGFxEngine->GetOpenMovie( Index );
					Ar.Logf( TEXT("  %2d %s  %dx%d  visible %d update %d"), Index, *Movie->FileName,
						Movie->Info.Width, Movie->Info.Height, Movie->fVisible, Movie->fUpdate );
				}
			}
			return TRUE;
		}
		if( ParseCommand( &Cmd, TEXT("CLOSE") ) )
		{
			if( GGFxEngine != NULL )
			{
				GGFxEngine->CloseTopmostScene();
			}
			return TRUE;
		}
	}
	return FALSE;
}

#endif // DISHONORED_GFXUI_GFX3_RUNTIME
