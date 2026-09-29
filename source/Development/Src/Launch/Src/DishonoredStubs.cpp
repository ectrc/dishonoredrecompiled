// Empty hooks for the modules DishonoredGame.exe links in the retail build but which this tree does
// not compile yet (Phase 3 milestone 1, resources/docs/agents/agentN.md). Every stub names the module
// that owns the real definition and why Launch/Core/Engine reference it. Remove a block when its
// module becomes a dishonored_module target (or the excluded unit comes back).
#include "LaunchPrivate.h"
#include "AVIWriter.h"
#include "UnitTest.h"

// ---- DishonoredGame (source/Development/Src/DishonoredGame, skeleton only) -------------------
// LaunchEngineLoop.cpp InitializeRegistrantsAndRegisterNames / CheckNativeClassSizes call the
// generated per-package hooks of the game module (DishonoredGame.upk, 1,485 script classes).
// DISHONORED(port): agent T generates the real hooks (DishonoredGame/Src/DishonoredGameRegistrants.cpp,
// same for AkAudio/GFxUI/OnlineSubsystemSteamworks); Launch/CMakeLists.txt defines DISHONORED_HAVE_<MODULE> when the module is a target.
#if !DISHONORED_HAVE_DISHONOREDGAME
void AutoInitializeRegistrantsDishonoredGame( INT& Lookup ) {}
void AutoGenerateNamesDishonoredGame() {}
void AutoCheckNativeClassSizesDishonoredGame( UBOOL& Mismatch ) {}
#endif

// ---- AkAudio (Wwise sound engine module, replaces XAudio2; module_map.md) --------------------
// Same generated hooks for AkAudio.upk (UAkAudioDevice & co.).
#if !DISHONORED_HAVE_AKAUDIO
void AutoInitializeRegistrantsAkAudio( INT& Lookup ) {}
void AutoGenerateNamesAkAudio() {}
#endif

// ---- GFxUI (Scaleform GFx 4 integration, WITH_GFx=0 until Phase 4) --------------------------
// LaunchEngineLoop.cpp calls the GFxUI registrants unconditionally (not under WITH_GFx).
#if !DISHONORED_HAVE_GFXUI
void AutoInitializeRegistrantsGFxUI( INT& Lookup ) {}
#endif

// ---- OnlineSubsystemSteamworks (WITH_STEAMWORKS=0 until Phase 4) ----------------------------
// The DISHONOREDGAME branch keeps the retail OSS selection (OnlineSubsystemSteamworks.upk).
#if !DISHONORED_HAVE_OSS
void AutoInitializeRegistrantsOnlineSubsystemSteamworks( INT& Lookup ) {}
void AutoGenerateNamesOnlineSubsystemSteamworks() {}
#endif

// ---- D3D9Drv: a dishonored_module target since wave 2 (agentP.md); Launch links D3D9CreateRHI, the PIX
// markers (D3D9Util.cpp), the shader compiler entry points and the compatibility evaluator from D3D9Drv.lib.
// With DISHONORED_ENABLE_D3D9DRV=OFF the null RHI stands in again.
#if !DISHONORED_HAVE_D3D9DRV
// DynamicRHI.cpp RHIInit: the retail D3D9CreateRHI (PDB rva 0x60a180, D3D9Device.cpp) creates the
// D3D9 device; without the module the null RHI keeps everything after appInit alive.
extern FDynamicRHI* NullCreateRHI();
FDynamicRHI* D3D9CreateRHI()
{
	return NullCreateRHI();
}
// UnSceneUtils.h PIX event markers / counters, defined by D3D9Util.cpp (NullRHI.cpp only defines
// them under USE_NULL_RHI, which stays 0 so DynamicRHI.cpp keeps the retail code path).
void appBeginDrawEvent(const FColor& Color, const TCHAR* Text) {}
void appEndDrawEvent(void) {}
void appSetCounterValue(const TCHAR* CounterName, FLOAT Value) {}
// ShaderCompiler.cpp: D3D9ShaderCompiler.cpp compiles shaders through D3DX; cooked builds load
// them from the shader cache instead, so failing every compile request is safe here.
UBOOL D3D9BeginCompileShader(INT JobId, UINT ThreadId, const TCHAR* SourceFilename, const TCHAR* FunctionName, FShaderTarget Target, const FShaderCompilerEnvironment& Environment, FShaderCompilerOutput& Output, UBOOL bDebugDump, const TCHAR* ShaderSubDir)
{
	return FALSE;
}
UBOOL D3D9FinishCompilingShaderThroughWorker(FShaderTarget Target, INT& CurrentPosition, const TArray<BYTE>& WorkerOutput, FShaderCompilerOutput& Output)
{
	return FALSE;
}
// UnVcWin32.cpp appGetCompatibilityLevel / appSetCompatibilityLevel forward to CompatibilityEvaluator.cpp
FCompatibilityLevelInfo GetCompatibilityLevelWindows()
{
	return FCompatibilityLevelInfo(0, 0, 0);
}
UBOOL SetCompatibilityLevelWindows(FCompatibilityLevelInfo Level, UBOOL bWriteToIni)
{
	return FALSE;
}
// LaunchEngineLoop.cpp PreInit (-firstinstall only): D3D9HardwareSurvey.cpp picks the desktop resolution
VOID SetDefaultResolutionForDevice() {}
#endif // !DISHONORED_HAVE_D3D9DRV

