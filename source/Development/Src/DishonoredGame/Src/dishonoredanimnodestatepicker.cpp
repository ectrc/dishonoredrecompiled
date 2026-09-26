// DishonoredGame/src/dishonoredanimnodestatepicker.cpp
// The Arkane state picker: a blend base whose children are UDishonoredAnimNodeTreeRef_Dynamic slots. An anim state
// claims a slot (SetState), blends in at m_CurBlendSpeed and the other slots are scaled down to fill the remainder.
// With no state claimed every child weight is 0, which is why GetBoneAtoms answers the reference pose instead of
// entering UAnimNodeBlendBase::GetBoneAtoms (which requires the weights to sum to 1).
// PDB functions attributed to this file (21), 2012 rvas:
//   0x6cff90  UDishonoredAnimNodeStatePicker::InitializePrivateStaticClass      (generated)
//   0x6cffb0  UDishonoredAnimNodeStatePicker::BuildEdgeAnimTree                 (not ported: the whole-tree Edge path does not exist here, agent W follow-up 5)
//   0x6d0010  UDishonoredAnimNodeStatePicker::HasActiveState                    (inline in CppText)
//   0x6d49f0  UDishonoredAnimNodeStatePicker::GetBoneAtoms
//   0x6d9190  UDishonoredAnimNodeStatePicker::ClearState
//   0x6d9210  UDishonoredAnimNodeStatePicker::OnCeaseRelevant
//   0x6d9290  UDishonoredAnimNodeStatePicker::GetDominantTree
//   0x6d9300  UDishonoredAnimNodeStatePicker::IsStateFullWeight
//   0x6e47d0  UDishonoredAnimNodeStatePicker::RenameChildren
//   0x6ed460  UDishonoredAnimNodeStatePicker::GetChildBoneAtoms                 (not ported, see the warn-once below)
//   0x6eed40  UDishonoredAnimNodeStatePicker::CleanupForAnimStates
//   0x6f29a0  UDishonoredAnimNodeStatePicker::InitStateSlots
//   0x6f2ab0  UDishonoredAnimNodeStatePicker::InitAnim
//   0x6f2ae0  UDishonoredAnimNodeStatePicker::TickAnim
//   0x6f2ee0  UDishonoredAnimNodeStatePicker::OnRemoveChild
//   0x6f2f10  UDishonoredAnimNodeStatePicker::SetState (3 args)
//   0x6f6bc0  UDishonoredAnimNodeStatePicker::SetState (4 args)

#include "DishonoredGame.h"

// DISHONORED(bringup): -disanimstate[=<TreeName>] drives the pickers from the command line, because the retail driver
// (ADishonoredPawn::PreBeginPlay -> ADishonoredPlayerPawn::PreBeginPlay_AnimStates -> UDisAnimStateComponent::SetAnimState)
// is not ported. Not a retail code path.
static FName DisBringupAnimStateName();

static UBOOL DisBringupAnimStateEnabled()
{
	static UBOOL bEnabled = ParseParam( appCmdLine(), TEXT("disanimstate") ) || DisBringupAnimStateName() != NAME_None;
	return bEnabled;
}

static FName DisBringupAnimStateName()
{
	static UBOOL bParsed = FALSE;
	static FName Wanted = NAME_None;
	if( !bParsed )
	{
		bParsed = TRUE;
		FString Value;
		if( Parse( appCmdLine(), TEXT("disanimstate="), Value ) && Value.Len() > 0 )
		{
			Wanted = FName( *Value );
		}
	}
	return Wanted;
}


