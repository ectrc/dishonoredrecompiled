/*=============================================================================
	DishonoredLoadAll.cpp: -loadall=<pkg+pkg|@listfile> streaming-package sweep (agent AD, wave 4).

	DISHONORED(bringup): after "Initial startup" every listed package is loaded with UObject::LoadPackage, one
	"DISHONORED(bringup): loadall <pkg>: <N> exports, <E> errors" line is logged per package and the process exits.
	The linker aborts (ULinkerLoad::IndexToObject "Bad export index", Preload "Serial size mismatch", "Bad name index")
	go through appErrorf, which under GIsGuarded throws (FOutputDeviceWindowsError::Serialize) after
	UObject::StaticShutdownAfterError has run; the sweep catches the throw, counts it, logs the package line with
	the GErrorHist text and exits right away since the object system is shut down (the driver
	build/agentAD/loadall_driver.py restarts the sweep at the next package). No retail counterpart.

	DISHONORED(bringup): agent AX additions for the milestone 3 exit sweep over all 471 cooked packages
	(additive; agent AD owns this file). -loadallpurge collects with RF_Native|RF_Marked instead of
	RF_Native|RF_Marked|RF_Standalone so the swept package's assets are actually reclaimed - without it a
	few dozen packages exhaust the 32-bit address space, with it the whole cooked tree sweeps in one
	process. A LoadPackage that returns NULL (an unresolvable package name) is counted and the sweep goes
	on: only a linker abort shuts the object system down and forces the exit. A second line,
	"loadall purged <pkg>: <N> objects live", is logged after the collect: its absence for a package whose load
	line is there is a crash in the *teardown* of that package's objects, not in its serializers.
=============================================================================*/

#include "EnginePrivate.h"

static void DishonoredLoadAllParseList(const FString& Spec, TArray<FString>& OutPackages)
{
	FString Text = Spec;
	if( Text.StartsWith(TEXT("@")) )
	{
		const FString ListFile = Text.Mid(1);
		if( !appLoadFileToString(Text, *ListFile) )
		{
			debugf(TEXT("DISHONORED(bringup): loadall: cannot read list file %s"), *ListFile);
			return;
		}
		Text = Text.Replace(TEXT("\r"), TEXT("+")).Replace(TEXT("\n"), TEXT("+"));
	}
	TArray<FString> Parts;
	Text.ParseIntoArray(&Parts, TEXT("+"), TRUE);
	for( INT Index=0; Index<Parts.Num(); ++Index )
	{
		FString Name = Parts(Index).Trim().TrimTrailing();
		if( Name.Len() > 0 && !Name.StartsWith(TEXT("#")) )
		{
			OutPackages.AddItem(Name);
		}
	}
}

static INT DishonoredLoadAllCountExports(UPackage* Package, INT& OutCreated)
{
	OutCreated = 0;
	ULinkerLoad* Linker = Package->GetLinker();
	if( Linker != NULL )
	{
		for( INT ExportIndex=0; ExportIndex<Linker->ExportMap.Num(); ++ExportIndex )
		{
			if( Linker->ExportMap(ExportIndex)._Object != NULL )
			{
				++OutCreated;
			}
		}
		return Linker->ExportMap.Num();
	}
	INT Count = 0;
	for( TObjectIterator<UObject> It; It; ++It )
	{
		if( It->GetOutermost() == Package && *It != Package )
		{
			++Count;
		}
	}
	OutCreated = Count;
	return Count;
}

static UBOOL DishonoredLoadAllOne(const FString& PackageName, INT& OutExports, INT& OutCreated, FString& OutError, UBOOL& bOutFatal)
{
	OutExports = 0;
	OutCreated = 0;
	bOutFatal = FALSE;
	try
	{
		UPackage* Package = UObject::LoadPackage(NULL, *PackageName, LOAD_None);
		if( Package == NULL )
		{
			OutError = TEXT("LoadPackage returned NULL");
			return FALSE;
		}
		OutExports = DishonoredLoadAllCountExports(Package, OutCreated);
		return TRUE;
	}
	catch( ... )
	{
		// appErrorf ran UObject::StaticShutdownAfterError before throwing: nothing can be loaded after this
		bOutFatal = TRUE;
		OutError = FString(GErrorHist).Replace(TEXT("\r"), TEXT(" ")).Replace(TEXT("\n"), TEXT(" ")).Left(400);
		return FALSE;
	}
}

void DishonoredLoadAllPackages()
{
	FString Spec;
	if( !Parse(appCmdLine(), TEXT("-loadall="), Spec) )
	{
		return;
	}
	TArray<FString> Packages;
	DishonoredLoadAllParseList(Spec, Packages);
	const UBOOL bPurge = ParseParam(appCmdLine(), TEXT("loadallpurge"));
	const EObjectFlags KeepFlags = bPurge ? (RF_Native | RF_Marked) : (RF_Native | RF_Marked | RF_Standalone);
	debugf(TEXT("DISHONORED(bringup): loadall: %d packages%s"), Packages.Num(), bPurge ? TEXT(", purging") : TEXT(""));

	INT TotalErrors = 0;
	for( INT Index=0; Index<Packages.Num(); ++Index )
	{
		const FString& PackageName = Packages(Index);
		INT Exports = 0;
		INT Created = 0;
		FString Error;
		UBOOL bFatal = FALSE;
		const DOUBLE StartTime = appSeconds();
		const UBOOL bLoaded = DishonoredLoadAllOne(PackageName, Exports, Created, Error, bFatal);
		if( bLoaded )
		{
			// the load result is logged before the collect, so a package whose *teardown* crashes still has its load line
			debugf(TEXT("DISHONORED(bringup): loadall %s: %d exports, 0 errors (%d objects created, %.2fs)"),
				*PackageName, Exports, Created, appSeconds() - StartTime);
			UObject::CollectGarbage(KeepFlags);
			debugf(TEXT("DISHONORED(bringup): loadall purged %s: %d objects live"), *PackageName, UObject::GetObjectArrayNum());
		}
		else
		{
			++TotalErrors;
			debugf(TEXT("DISHONORED(bringup): loadall %s: %d exports, 1 errors: %s"), *PackageName, Exports, *Error);
			if( bFatal )
			{
				debugf(TEXT("DISHONORED(bringup): loadall: aborting after the error in %s (%d of %d packages done), object system shut down by appErrorf"),
					*PackageName, Index + 1, Packages.Num());
				GLog->Flush();
				appRequestExit(TRUE);
				return;
			}
			UObject::CollectGarbage(KeepFlags);
		}
	}
	debugf(TEXT("DISHONORED(bringup): loadall done: %d packages, %d errors"), Packages.Num(), TotalErrors);
	GLog->Flush();
	appRequestExit(FALSE);
}
