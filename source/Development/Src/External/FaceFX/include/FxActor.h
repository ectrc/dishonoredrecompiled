/*=============================================================================
	FxActor.h - FaceFX 1.7.3.1 actor (fxactor.h): the master bone list, the compiled face graph (and its editable
	form), the default/owned/mounted animation groups, the instances playing on it and the phoneme map.

	DISHONORED(port): agent FX3 (wave 26). 0x60A4 bytes (2012 PDB, AllocateStaticClassFxActor 0xb0c1d0, class
	version 3). Header inlines retail emitted: FindAnimGroup 0x238ac0 and GetAnimGroup 0x238c20 (engine units), GetAnimPtr
	0xb0ba10 (FxActorInstance.obj). FindAnimGroup searches the mounted groups first (index numAnimGroups + 1 + k), then
	the owned groups (index i + 1), then the default group (0); FxInvalidIndex when none matches.
	GetNumAnimGroups (1 + owned + mounted) and IsAnimGroupMounted (a walk of the mounted list, inlined into
	UFaceFXAsset::GetDesc 2012 0x56c020) are reconstructions; no out-of-line copy exists.
=============================================================================*/
#ifndef FxActor_H__
#define FxActor_H__

#include "FxMasterBoneList.h"
#include "FxPhonemeMap.h"
#include "FxAnim.h"
#include "FxList.h"

namespace OC3Ent
{
namespace Face
{

class FxActorInstance;

enum FxMountUnmountExtendedInfo
{
	MUE_Duplicate = 0,
	MUE_InvalidMountedAnimSet,
	MUE_CurrentlyPlayingTryLater,
	MUE_None
};

class FxActor : public FxNamedObject
{
	FX_DECLARE_CLASS(FxActor, FxNamedObject)
public:
	FxActor();
	virtual ~FxActor();

	static void Startup();
	static void Shutdown();

	FxBool ShouldClientRelink() const
	{
		return _shouldClientRelink;
	}

	void SetShouldClientRelink(FxBool shouldClientRelink);

	FxBool IsOpenInStudio() const
	{
		return _isOpenInStudio;
	}

	void SetIsOpenInStudio(FxBool isOpenInStudio);

	FxMasterBoneList& GetMasterBoneList()
	{
		return _masterBoneList;
	}

	FxFaceGraph& GetDecompiledFaceGraph()
	{
		return _decompiledFaceGraph;
	}

	FxCompiledFaceGraph& GetCompiledFaceGraph()
	{
		return _compiledFaceGraph;
	}

	void CompileFaceGraph();
	void DecompileFaceGraph();

	FxSize GetNumAnimGroups() const
	{
		return 1 + _animGroups.Length() + _mountedAnimGroups.Length();
	}

	FxBool IsAnimGroupMounted(const FxName& name) const
	{
		for( FxList<FxAnimGroup>::ConstIterator iter = _mountedAnimGroups.Begin(); iter != _mountedAnimGroups.End(); ++iter )
		{
			if( iter->GetName() == name )
			{
				return FxTrue;
			}
		}
		return FxFalse;
	}

	/** 2012 0x238ac0. */
	FxSize FindAnimGroup(const FxName& name) const
	{
		FxSize numAnimGroups = _animGroups.Length();
		FxSize mountedIndex = 0;
		for( FxList<FxAnimGroup>::ConstIterator iter = _mountedAnimGroups.Begin(); iter != _mountedAnimGroups.End(); ++iter, ++mountedIndex )
		{
			if( iter->GetName() == name )
			{
				return mountedIndex + numAnimGroups + 1;
			}
		}
		for( FxSize i = 0; i < numAnimGroups; ++i )
		{
			if( _animGroups[i].GetName() == name )
			{
				return i + 1;
			}
		}
		return _defaultAnimGroup.GetName() == name ? 0 : FxInvalidIndex;
	}

