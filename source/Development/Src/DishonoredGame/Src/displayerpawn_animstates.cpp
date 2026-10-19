// DishonoredGame/src/displayerpawn_animstates.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x6fbae0  public: void __thiscall ADishonoredPlayerPawn::ClearAnimState(enum eAnimState_Picker, float)
//   0x6fbb00  public: unsigned int __thiscall ADishonoredPlayerPawn::IsCurAnimStateFullWeight(enum eAnimState_Picker)
//   0x6fbb10  public: class UAnimNode * __thiscall ADishonoredPlayerPawn::FindNodeInCurrentAnimStateByName(enum eAnimState_Picker, class FName const &)const
//   0x71b9b0  protected: void __thiscall ADishonoredPlayerPawn::BuildAnimStateMap(struct FDisPawnAnimState_PickerInfo &)
//   0x71ed50  protected: void __thiscall ADishonoredPlayerPawn::FindStatePickers(void)
//   0x71f740  public: unsigned int __thiscall ADishonoredPlayerPawn::FindAnimState(enum eAnimState_Picker, class FName, struct FDisPawnAnimState * &)
//   0x71f800  protected: struct FDisPawnAnimState * __thiscall ADishonoredPlayerPawn::AnimState_AddItemPrefix(struct FDisPawnAnimState_PickerInfo &, enum eAnimState_Picker, struct FDisPawnAnimState const *, class FName &, enum EDisEquipUsage)
//   0x71fe40  public: struct FDisPawnAnimState * __thiscall ADishonoredPlayerPawn::SetAnimState(class FName, enum eAnimState_Picker, class UDishonoredNativeState *, float, class UObject *)
//   0x71ff40  public: unsigned int __thiscall ADishonoredPlayerPawn::SupportsAnimState(class FName, enum eAnimState_Picker)
//   0x7200e0  public: void __thiscall ADishonoredPlayerPawn::PreBeginPlay_AnimStates(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the player's anim states (Inc/CppText/ADishonoredPlayerPawn.h, IM block). Three pickers - full body (no
	parent), upper body (child of the full body), special / left hand (child of the upper body) - each with the pawn's state arrays
	(ApplyTweakChanges_Derived copies them from the animation tweaks) mapped by name; SetAnimState looks the name up, adds the equipped
	item's prefix when the state asks for it (a prefixed copy is appended to the picker's first array once and remembered in the prefix
	list), and hands the state to the component; a full-body state also sets the root-motion modes.
	2013 bodies: FindStatePickers 0x6e0f90 (970 B, 2013 tests the mesh's anim tree before each FindAnimNode), the others as 2012.
	DISHONORED(written): retail asserts on a picker index out of range; the lookups here answer "not found" for a pawn whose pickers
	were never built (no tweaks).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_PlayerPawnAnimStates()
{
}

static inline UBOOL operator==( const FDisAnimStateMapKey_WithPrefix& A, const FDisAnimStateMapKey_WithPrefix& B )
{
	return A.m_PrefixName == B.m_PrefixName && A.m_StateName == B.m_StateName;
}

static inline DWORD GetTypeHash( const FDisAnimStateMapKey_WithPrefix& Key )
{
	return GetTypeHash( Key.m_PrefixName ) ^ ( GetTypeHash( Key.m_StateName ) << 1 );
}

static UDisTweaks_Pawn_Animation* DisPlayerAnimationTweaks( ADishonoredPlayerPawn* Pawn )
{
	UDisTweaks_Pawn* Tweaks = Cast<UDisTweaks_Pawn>( Pawn->GetTweaks_Derived() );
	if( Tweaks == NULL )
	{
		Tweaks = UDisTweaks_Pawn::StaticClass()->GetDefaultObject<UDisTweaks_Pawn>();
	}
	return Tweaks->m_pAnimationTweaks;
}

// 2012 0x704a00 (203 B; 2013 0x6aac90): the pawn's own state arrays behind each picker, after the tweak's misc array
void ADishonoredPlayerPawn::GatherAnimStateArrays( INT StateType, FDisPawnAnimState_PickerInfo& PickerInfo )
{
	if( StateType == ANIMSTATE_FULL_BODY )
	{
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_FullBody_Navigation );
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_FullBody_Assassination );
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_FullBody_Fatality );
	}
	else if( StateType == ANIMSTATE_UPPER_BODY )
	{
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_UpperBody_Items );
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_UpperBody_Items_Melee );
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_UpperBody_Items_Ranged );
	}
	else if( StateType == ANIMSTATE_SPECIAL )
	{
		PickerInfo.m_StateArrays.AddItem( &m_AnimStates_LeftHand_Items );
	}
}

