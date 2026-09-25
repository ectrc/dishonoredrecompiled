// CoreSmoke: links Core.lib and checks the basic contracts (P2.12). Exit code = number of failures.
#include "Core.h"

#include <dbghelp.h>

#include <cstdio>
#include <cstring>

namespace
{
int NumFailures = 0;
int NumPassed = 0;
int NumSkipped = 0;

void Report(bool Passed, const char* Name, const char* Detail = nullptr)
{
	if (Passed)
	{
		NumPassed++;
		printf("PASS  %s\n", Name);
	}
	else
	{
		NumFailures++;
		printf("FAIL  %s%s%s\n", Name, Detail ? " -- " : "", Detail ? Detail : "");
	}
}

void Skip(const char* Name, const char* Reason)
{
	NumSkipped++;
	printf("SKIP  %s -- %s\n", Name, Reason);
}

#define CHECK_EQ(Name, Actual, Expected) \
	do { \
		const long long ActualValue = (long long)(Actual); \
		const long long ExpectedValue = (long long)(Expected); \
		char Detail[256]; \
		snprintf(Detail, sizeof(Detail), "%s = %lld, expected %lld", #Actual, ActualValue, ExpectedValue); \
		Report(ActualValue == ExpectedValue, Name, Detail); \
	} while (0)

bool ReadHardcodedNameIndex(const char* Wanted, int& OutIndex)
{
	FILE* File = fopen("resources/docs/symbols/hardcoded_names.csv", "r");
	if (!File)
	{
		return false;
	}
	char Line[512];
	bool Found = false;
	while (fgets(Line, sizeof(Line), File))
	{
		char* Comma = strchr(Line, ',');
		if (!Comma)
		{
			continue;
		}
		*Comma = 0;
		char* Name = Comma + 1;
		Name[strcspn(Name, "\r\n")] = 0;
		if (strcmp(Name, Wanted) == 0)
		{
			OutIndex = atoi(Line);
			Found = true;
			break;
		}
	}
	fclose(File);
	return Found;
}

void TestSizes()
{
	CHECK_EQ("sizeof(FName) == 8 (sizes.csv)", sizeof(FName), 8);
	CHECK_EQ("sizeof(FString) == 12 (sizes.csv)", sizeof(FString), 12);
	CHECK_EQ("sizeof(UObject) == 56 (sizes.csv)", sizeof(UObject), 56);
	CHECK_EQ("sizeof(FPackageFileSummary) == 164 (sizes.csv)", sizeof(FPackageFileSummary), 164);
	CHECK_EQ("sizeof(FArchive) == 136 (sizes.csv)", sizeof(FArchive), 136);
	CHECK_EQ("sizeof(FMemoryReader) == 144 (sizes.csv)", sizeof(FMemoryReader), 144);
}

void TestAppHelpers()
{
	BYTE Buffer[32];
	memset(Buffer, 0xAB, sizeof(Buffer));
	appMemzero(Buffer, sizeof(Buffer));
	bool AllZero = true;
	for (BYTE Byte : Buffer)
	{
		AllZero = AllZero && Byte == 0;
	}
	Report(AllZero, "appMemzero clears the buffer");

	CHECK_EQ("appStrlen(TEXT(\"Dunwall\")) == 7", appStrlen(TEXT("Dunwall")), 7);
	CHECK_EQ("appStrlen(TEXT(\"\")) == 0", appStrlen(TEXT("")), 0);

	TCHAR Formatted[MAX_SPRINTF];
	const INT Written = appSprintf(Formatted, TEXT("%s-%d-%.1f"), TEXT("Rat"), 42, 1.5f);
	CHECK_EQ("appSprintf returns the character count", Written, 10);
	Report(appStrcmp(Formatted, TEXT("Rat-42-1.5")) == 0, "appSprintf formats %s %d %.1f");
}

void TestContainers()
{
	TArray<INT> Ints;
	for (INT Value = 0; Value < 100; Value++)
	{
		Ints.AddItem(Value * 3);
	}
	CHECK_EQ("TArray::AddItem x100 -> Num()", Ints.Num(), 100);
	CHECK_EQ("TArray element access", Ints(17), 51);
	Ints.Remove(0, 10);
	CHECK_EQ("TArray::Remove(0,10) -> Num()", Ints.Num(), 90);
	CHECK_EQ("TArray element after Remove", Ints(0), 30);
	CHECK_EQ("TArray::FindItemIndex", Ints.FindItemIndex(60), 10);
	Report(Ints.ContainsItem(297) && !Ints.ContainsItem(298), "TArray::ContainsItem");
	Ints.Empty();
	CHECK_EQ("TArray::Empty -> Num()", Ints.Num(), 0);

	FString Str(TEXT("Corvo"));
	Str += TEXT(" Attano");
	CHECK_EQ("FString += -> Len()", Str.Len(), 12);
	Report(Str == TEXT("Corvo Attano"), "FString operator==");
	Report(Str.Left(5) == TEXT("Corvo") && Str.Right(6) == TEXT("Attano"), "FString Left/Right");
	Report(Str.ToUpper() == TEXT("CORVO ATTANO"), "FString ToUpper");
	CHECK_EQ("FString InStr", Str.InStr(TEXT("Attano")), 6);
	const FString Printed = FString::Printf(TEXT("%d/%s"), 7, TEXT("x"));
	Report(Printed == TEXT("7/x"), "FString::Printf");
	TArray<FString> Parts;
	Str.ParseIntoArray(&Parts, TEXT(" "), TRUE);
	CHECK_EQ("FString::ParseIntoArray count", Parts.Num(), 2);

	TMap<FString, INT> Map;
	Map.Set(TEXT("Emily"), 10);
	Map.Set(TEXT("Daud"), 20);
	Map.Set(TEXT("Emily"), 11);
	CHECK_EQ("TMap::Set twice on the same key -> Num()", Map.Num(), 2);
	INT* Found = Map.Find(TEXT("Emily"));
	Report(Found && *Found == 11, "TMap::Find returns the updated value");
	Report(Map.Find(TEXT("Outsider")) == NULL, "TMap::Find on a missing key returns NULL");
	Map.Remove(TEXT("Daud"));
	CHECK_EQ("TMap::Remove -> Num()", Map.Num(), 1);

	TMap<INT, FName> NameMap;
	NameMap.Set(1, NAME_None);
	Report(NameMap.FindRef(1) == NAME_None, "TMap<INT,FName>::FindRef");
}

void TestNames()
{
	const FName None(NAME_None);
	CHECK_EQ("FName(NAME_None).GetIndex() == 0", None.GetIndex(), 0);
	Report(None.ToString() == TEXT("None"), "FName(NAME_None).ToString() == \"None\"");

	const FName Corvo(TEXT("Corvo"));
	Report(Corvo.ToString() == TEXT("Corvo"), "FName(\"Corvo\").ToString() round-trip");
	const FName CorvoAgain(TEXT("Corvo"));
	CHECK_EQ("FName(\"Corvo\") twice yields the same index", CorvoAgain.GetIndex(), Corvo.GetIndex());
	Report(Corvo == CorvoAgain, "FName equality");
	const FName CorvoLower(TEXT("corvo"));
	CHECK_EQ("FName lookup is case-insensitive", CorvoLower.GetIndex(), Corvo.GetIndex());
	Report(Corvo.GetIndex() > NAME_None, "New FName index is above the hardcoded range");
	const FName Missing(TEXT("DefinitelyNotAName_Xyz"), FNAME_Find);
	Report(Missing == NAME_None, "FName(..., FNAME_Find) of an unknown name is NAME_None");
	const FName Numbered(TEXT("Corvo_7"));
	Report(Numbered.GetNumber() == 8 && Numbered.GetIndex() == Corvo.GetIndex(), "FName(\"Corvo_7\") splits the instance number");

	int ExpectedEngine = -1;
	int ExpectedCore = -1;
	if (ReadHardcodedNameIndex("Engine", ExpectedEngine) && ReadHardcodedNameIndex("Core", ExpectedCore))
	{
		CHECK_EQ("NAME_Engine matches hardcoded_names.csv", (INT)NAME_Engine, ExpectedEngine);
		CHECK_EQ("NAME_Core matches hardcoded_names.csv", (INT)NAME_Core, ExpectedCore);
		CHECK_EQ("FName(\"Engine\").GetIndex() matches hardcoded_names.csv", FName(TEXT("Engine")).GetIndex(), ExpectedEngine);
		CHECK_EQ("FName(\"Core\").GetIndex() matches hardcoded_names.csv", FName(TEXT("Core")).GetIndex(), ExpectedCore);
	}
	else
	{
		Report(false, "NAME_Engine/NAME_Core vs hardcoded_names.csv", "resources/docs/symbols/hardcoded_names.csv not readable; run from the repo root");
	}
	Report(FName(NAME_Engine).ToString() == TEXT("Engine") && FName(NAME_Core).ToString() == TEXT("Core"), "NAME_Engine/NAME_Core ToString");
}

void TestPackageSummary()
{
	const char* Path = "D:/RecompileDishonored/Dishonored_Latest2026/DishonoredGame/CookedPCConsole/Core.upk";
	FILE* File = fopen(Path, "rb");
	if (!File)
	{
		Skip("FPackageFileSummary from Core.upk", "Core.upk not found");
		return;
	}
	TArray<BYTE> Bytes;
	Bytes.Add(64 * 1024);
	const size_t Read = fread(Bytes.GetData(), 1, Bytes.Num(), File);
	fclose(File);
	Bytes.Remove((INT)Read, Bytes.Num() - (INT)Read);
	Report(Read > 0, "Core.upk read (first 64 KB)");

	FMemoryReader Reader(Bytes, TRUE);
	FPackageFileSummary Summary;
	Reader << Summary;
	Report(!Reader.IsError(), "FMemoryReader << FPackageFileSummary without archive error");
	CHECK_EQ("Summary.Tag == PACKAGE_FILE_TAG", (DWORD)Summary.Tag, (DWORD)PACKAGE_FILE_TAG);
	CHECK_EQ("Summary.GetFileVersion() == 801", Summary.GetFileVersion(), 801);
	CHECK_EQ("Summary.GetFileVersionLicensee() == 30", Summary.GetFileVersionLicensee(), 30);
	CHECK_EQ("Summary.EngineVersion == 9411", Summary.EngineVersion, 9411);
	CHECK_EQ("Summary.NameCount == 720", Summary.NameCount, 720);
	CHECK_EQ("Summary.ExportCount == 1356", Summary.ExportCount, 1356);
	CHECK_EQ("Summary.ImportCount == 18", Summary.ImportCount, 18);
	CHECK_EQ("Summary.TotalHeaderSize == 116078", Summary.TotalHeaderSize, 116078);
	CHECK_EQ("Summary.PackageFlags == 0x22a80008", Summary.PackageFlags, 0x22a80008u);
	CHECK_EQ("Summary.CompressionFlags == 2", Summary.CompressionFlags, 2);
	CHECK_EQ("Summary.CompressedChunks.Num() == 1", Summary.CompressedChunks.Num(), 1);
	CHECK_EQ("Summary.Generations.Num() == 2", Summary.Generations.Num(), 2);
	CHECK_EQ("Summary.GetCookedContentVersion() == 133", Summary.GetCookedContentVersion() & 0xffff, 133);
	CHECK_EQ("Summary.PackageSource == 0xc082c6b6", Summary.PackageSource, 0xc082c6b6u);
	CHECK_EQ("Summary.AdditionalPackagesToCook.Num() == 0", Summary.AdditionalPackagesToCook.Num(), 0);
	Report(Summary.FolderName == TEXT("None"), "Summary.FolderName == \"None\"");
	CHECK_EQ("archive position after summary == 157 (summary_size)", Reader.Tell(), 157);
	CHECK_EQ("Summary matches the engine's package version (GPackageFileVersion)", Summary.GetFileVersion(), GPackageFileVersion);
	CHECK_EQ("Summary matches the engine's licensee version (GPackageFileLicenseeVersion)", Summary.GetFileVersionLicensee(), GPackageFileLicenseeVersion);
	CHECK_EQ("Summary matches the engine version (GEngineVersion)", Summary.EngineVersion, GEngineVersion);
}
}

LONG WINAPI CrashFilter(EXCEPTION_POINTERS* Info)
{
	const EXCEPTION_RECORD* Record = Info->ExceptionRecord;
	printf("\nCRASH exception 0x%08lX at 0x%p\n", Record->ExceptionCode, Record->ExceptionAddress);
	const HANDLE Process = GetCurrentProcess();
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	SymInitialize(Process, NULL, TRUE);
	CONTEXT Context = *Info->ContextRecord;
	STACKFRAME64 Frame;
	memset(&Frame, 0, sizeof(Frame));
	Frame.AddrPC.Offset = Context.Eip;
	Frame.AddrPC.Mode = AddrModeFlat;
	Frame.AddrFrame.Offset = Context.Ebp;
	Frame.AddrFrame.Mode = AddrModeFlat;
	Frame.AddrStack.Offset = Context.Esp;
	Frame.AddrStack.Mode = AddrModeFlat;
	for (int Depth = 0; Depth < 32; Depth++)
	{
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, Process, GetCurrentThread(), &Frame, &Context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) || Frame.AddrPC.Offset == 0)
		{
			break;
		}
		char SymbolBuffer[sizeof(SYMBOL_INFO) + 512];
		SYMBOL_INFO* Symbol = (SYMBOL_INFO*)SymbolBuffer;
		memset(SymbolBuffer, 0, sizeof(SymbolBuffer));
		Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		Symbol->MaxNameLen = 512;
		DWORD64 Displacement = 0;
		const char* Name = SymFromAddr(Process, Frame.AddrPC.Offset, &Displacement, Symbol) ? Symbol->Name : "?";
		IMAGEHLP_LINE64 Line;
		memset(&Line, 0, sizeof(Line));
		Line.SizeOfStruct = sizeof(Line);
		DWORD LineDisplacement = 0;
		if (SymGetLineFromAddr64(Process, Frame.AddrPC.Offset, &LineDisplacement, &Line))
		{
			printf("  #%d 0x%08llX %s  (%s:%lu)\n", Depth, Frame.AddrPC.Offset, Name, Line.FileName, Line.LineNumber);
		}
		else
		{
			printf("  #%d 0x%08llX %s\n", Depth, Frame.AddrPC.Offset, Name);
		}
	}
	printf("%d passed, %d failed, %d skipped before the crash\n", NumPassed, NumFailures, NumSkipped);
	return EXCEPTION_EXECUTE_HANDLER;
}

void InstallSmokeOutputDevices();

int main(int, char**)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	SetUnhandledExceptionFilter(CrashFilter);
	InstallSmokeOutputDevices();
	printf("CoreSmoke: Core.lib smoke test\n");
	TestSizes();
	TestAppHelpers();
	// Core's FName constructors call StaticInit on first use (UnName.cpp Init), so a static FName in
	// Core.lib or the stubs has usually initialized the table before main; StaticInit checks against
	// being run twice.
	const bool NamesInitializedBeforeMain = FName::GetInitialized() != 0;
	if (!NamesInitializedBeforeMain)
	{
		FName::StaticInit();
	}
	Report(FName::GetInitialized() != 0, NamesInitializedBeforeMain ? "FName table initialized during static init (FName::StaticInit skipped)" : "FName::StaticInit");
	TestNames();
	TestContainers();
	TestPackageSummary();
	printf("\n%d passed, %d failed, %d skipped\n", NumPassed, NumFailures, NumSkipped);
	return NumFailures;
}
static const int StaticInitMarker = (fputs("CoreSmoke.cpp static init\n", stderr), 0);
