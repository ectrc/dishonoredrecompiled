/*=============================================================================
	ShowFlags.h: Show Flag Definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_SHOW_FLAGS
#define _INC_SHOW_FLAGS

/*-----------------------------------------------------------------------------
	EShowFlags
-----------------------------------------------------------------------------*/

// DISHONORED(layout): Dishonored's engine keeps the pre-2011 UE3 64-bit show flags, not the reference
// TStaticBitArray<128>: the 2012 Shipping PDB types UGameViewportClient::ShowFlags and
// FSceneViewFamily::ShowFlags as unsigned __int64 (@96 / @24) and USceneCaptureComponent::GetSceneShowFlags
// returns unsigned __int64 (rva 0x2e65d0); the retail 2013 SDK dump has UGameViewportClient::ShowFlags as
// FQWord @96 (sizeof 284). The bit assignments are in Scene.h (SHOW_*), each cited to the 2012/2013
// decompiles (resources/docs/agents/agentV.md). The CONSOLE && FINAL_RELEASE FShippingShowFlags wrapper of
// the reference is gone: this is a Win32 build.
typedef QWORD EShowFlags;

#endif // _INC_SHOW_FLAGS
