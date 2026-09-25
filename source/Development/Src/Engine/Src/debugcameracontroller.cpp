// Engine/src/debugcameracontroller.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x101b10  public: static class UClass * __cdecl ADebugCameraController::StaticClassNoInline(void)
//   0xd6cb0  public: static void __cdecl ADebugCameraController::InitializePrivateStaticClassADebugCameraController(void)
//   0xd6cd0  public: virtual void __thiscall ADebugCameraController::UpdateHiddenComponents(class FVector const &, class TSet<class UPrimitiveComponent *, struct DefaultKeyFuncs<class UPrimitiveComponent *, 0>, class FDefaultSetAllocator> &)
//   0xdb630  public: virtual void __thiscall ADebugCameraController::SecondarySelect(class FVector, class FVector, struct FTraceHitInfo)
//   0xe94b0  public: virtual void __thiscall ADebugCameraController::PrimarySelect(class FVector, class FVector, struct FTraceHitInfo)
//   0xe95f0  public: virtual void __thiscall ADebugCameraController::Unselect(void)
//   0xeeaf0  public: virtual class FString __thiscall ADebugCameraController::ConsoleCommand(class FString const &, unsigned int)
//   0xfddb0  public: static class UClass * __cdecl ADebugCameraController::GetPrivateStaticClassADebugCameraController(wchar_t const *)

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "DebugCameraController.h"

// DISHONORED(port): moved from GameFramework/Src/DebugCameraController.cpp (reference 10897) to Engine like retail. 2013 rvas:
// PrimarySelect 0xe9020, SecondarySelect 0xdcea0, Unselect 0xe9160, ConsoleCommand 0xed9b0, UpdateHiddenComponents 0xd92d0,
// execPrimarySelect 0x1d3dd0, execUnselect 0x5f3c30 -- same bodies as the reference (the Track/UntrackTexture calls are folded
// into an empty function in the retail build); member offsets as decompiled (Player @896, OryginalControllerRef @1348,
// SelectedActor @1360, SelectedComponent @1364).
IMPLEMENT_CLASS(ADebugCameraController);
static_assert(sizeof(ADebugCameraController) == 1376, "ADebugCameraController: retail 2013 size is 1376");

FNativeFunctionLookup GEngineADebugCameraControllerNatives[] =
{
	MAP_NATIVE(ADebugCameraController, execConsoleCommand)
	MAP_NATIVE(ADebugCameraController, execUnselect)
	MAP_NATIVE(ADebugCameraController, execSecondarySelect)
	MAP_NATIVE(ADebugCameraController, execPrimarySelect)
	{NULL, NULL}
};

/** The currently selected actor. */
AActor* GDebugSelectedActor = NULL;
/** The currently selected component in the actor. */
UPrimitiveComponent* GDebugSelectedComponent = NULL;
/** The lightmap used by the currently selected component, if it's a static mesh component. */
FLightMap2D* GDebugSelectedLightmap = NULL;

extern UBOOL UntrackTexture( const FString& TextureName );
extern UBOOL TrackTexture( const FString& TextureName );

/**
 * Called when an actor has been selected with the primary key (e.g. left mouse button).
 *
 * @param HitLoc	World-space position of the selection point.
 * @param HitNormal	World-space normal of the selection point.
 * @param HitInfo	Info struct for the selection point.
 */
