/*=============================================================================
	FxSDK.cpp - FaceFX 1.7.3.1 SDK startup/shutdown and version identity (fxsdk\src\fxsdk.cpp, FxSDK.obj).

	DISHONORED(port): agent FX1 (wave 26). FxSDKStartup (0xb14930) starts memory, the class table and the name
	table, registers every SDK class by calling its StaticClass in retail's order, then starts the link
	functions, the mounted-archive registry and the actor registry; FxSDKShutdown (0xb149e0) reverses it.
	The face graph and animation classes (FX2's headers: FxFaceGraphNode.h holds the node, user property and link
	classes, FxGenericTargetNode.h the morph target and FUnrealFaceFX* nodes, FxAnim.h FxAnimGroup/FxAnimSet/FxAnim)
	and the runtime classes (FX3: FxActor.h, FxActorInstance.h, FxPhonemeMap.h) are registered by the same calls in
	retail's order, each compiled when its header exists in include/ (__has_include). Named deviation until FX3
	lands: FxActor, FxActorInstance, FxPhonemeMap are not registered and FxActor::Startup / Shutdown are skipped.
=============================================================================*/
#include "FxSDK.h"
#include "FxRefObject.h"
#include "FxNamedObject.h"
#include "FxBone.h"

#if __has_include("FxFaceGraphNode.h")
#include "FxFaceGraphNode.h"
#define FX1_HAS_FACEGRAPHNODE 1
#else
#define FX1_HAS_FACEGRAPHNODE 0
#endif
#if __has_include("FxBonePoseNode.h")
#include "FxBonePoseNode.h"
#define FX1_HAS_BONEPOSENODE 1
#else
#define FX1_HAS_BONEPOSENODE 0
#endif
#if __has_include("FxCombinerNode.h")
#include "FxCombinerNode.h"
#define FX1_HAS_COMBINERNODE 1
#else
#define FX1_HAS_COMBINERNODE 0
#endif
#if __has_include("FxCurrentTimeNode.h")
#include "FxCurrentTimeNode.h"
#define FX1_HAS_CURRENTTIMENODE 1
#else
#define FX1_HAS_CURRENTTIMENODE 0
#endif
#if __has_include("FxDeltaNode.h")
#include "FxDeltaNode.h"
#define FX1_HAS_DELTANODE 1
#else
#define FX1_HAS_DELTANODE 0
#endif
#if __has_include("FxGenericTargetNode.h")
#include "FxGenericTargetNode.h"
#define FX1_HAS_GENERICTARGETNODE 1
#else
#define FX1_HAS_GENERICTARGETNODE 0
#endif
#if __has_include("FxFaceGraph.h")
#include "FxFaceGraph.h"
#define FX1_HAS_FACEGRAPH 1
#else
#define FX1_HAS_FACEGRAPH 0
#endif
#if __has_include("FxLinkFn.h")
#include "FxLinkFn.h"
#define FX1_HAS_LINKFN 1
#else
#define FX1_HAS_LINKFN 0
#endif
#if __has_include("FxAnimCurve.h")
#include "FxAnimCurve.h"
#define FX1_HAS_ANIMCURVE 1
#else
#define FX1_HAS_ANIMCURVE 0
#endif
#if __has_include("FxAnim.h")
#include "FxAnim.h"
#define FX1_HAS_ANIM 1
#else
#define FX1_HAS_ANIM 0
#endif
#if __has_include("FxActor.h")
#include "FxActor.h"
#define FX1_HAS_ACTOR 1
#else
#define FX1_HAS_ACTOR 0
#endif
#if __has_include("FxActorInstance.h")
#include "FxActorInstance.h"
#define FX1_HAS_ACTORINSTANCE 1
#else
#define FX1_HAS_ACTORINSTANCE 0
#endif
#if __has_include("FxPhonemeMap.h")
#include "FxPhonemeMap.h"
#define FX1_HAS_PHONEMEMAP 1
#else
#define FX1_HAS_PHONEMEMAP 0
#endif

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb148d0. */
void FxSDKStartup()
{
	FxMemoryAllocationPolicy defaultAllocationPolicy;
	FxSDKStartup(defaultAllocationPolicy);
}

