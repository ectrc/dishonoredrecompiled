/*=============================================================================
	UnObjVer.cpp: Unreal version definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

// Used by the build system to set the Windows version number - it is defined as MAJOR.MINOR.ENGINE.PRIVATE
#define MAJOR_VERSION			1
#define MINOR_VERSION			0
#define PRIVATE_VERSION			131

// Defined separately so the build script can get to it easily (DO NOT CHANGE THIS MANUALLY)
#define	ENGINE_VERSION	9411	// DISHONORED: retail 2013 build (resources/docs/symbols/package_summary.md)

#define	BUILT_FROM_CHANGELIST	334700	// DISHONORED: Arkane changelist of the retail 2013 build


INT	GEngineVersion				= ENGINE_VERSION;
INT	GBuiltFromChangeList		= BUILT_FROM_CHANGELIST;

#if _XBOX
	// Prevent patched and unpatched network clients from seeing each other in system link
	// NOTE: This is not a Live problem, because you must patch to play on Live
	INT	GEngineMinNetVersion		= ENGINE_VERSION;
#else
	INT	GEngineMinNetVersion		= 9188;
#endif
INT	GEngineNegotiationVersion	= 3077;

// @see UnObjVer.h for the list of changes/defines
INT	GPackageFileVersion			= 801;	// DISHONORED: cooked packages are file version 801 (VER_PRESERVE_SMC_VERT_COLORS), not VER_LATEST_ENGINE
INT	GPackageFileMinVersion		= 491;
INT	GPackageFileLicenseeVersion = 30;	// DISHONORED: Arkane licensee version 30; ULinkerLoad rejects packages newer than this
INT GPackageFileCookedContentVersion = 133 | (0 << 16);	// DISHONORED: cooked content version of the 2013 packages (2012: 132)

