/*=============================================================================
	UnFaceFXSupport.cpp: FaceFX support.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_FACEFX

#include "UnFaceFXSupport.h"
#include "UnFaceFXRegMap.h"
#include "UnFaceFXMaterialParameterProxy.h"
#include "UnFaceFXMorphTargetProxy.h"

#include "FxGenericTargetNode.h"

// DISHONORED(port): agent FX3 (wave 26). The SDK is compiled into the exe from External/FaceFX (cmake/FaceFX.cmake), as
// retail compiles it; the reference's FxSDK_Unreal.lib pragmas and the integration's own UE3 node headers
// (../FaceFX/UnFaceFX*Node.h, FX2 ported those classes into FxGenericTargetNode.h) are gone.
// UnInitFaceFX = 2012 0x568670 / 2013 0x527510 (bytes 1.000): the custom allocation policy, no memory pools, appMalloc /
// appFree (FxAllocateUE3 / FxFreeUE3 2012 0x568060; retail's debug allocator is FxAllocateUE3 itself, ICF-folded), the
// compiled graph's pre-destroy and pre-compile callbacks. The two debugf lines are the reference's (Shipping strips them).

using namespace OC3Ent;
using namespace Face;

void* FxAllocateUE3( FxSize NumBytes )
{
	return appMalloc(NumBytes);
}

void* FxAllocateDebugUE3( FxSize NumBytes, const FxChar* /*system*/ )
{
	return appMalloc(NumBytes);
}

void FxFreeUE3( void* Ptr, FxSize /*n*/ )
{
	appFree(Ptr);
}

void FxRemoveNodeUserDataUE3( FxCompiledFaceGraph& cg )
{
	FxSize numNodes = cg.nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( cg.nodes[i].pUserData )
		{
			switch( cg.nodes[i].nodeType )
			{
			case NT_MorphTargetUE3:
				{
					if( cg.nodes[i].pUserData )
					{
						FFaceFXMorphTargetProxy* pUserData = reinterpret_cast<FFaceFXMorphTargetProxy*>(cg.nodes[i].pUserData);
						delete pUserData;
						cg.nodes[i].pUserData = NULL;
					}
				}
				break;
			case NT_MaterialParameterUE3:
				{
					if( cg.nodes[i].pUserData )
					{
						FFaceFXMaterialParameterProxy* pUserData = reinterpret_cast<FFaceFXMaterialParameterProxy*>(cg.nodes[i].pUserData);
						delete pUserData;
						cg.nodes[i].pUserData = NULL;
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void FxPreDestroyCallbackFuncUE3( FxCompiledFaceGraph& cg )
{
	FxRemoveNodeUserDataUE3(cg);
}

void FxPreCompileCallbackFuncUE3( FxCompiledFaceGraph& cg, FxFaceGraph& /*faceGraph*/ )
{
	FxRemoveNodeUserDataUE3(cg);
}

// This code assumes that only one thread will initialize/shutdown FaceFX...
// This is done to prevent multiple calls to UnShutdownFaceFX from actually
// crashing the application.
UBOOL GFaceFXIsInitialized = FALSE;
void UnInitFaceFX( void )
{
	if (GFaceFXIsInitialized == FALSE)
	{
		GFaceFXIsInitialized = TRUE;

		debugf(NAME_Init,TEXT("Initializing FaceFX..."));
		FxMemoryAllocationPolicy allocPolicyUE3(MAT_Custom, FxFalse, FxAllocateUE3, FxAllocateDebugUE3, FxFreeUE3);
		FxSDKStartup(allocPolicyUE3);
		FxCompiledFaceGraph::SetPreDestroyCallback(FxPreDestroyCallbackFuncUE3);
		FxCompiledFaceGraph::SetCompilationCallbacks(FxPreCompileCallbackFuncUE3, NULL, NULL, NULL);
		FFaceFXRegMap::Startup();
		debugf(NAME_Init,TEXT("FaceFX %s initialized."),*FString(FxSDKGetVersionString().GetCstr()));
	}
}

void UnShutdownFaceFX( void )
{
	if (GFaceFXIsInitialized == TRUE)
	{
		debugf(TEXT("Shutting down FaceFX..."));
		FFaceFXRegMap::Shutdown();
		FxSDKShutdown();
		debugf(TEXT("FaceFX shutdown."));
		GFaceFXIsInitialized = FALSE;
	}
}

TArray<FFaceFXRegMapEntry> FFaceFXRegMap::RegMap;

void FFaceFXRegMap::Startup( void )
{
}

void FFaceFXRegMap::Shutdown( void )
{
	RegMap.Empty();
}

FFaceFXRegMapEntry* FFaceFXRegMap::GetRegisterMapping( const FName& RegName )
{
	INT NumRegMapEntries = RegMap.Num();
	for( INT i = 0; i < NumRegMapEntries; ++i )
	{
		if( RegMap(i).UnrealRegName == RegName )
		{
			return &RegMap(i);
		}
	}
	return NULL;
}

void FFaceFXRegMap::AddRegisterMapping( const FName& RegName )
{
	if( !GetRegisterMapping(RegName) )
	{
		RegMap.AddItem(FFaceFXRegMapEntry(RegName, FxName(TCHAR_TO_ANSI(*RegName.ToString()))));
	}
}

#endif // WITH_FACEFX

