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

// DISHONORED(retail): 1274963 is the DWORD at retail rva 0xe6a6e4 (?GBuiltFromChangeList@@3HA) in
// Dishonored_Latest2026/Binaries/Win32/Dishonored.exe; 2012 Shipping carries 254295 at 0xe2a6e8. It was 334700 here,
// which is not either build's value, and DisSaveLoad::FGameState::Load (2013 rva 0x614020) refuses any save whose
// stored changelist is greater than ours - so with 334700 every retail .sav (all carry 1274963) was rejected.
#define	BUILT_FROM_CHANGELIST	1274963


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
INT	GPackageFileVersion			= VER_DIS_LATEST_ENGINE;	// DISHONORED: cooked packages are file version 801, not VER_LATEST_ENGINE (DISHONORED(port): named in UnObjVer.h)
INT	GPackageFileMinVersion		= 491;
INT	GPackageFileLicenseeVersion = VER_LATEST_ENGINE_LICENSEE;	// DISHONORED: Arkane licensee version 30; ULinkerLoad rejects packages newer than this
INT GPackageFileCookedContentVersion = 133 | (0 << 16);	// DISHONORED: cooked content version of the 2013 packages (2012: 132)