// DISHONORED(written): 2013 rva 0x69e660 (2012 0x69e660 -> 0x6f29a0): one slot per child; each child TreeRef_Dynamic
// receives the picker's own template list, so any state tree of the picker can be instanced in any slot.
void UDishonoredAnimNodeStatePicker::InitStateSlots()
{
	if( m_StateSlots.Num() >= Children.Num() )
	{
		return;
	}

	if( DisBringupAnimStateEnabled() )
	{
		for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
		{
			const FDynamicTreeTemplate& Template = m_PossibleTemplates(TemplateIdx);
			debugf( TEXT("DISHONORED(bringup): picker '%s' template[%d] %s tree %s pooled=%d"), *NodeName.ToString(), TemplateIdx,
				*Template.m_AnimTree_Name.ToString(),
				Template.m_pAnimTree_Template ? *Template.m_pAnimTree_Template->GetName() : TEXT("NULL"),
				(INT)Template.m_bUseAnimTreePooling );
		}
	}
	m_StateSlots.Add( Children.Num() - m_StateSlots.Num() );
	for( INT SlotIdx = 0; SlotIdx < m_StateSlots.Num(); SlotIdx++ )
	{
		FStateSlotInfo& Slot = m_StateSlots(SlotIdx);
		Slot.m_bActive = FALSE;
		Slot.m_pTree = NULL;
		// DISHONORED(bringup): retail stores Children(i).Anim without a type check; a Cast keeps a malformed tree from
		// writing through a non-TreeRef node
		UDishonoredAnimNodeTreeRef_Dynamic* TreeRef = Cast<UDishonoredAnimNodeTreeRef_Dynamic>( Children(SlotIdx).Anim );
		if( TreeRef )
		{
			Slot.m_pTreeRef = TreeRef;
			TreeRef->m_PossibleTemplates = m_PossibleTemplates;
		}
	}
	m_CurStateSlot = NAME_None;
}

// DISHONORED(written): 2012 rva 0x6e47d0 (unnamed in the 2013 db): the children are the designer-visible state slots
void UDishonoredAnimNodeStatePicker::RenameChildren()
{
	for( INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++ )
	{
		Children(ChildIdx).Name = FName( *FString::Printf( TEXT("StateTree%d"), ChildIdx + 1 ) );
	}
}

// DISHONORED(written): 2013 rva 0x69e770 (2012 0x6f2ab0)
void UDishonoredAnimNodeStatePicker::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim( MeshComp, Parent );
	InitStateSlots();
}

// DISHONORED(written): 2012 rva 0x6f2ee0 (unnamed in the 2013 db): retail calls OnAddChild here, not OnRemoveChild
void UDishonoredAnimNodeStatePicker::OnRemoveChild( INT ChildNum )
{
	Super::OnAddChild( ChildNum );
	RenameChildren();
	InitStateSlots();
}

// DISHONORED(written): 2012 rva 0x6f4c30 (unnamed in the 2013 db): an instanced picker starts with no slots
void UDishonoredAnimNodeStatePicker::PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap )
{
	m_StateSlots.Empty();
	InitStateSlots();
}

// DISHONORED(written): 2013 rva 0x689cf0 (2012 0x6d9190)
void UDishonoredAnimNodeStatePicker::ClearState()
{
	if( m_iCurStateSlot != INDEX_NONE )
	{
		m_StateSlots(m_iCurStateSlot).m_bActive = FALSE;
		m_CurStateSlot = NAME_None;
		m_iCurStateSlot = INDEX_NONE;
		m_CurBlendSpeed = 0.f;
	}
}

// DISHONORED(written): 2013 rva 0x689d70 (2012 0x6d9210)
void UDishonoredAnimNodeStatePicker::OnCeaseRelevant()
{
	UAnimNode::OnCeaseRelevant();

	for( INT SlotIdx = 0; SlotIdx < m_StateSlots.Num(); SlotIdx++ )
	{
		FStateSlotInfo& Slot = m_StateSlots(SlotIdx);
		if( Slot.m_bActive )
		{
			Slot.m_bActive = FALSE;
			Slot.m_pTree = NULL;
		}
	}
	ClearState();
}

// DISHONORED(written): 2013 rva 0x689e50 (2012 0x6d9300)
UBOOL UDishonoredAnimNodeStatePicker::IsStateFullWeight() const
{
	if( m_iCurStateSlot == INDEX_NONE )
	{
		return FALSE;
	}
	return Children(m_iCurStateSlot).Weight >= (1.f - ZERO_ANIMWEIGHT_THRESH);
}

