/*=============================================================================
	DynamicRHI.cpp: Dynamically bound Render Hardware Interface implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if USE_DYNAMIC_RHI

// External dynamic RHI factory functions.
extern FDynamicRHI* NullCreateRHI();
#if PLATFORM_DESKTOP && !USE_NULL_RHI
	#if _WINDOWS
		extern FDynamicRHI* D3D9CreateRHI();
		extern FDynamicRHI* D3D11CreateRHI();
		extern UBOOL IsDirect3D11Supported(UBOOL& OutSupportsD3D11Features);
		#if USE_DYNAMIC_ES2_RHI
			extern FDynamicRHI* ES2CreateRHI();
		#endif
	#endif
	extern FDynamicRHI* OpenGLCreateRHI();
#endif

#if PLATFORM_DESKTOP
/** In bytes. */
INT GCurrentTextureMemorySize = 0;
/** In bytes. 0 means unlimited. */
INT GTexturePoolSize = 0 * 1024 * 1024;
/** Whether to read the texture pool size from engine.ini on PC. Can be turned on with -UseTexturePool on the command line. */
UBOOL GReadTexturePoolSizeFromIni = FALSE;
#endif

// Globals.
FDynamicRHI* GDynamicRHI = NULL;

void RHIInit( UBOOL bIsEditor )
{
	if(!GDynamicRHI)
	{		
#if USE_NULL_RHI
		// Use the null RHI explicitly.
		GDynamicRHI = NullCreateRHI();
		GUsingNullRHI = TRUE;
#else
		const TCHAR* CmdLine = appCmdLine();
		FString Token = ParseToken(CmdLine, FALSE);

		if ( ParseParam(appCmdLine(),TEXT("UseTexturePool")) )
		{
			GReadTexturePoolSizeFromIni = TRUE;
		}

		if(ParseParam(appCmdLine(),TEXT("nullrhi")) || GIsUCC || Token == TEXT("SERVER"))
		{
			// Use the null RHI if it was specified on the command line, or if a commandlet is running.
			GDynamicRHI = NullCreateRHI();
			GUsingNullRHI = TRUE;
		}
#if _WINDOWS
		else
		{
#if USE_DYNAMIC_ES2_RHI
			UBOOL bWantsES2 = FALSE;

			if( ParseParam( appCmdLine(), TEXT("es2") ) ||
				ParseParam( appCmdLine(), TEXT("simmobile") ) )
			{
				bWantsES2 = TRUE;
			}
			
			// Only display the conflict message if we are in the editor but not running a commandlet
			if( bIsEditor && !GIsUCC && bWantsES2 )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd( "Startup_ES2NotSupportedInEditorMode" ) );
				bWantsES2 = FALSE;
			}

			if( bWantsES2 )
			{
				GDynamicRHI = ES2CreateRHI();
				// we don't allow texture streaming with ES2

				GUseTextureStreaming = FALSE;
			}
			else
#endif
			{
				// DISHONORED(retail): the retail exe has one PC RHI, D3D9 (no D3D11/OpenGL dll, no RENDER_MODE_DX11 path:
				// 2013 rva 0x5bc1e0 creates the device directly). The reference block forced bForceD3D11 = TRUE (a UDK
				// leftover) and warned "Command line -d3d11 set" on every start.
				GForcedRenderMode = RENDER_MODE_DX9;
				GDynamicRHI = D3D9CreateRHI();
			}
		}
#elif PLATFORM_MACOSX
		GDynamicRHI = OpenGLCreateRHI();
#endif // _WINDOWS
#endif // USE_NULL_RHI
		check(GDynamicRHI);
	}
}

void RHIExit()
{
	// Destruct the dynamic RHI.
	delete GDynamicRHI;
	GDynamicRHI = NULL;
}


#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT DynamicRHILinkerHelper;

#endif // USE_DYNAMIC_RHI


#if !CONSOLE || USE_NULL_RHI

/**
 * Defragment the texture pool.
 */
void appDefragmentTexturePool()
{
}

/**
 * Checks if the texture data is allocated within the texture pool or not.
 */
UBOOL appIsPoolTexture( FTextureRHIParamRef TextureRHI )
{
	return FALSE;
}

/**
 * Log the current texture memory stats.
 *
 * @param Message	This text will be included in the log
 */
void appDumpTextureMemoryStats(const TCHAR* /*Message*/)
{
}

#endif	//#if !CONSOLE
