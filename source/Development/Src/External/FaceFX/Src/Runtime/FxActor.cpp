/*=============================================================================
	FxActor.cpp - FaceFX 1.7.3.1 actor (fxactor.cpp): the actor list, anim group and anim set management, the
	archive, and the actor load/save helpers.

	DISHONORED(port): agent FX3 (wave 26), from the 2012 decompile (FxActor.obj). _shouldFreeActorMemory starts true
	(2012 .data 0xe42ec4 = 1). The cooked heads are class version 3 (compiled face graph stored, default group then the
	owned groups, then the phoneme map); versions 0-2 (an editable graph compiled on load, the groups as one array, the
	register definitions dropped) are read as retail reads them.
	FxSaveActorToMemory copies the store with FxMemcpy (retail FxCharTraits<char>::Copy, same bytes).
=============================================================================*/
#include "FxActor.h"
#include "FxActorInstance.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreFileFast.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxFaceGraph) == 32, "FxFaceGraph: 2012 PDB sizeof 32");
static_assert(sizeof(FxCompiledFaceGraph) == 24604, "FxCompiledFaceGraph: 2012 PDB sizeof 24604");
static_assert(sizeof(FxAnimGroup) == 20, "FxAnimGroup: 2012 PDB sizeof 20");
static_assert(sizeof(FxActor) == 0x60A4, "FxActor: AllocateStaticClassFxActor 0x60A4");

/** 2012 0xb0c1d0 (AllocateStaticClassFxActor). */
FX_IMPLEMENT_CLASS(FxActor, 3, FxNamedObject)

FxName FxActor::NewActor;
FxList<FxActor*>* FxActor::_pActorList = NULL;
FxBool FxActor::_shouldFreeActorMemory = FxTrue;

/** 2012 0xb0c260. */
FxActor::FxActor()
	: _shouldClientRelink(FxFalse)
	, _isOpenInStudio(FxFalse)
{
	SetName(NewActor);
	_defaultAnimGroup.SetName(FxAnimGroup::Default);
}

/** 2012 0xb0c370. Every instance still on the actor is detached from it. */
FxActor::~FxActor()
{
	for( FxList<FxActorInstance*>::Iterator curr = _instanceList.Begin(); curr != _instanceList.End(); ++curr )
	{
		(*curr)->SetActor(NULL);
	}
}

/** 2012 0xb0c4b0. */
void FxActor::Startup()
{
	if( !_pActorList )
	{
		_pActorList = static_cast<FxList<FxActor*>*>(FxAllocate(sizeof(FxList<FxActor*>)));
		FxDefaultConstruct(_pActorList);
	}
}

/** 2012 0xb0c4e0. */
void FxActor::Shutdown()
{
	FlushActors();
	FxDelete(_pActorList);
	_pActorList = NULL;
}

/** 2012 0xb0c510. */
void FxActor::SetShouldClientRelink(FxBool shouldClientRelink)
{
	_shouldClientRelink = shouldClientRelink;
}

/** 2012 0xb0c530. */
void FxActor::SetIsOpenInStudio(FxBool isOpenInStudio)
{
	_isOpenInStudio = isOpenInStudio;
}

/** 2012 0xb0c550. */
void FxActor::CompileFaceGraph()
{
	_compiledFaceGraph.Compile(_decompiledFaceGraph);
	_masterBoneList.PullBones(_decompiledFaceGraph, _compiledFaceGraph);
	for( FxList<FxActorInstance*>::Iterator curr = _instanceList.Begin(); curr != _instanceList.End(); ++curr )
	{
		(*curr)->SetActor(this);
	}
}

/** 2012 0xb0c5e0. */
void FxActor::DecompileFaceGraph()
{
	_compiledFaceGraph.Decompile(_decompiledFaceGraph);
	_masterBoneList.PushBones(_decompiledFaceGraph, _compiledFaceGraph);
}

/** 2012 0xb0c620. */
FxBool FxActor::AddAnim(const FxName& group, const FxAnim& anim)
{
	AddAnimGroup(group);
	FxSize groupIndex = FindAnimGroup(group);
	if( groupIndex == FxInvalidIndex )
	{
		return FxFalse;
	}
	GetAnimGroup(groupIndex).AddAnim(anim);
	return FxTrue;
}