// 2012 0x71b9b0 / 2013 0x6dfc70 (212 B): every state of every array, by name
void ADishonoredPlayerPawn::BuildAnimStateMap( FDisPawnAnimState_PickerInfo& PickerInfo )
{
	for( INT ArrayIndex = 0; ArrayIndex < PickerInfo.m_StateArrays.Num(); ArrayIndex++ )
	{
		TArray<FDisPawnAnimState>& States = *(TArray<FDisPawnAnimState>*)PickerInfo.m_StateArrays( ArrayIndex );
		for( INT StateIndex = 0; StateIndex < States.Num(); StateIndex++ )
		{
			PickerInfo.m_AnimStateMap.Set( States( StateIndex ).m_StateName, &States( StateIndex ) );
		}
	}
}

/**
 * 2013 0x6e0f90 (970 B). For each picker in order: its node by the tweaks' picker name in the mesh's tree, its parent (full body:
 * none; upper body: the full body; special: the upper body), its per-bone filter by the tweaks' filter name, its state arrays (the
 * tweak misc array, copied onto the pawn, first) and their name map; a picker without tweaks gets no node and no arrays.
 */
void ADishonoredPlayerPawn::FindStatePickers()
{
	UDisTweaks_Pawn_Animation* AnimTweaks = DisPlayerAnimationTweaks( this );
	m_AnimStatePickerInfo.Empty();
	UAnimNode* Tree = Mesh ? Mesh->Animations : NULL;
	for( INT StateType = 0; StateType < 3; StateType++ )
	{
		FDisPawnAnimState_PickerInfo NewPickerInfo;
		appMemzero( &NewPickerInfo.m_StateArrays, sizeof(NewPickerInfo.m_StateArrays) );
		FDisAnimState_Picker NewPicker;
		appMemzero( &NewPicker, sizeof(FDisAnimState_Picker) );
		BYTE ParentPicker = 3;
		UAnimNodeBlendPerBone* Filter = NULL;
		FName PickerName = NAME_None;
		if( StateType == ANIMSTATE_UPPER_BODY )
		{
			UDisTweaks_Pawn_UpperBodyAnimation* Upper = AnimTweaks ? AnimTweaks->m_pUpperBodyTweaks : NULL;
			if( Upper )
			{
				PickerName = Upper->m_UpperBody_StatePicker_Name;
				ParentPicker = ANIMSTATE_FULL_BODY;
				if( Upper->m_UpperBody_Filter_Name != NAME_None && Tree )
				{
					Filter = Cast<UAnimNodeBlendPerBone>( Tree->FindAnimNode( Upper->m_UpperBody_Filter_Name ) );
				}
				NewPickerInfo.m_StateArrays.AddItem( &m_AnimStates_UpperBody_Misc );
			}
		}
		else if( StateType == ANIMSTATE_SPECIAL )
		{
			UDisTweaks_Pawn_SpecialAnimation* Special = AnimTweaks ? AnimTweaks->m_pSpecialTweaks : NULL;
			if( Special )
			{
				PickerName = Special->m_Special_StatePicker_Name;
				ParentPicker = ANIMSTATE_UPPER_BODY;
				if( Special->m_Special_Filter_Name != NAME_None && Tree )
				{
					Filter = Cast<UAnimNodeBlendPerBone>( Tree->FindAnimNode( Special->m_Special_Filter_Name ) );
				}
				NewPickerInfo.m_StateArrays.AddItem( &m_AnimStates_Special_Misc );
			}
		}
		else
		{
			UDisTweaks_Pawn_FullBodyAnimation* FullBody = AnimTweaks ? AnimTweaks->m_pFullBodyTweaks : NULL;
			if( FullBody )
			{
				PickerName = FullBody->m_FullBody_StatePicker_Name;
				NewPickerInfo.m_StateArrays.AddItem( &m_AnimStates_FullBody_Misc );
			}
		}
		if( PickerName != NAME_None )
		{
			NewPicker.m_pStatePicker = Tree ? Cast<UDishonoredAnimNodeStatePicker>( Tree->FindAnimNode( PickerName ) ) : NULL;
			NewPicker.m_parentPicker = ParentPicker;
			NewPicker.m_pFilter = Filter;
			NewPicker.m_pCurState = NULL;
			GatherAnimStateArrays( StateType, NewPickerInfo );
			BuildAnimStateMap( NewPickerInfo );
		}
		else
		{
			NewPicker.m_parentPicker = 3;
			NewPickerInfo.m_StateArrays.Empty();
		}
		if( m_pAnimStateComp )
		{
			m_pAnimStateComp->AddAnimStatePicker( NewPicker );
		}
		new( m_AnimStatePickerInfo ) FDisPawnAnimState_PickerInfo( NewPickerInfo );
	}
}

