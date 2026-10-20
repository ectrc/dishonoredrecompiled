/*=============================================================================
	UnFaceFXAsset.cpp: Code for managing FaceFX Assets
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// DISHONORED(port): agent FX3 (wave 26), with the reconstructed SDK (External/FaceFX). Checked against retail:
//   Serialize 2013 0x52b280 (2012 0x56c360, 0.62): the reference body is retail's - load: empty MountedFaceFXAnimSets and both
//     byte arrays, read them, RemoveActor the old actor, new FxActor (0x60A4 B) from RawFaceFXActorBytes, rename a "NewActor"
//     to the object name, SetShouldClientRelink, AddActor (a duplicate is deleted), and drop the bytes outside the editor;
//     save: FxSaveActorToMemory, big endian when cooking for a console (GCookingTarget & 0x28C). The 0.62 ratio is the
//     reference's warnf lines, which Shipping strips.
//   MountFaceFXAnimSet 0x52fe40 / UnmountFaceFXAnimSet 0x52fec0 (bytes 1.000, the reference's minus its check()s),
//   PostLoad 0x52d510, FinishDestroy 0x52ff80. FixupReferencedSoundCues: editor only, not ported (below).
#include "EnginePrivate.h"
#include "UnFaceFXSupport.h"
#include "EngineAnimClasses.h"

#if WITH_FACEFX

#include "FxArchiveStoreNull.h"

using namespace OC3Ent;
using namespace Face;
#endif

IMPLEMENT_CLASS(UFaceFXAsset);

#if WITH_FACEFX
/** Map of all the FaceFXAssets to which a FaceFXAnimSet is mounted. */
extern TMultiMap<UFaceFXAnimSet*, UFaceFXAsset*> GFaceFXAnimSetToAssetMap;

/** Map of all the FaceFXAnimSets which are mounted to a particular FaceFXAsset. */
extern TMultiMap<UFaceFXAsset*, UFaceFXAnimSet*> GFaceFXAssetToAnimSetMap;
#endif

void UFaceFXAsset::MountFaceFXAnimSet( UFaceFXAnimSet* AnimSet )
{
#if WITH_FACEFX
#if LOG_FACEFX_PERF
	DWORD StartMount = appCycles();
#endif
	SCOPE_CYCLE_COUNTER(STAT_FaceFX_Mount);
	FxActor* InternalActor = GetFxActor();
	if( InternalActor && AnimSet )
	{
		FxAnimSet* InternalAnimSet = AnimSet->GetFxAnimSet();
		if( InternalAnimSet )
		{
			if( InternalActor->MountAnimSet(*InternalAnimSet) )
			{
				// Only track mounted AnimSets in the editor (or when cooking) - in the game this prevents them from being GC'd
				if((GIsEditor && !GIsGame) || GIsUCC || GIsCooking)
				{
					MountedFaceFXAnimSets.AddUniqueItem(AnimSet);
				}

				// Update maps between AnimSet and Asset.
				check( !GFaceFXAnimSetToAssetMap.FindPair(AnimSet, this) );
				GFaceFXAnimSetToAssetMap.Add(AnimSet, this);

				check( !GFaceFXAssetToAnimSetMap.FindPair(this, AnimSet) );
				GFaceFXAssetToAnimSetMap.Add(this, AnimSet);
			}
		}
	}
#if LOG_FACEFX_PERF
	DWORD EndMount = appCycles();
	debugf(TEXT("DevFaceFX_Perf: Mounting %s in %s: %f ms"), 
		*GetPathName(),
		*AnimSet->GetPathName(),
		(EndMount-StartMount)*GSecondsPerCycle*1000.0f);
#endif
#endif
}

void UFaceFXAsset::UnmountFaceFXAnimSet( UFaceFXAnimSet* AnimSet )
{
#if WITH_FACEFX
#if LOG_FACEFX_PERF
	DWORD StartUnmount = appCycles();
#endif
	SCOPE_CYCLE_COUNTER(STAT_FaceFX_UnMount);
	FxActor* InternalActor = GetFxActor();
	FxBool bUnmountResult = FxFalse;
	if( InternalActor && AnimSet )
	{
		FxAnimSet* InternalAnimSet = AnimSet->GetFxAnimSet();
		if( InternalAnimSet )
		{
			FxName SetName = InternalAnimSet->GetAnimGroup().GetName();
			bUnmountResult = InternalActor->UnmountAnimSet(SetName);
		}
	}

	if (bUnmountResult)
	{
		MountedFaceFXAnimSets.RemoveItem(AnimSet);

		// Update maps between AnimSet and Asset.
		check( GFaceFXAnimSetToAssetMap.FindPair(AnimSet, this) );
		GFaceFXAnimSetToAssetMap.RemovePair(AnimSet, this);

		check( GFaceFXAssetToAnimSetMap.FindPair(this, AnimSet) );
		GFaceFXAssetToAnimSetMap.RemovePair(this, AnimSet);
	}

#if LOG_FACEFX_PERF
	DWORD EndUnmount = appCycles();
	debugf(TEXT("DevFaceFX_Perf: Unmounting %s from %s: %f ms"), 
		*GetPathName(),
		*AnimSet->GetPathName(),
		(EndUnmount-StartUnmount)*GSecondsPerCycle*1000.0f);
#endif
#endif
}

