/*=============================================================================
	DishonoredLoadAll.cpp: -loadall=<pkg+pkg|@listfile> streaming-package sweep (agent AD, wave 4).

	DISHONORED(bringup): after "Initial startup" every listed package is loaded with UObject::LoadPackage, one
	"DISHONORED(bringup): loadall <pkg>: <N> exports, <E> errors" line is logged per package and the process exits.
	The linker aborts (ULinkerLoad::IndexToObject "Bad export index", Preload "Serial size mismatch", "Bad name index")
	go through appErrorf, which under GIsGuarded throws (FOutputDeviceWindowsError::Serialize) after
	UObject::StaticShutdownAfterError has run; the sweep catches the throw, counts it, logs the package line with
	the GErrorHist text and exits right away since the object system is shut down (the driver
	build/agentAD/loadall_driver.py restarts the sweep at the next package). No retail counterpart.
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

static UBOOL DishonoredLoadAllOne(const FString& PackageName, INT& OutExports, INT& OutCreated, FString& OutError)
{
	OutExports = 0;
	OutCreated = 0;
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
	debugf(TEXT("DISHONORED(bringup): loadall: %d packages"), Packages.Num());

	INT TotalErrors = 0;
	for( INT Index=0; Index<Packages.Num(); ++Index )
	{
		const FString& PackageName = Packages(Index);
		INT Exports = 0;
		INT Created = 0;
		FString Error;
		const DOUBLE StartTime = appSeconds();
		const UBOOL bLoaded = DishonoredLoadAllOne(PackageName, Exports, Created, Error);
		if( bLoaded )
		{
			debugf(TEXT("DISHONORED(bringup): loadall %s: %d exports, 0 errors (%d objects created, %.2fs)"), *PackageName, Exports, Created, appSeconds() - StartTime);
			UObject::CollectGarbage(RF_Native | RF_Marked | RF_Standalone);
		}
		else
		{
			++TotalErrors;
			debugf(TEXT("DISHONORED(bringup): loadall %s: %d exports, 1 errors: %s"), *PackageName, Exports, *Error);
			debugf(TEXT("DISHONORED(bringup): loadall: aborting after the error in %s (%d of %d packages done), object system shut down by appErrorf"),
				*PackageName, Index + 1, Packages.Num());
			GLog->Flush();
			appRequestExit(TRUE);
			return;
		}
	}
	debugf(TEXT("DISHONORED(bringup): loadall done: %d packages, %d errors"), Packages.Num(), TotalErrors);
	GLog->Flush();
	appRequestExit(FALSE);
}