void ADebugCameraController::PrimarySelect( FVector HitLoc, FVector HitNormal, FTraceHitInfo HitInfo )
{
	// First untrack the currently tracked lightmap.
	UTexture2D* Texture2D = GDebugSelectedLightmap ? GDebugSelectedLightmap->GetTexture(0) : NULL;
	if ( Texture2D )
	{
		UntrackTexture( Texture2D->GetName() );
	}

	GDebugSelectedActor = SelectedActor;
	GDebugSelectedComponent = SelectedComponent;
	GDebugSelectedLightmap = NULL;
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>( GDebugSelectedComponent );
	if ( StaticMeshComponent && StaticMeshComponent->LODData.Num() > 0 )
	{
		const FStaticMeshComponentLODInfo& LODInfo = StaticMeshComponent->LODData(0);
		if ( LODInfo.LightMap )
		{
			GDebugSelectedLightmap = LODInfo.LightMap->GetLightMap2D();
			UTexture2D* Texture2D = GDebugSelectedLightmap ? GDebugSelectedLightmap->GetTexture(0) : NULL;
			if ( Texture2D )
			{
				extern UBOOL TrackTexture( const FString& TextureName );
				TrackTexture( Texture2D->GetName() );
			}
		}
	}
}

/**
 * Called when an actor has been selected with the secondary key (e.g. right mouse button).
 *
 * @param HitLoc	World-space position of the selection point.
 * @param HitNormal	World-space normal of the selection point.
 * @param HitInfo	Info struct for the selection point.
 */
void ADebugCameraController::SecondarySelect( FVector HitLoc, FVector HitNormal, FTraceHitInfo HitInfo )
{
	PrimarySelect( HitLoc, HitNormal, HitInfo );
}

/**
 * Called when the user pressed the unselect key, just before the selected actor is cleared.
 */
void ADebugCameraController::Unselect()
{
	UTexture2D* Texture2D = GDebugSelectedLightmap ? GDebugSelectedLightmap->GetTexture(0) : NULL;
	if ( Texture2D )
	{
		extern UBOOL UntrackTexture( const FString& TextureName );
		UntrackTexture( Texture2D->GetName() );
	}

	GDebugSelectedActor = NULL;
	GDebugSelectedComponent = NULL;
	GDebugSelectedLightmap = NULL;
}



/**
 * This is the same as PlayerController::ConsoleCommand(), except with some extra code to 
 * give our regular PC a crack at handling the command.
 */
FString ADebugCameraController::ConsoleCommand(const FString& Cmd,UBOOL bWriteToLog)
{
	if (Player != NULL)
	{
		UConsole* ViewportConsole = (GEngine->GameViewport != NULL) ? GEngine->GameViewport->ViewportConsole : NULL;
		FConsoleOutputDevice StrOut(ViewportConsole);
	
		const INT CmdLen = Cmd.Len();
		TCHAR* CommandBuffer = (TCHAR*)appMalloc((CmdLen+1)*sizeof(TCHAR));
		TCHAR* Line = (TCHAR*)appMalloc((CmdLen+1)*sizeof(TCHAR));

		const TCHAR* Command = CommandBuffer;
		// copy the command into a modifiable buffer
		appStrcpy(CommandBuffer, (CmdLen+1), *Cmd.Left(CmdLen)); 

		// iterate over the line, breaking up on |'s
		while (ParseLine(&Command, Line, CmdLen+1))	// The ParseLine function expects the full array size, including the NULL character.
		{
			if (Player->Exec(Line, StrOut) == FALSE)
			{
				Player->Actor = OryginalControllerRef;
				Player->Exec(Line, StrOut);
				Player->Actor = this;
			}
		}

		// Free temp arrays
		appFree(CommandBuffer);
		CommandBuffer=NULL;

		appFree(Line);
		Line=NULL;

		if (!bWriteToLog)
		{
			return *StrOut;
		}
	}

	return TEXT("");
}

/**
 * Builds a list of components that are hidden based upon gameplay
 *
 * @param ViewLocation the view point to hide/unhide from
 * @param HiddenComponents the list to add to/remove from
 */
void ADebugCameraController::UpdateHiddenComponents(const FVector& ViewLocation,TSet<UPrimitiveComponent*>& HiddenComponents)
{
	if (OryginalControllerRef != NULL)
	{
		OryginalControllerRef->UpdateHiddenComponents(ViewLocation,HiddenComponents);
	}
}

