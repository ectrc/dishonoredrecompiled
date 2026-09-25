// Minimal definitions of the globals and hooks Core.lib expects the application (Launch) and the
// sibling modules (Engine, WinDrv, IpDrv, D3D9Drv) to provide. Modelled on the reference
// Launch/Src/LaunchEngineLoop.cpp, Launch.cpp and LaunchMisc.cpp; the origin of every symbol is
// recorded in resources/docs/agents/agentD.md. Only the mangled names matter here, so the types
// Core.h does not declare (UWindowsClient, FDebugServer, EShaderPlatform, ...) are declared just
// enough to define the symbol.
#include "Core.h"
#include "FMallocAnsi.h"
#include "FMallocThreadSafeProxy.h"

#include <cstdio>
#include <cstdlib>

// ---- Launch (LaunchEngineLoop.cpp / Launch.cpp / LaunchMisc.cpp) ----------------------------

// Core.cpp leaves GError and GWarn NULL and GLog with no devices; Launch installs
// FOutputDeviceWindowsError / FFeedbackContextAnsi / FOutputDeviceFile before appInit. A failed
// check() dereferences GError, so the test installs console equivalents up front.
class FSmokeStdoutDevice : public FOutputDevice
{
public:
	void Serialize(const TCHAR* Text, EName Event)
	{
		printf("  [%ls] %ls\n", FName::SafeString(Event), Text);
	}
};

class FSmokeErrorDevice : public FOutputDeviceError
{
public:
	void Serialize(const TCHAR* Text, EName Event)
	{
		printf("\nappError: %ls\n", Text);
		GIsCriticalError = TRUE;
		exit(100);
	}
	void HandleError() {}
};

class FSmokeFeedbackContext : public FFeedbackContext
{
public:
	void Serialize(const TCHAR* Text, EName Event)
	{
		printf("  [warn %ls] %ls\n", FName::SafeString(Event), Text);
	}
	VARARG_BODY(UBOOL, YesNof, const TCHAR*, VARARG_NONE)
	{
		return FALSE;
	}
	void BeginSlowTask(const TCHAR* Task, UBOOL ShowProgressDialog, UBOOL bShowCancelButton) {}
	void EndSlowTask() {}
	VARARG_BODY(UBOOL VARARGS, StatusUpdatef, const TCHAR*, VARARG_EXTRA(INT Numerator) VARARG_EXTRA(INT Denominator))
	{
		return TRUE;
	}
};

static FSmokeStdoutDevice SmokeLog;
static FSmokeErrorDevice SmokeError;
static FSmokeFeedbackContext SmokeWarn;

void InstallSmokeOutputDevices()
{
	GLog->AddOutputDevice(&SmokeLog);
	GError = &SmokeError;
	GWarn = &SmokeWarn;
}

// UnAnsi.cpp appMalloc calls this on the first allocation; the reference picks FMallocDebug/FMallocBinned per platform
void GCreateMalloc()
{
	GMalloc = new FMallocAnsi();
	if (!GMalloc->IsInternallyThreadSafe())
	{
		GMalloc = new FMallocThreadSafeProxy(GMalloc);
	}
}

// UnVcWin32.cpp appPlatformInit/appShowGameWindow register the game window class with it (Launch.cpp sets it in WinMain)
extern "C" { HINSTANCE hInstance = NULL; }

// UnVcWin32.cpp/UnOutputDevices.cpp use the package name for the log header and crash reports
extern "C" { TCHAR GPackage[64] = TEXT("CoreSmoke"); }

// UnVcWin32.cpp appShowGameWindow loads the window icon by this resource id
INT GGameIcon = 0;

// UnVcWin32.cpp crash handling asks whether to report every crash (Launch.cpp reads it from the command line)
UBOOL GAlwaysReportCrash = FALSE;

// UnMisc.cpp/UnVcWin32.cpp minidump writer target file name (LaunchMisc.cpp)
TCHAR MiniDumpFilenameW[1024] = TEXT("");

// ---- IpDrv (UnSocketWin.cpp / HardwareSurvey.cpp / IpDrv.cpp / FDebugServer.cpp) ---------------

// UnMisc.cpp appInit initializes WinSock through it
void appSocketInit(UBOOL bIsEarlyInit) {}

// UnMisc.cpp appInit uploads the hardware survey on PC builds
void UploadHardwareSurveyIfNecessary() {}

// UnVcWin32.cpp appOutputDebugString mirrors the log to the remote debug channel when one exists
class FDebugServer
{
public:
	void SendText(const TCHAR* Text);
};
FDebugServer* GDebugChannel = NULL;
void FDebugServer::SendText(const TCHAR* Text) {}

// ---- Engine (UnEngine.cpp / DynamicRHI.cpp / RHI.cpp / ShaderManager.cpp / RenderingThread.cpp / UnPrefab.cpp / UnWorld.cpp / Texture2D.cpp) ----

// UnMisc.cpp/UnVcWin32.cpp check the engine pointer before touching game state
class UEngine;
UEngine* GEngine = NULL;

