// DishonoredGame/src/distweaksbase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (37):
//   0x8d94d0  public: static void __cdecl UDisTweaksBase::InitializePrivateStaticClassUDisTweaksBase(void)
//   0x8d94f0  public: static void __cdecl UDisTweaksInterface::InitializePrivateStaticClassUDisTweaksInterface(void)
//   0x8d9510  public: static void __cdecl UDisTweaksList::InitializePrivateStaticClassUDisTweaksList(void)
//   0x8d9530  public: void __thiscall FDisTweakChildInfo::SetTweakChildInfo(class FName const &, class UDisTweaksBase *, int, class FName const &)
//   0x8d9570  public: void __thiscall FDisTweakChildInfo::SetTweakChildInfo(struct FDisTweakChildInfo_ChildName const &, class UDisTweaksBase *, int)
//   0x8d95a0  public: __thiscall FDisTweaksGenericThumbnailItem::FDisTweaksGenericThumbnailItem(class FName const &, class USkeletalMesh *, int)
//   0x8d95e0  public: __thiscall FDisTweaksGenericThumbnailItem::FDisTweaksGenericThumbnailItem(class FName const &, class UStaticMesh *, int)
//   0x8d9620  public: virtual class UClass * __thiscall UDisTweaksBase::GetSpawnedObjectClass(enum eDisTweaksSpawnType)const
//   0x8d9640  public: class AActor * __thiscall UDisTweaksBase::SpawnActor(enum eDisTweaksSpawnType, class FName, class FVector const &, class FRotator const &, class AActor *, unsigned int, unsigned int, class AActor *, class APawn *, unsigned int, unsigned int)const
//   0x8dcbc0  public: static class UClass * __cdecl UDisTweaksInterface::GetPrivateStaticClassUDisTweaksInterface(wchar_t const *)
//   0x8dcc50  protected: int __thiscall UDisTweaksBase::FindFallbackSkip(class FString const &)
//   0x8dccd0  public: unsigned int __thiscall UDisTweaksBase::IsFallbackDerivedFrom(class UDisTweaksBase const * const)const
//   0x8dcdc0  public: unsigned int __thiscall UDisTweaksBase::IsFallbackDerivedFrom(class FString const &)const
//   0x8dcf80  public: unsigned int __thiscall UDisTweaksBase::IsFallbackRelated(class UDisTweaksBase const * const)const
//   0x8dcfc0  public: unsigned int __thiscall UDisTweaksBase::CanSpawnObjectOfClass(class UClass const *, enum eDisTweaksSpawnType)
//   0x8dd000  protected: virtual class AActor * __thiscall UDisTweaksBase::SpawnActor_Derived(enum eDisTweaksSpawnType, class FName, class FVector const &, class FRotator const &, class AActor *, unsigned int, unsigned int, class AActor *, class APawn *, unsigned int, unsigned int)
//   0x8e0400  public: static class UClass * __cdecl UDisTweaksInterface::StaticClassNoInline(void)
//   0x8e0430  public: virtual unsigned int __thiscall UDisTweaksBase::ComparePropForPerObjectLocalizationExport(class UProperty *, unsigned char *, int, wchar_t const *)
//   0x8e0500  protected: void __thiscall UDisTweaksBase::ApplyFallbackChain_Struct(class UStruct const *, class UProperty const *, class FString const &, int)
//   0x8e0870  void __cdecl TweakBuildPropPath(class FEditPropertyChain *, class FString &)
//   0x8e0a00  public: virtual unsigned int __thiscall UDisTweaksBase::EditConditionAskObj_IsConditionMet(class FEditPropertyChain *, wchar_t const *)
//   0x8e48c0  protected: void __thiscall UDisTweaksBase::InvalidateFallback_Recurse(void)
//   0x8e6fc0  public: void __thiscall UDisTweaksBase::ApplyFallbackChain(void)
//   0x8e7650  public: void __thiscall UDisTweaksBase::SkipFallback(class FName const &)
//   0x8ea0c0  public: static class UClass * __cdecl UDisTweaksList::GetPrivateStaticClassUDisTweaksList(wchar_t const *)
//   0x8ea150  public: void __thiscall IDisTweaksInterface::ApplyTweakChanges(void)
//   0x8ea190  public: virtual void __thiscall UDisTweaksBase::PostLoad(void)
//   0x8ea1c0  public: virtual void __thiscall UDisTweaksBase::EditConditionAskObj_SetCondition(class FEditPropertyChain *, wchar_t const *, unsigned int)
//   0x8ec4d0  public: static class UClass * __cdecl UDisTweaksList::StaticClassNoInline(void)
//   0x8ec500  public: virtual void __thiscall FSpawnActor_TweakObj::DoInit(class AActor *)
//   0x8ec550  public: virtual void __thiscall FSpawnNPCPawn_TweakObj::DoInit(class AActor *)
//   0x8ee320  public: static class UClass * __cdecl UDisTweaksBase::GetPrivateStaticClassUDisTweaksBase(wchar_t const *)
//   0x8ef5a0  public: static class UClass * __cdecl UDisTweaksBase::StaticClassNoInline(void)
//   0x8f0600  public: virtual void __thiscall UDisTweaksBase::Serialize(class FArchive &)
//   0x8f1010  private: virtual unsigned int __thiscall IDisTweaksInterface::HasTweaks_Derived(class UDisEngineTweaksBase const &)const
//   0x8f1040  public: void __thiscall UDisTweaksBase::PostEditChangeChainProperty(struct FPropertyChangedChainEvent &)
//   0x8f12d0  public: static class FString __cdecl UDisTweaksBase::GetTweaksOwnerDisplayName(class UObject const *)


