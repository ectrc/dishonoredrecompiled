// Engine symbols that Core object files reference (Core's UnMisc.cpp / UnVcWin32.cpp / Database.cpp /
// UnStatsNotifyProviders.cpp include Engine.h) but that are defined in Engine .cpp files. CoreSmoke
// links Core alone, so these get harmless stand-ins. Every entry names the Engine file that owns it.
// Added after agent M's layout convergence turned the inline reference definitions into declarations.
#include "Engine.h"

// EngineClasses.h / UnActor.cpp: FBasedPosition() used by AActor default-property initialisers
FBasedPosition::FBasedPosition()
{
	appMemzero(this, sizeof(*this));
}

// Engine/Src/PostProcessSettings / SceneColorGrading: FLUTBlender() inside FPostProcessSettings
FLUTBlender::FLUTBlender()
{
}

// Engine/Src/UnSkeletalMesh.cpp, UnStaticMesh.cpp: source-data holders (editor mesh data)
FSkeletalMeshSourceData::FSkeletalMeshSourceData()
{
}
FSkeletalMeshSourceData::~FSkeletalMeshSourceData()
{
}
FStaticMeshSourceData::FStaticMeshSourceData()
{
}
FStaticMeshSourceData::~FStaticMeshSourceData()
{
}

// Engine/Src/UnWorld.cpp: FNetworkNotify implementation that agent M's FDisWorldNetworkNotify adapter forwards to
EAcceptConnection UWorld::NotifyAcceptingConnection()
{
	return ACCEPTC_Reject;
}
void UWorld::NotifyAcceptedConnection(UNetConnection*)
{
}
UBOOL UWorld::NotifyAcceptingChannel(UChannel*)
{
	return FALSE;
}
UBOOL UWorld::NotifySendingFile(UNetConnection*, FGuid)
{
	return FALSE;
}
void UWorld::NotifyReceivedFile(UNetConnection*, INT, const TCHAR*, UBOOL)
{
}
void UWorld::NotifyProgress(EProgressMessageType, const FString&, const FString&)
{
}

// Engine/Src/Texture2D.cpp: mip bulk data resource memory (streaming)
void* FTextureMipBulkData::GetBulkDataResourceMemory(UObject*, INT)
{
	return NULL;
}