/**
 * 2013 0x6e2060 (372 B). The pickers are found, the component is bound to this pawn as its owner, and each picker starts in the
 * tweaks' default state at once (the upper and special pickers are cleared when there is none).
 */
void ADishonoredPlayerPawn::PreBeginPlay_AnimStates()
{
	UDisTweaks_Pawn_Animation* AnimTweaks = DisPlayerAnimationTweaks( this );
	FindStatePickers();
	if( m_pAnimStateComp == NULL )
	{
		IMDIAG( TEXT("init: %s has no m_pAnimStateComp; the anim states cannot be set"), *GetName() );
		return;
	}
	TScriptInterface<IDisAnimStateOwnerInterface> Owner;
	Owner = this;
	m_pAnimStateComp->BindToAnimStateOwner( Owner );
	UDisTweaks_Pawn_FullBodyAnimation* FullBody = AnimTweaks ? AnimTweaks->m_pFullBodyTweaks : NULL;
	if( FullBody && FullBody->m_DefaultAnimState_FullBody != NAME_None )
	{
		SetAnimState( FullBody->m_DefaultAnimState_FullBody, ANIMSTATE_FULL_BODY, NULL, 0.f, NULL );
	}
	UDisTweaks_Pawn_UpperBodyAnimation* Upper = AnimTweaks ? AnimTweaks->m_pUpperBodyTweaks : NULL;
	if( Upper && Upper->m_DefaultAnimState_UpperBody != NAME_None )
	{
		SetAnimState( Upper->m_DefaultAnimState_UpperBody, ANIMSTATE_UPPER_BODY, NULL, 0.f, NULL );
	}
	else
	{
		m_pAnimStateComp->ClearAnimState( ANIMSTATE_UPPER_BODY, 0.f );
	}
	UDisTweaks_Pawn_SpecialAnimation* Special = AnimTweaks ? AnimTweaks->m_pSpecialTweaks : NULL;
	if( Special && Special->m_DefaultAnimState_Special != NAME_None )
	{
		SetAnimState( Special->m_DefaultAnimState_Special, ANIMSTATE_SPECIAL, NULL, 0.f, NULL );
	}
	else
	{
		m_pAnimStateComp->ClearAnimState( ANIMSTATE_SPECIAL, 0.f );
	}
}