/** 2012 0xb0c680. */
FxBool FxActor::RemoveAnim(const FxName& group, const FxName& anim)
{
	FxSize groupIndex = FindAnimGroup(group);
	if( groupIndex == FxInvalidIndex )
	{
		return FxFalse;
	}
	return GetAnimGroup(groupIndex).RemoveAnim(anim);
}

/** 2012 0xb0c6d0. */
FxBool FxActor::AddAnimGroup(const FxName& name)
{
	if( FindAnimGroup(name) != FxInvalidIndex )
	{
		return FxFalse;
	}
	FxAnimGroup newGroup;
	newGroup.SetName(name);
	_animGroups.PushBack(newGroup);
	return FxTrue;
}

/** 2012 0xb0c770. Only an owned group (index 1 .. numAnimGroups) is removed. */
FxBool FxActor::RemoveAnimGroup(const FxName& name)
{
	FxSize groupIndex = FindAnimGroup(name);
	if( groupIndex == FxInvalidIndex || groupIndex == 0 || groupIndex > _animGroups.Length() )
	{
		return FxFalse;
	}
	_animGroups.Remove(groupIndex - 1);
	return FxTrue;
}

/** 2012 0xb0c7d0. The anim set's group is copied into the mounted list. */
FxBool FxActor::MountAnimSet(const FxAnimSet& animSet, FxMountUnmountExtendedInfo* pExtendedInfo)
{
	const FxAnimGroup& animGroup = animSet.GetAnimGroup();
	if( FindAnimGroup(animGroup.GetName()) != FxInvalidIndex )
	{
		if( pExtendedInfo )
		{
			*pExtendedInfo = MUE_Duplicate;
		}
		return FxFalse;
	}
	_mountedAnimGroups.PushBack(animGroup);
	if( pExtendedInfo )
	{
		*pExtendedInfo = MUE_None;
	}
	return FxTrue;
}

/** 2012 0xb0c840. An instance playing from the group is stopped (bStopPlayback) or the unmount refused. */
FxBool FxActor::UnmountAnimSet(const FxName& animSetName, FxBool bStopPlayback, FxMountUnmountExtendedInfo* pExtendedInfo)
{
	FxList<FxAnimGroup>::Iterator magIter = _mountedAnimGroups.Begin();
	FxList<FxAnimGroup>::Iterator magEndIter = _mountedAnimGroups.End();
	for( ; magIter != magEndIter; ++magIter )
	{
		if( magIter->GetName() == animSetName )
		{
			break;
		}
	}
	if( magIter == magEndIter )
	{
		if( pExtendedInfo )
		{
			*pExtendedInfo = MUE_InvalidMountedAnimSet;
		}
		return FxFalse;
	}
	for( FxList<FxActorInstance*>::Iterator curr = _instanceList.Begin(); curr != _instanceList.End(); ++curr )
	{
		FxActorInstance* pActorInstance = *curr;
		if( pActorInstance->IsPlayingAnim() && pActorInstance->GetCurrentAnimGroupName() == animSetName )
		{
			if( !bStopPlayback )
			{
				if( pExtendedInfo )
				{
					*pExtendedInfo = MUE_CurrentlyPlayingTryLater;
				}
				return FxFalse;
			}
			pActorInstance->StopAnim();
		}
	}
	_mountedAnimGroups.Remove(magIter);
	if( pExtendedInfo )
	{
		*pExtendedInfo = MUE_None;
	}
	return FxTrue;
}

/** 2012 0xb0c9a0. */
FxBool FxActor::ImportAnimSet(const FxAnimSet& animSet)
{
	const FxAnimGroup& animGroup = animSet.GetAnimGroup();
	if( FindAnimGroup(animGroup.GetName()) != FxInvalidIndex )
	{
		return FxFalse;
	}
	_animGroups.PushBack(animGroup);
	return FxTrue;
}

/** 2012 0xb0c9f0. */
FxBool FxActor::ExportAnimSet(const FxName& animGroupName, FxAnimSet& exportedAnimSet)
{
	FxSize animGroupIndex = FindAnimGroup(animGroupName);
	if( animGroupIndex == FxInvalidIndex || animGroupIndex == 0 || animGroupIndex > _animGroups.Length() )
	{
		return FxFalse;
	}
	const FxAnimGroup& animGroup = GetAnimGroup(animGroupIndex);
	exportedAnimSet.SetOwningActorName(GetName());
	exportedAnimSet.SetName(animGroup.GetName());
	exportedAnimSet.SetAnimGroup(animGroup);
	return RemoveAnimGroup(animGroupName);
}