// DISHONORED(written): 2013 rva 0x689df0 (2012 0x6d9290)
UDishonoredAnimTree* UDishonoredAnimNodeStatePicker::GetDominantTree() const
{
	if( m_iCurStateSlot == INDEX_NONE )
	{
		return NULL;
	}
	return m_StateSlots(m_iCurStateSlot).m_pTree;
}

// DISHONORED(written): 2013 rva 0x69c5a0 (2012 0x6eed40)
void UDishonoredAnimNodeStatePicker::CleanupForAnimStates( UBOOL bFullDestroy )
{
	for( INT SlotIdx = 0; SlotIdx < m_StateSlots.Num(); SlotIdx++ )
	{
		FStateSlotInfo& Slot = m_StateSlots(SlotIdx);
		if( Slot.m_pTreeRef )
		{
			Slot.m_pTreeRef->CleanupForAnimStates( bFullDestroy );
		}
		Slot.m_pTree = NULL;
		Children(SlotIdx).Weight = 0.f;
	}
	ClearState();
}

// DISHONORED(written): 2013 rva 0x69e7a0 (2012 0x6f2ae0): slots whose tree reference has gone are released, the active
// slot blends in at m_CurBlendSpeed and the remaining active slots are scaled to fill 1 - the active weight
void UDishonoredAnimNodeStatePicker::TickAnim( FLOAT DeltaSeconds )
{
	InitStateSlots();

	if( DisBringupAnimStateEnabled() && m_iCurStateSlot == INDEX_NONE && m_PossibleTemplates.Num() > 0 && SkelComponent )
	{
		const FName Wanted = DisBringupAnimStateName();
		INT PickIdx = ( Wanted == NAME_None ) ? 0 : INDEX_NONE;
		for( INT TemplateIdx = 0; Wanted != NAME_None && TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
		{
			if( m_PossibleTemplates(TemplateIdx).m_AnimTree_Name == Wanted )
			{
				PickIdx = TemplateIdx;
				break;
			}
		}
		if( PickIdx != INDEX_NONE )
		{
			const FName TreeName = m_PossibleTemplates(PickIdx).m_AnimTree_Name;
			UDishonoredAnimTree* Started = SetState( TreeName, TreeName, FALSE, 4.f );
			AActor* Owner = SkelComponent->GetOwner();
			debugf( TEXT("DISHONORED(bringup): -disanimstate set %s on %s '%s' (owner %s) -> tree %s"),
				*TreeName.ToString(), *GetClass()->GetName(), *NodeName.ToString(),
				Owner ? *Owner->GetName() : TEXT("NULL"),
				Started ? *Started->GetName() : TEXT("NULL") );
		}
	}

	for( INT SlotIdx = 0; SlotIdx < m_StateSlots.Num(); SlotIdx++ )
	{
		FStateSlotInfo& Slot = m_StateSlots(SlotIdx);
		if( Slot.m_bActive && Slot.m_pTreeRef && Slot.m_pTreeRef->m_iCurRefIndex == INDEX_NONE )
		{
			if( SlotIdx == m_iCurStateSlot )
			{
				ClearState();
			}
			else
			{
				Children(SlotIdx).Weight = 0.f;
			}
			Slot.m_bActive = FALSE;
			Slot.m_pTree = NULL;
		}
	}

	if( m_iCurStateSlot != INDEX_NONE )
	{
		FAnimBlendChild& ActiveChild = Children(m_iCurStateSlot);
		if( m_StateSlots(m_iCurStateSlot).m_bActive )
		{
			ActiveChild.Weight += m_CurBlendSpeed * DeltaSeconds;
			if( ActiveChild.Weight > 1.f )
			{
				ActiveChild.Weight = 1.f;
			}
		}

		FLOAT AccumWeight = 0.f;
		for( INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++ )
		{
			if( m_StateSlots(ChildIdx).m_bActive && m_iCurStateSlot != ChildIdx )
			{
				FAnimBlendChild& Child = Children(ChildIdx);
				Child.Weight = (1.f - ActiveChild.Weight) * Child.Weight;
				if( Child.Weight < ZERO_ANIMWEIGHT_THRESH )
				{
					ActiveChild.Weight += Child.Weight;
					Child.Weight = 0.f;
					m_StateSlots(ChildIdx).m_bActive = FALSE;
					m_StateSlots(ChildIdx).m_pTree = NULL;
				}
				AccumWeight += Child.Weight;
			}
		}

		if( AccumWeight <= ZERO_ANIMWEIGHT_THRESH )
		{
			Children(m_iCurStateSlot).Weight = 1.f;
		}
		else
		{
			const FLOAT Scale = (1.f - ActiveChild.Weight) / AccumWeight;
			for( INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++ )
			{
				if( m_StateSlots(ChildIdx).m_bActive && m_iCurStateSlot != ChildIdx )
				{
					Children(ChildIdx).Weight *= Scale;
				}
			}
		}
	}

	Super::TickAnim( DeltaSeconds );
}

// DISHONORED(written): 2013 rva 0x67de60 (2012 0x6d49f0): the picker is the only node in an Arkane tree whose children
// legitimately all sit at weight 0 (no anim state playing); it answers the reference pose instead of asserting in
// UAnimNodeBlendBase::GetBoneAtoms (UnAnimTree.cpp:1646). This is what the -distweakanimtree gate was deferring.
void UDishonoredAnimNodeStatePicker::GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys )
{
	if( GetChildWeightTotal() < ZERO_ANIMWEIGHT_THRESH )
	{
		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion = 0;
		if( SkelComponent && SkelComponent->SkeletalMesh )
		{
			FillWithRefPose( Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton );
		}
	}
	else
	{
		Super::GetBoneAtoms( Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
	}

	// DISHONORED(bringup): UDishonoredAnimNodeStatePicker::GetChildBoneAtoms (2013 rva 0x69ad60, 2012 0x6ed460) is not
	// ported: while one state blends out, retail overwrites the anchor bone (a UDisTweaks_Pawn bone name) of every
	// non-dominant child with the dominant state tree's atom for that bone, so the two states pivot around the same
	// joint. Without it a cross-fade between two anim states can drift at the anchor.
	static UBOOL bWarnedAnchorBone = FALSE;
	if( !bWarnedAnchorBone && m_iCurStateSlot != INDEX_NONE && GetChildWeightTotal() > ZERO_ANIMWEIGHT_THRESH )
	{
		bWarnedAnchorBone = TRUE;
		debugf( TEXT("DISHONORED(bringup): DishonoredAnimNodeStatePicker::GetChildBoneAtoms not ported (anchor-bone fixup skipped)") );
	}
}

// DISHONORED(written): 2013 rva 0x69ebd0 (2012 0x6f2f10): the single-slot form, used when the picker's state is
// replaced outright; every other slot is dropped and slot 0 takes the new state at full weight
UDishonoredAnimTree* UDishonoredAnimNodeStatePicker::SetState( FName StateName, FName TreeName, UBOOL bNonLooping )
{
	if( !GIsGame && !GIsEditor )
	{
		return NULL;
	}

	if( m_CurStateSlot == StateName && !bNonLooping )
	{
		return ( m_iCurStateSlot != INDEX_NONE ) ? m_StateSlots(m_iCurStateSlot).m_pTree : NULL;
	}

	InitStateSlots();
	if( Children.Num() == 0 )
	{
		return NULL;
	}

	for( INT ChildIdx = 1; ChildIdx < Children.Num(); ChildIdx++ )
	{
		m_StateSlots(ChildIdx).m_bActive = FALSE;
		m_StateSlots(ChildIdx).m_pTree = NULL;
		Children(ChildIdx).Weight = 0.f;
	}

	FStateSlotInfo& Slot = m_StateSlots(0);
	Slot.m_StateName = StateName;
	Slot.m_bActive = TRUE;
	Children(0).Weight = 1.f;
	m_CurStateSlot = StateName;
	m_CurBlendSpeed = 0.f;
	m_iCurStateSlot = 0;
	Slot.m_pTree = Slot.m_pTreeRef ? Slot.m_pTreeRef->SetActiveTreeReference( TreeName ) : NULL;
	return Slot.m_pTree;
}

// DISHONORED(written): 2013 rva 0x69edf0 (2012 0x6f6bc0): the blending form. An already-active slot with the same state
// name is re-used, otherwise the first free slot, otherwise the lowest-weighted one (whose tree reference is dropped).
UDishonoredAnimTree* UDishonoredAnimNodeStatePicker::SetState( FName StateName, FName TreeName, UBOOL bNonLooping, FLOAT BlendSpeed )
{
	if( !GIsGame && !GIsEditor )
	{
		return NULL;
	}

	if( m_CurStateSlot == StateName && !bNonLooping )
	{
		return ( m_iCurStateSlot != INDEX_NONE ) ? m_StateSlots(m_iCurStateSlot).m_pTree : NULL;
	}

	InitStateSlots();

	INT iEmpty = INDEX_NONE;
	INT iLowestWeight = INDEX_NONE;
	INT NumEmpty = 0;
	FLOAT MinWeight = 1.f;

	for( INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++ )
	{
		UAnimNode* ChildAnim = Children(ChildIdx).Anim;
		if( !ChildAnim || !ChildAnim->IsA( UDishonoredAnimNodeTreeRef_Dynamic::StaticClass() ) )
		{
			continue;
		}
		FStateSlotInfo& Slot = m_StateSlots(ChildIdx);
		if( !Slot.m_bActive )
		{
			if( iEmpty == INDEX_NONE )
			{
				iEmpty = ChildIdx;
			}
			NumEmpty++;
			continue;
		}
		if( !bNonLooping && Slot.m_StateName == StateName )
		{
			// the state is already blending in this slot: make it the dominant one again
			m_CurStateSlot = StateName;
			m_iCurStateSlot = ChildIdx;
			m_CurBlendSpeed = BlendSpeed;
			Slot.m_pTree = Slot.m_pTreeRef ? Slot.m_pTreeRef->SetActiveTreeReference( TreeName ) : NULL;
			return Slot.m_pTree;
		}
		if( MinWeight > Children(ChildIdx).Weight )
		{
			MinWeight = Children(ChildIdx).Weight;
			iLowestWeight = ChildIdx;
		}
	}

	if( iEmpty == INDEX_NONE )
	{
		iEmpty = iLowestWeight;
		if( iEmpty == INDEX_NONE )
		{
			return NULL;
		}
		if( m_StateSlots(iEmpty).m_pTreeRef )
		{
			m_StateSlots(iEmpty).m_pTreeRef->ClearActiveTreeReference( TRUE );
		}
	}

	FStateSlotInfo& Slot = m_StateSlots(iEmpty);
	Slot.m_StateName = StateName;
	Slot.m_bActive = TRUE;
	m_CurStateSlot = StateName;
	m_iCurStateSlot = iEmpty;
	m_CurBlendSpeed = BlendSpeed;
	Slot.m_pTree = Slot.m_pTreeRef ? Slot.m_pTreeRef->SetActiveTreeReference( TreeName ) : NULL;

	if( NumEmpty >= Children.Num() )
	{
		// nothing was playing: the new state takes the whole node
		Children(iEmpty).Weight = 1.f;
		for( INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++ )
		{
			if( ChildIdx != iEmpty )
			{
				Children(ChildIdx).Weight = 0.f;
			}
		}
	}
	else
	{
		Children(iEmpty).Weight = 0.f;
	}
	return Slot.m_pTree;
}