// Database.cpp/UnStatsNotifyProviders.cpp/UnVcWin32.cpp read the RHI pointer for GPU stats and device resets
class FDynamicRHI;
FDynamicRHI* GDynamicRHI = NULL;

// UnMisc.cpp guards RHI resets with it
UBOOL GAllowFullRHIReset = FALSE;

// UnVcWin32.cpp appPlatformPostInit logs the shader platform name (Engine/Inc/ShaderCompiler.h enum, SP_PCD3D_SM3 = 0)
enum EShaderPlatform { SP_PCD3D_SM3 = 0 };
EShaderPlatform GRHIShaderPlatform = SP_PCD3D_SM3;
const TCHAR* ShaderPlatformToText(EShaderPlatform ShaderPlatform, UBOOL bUseAbbreviation, UBOOL bIncludeGLES2)
{
	return TEXT("PC-D3D-SM3");
}

// Rendering-thread hooks used by Core's threading and async-loading code; no rendering thread exists here
UBOOL IsInRenderingThread()
{
	return TRUE;
}
void FlushRenderingCommands() {}
void FlushDeferredDeletion() {}

// EngineControllerClasses.h eventGetPlayerViewPoint (inlined into Core units that include Engine.h) looks up this script event name
FName ENGINE_GetPlayerViewPoint;

// UnMisc.cpp appendix in crash/log output names the current map
const FString GetMapNameStatic()
{
	return FString(TEXT(""));
}

// UObject virtuals implemented by Engine's UnPrefab.cpp; Core's object iteration calls them through the vtable
UBOOL UObject::IsAPrefabArchetype(UObject** OwnerPrefab) const
{
	return FALSE;
}
UBOOL UObject::IsInPrefabInstance() const
{
	return FALSE;
}

// FPackageFileSummary's texture preallocation block is serialized by Engine's Texture2D.cpp; the loading path only reads TextureTypes
FTextureAllocations::FTextureType::FTextureType()
:	SizeX(0)
,	SizeY(0)
,	NumMips(0)
,	Format(0)
,	TexCreateFlags(0)
,	NumExportIndicesProcessed(0)
{
}
FArchive& operator<<(FArchive& Ar, FTextureAllocations::FTextureType& TextureType)
{
	Ar << TextureType.SizeX;
	Ar << TextureType.SizeY;
	Ar << TextureType.NumMips;
	Ar << TextureType.Format;
	Ar << TextureType.TexCreateFlags;
	Ar << TextureType.ExportIndices;
	return Ar;
}
FArchive& operator<<(FArchive& Ar, FTextureAllocations& TextureAllocations)
{
	Ar << TextureAllocations.TextureTypes;
	TextureAllocations.PendingAllocationSize = 0;
	TextureAllocations.PendingAllocationCount.Reset();
	return Ar;
}
void FTextureAllocations::CancelRemainingAllocations(UBOOL bCancelEverything) {}

// ULinkerLoad::VerifyImportInner kicks off texture memory preallocation through Engine's Texture2D.cpp
UBOOL ULinkerLoad::StartTextureAllocation()
{
	return TRUE;
}

template TAccumulator<FLOAT>::TAccumulator(const TCHAR*, DWORD, DWORD);
template TAccumulator<DWORD>::TAccumulator(const TCHAR*, DWORD, DWORD);
template TCounter<FLOAT>::TCounter(const TCHAR*, DWORD, DWORD);
template TCounter<DWORD>::TCounter(const TCHAR*, DWORD, DWORD);

// ---- WinDrv (WinViewport.cpp / WinClient.cpp) ------------------------------------------------

// UnVcWin32.cpp appShowGameWindow/appPlatformInit create and size the startup window with these
HWND GGameWindow = NULL;
UBOOL GGameWindowUsingStartupWindowProc = FALSE;
DWORD GGameWindowStyle = 0;
INT GGameWindowPosX = 0;
INT GGameWindowPosY = 0;
INT GGameWindowWidth = 0;
INT GGameWindowHeight = 0;
INT GPrimaryMonitorWidth = 0;
INT GPrimaryMonitorHeight = 0;
RECT GPrimaryMonitorWorkRect = {0, 0, 0, 0};
RECT GVirtualScreenRect = {0, 0, 0, 0};

// UnVcWin32.cpp appShowGameWindow swaps the startup window procedure for the client's
class UWindowsClient
{
public:
	static LRESULT APIENTRY StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
};
LRESULT APIENTRY UWindowsClient::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

// ---- D3D9Drv (CompatibilityEvaluator.cpp) ----------------------------------------------------

// UnVcWin32.cpp appGetCompatibilityLevel/appSetCompatibilityLevel forward to the D3D9 evaluator
FCompatibilityLevelInfo GetCompatibilityLevelWindows()
{
	return FCompatibilityLevelInfo(0, 0, 0);
}
UBOOL SetCompatibilityLevelWindows(FCompatibilityLevelInfo Level, UBOOL bWriteToIni)
{
	return FALSE;
}