/** 2012 0xb0ca80. */
FxActor* FxActor::FindActor(const FxName& name)
{
	if( _pActorList )
	{
		for( FxList<FxActor*>::Iterator curr = _pActorList->Begin(); curr != _pActorList->End(); ++curr )
		{
			if( (*curr)->GetName() == name )
			{
				return *curr;
			}
		}
	}
	return NULL;
}

/** 2012 0xb0cb00. */
FxBool FxActor::FindActorPtr(FxActor* actor)
{
	if( _pActorList )
	{
		for( FxList<FxActor*>::Iterator curr = _pActorList->Begin(); curr != _pActorList->End(); ++curr )
		{
			if( *curr == actor )
			{
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

/** 2012 0xb0cb70. */
void FxActor::SetShouldFreeActorMemory(FxBool shouldFreeActorMemory)
{
	_shouldFreeActorMemory = shouldFreeActorMemory;
}

/** 2012 0xb0cb80. */
FxBool FxActor::GetShouldFreeActorMemory()
{
	return _shouldFreeActorMemory;
}

/** 2012 0xb0cb90. Refused when an actor of the same name is already listed. */
FxBool FxActor::AddActor(FxActor* actor)
{
	if( !_pActorList || !actor || FindActor(actor->GetName()) )
	{
		return FxFalse;
	}
	_pActorList->PushBack(actor);
	return FxTrue;
}

/** 2012 0xb0cbd0. The actor is destroyed (its class size freed) when the SDK owns actor memory. */
FxBool FxActor::RemoveActor(FxActor* actor)
{
	if( !_pActorList || !actor )
	{
		return FxFalse;
	}
	FxList<FxActor*>::Iterator pos = _pActorList->Find(actor);
	if( pos == _pActorList->End() )
	{
		return FxFalse;
	}
	if( _shouldFreeActorMemory )
	{
		FxActor* pActorToDelete = *pos;
		FxSize size = pActorToDelete->GetClassDesc()->GetSize();
		pActorToDelete->~FxActor();
		FxFree(pActorToDelete, size);
	}
	_pActorList->Remove(pos);
	return FxTrue;
}

/** 2012 0xb0cc90. */
void FxActor::FlushActors()
{
	if( _pActorList )
	{
		if( _shouldFreeActorMemory )
		{
			for( FxList<FxActor*>::Iterator curr = _pActorList->Begin(); curr != _pActorList->End(); ++curr )
			{
				FxActor* pActorToDelete = *curr;
				FxSize size = pActorToDelete->GetClassDesc()->GetSize();
				pActorToDelete->~FxActor();
				FxFree(pActorToDelete, size);
			}
		}
		_pActorList->Clear();
	}
}

/** 2012 0xb0cd50. */
void FxActor::AddInstance(FxActorInstance* pActorInstance)
{
	FxList<FxActorInstance*>::Iterator instanceIter(NULL);
	if( !_findInstance(pActorInstance, instanceIter) )
	{
		_instanceList.PushBack(pActorInstance);
	}
}

/** 2012 0xb0cda0. */
void FxActor::RemoveInstance(FxActorInstance* pActorInstance)
{
	FxList<FxActorInstance*>::Iterator instanceIter(NULL);
	if( _findInstance(pActorInstance, instanceIter) )
	{
		_instanceList.Remove(instanceIter);
	}
}

/** 2012 0xb0d630. */
FxBool FxActor::_findInstance(FxActorInstance* pActorInstance, FxList<FxActorInstance*>::Iterator& instanceIter)
{
	for( FxList<FxActorInstance*>::Iterator curr = _instanceList.Begin(); curr != _instanceList.End(); ++curr )
	{
		if( *curr == pActorInstance )
		{
			instanceIter = curr;
			return FxTrue;
		}
	}
	return FxFalse;
}

/** 2012 0xb0cdf0. */
void FxActor::Serialize(FxArchive& arc)
{
	Super::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxActor");
	arc << _masterBoneList;
	if( version >= 3 )
	{
		arc << _compiledFaceGraph;
	}
	else if( arc.IsLoading() )
	{
		arc << _decompiledFaceGraph;
		CompileFaceGraph();
		_decompiledFaceGraph.Clear();
	}
	if( version < 2 )
	{
		arc << _animGroups;
		FxSize numAnimGroups = _animGroups.Length();
		for( FxSize i = 0; i < numAnimGroups; ++i )
		{
			if( _animGroups[i].GetName() == FxAnimGroup::Default )
			{
				_defaultAnimGroup = _animGroups[i];
				_animGroups.Remove(i);
				break;
			}
		}
	}
	else
	{
		arc << _defaultAnimGroup << _animGroups;
	}
	if( arc.IsLoading() && version < 3 )
	{
		FxSize trashedRegisterDefsLength = 0;
		arc << trashedRegisterDefsLength;
		for( FxSize j = 0; j < trashedRegisterDefsLength; ++j )
		{
			FxName trashedRegisterDefName;
			arc << trashedRegisterDefName;
		}
	}
	if( (arc.IsLoading() && version >= 1) || arc.IsSaving() )
	{
		arc << _phonemeMap;
	}
}

/** 2012 0xb0d040. */
FxBool FxLoadActorFromFile(FxActor& actor, const FxChar* filename, const FxBool bUseFastMethod, void (*callbackFunction)(FxReal), FxReal updateFrequency)
{
	FxArchiveStore* pStore = bUseFastMethod ? static_cast<FxArchiveStore*>(FxArchiveStoreFileFast::Create(filename)) : static_cast<FxArchiveStore*>(FxArchiveStoreFile::Create(filename));
	FxArchive actorArchive(pStore, FxArchive::AM_LinearLoad);
	actorArchive.Open();
	if( !actorArchive.IsValid() )
	{
		return FxFalse;
	}
	actorArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	actorArchive << actor;
	return FxTrue;
}

/** 2012 0xb0d130. A directory pass over a null store, then the save with that directory. */
FxBool FxSaveActorToFile(FxActor& actor, const FxChar* filename, FxArchive::FxArchiveByteOrder byteOrder, void (*callbackFunction)(FxReal), FxReal updateFrequency)
{
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << actor;
	FxArchive actorArchive(FxArchiveStoreFile::Create(filename), FxArchive::AM_Save, byteOrder);
	actorArchive.Open();
	if( !actorArchive.IsValid() )
	{
		return FxFalse;
	}
	actorArchive.SetInternalDataState(directoryCreater.GetInternalData());
	actorArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	actorArchive << actor;
	return FxTrue;
}

/** 2012 0xb0d270. */
FxBool FxLoadActorFromMemory(FxActor& actor, const FxByte* pMemory, const FxSize numBytes, void (*callbackFunction)(FxReal), FxReal updateFrequency)
{
	if( !pMemory )
	{
		return FxFalse;
	}
	FxArchive actorArchive(FxArchiveStoreMemoryNoCopy::Create(pMemory, numBytes), FxArchive::AM_LinearLoad);
	actorArchive.Open();
	if( !actorArchive.IsValid() )
	{
		return FxFalse;
	}
	actorArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	actorArchive << actor;
	return FxTrue;
}

/** 2012 0xb0d340. Refused when pMemory already points somewhere; the caller frees the result with FxFree(pMemory, numBytes). */
FxBool FxSaveActorToMemory(FxActor& actor, FxByte*& pMemory, FxSize& numBytes, FxArchive::FxArchiveByteOrder byteOrder, void (*callbackFunction)(FxReal), FxReal updateFrequency)
{
	if( pMemory )
	{
		return FxFalse;
	}
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << actor;
	FxArchiveStoreMemory* pStore = FxArchiveStoreMemory::Create(0, 0);
	FxArchive actorArchive(pStore, FxArchive::AM_Save, byteOrder);
	actorArchive.Open();
	if( !actorArchive.IsValid() )
	{
		return FxFalse;
	}
	actorArchive.SetInternalDataState(directoryCreater.GetInternalData());
	actorArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	actorArchive << actor;
	numBytes = pStore->GetSize();
	pMemory = static_cast<FxByte*>(FxAllocate(numBytes));
	FxMemcpy(pMemory, pStore->GetMemory(), numBytes);
	return FxTrue;
}

} // namespace Face
} // namespace OC3Ent