void UFaceFXAsset::CreateFxActor( USkeletalMesh* SkelMesh )
{
#if WITH_EDITORONLY_DATA
	DefaultSkelMesh = SkelMesh;

#if WITH_FACEFX
	if( DefaultSkelMesh && !FaceFXActor )
	{
		FxName ActorName(TCHAR_TO_ANSI(*GetName()));
		FxActor* Actor = FxActor::FindActor(ActorName);
		if( Actor )
		{
			warnf(NAME_DevFaceFX, TEXT("FaceFX: Duplicate Actor Detected -> %s"), *GetName());
		}
		else
		{
			Actor = new FxActor();
			Actor->SetName(ActorName);
			Actor->SetShouldClientRelink(FxTrue);
			FxActor::AddActor(Actor);
			FaceFXActor = Actor;
			DefaultSkelMesh->FaceFXAsset = this;
		}
	}
#endif
#endif // WITH_EDITORONLY_DATA
}

#if WITH_FACEFX
class FxActor* UFaceFXAsset::GetFxActor( void )
{
	if( !FaceFXActor )
		return NULL;
	
	FxActor* Actor = (FxActor*)FaceFXActor;
	return Actor;
}
#endif

// DISHONORED(not ported): agent FX3. Editor only - PostLoad calls it when GIsEditor && !GIsCooking && !GIsUCC (retail 2012
// 0x56e320 / 0x56d370, 45 B, as here). Retail's body (2012 0x56d4c0, 3,675 B) relinks the anims' Wwise events (Arkane's FxAnim
// version 8: event path / index / pointer through ReferencedAkEvents); the reference body this replaces loads UE3 sound cues,
// which Arkane's FxAnim does not carry. The game never runs it.
void UFaceFXAsset::FixupReferencedSoundCues()
{
}

FString UFaceFXAsset::GetDesc()
{
	INT NumFaceGraphNodes = 0;
	INT NumRefBones = 0;
	INT NumAnimGroups = 0;
	INT NumAnims = 0;
	FString ActorName;
#if WITH_FACEFX
	FxActor* Actor = GetFxActor();
	if( Actor )
	{
		NumFaceGraphNodes = static_cast<INT>(Actor->GetCompiledFaceGraph().nodes.Length());
		NumRefBones = static_cast<INT>(Actor->GetMasterBoneList().GetNumRefBones());
		NumAnimGroups = static_cast<INT>(Actor->GetNumAnimGroups());
		INT NumTotalAnimGroups = NumAnimGroups;
		for( INT i = 0 ; i < NumTotalAnimGroups; ++i )
		{
			// Ignore mounted animation groups to prevent confusion.
			if( Actor->IsAnimGroupMounted(Actor->GetAnimGroup(i).GetName()) )
			{
				NumAnimGroups--;
			}
			else
			{
				NumAnims += Actor->GetAnimGroup(i).GetNumAnims();
			}
		}
		ActorName = ANSI_TO_TCHAR(Actor->GetNameAsCstr());
	}
#endif
	return FString::Printf(TEXT("%d FG Nodes, %d Ref. Bones, %d Anim Groups, %d Anims, FaceFX Actor Name: %s"), 
		NumFaceGraphNodes, NumRefBones, NumAnimGroups, NumAnims, *ActorName);
}

void UFaceFXAsset::PostLoad()
{
    Super::PostLoad();

	if( ( GIsEditor == TRUE ) && ( GIsCooking == FALSE ) && ( GIsUCC == FALSE ) )
	{
		FixupReferencedSoundCues();
	}
}

