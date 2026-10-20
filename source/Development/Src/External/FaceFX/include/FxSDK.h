/*=============================================================================
	FxSDK.h - FaceFX 1.7.3.1 SDK entry points (fxsdk.h): startup/shutdown and the version identity retail
	writes into every archive (sdk 1731, licensee "Unreal Engine 3 Licensee" / "Unreal Engine 3 Project",
	licensee version 1000, build id "1731100780").

	DISHONORED(port): agent FX1 (wave 26).
=============================================================================*/
#ifndef FxSDK_H__
#define FxSDK_H__

#include "FxMemory.h"
#include "FxString.h"

namespace OC3Ent
{
namespace Face
{

void FxSDKStartup();
void FxSDKStartup(const FxMemoryAllocationPolicy& allocationPolicy);
void FxSDKShutdown();

FxSize FxSDKGetVersion();
FxString FxSDKConvertVersionToString(FxSize version);
FxString FxSDKGetVersionString();
FxString FxSDKGetLicenseeName();
FxString FxSDKGetLicenseeProjectName();
FxSize FxSDKGetLicenseeVersion();
FxString FxSDKGetLicenseeVersionString();
FxString FxSDKGetBuildId();

} // namespace Face
} // namespace OC3Ent

#endif