// 2012 0x71f740 / 2013 0x6e16c0 (188 B; the editor's temporary picker refresh is not written - GIsGame)
UBOOL ADishonoredPlayerPawn::FindAnimState( INT StateType, FName StateName, FDisPawnAnimState*& OutState )
{
	if( StateType < 0 || StateType >= m_AnimStatePickerInfo.Num() )
	{
		return FALSE;
	}
	FDisPawnAnimState** Found = m_AnimStatePickerInfo( StateType ).m_AnimStateMap.Find( StateName );
	if( Found == NULL )
	{
		return FALSE;
	}
	OutState = *Found;
	return TRUE;
}

/**
 * 2012 0x71f800 / 2013 0x6e1780 (1216 B). The prefixed copy of a state for the item in the given hand (the primary for none): found in
 * the picker's prefix list, or made once - named <prefix><state>, every custom sequence renamed the same way, its prefix flag off -
 * appended to the picker's first state array, the map rebuilt (the append may move the array) and the picker's current state
 * re-found by name. The StateName out-parameter becomes the prefixed name. No inventory or no item: none.
 */
FDisPawnAnimState* ADishonoredPlayerPawn::AnimState_AddItemPrefix( FDisPawnAnimState_PickerInfo& PickerInfo, INT StateType, const FDisPawnAnimState* State, FName& StateName, BYTE EquipUsage )
{
	if( m_pInventory == NULL )
	{
		return NULL;
	}
	UDishonoredInventoryItem* Item = m_pInventory->GetEquippedItem( EquipUsage == EDisEquipUsage_None ? (BYTE)EDisEquipUsage_Primary : EquipUsage );
	if( Item == NULL )
	{
		return NULL;
	}
	TArray<FDisPawnAnimState>& FirstArray = *(TArray<FDisPawnAnimState>*)PickerInfo.m_StateArrays( 0 );
	FDisAnimStateMapKey_WithPrefix Key;
	Key.m_PrefixName = Item->GetItemPrefix();
	Key.m_StateName = State->m_StateName;
	INT* FoundIndex = PickerInfo.m_AnimStateMap_PrefixList.Find( Key );
	if( FoundIndex )
	{
		FDisPawnAnimState* Prefixed = &FirstArray( *FoundIndex );
		StateName = Prefixed->m_StateName;
		return Prefixed;
	}
	const FName IncomingStateName = State->m_StateName;
	const FString Prefix = Item->GetItemPrefix().ToString();
	StateName = FName( *( Prefix + IncomingStateName.ToString() ) );
	FDisPawnAnimState NewState = *State;
	NewState.m_StateName = StateName;
	NewState.m_bAddItemPrefix = FALSE;
	for( INT SeqIndex = 0; SeqIndex < NewState.m_CustomSequences.Num(); SeqIndex++ )
	{
		FDisAnimState_CustomSeq& CustomSeq = NewState.m_CustomSequences( SeqIndex );
		CustomSeq.m_Sequence = FName( *( Prefix + CustomSeq.m_Sequence.ToString() ) );
	}
	FDisAnimState_Picker* Picker = m_pAnimStateComp ? m_pAnimStateComp->GetAnimStatePicker( StateType ) : NULL;
	const FName CurStateName = ( Picker && Picker->m_pCurState ) ? ((FDisPawnAnimState*)Picker->m_pCurState)->m_StateName : FName( NAME_None );
	const INT NewIndex = FirstArray.AddItem( NewState );
	BuildAnimStateMap( PickerInfo );
	Key.m_StateName = IncomingStateName;
	PickerInfo.m_AnimStateMap_PrefixList.Set( Key, NewIndex );
	if( CurStateName != NAME_None )
	{
		FDisPawnAnimState* CurState = NULL;
		FindAnimState( StateType, CurStateName, CurState );
		Picker->m_pCurState = CurState;
	}
	return &FirstArray( NewIndex );
}