	/** 2012 0x238c20. */
	FxAnimGroup& GetAnimGroup(FxSize index)
	{
		FxSize numAnimGroups = _animGroups.Length();
		if( index == 0 )
		{
			return _defaultAnimGroup;
		}
		if( index <= numAnimGroups )
		{
			return _animGroups[index - 1];
		}
		FxList<FxAnimGroup>::Iterator iter = _mountedAnimGroups.Begin();
		for( FxSize i = index - numAnimGroups - 1; i; --i )
		{
			++iter;
		}
		return *iter;
	}

	/** 2012 0xb0ba10. */
	FxAnim* GetAnimPtr(const FxName& group, const FxName& anim)
	{
		FxSize groupIndex = FindAnimGroup(group);
		if( groupIndex == FxInvalidIndex )
		{
			return NULL;
		}
		FxAnimGroup& animGroup = GetAnimGroup(groupIndex);
		FxSize animIndex = animGroup.FindAnim(anim);
		if( animIndex == FxInvalidIndex )
		{
			return NULL;
		}
		return animGroup.GetAnimPtr(animIndex);
	}

	FxBool AddAnim(const FxName& group, const FxAnim& anim);
	FxBool RemoveAnim(const FxName& group, const FxName& anim);
	FxBool AddAnimGroup(const FxName& name);
	FxBool RemoveAnimGroup(const FxName& name);

	FxBool MountAnimSet(const FxAnimSet& animSet, FxMountUnmountExtendedInfo* pExtendedInfo = NULL);
	FxBool UnmountAnimSet(const FxName& animSetName, FxBool bStopPlayback = FxTrue, FxMountUnmountExtendedInfo* pExtendedInfo = NULL);
	FxBool ImportAnimSet(const FxAnimSet& animSet);
	FxBool ExportAnimSet(const FxName& animGroupName, FxAnimSet& exportedAnimSet);

	FxPhonemeMap& GetPhonemeMap()
	{
		return _phonemeMap;
	}

	static FxActor* FindActor(const FxName& name);
	static FxBool FindActorPtr(FxActor* actor);
	static void SetShouldFreeActorMemory(FxBool shouldFreeActorMemory);
	static FxBool GetShouldFreeActorMemory();
	static FxBool AddActor(FxActor* actor);
	static FxBool RemoveActor(FxActor* actor);
	static void FlushActors();

	void AddInstance(FxActorInstance* pActorInstance);
	void RemoveInstance(FxActorInstance* pActorInstance);

	virtual void Serialize(FxArchive& arc);

	static FxName NewActor;

protected:
	FxBool _findInstance(FxActorInstance* pActorInstance, FxList<FxActorInstance*>::Iterator& instanceIter);

	FxBool _shouldClientRelink;
	FxBool _isOpenInStudio;
	FxMasterBoneList _masterBoneList;
	FxFaceGraph _decompiledFaceGraph;
	FxCompiledFaceGraph _compiledFaceGraph;
	FxAnimGroup _defaultAnimGroup;
	FxArray<FxAnimGroup> _animGroups;
	FxList<FxAnimGroup> _mountedAnimGroups;
	FxList<FxActorInstance*> _instanceList;
	FxPhonemeMap _phonemeMap;

	static FxList<FxActor*>* _pActorList;
	static FxBool _shouldFreeActorMemory;
};

FxBool FxLoadActorFromFile(FxActor& actor, const FxChar* filename, const FxBool bUseFastMethod, void (*callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f);
FxBool FxSaveActorToFile(FxActor& actor, const FxChar* filename, FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian, void (*callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f);
FxBool FxLoadActorFromMemory(FxActor& actor, const FxByte* pMemory, const FxSize numBytes, void (*callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f);
FxBool FxSaveActorToMemory(FxActor& actor, FxByte*& pMemory, FxSize& numBytes, FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian, void (*callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f);

} // namespace Face
} // namespace OC3Ent

#endif