void UFaceFXAsset::FinishDestroy()
{
	// Clean up FaceFX stuff when the asset gets garbage collected.
#if WITH_FACEFX
	// Clean up GFaceFXAnimSetToAssetMap of any refs to this Asset.
	// We don't need to explicitly unmount AnimSets here - we just update the map so we dont have pointer to bogus Assets.
	TArray<UFaceFXAnimSet*> OutAnimSets;
	GFaceFXAssetToAnimSetMap.MultiFind(this, OutAnimSets);
	for(INT i=0; i<OutAnimSets.Num(); i++)
	{
		UFaceFXAnimSet* AnimSet = OutAnimSets(i);
		check(AnimSet);

		check(GFaceFXAnimSetToAssetMap.FindPair(AnimSet, this));
		GFaceFXAnimSetToAssetMap.RemovePair(AnimSet, this);

		check(GFaceFXAssetToAnimSetMap.FindPair(this, AnimSet));
		GFaceFXAssetToAnimSetMap.RemovePair(this, AnimSet);
	}

	FxActor* Actor = GetFxActor();
	if( Actor )
	{
        FxActor::RemoveActor(Actor);
		FaceFXActor = NULL;
	}
#endif
	
	Super::FinishDestroy();
}

void UFaceFXAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

#if WITH_FACEFX
	FxActor* Actor = GetFxActor();
#endif

    // FaceFX data is embedded as raw bytes straight into packages.
	if( Ar.IsSaving() )
	{
		RawFaceFXActorBytes.Empty();

#if WITH_FACEFX
        if( Actor )
		{
			FxArchive::FxArchiveByteOrder ByteOrder = FxArchive::ABO_LittleEndian;
			// If cooking, save the data in Big Endian format.
			if( GCookingTarget & UE3::PLATFORM_Console )
			{
#if __INTEL_BYTE_ORDER__
				ByteOrder = FxArchive::ABO_BigEndian;
#else
				ByteOrder = FxArchive::ABO_LittleEndian;
#endif
			}
			FxByte* ActorMemory = NULL;
			FxSize  NumActorMemoryBytes = 0;
			if( !FxSaveActorToMemory(*Actor, ActorMemory, NumActorMemoryBytes, ByteOrder) )
			{
				warnf(TEXT("FaceFX: Failed to save actor for %s"), *GetPathName());
			}
			RawFaceFXActorBytes.Add(NumActorMemoryBytes);
			appMemcpy(RawFaceFXActorBytes.GetData(), ActorMemory, NumActorMemoryBytes);
			FxFree(ActorMemory, NumActorMemoryBytes);
		}
#endif

		Ar << RawFaceFXActorBytes;
		Ar << RawFaceFXSessionBytes;
	}
	else if( Ar.IsLoading() )
	{
		MountedFaceFXAnimSets.Empty();
		RawFaceFXActorBytes.Empty();
		RawFaceFXSessionBytes.Empty();
	
		Ar << RawFaceFXActorBytes;
		Ar << RawFaceFXSessionBytes;

#if WITH_FACEFX
		if( Actor )
		{
			FxActor::RemoveActor(Actor);
		}
		Actor = new FxActor();
		FaceFXActor = Actor;

#if LOG_FACEFX_PERF
		DWORD StartSerialization = appCycles();
#endif
		if( !FxLoadActorFromMemory(*Actor, static_cast<FxByte*>(RawFaceFXActorBytes.GetData()), RawFaceFXActorBytes.Num()) )
		{
			warnf(TEXT("FaceFX: Failed to load actor for %s"), *GetPathName());
		}
#if LOG_FACEFX_PERF
		DWORD EndSerialization = appCycles();
#endif

		// this can be optional, but is probably a better idea than leaving "NewActor" named assets around
		if( Actor->GetName() == FxActor::NewActor ) // FxActor::NewActor is a static so you don't need to hardcode "NewActor" strings
		{
			// issue warning here, noting the name change and need to resave package

			FxName ActorName(TCHAR_TO_ANSI(*GetName()));
			Actor->SetName(ActorName);

			// mark package dirty for resave 
		}

		Actor->SetShouldClientRelink(FxTrue);

		// will return FxFalse if actor couldn't be added, meaning there was a duplicate
		if( !FxActor::AddActor(Actor) )
		{
			FaceFXActor = NULL;
			// issue duplicate actor warning here, just like in the code
			warnf(TEXT("FaceFX: Failed to add actor for %s."), *GetPathName());
			// free the memory allocated during load and make sure the actor is set to null
			delete Actor;
			Actor = NULL;
		}

#endif
#if LOG_FACEFX_PERF
		FxSize NumBytes = RawFaceFXActorBytes.Num();
#endif

		// Flush raw bytes that are only needed in the editor and ucc.
		if( !GIsEditor && !GIsUCC )
		{
			RawFaceFXActorBytes.Empty();
			RawFaceFXSessionBytes.Empty();
		}

#if LOG_FACEFX_PERF
		debugf(TEXT("DevFaceFX_Perf: Loading asset %s : (Total Bytes: %d) Serialization: %f ms"), 
			*GetPathName(),
			NumBytes,
			(EndSerialization-StartSerialization)*GSecondsPerCycle*1000.0f);
#endif
	}
	else if( Ar.IsCountingMemory() )
	{
		//TODO: Implement without actually going through the save code
		//Ar << FaceFXActor;
		//Ar << RawFaceFXActorBytes;
		//Ar << RawFaceFXSessionBytes;
	}
}