/**
 * 2012 0x71fe40 / 2013 0x6e1da0 (251 B). The named state of the picker (item-prefixed when it asks), set on the component; a full-body
 * success also sets the pawn's root-motion modes from the state. Success and Failed_AlreadySet answer the state, anything else NULL.
 */
FDisPawnAnimState* ADishonoredPlayerPawn::SetAnimState( FName StateName, INT StateType, UDishonoredNativeState* AttachToState, FLOAT BlendTime, UObject* FiringObj )
{
	const FName Requested = StateName;
	if( StateType < 0 || StateType >= m_AnimStatePickerInfo.Num() || m_pAnimStateComp == NULL )
	{
		DisIMNoteSetAnimState( this, StateType, Requested, NULL, -1, AttachToState );
		return NULL;
	}
	FDisPawnAnimState_PickerInfo& PickerInfo = m_AnimStatePickerInfo( StateType );
	FDisPawnAnimState** Found = PickerInfo.m_AnimStateMap.Find( StateName );
	FDisPawnAnimState* State = Found ? *Found : NULL;
	if( State && State->m_bAddItemPrefix )
	{
		State = AnimState_AddItemPrefix( PickerInfo, StateType, State, StateName, State->m_ApplyItemEquipPrefix );
	}
	if( State == NULL )
	{
		DisIMNoteSetAnimState( this, StateType, Requested, NULL, -1, AttachToState );
		return NULL;
	}
	const BYTE Status = m_pAnimStateComp->SetAnimState( State, StateType, AttachToState, BlendTime, FiringObj );
	DisIMNoteSetAnimState( this, StateType, Requested, State, Status, AttachToState );
	if( Status == eSetAnimStateStatus_Success )
	{
		if( StateType == ANIMSTATE_FULL_BODY )
		{
			ChangeAnimStateMotionModes( State->m_RootMotionMode, State->m_RootMotionRotationMode );
		}
		return State;
	}
	return Status == eSetAnimStateStatus_Failed_AlreadySet ? State : NULL;
}

// 2012 0x71ff40 / 2013 0x6e1ea0 (188 B)
UBOOL ADishonoredPlayerPawn::SupportsAnimState( FName StateName, INT StateType )
{
	if( StateType < 0 || StateType >= m_AnimStatePickerInfo.Num() )
	{
		return FALSE;
	}
	FDisPawnAnimState_PickerInfo& PickerInfo = m_AnimStatePickerInfo( StateType );
	FDisPawnAnimState** Found = PickerInfo.m_AnimStateMap.Find( StateName );
	FDisPawnAnimState* State = Found ? *Found : NULL;
	if( State && State->m_bAddItemPrefix )
	{
		State = AnimState_AddItemPrefix( PickerInfo, StateType, State, StateName, State->m_ApplyItemEquipPrefix );
	}
	return State != NULL;
}

// 2012 0x6fbae0 (29 B)
void ADishonoredPlayerPawn::ClearAnimState( INT StateType, FLOAT BlendTime )
{
	DisIMNoteClearAnimState( this, StateType, BlendTime );
	if( m_pAnimStateComp )
	{
		m_pAnimStateComp->ClearAnimState( StateType, BlendTime );
	}
}

// 2012 0x6fbb00 (15 B)
UBOOL ADishonoredPlayerPawn::IsCurAnimStateFullWeight( INT StateType )
{
	return m_pAnimStateComp ? m_pAnimStateComp->IsCurAnimStateFullWeight( StateType ) : TRUE;
}

// 2012 0x6fbb10 (15 B)
UAnimNode* ADishonoredPlayerPawn::FindNodeInCurrentAnimStateByName( INT StateType, const FName& NodeName ) const
{
	return m_pAnimStateComp ? m_pAnimStateComp->FindNodeInCurrentAnimStateByName( StateType, NodeName ) : NULL;
}