// ---- D3D11Drv (not imported: Dishonored ships no D3D11 path, module_map.md lists no d3d11drv) --
// DynamicRHI.cpp RHIInit and ShaderCompiler.cpp reference the D3D11 entry points unconditionally.
FDynamicRHI* D3D11CreateRHI()
{
	return NULL;
}
UBOOL IsDirect3D11Supported(UBOOL& OutSupportsD3D11Features)
{
	OutSupportsD3D11Features = FALSE;
	return FALSE;
}
UBOOL D3D11BeginCompileShader(INT JobId, UINT ThreadId, const TCHAR* SourceFilename, const TCHAR* FunctionName, FShaderTarget Target, const FShaderCompilerEnvironment& Environment, FShaderCompilerOutput& Output, UBOOL bDebugDump, const TCHAR* ShaderSubDir)
{
	return FALSE;
}
UBOOL D3D11FinishCompilingShaderThroughWorker(FShaderTarget Target, INT& CurrentPosition, const TArray<BYTE>& WorkerOutput, FShaderCompilerOutput& Output)
{
	return FALSE;
}

// ---- OpenGLDrv (not imported: no opengldrv in the retail exe, module_map.md) -----------------
// DynamicRHI.cpp RHIInit (-opengl / bAllowOpenGL), ShaderCompiler.cpp and Material.cpp reference them.
FDynamicRHI* OpenGLCreateRHI()
{
	return NULL;
}
UBOOL OpenGLBeginCompileShader(INT JobId, UINT ThreadId, const TCHAR* SourceFilename, const TCHAR* FunctionName, FShaderTarget Target, const FShaderCompilerEnvironment& Environment, FShaderCompilerOutput& Output, UBOOL bDebugDump, const TCHAR* ShaderSubDir)
{
	return FALSE;
}
UBOOL OpenGLFinishCompilingShaderThroughWorker(FShaderTarget Target, INT& CurrentPosition, const TArray<BYTE>& WorkerOutput, FShaderCompilerOutput& Output)
{
	return FALSE;
}
void AddMaterialToOpenGLProgramCache(const FString& MaterialName, const FMaterialResource* MaterialResource) {}

// ---- Engine/Src/AVIWriter.cpp (excluded: DirectShow base classes missing, agentB.md) ---------
// UnEngine.cpp / UnGame.cpp movie capture exec commands; no FAVIWriter function in the Shipping PDB.
FAVIWriter* FAVIWriter::GetInstance()
{
	return NULL;
}

// ---- Core/Src/UnitTest.cpp (excluded test harness, porting_notes.md) -------------------------
// UnEngine.cpp Exec("UNITTEST") drives it; no FUnitTestFramework function in the Shipping PDB.
void FUnitTestFramework::FUnitTestFeedbackContext::Serialize(const TCHAR* V, EName Event) {}
FUnitTestFramework::FUnitTestFramework() : bWasRunningUnattended(FALSE), CachedContext(NULL) {}
FUnitTestFramework::~FUnitTestFramework() {}
FUnitTestFramework& FUnitTestFramework::GetInstance()
{
	static FUnitTestFramework Framework;
	return Framework;
}
void FUnitTestFramework::DumpUnitTestExecutionInfoToContext(FFeedbackContext* InContext, const TMap<FString, FUnitTestExecutionInfo>& InInfoToDump) {}
UBOOL FUnitTestFramework::ContainsTest(const FString& InTestName) const
{
	return FALSE;
}
UBOOL FUnitTestFramework::RunAllValidTests(TMap<FString, FUnitTestExecutionInfo>& OutExecutionInfoMap)
{
	return FALSE;
}
UBOOL FUnitTestFramework::RunTestByName(const FString& InTestName, FUnitTestExecutionInfo& OutExecutionInfo)
{
	return FALSE;
}

// ---- Link anchors for static-library objects nothing references -----------------------------
// UBT links the modules as static libraries too, but every stat group / registrant global lives in
// an object that also exports something referenced, except this one: Core/Src/BestFitAllocator.cpp
// declares STATGROUP_TexturePool and nothing in a PC game build references the allocator, so the
// linker drops the object and FStatManager::Init asserts (check(Group) in FMemoryCounter, UnStats.cpp
// 118) when it creates Texture2D.cpp's STAT_TexturePool_PackMipTailSavings. Pull the object in.
// DISHONORED(bringup): only meaningful while STATS is compiled in. Shipping compiles the stat system
// out entirely, so the symbol this names does not exist and there is no FStatManager::Init to assert.
#if STATS
#pragma comment(linker, "/include:?GroupFactory_STATGROUP_TexturePool@@3UFStatGroupFactory@@A")
#endif