/**
 * Used by various commandlets to purge editor only and platform-specific data from various objects
 * 
 * @param PlatformsToKeep Platforms for which to keep platform-specific data
 * @param bStripLargeEditorData If TRUE, data used in the editor, but large enough to bloat download sizes, will be removed
 */
void UFaceFXAsset::StripData(UE3::EPlatformType PlatformsToKeep, UBOOL bStripLargeEditorData)
{
	Super::StripData(PlatformsToKeep, bStripLargeEditorData); 

#if WITH_EDITORONLY_DATA
	// if we aren't keeping any non-stripped platforms, we can toss the data
	if (!(PlatformsToKeep & ~UE3::PLATFORM_Stripped))
	{
//		RawFaceFXActorBytes.Empty();
		RawFaceFXSessionBytes.Empty();
	}
#endif // WITH_EDITORONLY_DATA
}

/** 
 *	Get list of FaceFX animations in this Asset. Names are in the form GroupName.AnimName.
 *	@param bExcludeMountedGroups	If true, do not show animations that are in separate FaceFXAnimSets currently mounted to the Asset.
 */
void UFaceFXAsset::GetSequenceNames(UBOOL bExcludeMountedGroups, TArray<FString>& OutNames)
{
#if WITH_FACEFX
	FxActor* fActor = GetFxActor();
	check(fActor);

	// Iterate over all groups mounted in this FxActor.
	INT NumGroups = fActor->GetNumAnimGroups();
	for(INT i=0; i<NumGroups; ++i)
	{
		const FxAnimGroup& fGroup = fActor->GetAnimGroup(i);
		const FxChar* fGroupName = fGroup.GetNameAsCstr();

		if( !bExcludeMountedGroups || !fActor->IsAnimGroupMounted(fGroupName) )
		{
			INT NumAnims = static_cast<INT>(fGroup.GetNumAnims());
			for(INT j=0; j<NumAnims; ++j)
			{
				const FxAnim& fAnim = fGroup.GetAnim(j);
				const FxChar* fAnimName = fAnim.GetNameAsCstr();

				FString SeqFullName = FString::Printf( TEXT("%s.%s"), ANSI_TO_TCHAR(fGroupName), ANSI_TO_TCHAR(fAnimName) );
				OutNames.AddItem(SeqFullName);
			}
		}
	}
#endif
}

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT UFaceFXAsset::GetResourceSize()
{
#if WITH_FACEFX
	UINT ResourceSize = 0;
	if ( !GExclusiveResourceSizeMode )
	{
		FArchiveCountMem CountBytesSize( this );
		CountBytesSize << RawFaceFXActorBytes;
#if WITH_EDITORONLY_DATA
		CountBytesSize << ReferencedSoundCues;
#endif // WITH_EDITORONLY_DATA
		ResourceSize += CountBytesSize.GetNum();
	}

	if (!GIsEditor && FaceFXActor)
	{
		//@todo: FaceFX crashes if this code is used in debug builds
#if !_DEBUG
		// This is an approximation of RawFaceFXActorBytes that works outside the editor
		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		directoryCreater << *(FxActor*)FaceFXActor;
		FxArchiveStoreNull* pStore = FxArchiveStoreNull::Create();
		FxArchive actorArchive(pStore, FxArchive::AM_ApproximateMemoryUsage);
		actorArchive.Open();
		if( actorArchive.IsValid() )
		{
			actorArchive.SetInternalDataState(directoryCreater.GetInternalData());
			actorArchive << *(FxActor*)FaceFXActor;
			ResourceSize += pStore->GetSize();
		}
#endif
		// Each asset makes a copy of any mounted animsets, so we need to include those sizes
		TArray<UFaceFXAnimSet*> OutAnimSets;
		GFaceFXAssetToAnimSetMap.MultiFind(this, OutAnimSets);
		for(INT i=0; i<OutAnimSets.Num(); i++)
		{
			UFaceFXAnimSet* AnimSet = OutAnimSets(i);
			if (AnimSet)
			{			
				ResourceSize += AnimSet->GetResourceSize();
			}
		}
	}
	return ResourceSize;
#else
	return 0;
#endif
}