#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x866050 (2012 0x8d9620, same bytes)
UClass* UDisTweaksBase::GetSpawnedObjectClass( BYTE SpawnType ) const
{
	return SpawnType == eDisTweaksSpawnType_InEditor ? m_pSpawnedObjectClass_Editor : m_pSpawnedObjectClass;
}

// DISHONORED(written): 2013 rva 0x869b10 (2012 0x8dcfc0)
UBOOL UDisTweaksBase::CanSpawnObjectOfClass( const UClass* Class, BYTE SpawnType ) const
{
	for( const UClass* SpawnedClass = GetSpawnedObjectClass( SpawnType ); SpawnedClass; SpawnedClass = SpawnedClass->GetSuperClass() )
	{
		if( SpawnedClass == Class )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x866070 (2012 0x8d9640): fallback-only tweaks never spawn
AActor* UDisTweaksBase::SpawnActor( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const
{
	if( m_bOnlyUseAsFallback )
	{
		return NULL;
	}
	return SpawnActor_Derived( SpawnType, InName, Location, Rotation, Template, bNoCollisionFail, bRemoteOwned, Owner, Instigator, bNoFail );
}

// DISHONORED(written): 2013 rva 0x869b50 (2012 0x8dd000): retail passes FSpawnActor_TweakObj(this) to UWorld::SpawnActor,
// whose DoInit (SetTweaks) runs between the actor's transform and the begin-play chain (2013 rva 0x256990, the init-functor
// parameter agent AI ported in UnLevAct.cpp), so a tweaked actor sees its tweaks in PostBeginPlay
AActor* UDisTweaksBase::SpawnActor_Derived( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const
{
	UClass* SpawnedClass = GetSpawnedObjectClass( SpawnType );
	if( !SpawnedClass || !SpawnedClass->IsChildOf( AActor::StaticClass() ) )
	{
		return NULL;
	}
	FSpawnActor_TweakObj Init( const_cast<UDisTweaksBase*>( this ) );
	return GWorld->SpawnActor( SpawnedClass, InName, Location, Rotation, Template, bNoCollisionFail, bRemoteOwned, Owner, Instigator, bNoFail, NULL, &Init );
}

// DISHONORED(written): 2013 rva 0x885650 (2012 0x8ec500): SetTweaks on the spawned actor's IDisTweaksInterface (a NULL actor
// still runs SetTweaks on a NULL interface in retail, which is a crash; we skip it)
void FSpawnActor_TweakObj::DoInit( AActor* Actor )
{
	if( !Actor )
	{
		return;
	}
	IDisTweaksInterface* Tweakable = (IDisTweaksInterface*)Actor->GetInterfaceAddress( UDisTweaksInterface::StaticClass() );
	if( Tweakable )
	{
		Tweakable->SetTweaks( m_pTweakObj );
	}
}

// DISHONORED(written): 2012 rva 0x8ec550 (the NPC spawner path, not reached by the map load): retail stores m_pSpawner in the
// NPC pawn's native spawner member before the tweaks are applied; that member is not in the retail SDK dump (native-only
// region), so only the tweaks are applied here (DISHONORED(bringup))
void FSpawnNPCPawn_TweakObj::DoInit( AActor* Actor )
{
	FSpawnActor_TweakObj::DoInit( Actor );
}

// DISHONORED(written): 2013 rva 0x661fe0 (2012 0x6b1fe0, same bytes): only a change of tweak object re-applies the tweaks
void IDisTweaksInterface::SetTweaks( UDisTweaksBase* Tweaks )
{
	if( GetTweaks_Derived() != Tweaks )
	{
		SetTweaks_Derived( Tweaks );
		ApplyTweakChanges();
	}
}

// DISHONORED(written): 2013 rva 0x882a30 (2012 0x8ea150, same bytes)
void IDisTweaksInterface::ApplyTweakChanges()
{
	UObject* Object = GetUObjectInterfaceDisTweaksInterface();
	if( Object && !Object->IsPendingKill() )
	{
		UDisTweaksBase* Tweaks = GetTweaks_Derived();
		if( Tweaks )
		{
			Tweaks->ApplyFallbackChain();
			ApplyTweakChanges_Derived();
		}
	}
}

// DISHONORED(written): 2013 rva 0x5fb110 (2012 0x6419b0, same bytes): valid when the tweaks are neither a class default nor an
// archetype, nor owned by one
UBOOL IDisTweaksInterface::IsTweaksValid() const
{
	for( UObject* Object = GetTweaks(); Object; Object = Object->GetOuter() )
	{
		if( Object->HasAnyFlags( RF_ClassDefaultObject | RF_ArchetypeObject ) )
		{
			return FALSE;
		}
	}
	return GetTweaks() != NULL;
}

// DISHONORED(written): 2013 rva 0x873d60 (2012 0x8f1010, same bytes): without tweaks the UDisTweaksBase default object answers
UBOOL IDisTweaksInterface::HasTweaks_Derived( const UDisEngineTweaksBase& Tweaks ) const
{
	UDisTweaksBase* Own = GetTweaks();
	if( !Own )
	{
		Own = UDisTweaksBase::StaticClass()->GetDefaultObject<UDisTweaksBase>();
	}
	return Own->IsFallbackDerivedFrom( Cast<UDisTweaksBase>( const_cast<UDisEngineTweaksBase*>( &Tweaks ) ) );
}

// DISHONORED(written): 2013 rva 0x86f360 (2012 0x8dccd0): seek-free builds compare the cooked fallback names, editor builds walk
// the live m_pFallbackTweaks chain
UBOOL UDisTweaksBase::IsFallbackDerivedFrom( const UDisTweaksBase* Tweaks ) const
{
	if( !GUseSeekFreeLoading )
	{
		for( const UDisTweaksBase* Cur = this; Cur; Cur = Cur->m_pFallbackTweaks )
		{
			if( Cur == Tweaks )
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	if( !Tweaks )
	{
		return FALSE;
	}
	if( Tweaks == this )
	{
		return TRUE;
	}
	for( INT Index = 0; Index < m_FallbackChainCooked.Num(); Index++ )
	{
		if( m_FallbackChainCooked(Index).m_FallbackName == Tweaks->m_TweakNameCooked )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x86f5c0 (2012 0x8dcf80, same bytes)
UBOOL UDisTweaksBase::IsFallbackRelated( const UDisTweaksBase* Tweaks ) const
{
	return IsFallbackDerivedFrom( Tweaks ) || ( Tweaks && Tweaks->IsFallbackDerivedFrom( this ) );
}

// DISHONORED(written): 2013 rva 0x86f2e0 (2012 0x8dcc50)
INT UDisTweaksBase::FindFallbackSkip( const FString& PropertyPath ) const
{
	for( INT Index = 0; Index < m_FallbackSkip.Num(); Index++ )
	{
		if( m_FallbackSkip(Index).m_PropertyPathName == PropertyPath )
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

// DISHONORED(written): 2013 rva 0x87f3e0 (2012 0x8e7650, same bytes): a fallback-flagged property (PropertyFlags bit 0x4000 of
// the high dword, Arkane's tweak-fallback flag) is added to the skip list once
void UDisTweaksBase::SkipFallback( const FName& PropertyName )
{
	UProperty* Property = FindField<UProperty>( GetClass(), *PropertyName.ToString() );
	if( !Property || ( Property->PropertyFlags & CPF_DisTweakFallback ) != CPF_DisTweakFallback )
	{
		return;
	}
	if( FindFallbackSkip( PropertyName.ToString() ) == INDEX_NONE )
	{
		FDisTweakFallbackInfo* Info = new( m_FallbackSkip ) FDisTweakFallbackInfo(EC_EventParm);
		Info->m_PropertyPathName = PropertyName.ToString();
		InvalidateFallback_Recurse();
	}
}

// DISHONORED(written): 2013 rva 0x87ba20 (2012 0x8e48c0): the fallback time stamp is reset here and in every owned child tweak
void UDisTweaksBase::InvalidateFallback_Recurse()
{
	m_dFallbackTimeStamp = 0.0;
	TArray<FDisTweakChildInfo> Children;
	GatherTweakChildren_Derived( Children );
	for( INT Index = 0; Index < Children.Num(); Index++ )
	{
		if( ( Children(Index).m_ChildTweakFlags & 1 ) && Children(Index).m_pChildTweak )
		{
			Children(Index).m_pChildTweak->InvalidateFallback_Recurse();
		}
	}
}

// DISHONORED(written): 2013 rva 0x87ed80 (2012 0x8e6fc0, distweaksbase.cpp:291): the fallback chain copies the fallback-flagged
// properties of m_pFallbackTweaks into this object (ApplyFallbackChain_Struct), once per change of the chain (time stamps), then
// recurses into the child tweaks GatherTweakChildren_Derived lists, pairing owned children (flag 1 without 2) with the fallback
// object's child of the same name. Cooking-only branches (m_TweakNameCooked / m_FallbackChainCooked) are kept for reference.
void UDisTweaksBase::ApplyFallbackChain()
{
	if( IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
	{
		return;
	}
	if( GIsCooking && m_TweakNameCooked == NAME_None )
	{
		m_TweakNameCooked = FName( *GetPathName() );
	}
	ConditionalPostLoadSubobjects();
	if( m_pFallbackTweaks )
	{
		m_pFallbackTweaks->ApplyFallbackChain();
		if( GIsCooking && m_FallbackChainCooked.Num() == 0 )
		{
			new( m_FallbackChainCooked ) FDisTweakCookedFallback(EC_EventParm);
			m_FallbackChainCooked.Last().m_FallbackName = m_pFallbackTweaks->m_TweakNameCooked;
			m_FallbackChainCooked.Append( m_pFallbackTweaks->m_FallbackChainCooked );
		}
		if( !m_bFixupDefaultsCalled )
		{
			m_bFixupDefaultsCalled = TRUE;
			if( FixupDefaults_Derived() )
			{
				InvalidateFallback_Recurse();
			}
		}
		if( m_dFallbackTimeStamp < 0.0001 || m_pFallbackTweaks->m_dFallbackTimeStamp > m_dFallbackTimeStamp + 0.0001 )
		{
			for( INT Index = 0; Index < m_FallbackSkip.Num(); Index++ )
			{
				m_FallbackSkip(Index).m_bProcessed = FALSE;
			}
			ApplyFallbackChain_Struct( GetClass(), NULL, FString(), 0 );
			for( INT Index = m_FallbackSkip.Num() - 1; Index >= 0; Index-- )
			{
				if( !m_FallbackSkip(Index).m_bProcessed )
				{
					m_FallbackSkip.Remove( Index );
				}
			}
			ApplyFallbackChain_Derived();
			m_dFallbackTimeStamp = appSeconds();
		}
		TArray<FDisTweakChildInfo> Children;
		GatherTweakChildren_Derived( Children );
		for( INT Index = 0; Index < Children.Num(); Index++ )
		{
			const FDisTweakChildInfo& Cur = Children(Index);
			UDisTweaksBase* Child = ( Cur.m_ChildTweakFlags & 1 ) ? Cur.m_pChildTweak : NULL;
			if( !Child )
			{
				continue;
			}
			if( Cur.m_ChildTweakFlags & 2 )
			{
				Child->ApplyFallbackChain();
				continue;
			}
			TArray<FDisTweakChildInfo> FallbackChildren;
			m_pFallbackTweaks->GatherTweakChildren_Derived( FallbackChildren );
			UDisTweaksBase* FallbackChild = NULL;
			for( INT FallbackIndex = 0; FallbackIndex < FallbackChildren.Num(); FallbackIndex++ )
			{
				const FDisTweakChildInfo& Candidate = FallbackChildren(FallbackIndex);
				if( Candidate.m_ChildTweakName.m_ChildTweakName == Cur.m_ChildTweakName.m_ChildTweakName && Candidate.m_ChildTweakName.m_iAdditionalIndex == Cur.m_ChildTweakName.m_iAdditionalIndex )
				{
					FallbackChild = Candidate.m_pChildTweak;
					break;
				}
			}
			Child->m_pFallbackTweaks = FallbackChild;
			Child->m_bBeenLocalized = m_bBeenLocalized;
			Child->ApplyFallbackChain();
		}
	}
	else
	{
		if( m_dFallbackTimeStamp < 0.0001 )
		{
			m_dFallbackTimeStamp = appSeconds();
		}
		if( !m_bFixupDefaultsCalled )
		{
			m_bFixupDefaultsCalled = TRUE;
			FixupDefaults_Derived();
		}
		ApplyFallbackChain_Derived();
		TArray<FDisTweakChildInfo> Children;
		GatherTweakChildren_Derived( Children );
		for( INT Index = 0; Index < Children.Num(); Index++ )
		{
			const FDisTweakChildInfo& Cur = Children(Index);
			if( !( Cur.m_ChildTweakFlags & 1 ) )
			{
				continue;
			}
			UDisTweaksBase* Child = Cur.m_pChildTweak;
			if( !( Cur.m_ChildTweakFlags & 2 ) )
			{
				Child->m_pFallbackTweaks = NULL;
				Child->m_dFallbackTimeStamp = m_dFallbackTimeStamp;
				Child->m_bBeenLocalized = m_bBeenLocalized;
			}
			Child->ApplyFallbackChain();
		}
	}
	if( GIsCooking )
	{
		m_pFallbackTweaks = NULL;
	}
}

// DISHONORED(written): 2013 through ApplyFallbackChain (2012 rva 0x8e0500, distweaksbase.cpp:470): every non-transient property
// of the struct (super classes included) is either in the skip list (marked processed), a struct property recursed per array
// element, or a fallback-flagged property copied from m_pFallbackTweaks when its owner class is on the fallback object's class chain
void UDisTweaksBase::ApplyFallbackChain_Struct( const UStruct* Struct, const UProperty* OwnerProperty, const FString& CurPath, INT ParentOffset )
{
	for( TFieldIterator<UProperty> It( Struct ); It; ++It )
	{
		UProperty* Property = *It;
		const FString NewPath = CurPath.Len() ? CurPath + TEXT(".") + Property->GetName() : Property->GetName();
		if( Property->PropertyFlags & CPF_Transient )
		{
			continue;
		}
		const INT SkipIndex = FindFallbackSkip( NewPath );
		if( SkipIndex != INDEX_NONE )
		{
			m_FallbackSkip(SkipIndex).m_bProcessed = TRUE;
			continue;
		}
		UStructProperty* StructProperty = Cast<UStructProperty>( Property );
		const UBOOL bFallback = ( Property->PropertyFlags & CPF_DisTweakFallback ) == CPF_DisTweakFallback;
		if( StructProperty && !bFallback )
		{
			for( INT Element = 0; Element < Property->ArrayDim; Element++ )
			{
				ApplyFallbackChain_Struct( StructProperty->Struct, OwnerProperty ? OwnerProperty : Property, NewPath, ParentOffset + Property->Offset + Element * Property->ElementSize );
			}
			continue;
		}
		if( !bFallback )
		{
			continue;
		}
		const UProperty* Owner = OwnerProperty ? OwnerProperty : Property;
		UClass* OwnerClass = Owner->GetOwnerClass();
		if( OwnerClass )
		{
			UBOOL bOnChain = FALSE;
			for( UClass* Cur = m_pFallbackTweaks->GetClass(); Cur; Cur = Cur->GetSuperClass() )
			{
				if( Cur == OwnerClass )
				{
					bOnChain = TRUE;
					break;
				}
			}
			if( !bOnChain )
			{
				continue;
			}
		}
		Property->CopyCompleteValue( (BYTE*)this + ParentOffset + Property->Offset, (BYTE*)m_pFallbackTweaks + ParentOffset + Property->Offset );
	}
}

// DISHONORED(written): 2013 rva 0x882e70 (2012 0x8f0600, distweaksbase.cpp:1591): a save outside the cooker records the class
// version (default object's m_VersionNum + 1) and the versions of every UDisTweaksBase base class; loading reads nothing extra
void UDisTweaksBase::Serialize( FArchive& Ar )
{
	if( Ar.IsSaving() && !IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) && !GIsCooking )
	{
		m_VersionNum = GetClass()->GetDefaultObject<UDisTweaksBase>()->m_VersionNum + 1;
		m_BaseClassVersions.Reset();
		for( UClass* Base = GetClass()->GetSuperClass(); Base && Base->IsChildOf( UDisTweaksBase::StaticClass() ); Base = Base->GetSuperClass() )
		{
			FDisTweakVersionBaseClass* Version = new( m_BaseClassVersions ) FDisTweakVersionBaseClass(EC_EventParm);
			Version->m_BaseClassName = Base->GetName();
			Version->m_VersionNum = Base->GetDefaultObject<UDisTweaksBase>()->m_VersionNum + 1;
		}
	}
	Super::Serialize( Ar );
}

// DISHONORED(written): 2013 rva 0x882a70 (2012 0x8ea190): loaded tweaks are marked localized, and the instances (not the class
// defaults/archetypes) resolve their fallback chain
void UDisTweaksBase::PostLoad()
{
	Super::PostLoad();
	m_bBeenLocalized = TRUE;
	if( !IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
	{
		ApplyFallbackChain();
	}
}
