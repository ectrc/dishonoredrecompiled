// DishonoredGame/src/disanimstatecomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (18):
//   0x6b1a90  public: static void __cdecl UDisAnimStateComponent::InitializePrivateStaticClassUDisAnimStateComponent(void)
//   0x6b1ab0  public: void __thiscall UDisAnimStateComponent::BindToAnimStateOwner(class TScriptInterface<class IDisAnimStateOwnerInterface> const &)
//   0x6bd8a0  public: void __thiscall UDisAnimStateComponent::InvalidateCurAnimState(int)
//   0x6bd920  public: struct FDisAnimState const * __thiscall UDisAnimStateComponent::GetCurAnimState(int)const
//   0x6bd970  public: void __thiscall UDisAnimStateComponent::AddAnimStatePicker(struct FDisAnimState_Picker const &)
//   0x6bd980  public: struct FDisAnimState_Picker * __thiscall UDisAnimStateComponent::GetAnimStatePicker(enum eAnimState_Picker)
//   0x6bd9e0  public: unsigned int __thiscall UDisAnimStateComponent::IsCurAnimStateFullWeight(int)
//   0x6bda90  public: class UAnimNode * __thiscall UDisAnimStateComponent::FindNodeInCurrentAnimStateByName(int, class FName const &)const
//   0x6bdb10  public: void __thiscall UDisAnimStateComponent::LogSeqWithHistory(int, class FName const &)
//   0x6c0a80  private: void __thiscall UDisAnimStateComponent::LockoutChildPickers(struct FDisAnimState_Picker &, unsigned int, float)
//   0x6c0c20  public: void __thiscall UDisAnimStateComponent::ClearAnimState(int, float)
//   0x6c3350  public: void __thiscall UDisAnimStateComponent::CleanupAnimStates(unsigned int)
//   0x6c33d0  public: void __thiscall UDisAnimStateComponent::LockPickerChildren(int, float)
//   0x6c3430  public: void __thiscall UDisAnimStateComponent::UnlockPickerChildren(int, float)
//   0x6c3490  public: void __thiscall UDisAnimStateComponent::UnlockPicker(int, float)
//   0x6cc0d0  public: enum eSetAnimStateStatus __thiscall UDisAnimStateComponent::SetAnimState(struct FDisAnimState *, int, class UDishonoredNativeState *, float, class UObject *)
//   0x6cea80  public: static class UClass * __cdecl UDisAnimStateComponent::GetPrivateStaticClassUDisAnimStateComponent(wchar_t const *)
//   0x6cfb90  public: static class UClass * __cdecl UDisAnimStateComponent::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the anim-state component (Inc/CppText/UDisAnimStateComponent.h). 2012 bodies; every 2013 match has the
	same size (match_2012_2013.csv) and SetAnimState's 2013 body (0x69f610, 1125 B) was read against the 2012 one: same flow.
	FDisAnimState_Picker's flag word: bit 0 m_bCurStatePotentiallyInvalid, bit 1 m_bLockedOut, bit 2 m_bLockedOut_Children.
	The engine has no UAnimTree::FindAnimNodeFast (retail's hashed lookup); UAnimNode::FindAnimNode answers the same node.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_DisAnimStateComponent()
{
}

static inline UBOOL operator==( const FDisCustomSeqMapEntry& A, const FDisCustomSeqMapEntry& B )
{
	return A.m_pAnimTree == B.m_pAnimTree && A.m_SeqAlias == B.m_SeqAlias;
}

static inline DWORD GetTypeHash( const FDisCustomSeqMapEntry& Key )
{
	return PointerHash( Key.m_pAnimTree ) ^ GetTypeHash( Key.m_SeqAlias );
}

// the dominant tree's anim-state bookkeeping reset (inlined at 2012 0x6bd8a0 and 0x6c0c20)
static void DisEndDominantTreeAnimState( UDishonoredAnimNodeStatePicker* Picker )
{
	UDishonoredAnimTree* DominantTree = Picker ? Picker->GetDominantTree() : NULL;
	if( DominantTree )
	{
		DominantTree->EndAnimState();
	}
}

// 2012 0x6bd8a0 / 2013 0x691680 (128 B)
void UDisAnimStateComponent::InvalidateCurAnimState( INT StateType )
{
	FDisAnimState_Picker& Picker = m_StatePickers( StateType );
	DisEndDominantTreeAnimState( Picker.m_pStatePicker );
	Picker.m_bCurStatePotentiallyInvalid = TRUE;
}

// 2012 0x6bd920 / 2013 0x688870 (73 B)
const FDisAnimState* UDisAnimStateComponent::GetCurAnimState( INT StateType ) const
{
	return (const FDisAnimState*)m_StatePickers( StateType ).m_pCurState;
}

// 2012 0x6bd980 / 2013 0x688900 (85 B)
FDisAnimState_Picker* UDisAnimStateComponent::GetAnimStatePicker( INT StateType )
{
	if( StateType == ANIMSTATE_MAX )
	{
		return NULL;
	}
	return &m_StatePickers( StateType );
}

// 2012 0x6bd9e0 / 2013 0x691700 (172 B): the filter's second child at full weight, and the picker's state too
UBOOL UDisAnimStateComponent::IsCurAnimStateFullWeight( INT StateType )
{
	FDisAnimState_Picker& Picker = m_StatePickers( StateType );
	if( Picker.m_pFilter && Picker.m_pFilter->Children( 1 ).Weight < 0.99998999f )
	{
		return FALSE;
	}
	return Picker.m_pStatePicker ? Picker.m_pStatePicker->IsStateFullWeight() : TRUE;
}

// 2012 0x6bda90 / 2013 0x6917b0 (126 B)
UAnimNode* UDisAnimStateComponent::FindNodeInCurrentAnimStateByName( INT StateType, const FName& NodeName ) const
{
	if( m_StatePickers.Num() == 0 )
	{
		return NULL;
	}
	const FDisAnimState_Picker& Picker = m_StatePickers( StateType );
	if( Picker.m_pStatePicker && Picker.m_pCurState )
	{
		UDishonoredAnimTree* DominantTree = Picker.m_pStatePicker->GetDominantTree();
		if( DominantTree )
		{
			return DominantTree->FindAnimNode( NodeName );
		}
	}
	return NULL;
}

// 2012 0x6bdb10 / 2013 0x688960 (142 B): only the sequences of a non-looping state are remembered
void UDisAnimStateComponent::LogSeqWithHistory( INT StateType, const FName& SeqName )
{
	const FDisAnimState* CurState = (const FDisAnimState*)m_StatePickers( StateType ).m_pCurState;
	if( CurState && CurState->m_bNonLooping )
	{
		if( ++m_iCurNewestSeqInHistory >= 12 )
		{
			m_iCurNewestSeqInHistory = 0;
		}
		m_AnimSeqHistory[m_iCurNewestSeqInHistory].m_StatePicker = StateType;
		m_AnimSeqHistory[m_iCurNewestSeqInHistory].m_SeqName = SeqName;
	}
}

/**
 * 2012 0x6c0a80 / 2013 0x691410 (416 B). The picker's children lockout follows bLocked; every other picker whose parent chain reaches
 * this one (and, when unlocking, has no other ancestor still locking its children) is locked out or let in through the owner.
 */
void UDisAnimStateComponent::LockoutChildPickers( FDisAnimState_Picker& CurPicker, UBOOL bLocked, FLOAT BlendTime )
{
	if( !bLocked && CurPicker.m_bLockedOut )
	{
		return;
	}
	CurPicker.m_bLockedOut_Children = bLocked ? TRUE : FALSE;
	const INT NumPickers = m_StatePickers.Num();
	for( INT PickerIndex = 0; PickerIndex < NumPickers; PickerIndex++ )
	{
		FDisAnimState_Picker& OtherPicker = m_StatePickers( PickerIndex );
		if( &OtherPicker == &CurPicker )
		{
			continue;
		}
		FDisAnimState_Picker* Parent = OtherPicker.m_parentPicker == 3 ? NULL : &m_StatePickers( OtherPicker.m_parentPicker );
		if( Parent == NULL )
		{
			continue;
		}
		UBOOL bIsChild = FALSE;
		UBOOL bParentLocksChildren = FALSE;
		while( Parent )
		{
			if( Parent == &CurPicker )
			{
				bIsChild = TRUE;
			}
			if( Parent->m_bLockedOut_Children )
			{
				bParentLocksChildren = TRUE;
			}
			if( Parent->m_parentPicker == 3 )
			{
				break;
			}
			Parent = &m_StatePickers( Parent->m_parentPicker );
		}
		if( bIsChild && ( bLocked || !bParentLocksChildren ) && (UBOOL)OtherPicker.m_bLockedOut != bLocked )
		{
			OtherPicker.m_bLockedOut_Children = FALSE;
			IDisAnimStateOwnerInterface* Owner = GetOwnerInterface();
			if( Owner )
			{
				Owner->AnimState_LockoutPicker( PickerIndex, BlendTime, bLocked );
			}
		}
	}
}

// 2012 0x6c0c20 / 2013 0x6915b0 (196 B): the dominant tree leaves its state, the filter blends out, the children are let in
void UDisAnimStateComponent::ClearAnimState( INT StateType, FLOAT BlendTime )
{
	if( StateType < 0 || StateType >= m_StatePickers.Num() )
	{
		return;
	}
	FDisAnimState_Picker& Picker = m_StatePickers( StateType );
	DisEndDominantTreeAnimState( Picker.m_pStatePicker );
	if( Picker.m_pFilter )
	{
		Picker.m_pFilter->SetBlendTarget( 0.f, BlendTime );
	}
	LockoutChildPickers( Picker, FALSE, 0.f );
	Picker.m_bCurStatePotentiallyInvalid = FALSE;
	Picker.m_pCurState = NULL;
}

// 2012 0x6c3350 / 2013 0x69d850 (114 B)
void UDisAnimStateComponent::CleanupAnimStates( UBOOL bFullDestroy )
{
	const INT NumPickers = m_StatePickers.Num();
	for( INT PickerIndex = 0; PickerIndex < NumPickers; PickerIndex++ )
	{
		if( m_StatePickers( PickerIndex ).m_pStatePicker )
		{
			m_StatePickers( PickerIndex ).m_pStatePicker->CleanupForAnimStates( bFullDestroy );
		}
		ClearAnimState( PickerIndex, 0.f );
	}
}

// 2012 0x6c33d0 (89 B)
void UDisAnimStateComponent::LockPickerChildren( INT StateType, FLOAT BlendTime )
{
	LockoutChildPickers( m_StatePickers( StateType ), TRUE, BlendTime );
}

// 2012 0x6c3430 (89 B)
void UDisAnimStateComponent::UnlockPickerChildren( INT StateType, FLOAT BlendTime )
{
	LockoutChildPickers( m_StatePickers( StateType ), FALSE, BlendTime );
}

// 2012 0x6c3490 / 2013 0x69a130 (104 B)
void UDisAnimStateComponent::UnlockPicker( INT StateType, FLOAT BlendTime )
{
	if( m_StatePickers( StateType ).m_bLockedOut )
	{
		IDisAnimStateOwnerInterface* Owner = GetOwnerInterface();
		if( Owner )
		{
			Owner->AnimState_LockoutPicker( StateType, BlendTime, FALSE );
		}
	}
}

/**
 * 2012 0x6cc0d0 / 2013 0x69f610 (1126 / 1125 B). A locked-out picker refuses (Failed_LockedOut); the state already held, active and
 * looping, and not invalidated, answers Failed_AlreadySet. The blend time is the smaller of the asked one and the state's own (the
 * state's when none is asked); a picker coming from nothing snaps. The picker's slot takes the state's tree; the old dominant tree
 * leaves its state and the new one records the picker, the firing object and the attached state. Each custom sequence alias is
 * found in the new tree (cached per tree in m_AliasNodeMap), given its sequence and played - through the firing object or the
 * owner, else directly - and a sequence the mesh does not have is reported as not found. The filter blends in; a state that blocks
 * its children locks them out, any other lets them in. DISHONORED(written): the status is the eSetAnimStateStatus byte.
 */
BYTE UDisAnimStateComponent::SetAnimState( FDisAnimState* State, INT StateType, UDishonoredNativeState* AttachToState, FLOAT BlendTime, UObject* FiringObj )
{
	FDisAnimState_Picker& Picker = m_StatePickers( StateType );
	if( Picker.m_bLockedOut )
	{
		return eSetAnimStateStatus_Failed_LockedOut;
	}
	if( Picker.m_pStatePicker == NULL )
	{
		return eSetAnimStateStatus_Failed;
	}
	if( State == (FDisAnimState*)Picker.m_pCurState && Picker.m_pStatePicker->HasActiveState() && !State->m_bNonLooping && !Picker.m_bCurStatePotentiallyInvalid )
	{
		return eSetAnimStateStatus_Failed_AlreadySet;
	}
	if( BlendTime < 0.f || State->m_fBlendTime < BlendTime )
	{
		BlendTime = State->m_fBlendTime;
	}
	FLOAT StateBlendTime = BlendTime;
	if( Picker.m_pCurState == NULL && !Picker.m_pStatePicker->HasActiveState() )
	{
		StateBlendTime = 0.f;
	}
	UDishonoredAnimTree* OldDominantTree = Picker.m_pStatePicker->GetDominantTree();
	UDishonoredAnimTree* NewTree = StateBlendTime < 0.0000099999997f
		? Picker.m_pStatePicker->SetState( State->m_StateName, State->m_AnimStateTree, State->m_bNonLooping )
		: Picker.m_pStatePicker->SetState( State->m_StateName, State->m_AnimStateTree, State->m_bNonLooping, 1.f / StateBlendTime );
	if( NewTree == NULL )
	{
		return eSetAnimStateStatus_Failed;
	}
	IDisAnimStateFiringInterface* FiringInterface = FiringObj ? IDisAnimStateFiringInterface::FromObject( FiringObj ) : NULL;
	if( OldDominantTree )
	{
		OldDominantTree->EndAnimState();
	}
	Picker.m_bCurStatePotentiallyInvalid = FALSE;
	Picker.m_pCurState = State;
	NewTree->m_AnimStateInfo.m_bIsAnimStateActive = TRUE;
	NewTree->m_AnimStateInfo.m_pAnimState_FiredFromObj = FiringObj;
	NewTree->m_AnimStateInfo.m_AnimStatePicker = StateType;
	NewTree->m_AnimStateInfo.m_pAnimState_AttachToState = AttachToState;
	IDisAnimStateOwnerInterface* Owner = GetOwnerInterface();
	for( INT SeqIndex = 0; SeqIndex < State->m_CustomSequences.Num(); SeqIndex++ )
	{
		const FDisAnimState_CustomSeq& CustomSeq = State->m_CustomSequences( SeqIndex );
		FDisCustomSeqMapEntry Key;
		Key.m_pAnimTree = NewTree;
		Key.m_SeqAlias = CustomSeq.m_Alias;
		UAnimNodeSequence* SeqNode = m_AliasNodeMap.FindRef( Key );
		if( SeqNode == NULL )
		{
			SeqNode = Cast<UAnimNodeSequence>( NewTree->FindAnimNode( CustomSeq.m_Alias ) );
			m_AliasNodeMap.Set( Key, SeqNode );
			if( SeqNode == NULL )
			{
				continue;
			}
		}
		SeqNode->SetAnim( CustomSeq.m_Sequence );
		if( !Owner || !Owner->AnimState_PlayAnim( FiringInterface, SeqNode ) )
		{
			SeqNode->PlayAnim( SeqNode->bLooping, SeqNode->Rate, 0.f );
		}
		USkeletalMeshComponent* SkelComp = Owner ? Owner->AnimState_GetSkeletalComponent() : NULL;
		if( SeqNode->AnimSeqName != NAME_None )
		{
			if( SkelComp && SkelComp->FindAnimSequence( SeqNode->AnimSeqName ) )
			{
				continue;
			}
			if( Owner )
			{
				Owner->AnimState_AnimNotFound( FiringInterface, SeqNode->AnimSeqName.ToString() );
			}
		}
		else if( Owner )
		{
			Owner->AnimState_AnimNotFound( FiringInterface, CustomSeq.m_Sequence.ToString() );
		}
	}
	if( Picker.m_pFilter )
	{
		Picker.m_pFilter->SetBlendTarget( 1.f, BlendTime );
	}
	if( State->m_bBlocksChildren )
	{
		LockoutChildPickers( Picker, TRUE, BlendTime );
	}
	else
	{
		LockoutChildPickers( Picker, FALSE, 0.f );
	}
	return eSetAnimStateStatus_Success;
}