/** 2012 0xb14930. */
void FxSDKStartup(const FxMemoryAllocationPolicy& allocationPolicy)
{
	FxMemoryStartup(allocationPolicy);
	FxClass::Startup();
	FxName::Startup();
	FxObject::StaticClass();
	FxRefObject::StaticClass();
	FxRefString::StaticClass();
	FxNamedObject::StaticClass();
	FxBone::StaticClass();
#if FX1_HAS_FACEGRAPHNODE
	FxFaceGraphNode::StaticClass();
	FxFaceGraphNodeUserProperty::StaticClass();
	FxFaceGraphNodeLink::StaticClass();
#endif
#if FX1_HAS_BONEPOSENODE
	FxBonePoseNode::StaticClass();
#endif
#if FX1_HAS_COMBINERNODE
	FxCombinerNode::StaticClass();
#endif
#if FX1_HAS_CURRENTTIMENODE
	FxCurrentTimeNode::StaticClass();
#endif
#if FX1_HAS_DELTANODE
	FxDeltaNode::StaticClass();
#endif
#if FX1_HAS_GENERICTARGETNODE
	FxGenericTargetNode::StaticClass();
	FxMorphTargetNode::StaticClass();
	FUnrealFaceFXNode::StaticClass();
	FUnrealFaceFXMaterialParameterNode::StaticClass();
	FUnrealFaceFXMorphNode::StaticClass();
#endif
#if FX1_HAS_FACEGRAPH
	FxFaceGraph::StaticClass();
#endif
#if FX1_HAS_ANIMCURVE
	FxAnimCurve::StaticClass();
#endif
#if FX1_HAS_ANIM
	FxAnimGroup::StaticClass();
	FxAnimSet::StaticClass();
	FxAnim::StaticClass();
#endif
#if FX1_HAS_ACTOR
	FxActor::StaticClass();
#endif
#if FX1_HAS_ACTORINSTANCE
	FxActorInstance::StaticClass();
#endif
#if FX1_HAS_PHONEMEMAP
	FxPhonemeMap::StaticClass();
#endif
#if FX1_HAS_LINKFN
	FxLinkFn::Startup();
#endif
	FxArchive::Startup();
#if FX1_HAS_ACTOR
	FxActor::Startup();
#endif
}

/** 2012 0xb149e0. */
void FxSDKShutdown()
{
#if FX1_HAS_ACTOR
	FxActor::Shutdown();
#endif
	FxArchive::Shutdown();
#if FX1_HAS_LINKFN
	FxLinkFn::Shutdown();
#endif
	FxName::Shutdown();
	FxClass::Shutdown();
	FxMemoryShutdown();
}

/** 2012 0xb14a10. */
FxSize FxSDKGetVersion()
{
	return 1731;
}

/** 2012 0xb14a20: "major.minor[.bugfix[.revision]]" from the decimal digits, 1731 -> "1.7.3.1". */
FxString FxSDKConvertVersionToString(FxSize version)
{
	FxSize revision = version % 10;
	version /= 10;
	FxSize bugfix = version % 10;
	version /= 10;
	FxString retval(static_cast<FxSize>(0));
	retval << static_cast<FxInt32>(version / 10 % 10) << "." << static_cast<FxInt32>(version % 10);
	if( bugfix || revision )
	{
		retval << "." << static_cast<FxInt32>(bugfix);
	}
	if( revision )
	{
		retval << "." << static_cast<FxInt32>(revision);
	}
	return retval;
}

/** 2012 0xb14b60. */
FxString FxSDKGetVersionString()
{
	return FxSDKConvertVersionToString(FxSDKGetVersion());
}

/** 2012 0xb14b90. */
FxString FxSDKGetLicenseeName()
{
	return FxString("Unreal Engine 3 Licensee");
}

/** 2012 0xb14bc0. */
FxString FxSDKGetLicenseeProjectName()
{
	return FxString("Unreal Engine 3 Project");
}

/** 2012 0xb14bf0. */
FxSize FxSDKGetLicenseeVersion()
{
	return 1000;
}

/** 2012 0xb14c00. */
FxString FxSDKGetLicenseeVersionString()
{
	return FxSDKConvertVersionToString(FxSDKGetLicenseeVersion());
}

/** 2012 0xb14c30. */
FxString FxSDKGetBuildId()
{
	return FxString("1731100780");
}

} // namespace Face
} // namespace OC3Ent
