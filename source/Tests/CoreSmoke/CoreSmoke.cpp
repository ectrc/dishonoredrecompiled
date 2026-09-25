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

// Milestone-3 blocker (PHASE3 K): every cooked package is PKG_StoreCompressed with COMPRESS_LZO. Reads the
// single FCompressedChunk of Core.upk, decodes its header the way FArchive::SerializeCompressed does
// ({PACKAGE_FILE_TAG, block size}, {total compressed, total uncompressed}, one FCompressedChunkInfo per
// block), runs block 0 through appUncompressMemory(COMPRESS_LZO), then the whole chunk through
// FArchive::SerializeCompressed (the ULinkerLoad path) and walks the name table that starts the chunk.
void TestCompressedChunk()
{
	const char* Path = "D:/RecompileDishonored/Dishonored_Latest2026/DishonoredGame/CookedPCConsole/Core.upk";
	FILE* File = fopen(Path, "rb");
	if (!File)
	{
		Skip("LZO chunk from Core.upk", "Core.upk not found");
		return;
	}
	fseek(File, 0, SEEK_END);
	const long FileSize = ftell(File);
	fseek(File, 0, SEEK_SET);
	TArray<BYTE> Bytes;
	Bytes.Add((INT)FileSize);
	const size_t Read = fread(Bytes.GetData(), 1, Bytes.Num(), File);
	fclose(File);
	CHECK_EQ("Core.upk read completely", Read, FileSize);

	FMemoryReader Reader(Bytes, TRUE);
	FPackageFileSummary Summary;
	Reader << Summary;
	CHECK_EQ("GBaseCompressionMethod == COMPRESS_LZO (retail exe: 2)", GBaseCompressionMethod, COMPRESS_LZO);
	CHECK_EQ("Summary.CompressionFlags == GBaseCompressionMethod", Summary.CompressionFlags, GBaseCompressionMethod);
	if (Summary.CompressedChunks.Num() < 1)
	{
		Report(false, "Summary.CompressedChunks has a chunk", "no compressed chunk");
		return;
	}
	const FCompressedChunk& Chunk = Summary.CompressedChunks(0);
	CHECK_EQ("Chunk.CompressedOffset == 157 (right after the stored summary)", Chunk.CompressedOffset, Reader.Tell());
	CHECK_EQ("Chunk.CompressedOffset + CompressedSize == file size", Chunk.CompressedOffset + Chunk.CompressedSize, Bytes.Num());
	CHECK_EQ("Chunk.UncompressedOffset == Summary.NameOffset (141: summary without chunk table)", Chunk.UncompressedOffset, Summary.NameOffset);
	CHECK_EQ("Chunk.UncompressedSize == 193512", Chunk.UncompressedSize, 193512);

	Reader.Seek(Chunk.CompressedOffset);
	FCompressedChunkInfo PackageFileTag;
	FCompressedChunkInfo Total;
	Reader << PackageFileTag << Total;
	CHECK_EQ("Chunk header tag == PACKAGE_FILE_TAG", (DWORD)PackageFileTag.CompressedSize, (DWORD)PACKAGE_FILE_TAG);
	CHECK_EQ("Chunk header block size == LOADING_COMPRESSION_CHUNK_SIZE", PackageFileTag.UncompressedSize, LOADING_COMPRESSION_CHUNK_SIZE);
	CHECK_EQ("Chunk header total uncompressed == Chunk.UncompressedSize", Total.UncompressedSize, Chunk.UncompressedSize);
	const INT BlockCount = (Total.UncompressedSize + PackageFileTag.UncompressedSize - 1) / PackageFileTag.UncompressedSize;
	CHECK_EQ("Chunk block count == 2", BlockCount, 2);
	TArray<FCompressedChunkInfo> Blocks;
	INT SumCompressed = 0;
	INT SumUncompressed = 0;
	for (INT BlockIndex = 0; BlockIndex < BlockCount; BlockIndex++)
	{
		FCompressedChunkInfo Block;
		Reader << Block;
		Blocks.AddItem(Block);
		SumCompressed += Block.CompressedSize;
		SumUncompressed += Block.UncompressedSize;
	}
	CHECK_EQ("Sum of block compressed sizes == header total", SumCompressed, Total.CompressedSize);
	CHECK_EQ("Sum of block uncompressed sizes == header total", SumUncompressed, Total.UncompressedSize);
	CHECK_EQ("Chunk header + block table + blocks == Chunk.CompressedSize", 2 * sizeof(FCompressedChunkInfo) + BlockCount * sizeof(FCompressedChunkInfo) + SumCompressed, Chunk.CompressedSize);
	CHECK_EQ("Block 0 uncompressed size == LOADING_COMPRESSION_CHUNK_SIZE", Blocks(0).UncompressedSize, LOADING_COMPRESSION_CHUNK_SIZE);

	// appUncompressMemory books STAT_UncompressorTime; appInit (not run here) does GStatManager.Init(),
	// which needs the synchronize factory Launch installs (LaunchEngineLoop.cpp: GSynchronizeFactory)
	// and a config cache (appInit: GConfig = ConfigFactory(); an empty one with file operations disabled
	// answers every query with FALSE without touching GFileManager, which the harness does not install).
	static FSynchronizeFactoryWin SynchronizeFactory;
	if (!GSynchronizeFactory)
	{
		GSynchronizeFactory = &SynchronizeFactory;
	}
	if (!GConfig)
	{
		GConfig = new FConfigCacheIni();
		GConfig->DisableFileOperations();
	}
	GStatManager.Init();
	Report(GStatManager.GetGroup(STATGROUP_AsyncIO) != NULL, "GStatManager.Init() registers STATGROUP_AsyncIO (STAT_UncompressorTime)");

	TArray<BYTE> Block0;
	Block0.Add(Blocks(0).UncompressedSize);
	const BYTE* Block0Compressed = Bytes.GetData() + Reader.Tell();
	const UBOOL bBlock0Ok = appUncompressMemory((ECompressionFlags)Summary.CompressionFlags, Block0.GetData(), Blocks(0).UncompressedSize, Block0Compressed, Blocks(0).CompressedSize);
	Report(bBlock0Ok != 0, "appUncompressMemory(COMPRESS_LZO) decodes block 0");
	Report(appUncompressMemory((ECompressionFlags)Summary.CompressionFlags, Block0.GetData(), Blocks(0).UncompressedSize, Block0Compressed, Blocks(0).CompressedSize - 1) == 0, "appUncompressMemory(COMPRESS_LZO) rejects a truncated block");
	Report(appUncompressMemory((ECompressionFlags)Summary.CompressionFlags, Block0.GetData(), Blocks(0).UncompressedSize - 1, Block0Compressed, Blocks(0).CompressedSize) == 0, "appUncompressMemory(COMPRESS_LZO) rejects a too small destination");

	Reader.Seek(Chunk.CompressedOffset);
	TArray<BYTE> Uncompressed;
	Uncompressed.Add(Chunk.UncompressedSize);
	Reader.SerializeCompressed(Uncompressed.GetData(), Chunk.UncompressedSize, (ECompressionFlags)Summary.CompressionFlags);
	Report(!Reader.IsError(), "FArchive::SerializeCompressed over the chunk without archive error");
	CHECK_EQ("SerializeCompressed consumed the whole chunk", Reader.Tell(), Chunk.CompressedOffset + Chunk.CompressedSize);
	Report(bBlock0Ok && appMemcmp(Block0.GetData(), Uncompressed.GetData(), Block0.Num()) == 0, "block 0 equals the start of the SerializeCompressed output");

	// The chunk starts at NameOffset: NameCount x (FString name, QWORD flags), as operator<<(FNameEntry&) reads it
	FMemoryReader NameReader(Uncompressed, TRUE);
	FString FirstName;
	FString LastName;
	INT NumValid = 0;
	bool SawNone = false;
	bool SawCore = false;
	bool SawObject = false;
	bool Sorted = true;
	FString Previous;
	for (INT NameIndex = 0; NameIndex < Summary.NameCount && !NameReader.IsError(); NameIndex++)
	{
		FString Name;
		QWORD Flags = 0;
		NameReader << Name << Flags;
		if (Name.Len() > 0 && Name.Len() < NAME_SIZE)
		{
			NumValid++;
		}
		if (NameIndex == 0)
		{
			FirstName = Name;
		}
		else if (appStricmp(*Previous, *Name) > 0)
		{
			Sorted = false;
		}
		Previous = Name;
		LastName = Name;
		SawNone = SawNone || Name == TEXT("None");
		SawCore = SawCore || Name == TEXT("Core");
		SawObject = SawObject || Name == TEXT("Object");
	}
	Report(!NameReader.IsError(), "Name table read without archive error");
	CHECK_EQ("All 720 names have a plausible length", NumValid, Summary.NameCount);
	CHECK_EQ("Name table ends at Summary.ImportOffset", Chunk.UncompressedOffset + NameReader.Tell(), Summary.ImportOffset);
	char Detail[256];
	snprintf(Detail, sizeof(Detail), "first \"%ls\", last \"%ls\"", *FirstName, *LastName);
	Report(FirstName == TEXT("!") && LastName == TEXT("~="), "First name \"!\" and last name \"~=\" (cooked name table sorted)", Detail);
	Report(Sorted, "Name table is sorted (appStricmp)");
	Report(SawNone && SawCore && SawObject, "Name table contains None, Core and Object");

	// Round trip through appCompressMemory: lzokay's LZO1X-1 output must decode back to the same bytes
	TArray<BYTE> Recompressed;
	Recompressed.Add(Blocks(0).UncompressedSize);
	INT RecompressedSize = Recompressed.Num();
	Report(appCompressMemory(COMPRESS_LZO, Recompressed.GetData(), RecompressedSize, Block0.GetData(), Block0.Num()) != 0, "appCompressMemory(COMPRESS_LZO) on block 0");
	Report(RecompressedSize > 0 && RecompressedSize < Block0.Num(), "appCompressMemory(COMPRESS_LZO) shrinks block 0");
	TArray<BYTE> RoundTrip;
	RoundTrip.Add(Block0.Num());
	Report(appUncompressMemory(COMPRESS_LZO, RoundTrip.GetData(), RoundTrip.Num(), Recompressed.GetData(), RecompressedSize) != 0 && appMemcmp(RoundTrip.GetData(), Block0.GetData(), Block0.Num()) == 0, "appCompressMemory -> appUncompressMemory round trip matches block 0");
	INT TooSmall = 16;
	BYTE Small[16];
	Report(appCompressMemory(COMPRESS_LZO, Small, TooSmall, Block0.GetData(), Block0.Num()) == 0 && TooSmall == RecompressedSize, "appCompressMemory(COMPRESS_LZO) reports the needed size when the buffer is too small");
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
	TestCompressedChunk();
	printf("\n%d passed, %d failed, %d skipped\n", NumPassed, NumFailures, NumSkipped);
	return NumFailures;
}
static const int StaticInitMarker = (fputs("CoreSmoke.cpp static init\n", stderr), 0);
