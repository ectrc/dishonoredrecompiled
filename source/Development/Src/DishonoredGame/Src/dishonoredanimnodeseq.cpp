// DishonoredGame/src/dishonoredanimnodeseq.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (24):
//   0x6d0c60  public: static void __cdecl UDishonoredAnimNodeSeq::InitializePrivateStaticClassUDishonoredAnimNodeSeq(void)
//   0x6d0c80  public: virtual void __thiscall UDishonoredAnimNodeSeq::PostAnimNodeInstance(class UAnimNode *, class TMap<class UAnimNode *, class UAnimNode *, class FDefaultSetAllocator> *)
//   0x6e03b0  public: virtual class FName __thiscall UDishonoredAnimNodeSeq::ApplySequenceNameModifiers(void)
//   0x6e0530  public: virtual void __thiscall UDishonoredAnimNodeSeq::PlayAnim(unsigned int, float, float)
//   0x6e06e0  protected: void __thiscall UDishonoredAnimNodeSeq::GetParentBones(int, class TArray<unsigned char, class FDefaultAllocator> &)
//   0x6e07d0  protected: void __thiscall UDishonoredAnimNodeSeq::GetFixedTimeBoneAtoms(int, float, class TArray<unsigned char, class FDefaultAllocator> &, class TArray<class FBoneAtom, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &)
//   0x6e0870  public: virtual void __thiscall UDishonoredAnimNodeSeq::OnBecomeRelevant(void)
//   0x6e09b0  public: unsigned int __thiscall UDishonoredAnimNodeSeq::IsDominantAnimStateSequence(void)const
//   0x6e0ad0  private: void __thiscall UDishonoredAnimNodeSeq::AddPendingRangedNotify(struct FAnimNotifyEvent const &, float)
//   0x6e0b40  public: virtual void __thiscall UDishonoredAnimNodeSeq::IssueSingleNotify(struct FAnimNotifyEvent const &, float)
//   0x6e0c00  public: virtual void __thiscall UDishonoredAnimNodeSeq::BecameNotifyMaster(class UAnimNodeSequence *)
//   0x6e0ce0  public: virtual void __thiscall UDishonoredAnimNodeSeq::DrawAnimNode(class FCanvas *, class TArray<class UAnimObject *, class FDefaultAllocator> const &, unsigned int)
//   0x6e0df0  public: virtual void __thiscall UDishonoredAnimNodeSeq::OnAnimComplete(float, float)
//   0x6eac80  public: virtual void __thiscall UDishonoredAnimNodeSeqAttack::PlayAnim(unsigned int, float, float)
//   0x6ee650  protected: void __thiscall UDishonoredAnimNodeSeq::TickPendingNotifies(float)
//   0x6ee740  public: virtual void __thiscall UDishonoredAnimNodeSeq::IssueNotifies(float)
//   0x6ef1b0  public: void __thiscall UDishonoredAnimNodeSeq::ClearPendingNotifies(unsigned int)
//   0x6ef2a0  public: virtual void __thiscall UDishonoredAnimNodeSeq::NoLongerNotifyMaster(class UAnimNodeSequence *)
//   0x6f16a0  public: virtual void __thiscall UDishonoredAnimNodeSeq::OnCeaseRelevant(void)
//   0x6f3bc0  public: static class UClass * __cdecl UDishonoredAnimNodeSeq::GetPrivateStaticClassUDishonoredAnimNodeSeq(wchar_t const *)
//   0x6f3c50  public: static class UClass * __cdecl UDishonoredAnimNodeSeqAttack::GetPrivateStaticClassUDishonoredAnimNodeSeqAttack(wchar_t const *)
//   0x6f4e50  public: static class UClass * __cdecl UDishonoredAnimNodeSeq::StaticClassNoInline(void)
//   0x6f4e80  public: static void __cdecl UDishonoredAnimNodeSeqAttack::InitializePrivateStaticClassUDishonoredAnimNodeSeqAttack(void)
//   0x6f52c0  public: static class UClass * __cdecl UDishonoredAnimNodeSeqAttack::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the Arkane sequence node (Inc/CppText/UDishonoredAnimNodeSeq.h). Flag word +296: bit 0
	m_bGotAnimStateExit, 1 m_bGotAnimStateAdvance, 2 m_bAddItemPrefix, 3 m_bRevaluateSeqNameOnRelevant, 4 m_bSequenceMustBeSetInAnimState,
	5 m_bPlayOnBecomeRelevant, 6 m_bContainsAnimStateExit, 7 m_bContainsAnimStateAdvance, 8 m_bScaleByLandAnimRate, 9 m_bScaleByMantleAnimRate.
	DISHONORED(bringup), named deviations:
	  * the sequence-name re-evaluation and the play on becoming relevant are applied to nodes owned by the player or an item only:
	    an NPC's sequences are driven by its anim states in retail, which are not ported (NPC trees keep their cooked names)
	  * PostAnimNodeInstance (2012 0x6d0c80: m_bSequenceMustBeSetInAnimState clears the instanced node's sequence name) is not written
	    for the same reason, so a custom-sequence slot keeps its cooked sequence until a state names one
	  * the ranged notifies (AddPendingRangedNotify, TickPendingNotifies, ClearPendingNotifies, IssueSingleNotify, IssueNotifies,
	    Become / NoLongerNotifyMaster) need virtual hooks in the engine's UAnimNodeSequence that it does not have
	  * PlayAnim's required-notify scan (m_RequiredAnimNotifies) only searches and reports nothing in retail; not written
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_DishonoredAnimNodeSeq()
{
}

static UBOOL DisIMSeqOwnedByNPC( const UAnimNodeSequence* Seq )
{
	return Seq->SkelComponent && Seq->SkelComponent->GetOwner() && Seq->SkelComponent->GetOwner()->IsA( ADishonoredNPCPawn::StaticClass() );
}

// 2013 0x697ca0 (386 B): <the hand's item prefix, "Empty_" without an item><m_SequenceNameRoot> for a prefixed node, else the sequence name
FName UDishonoredAnimNodeSeq::ApplySequenceNameModifiers()
{
	if( !m_bAddItemPrefix )
	{
		return AnimSeqName;
	}
	FString SeqName;
	ADishonoredPawn* Pawn = SkelComponent ? Cast<ADishonoredPawn>( SkelComponent->GetOwner() ) : NULL;
	UDishonoredInventoryItem* Item = Pawn ? Cast<UDishonoredInventoryItem>( Pawn->GetEquippedItem( m_ApplyItemEquipPrefix ) ) : NULL;
	if( Item )
	{
		SeqName += Item->GetItemPrefix().ToString();
	}
	else
	{
		SeqName += TEXT("Empty_");
	}
	SeqName += m_SequenceNameRoot;
	return FName( *SeqName );
}

/**
 * 2013 0x697e30 (462 B). The player's land and mantle sequences play at the pawn's Attribute_LandAnimRate / Attribute_MantleAnimRate;
 * a sequence of an anim state attached to a player action with a play-rate override plays at that rate. The exit / advance flags
 * are cleared for the new play.
 */
void UDishonoredAnimNodeSeq::PlayAnim( UBOOL bLoop, FLOAT InRate, FLOAT StartTime )
{
	static const FName NAME_LandAnimRate( TEXT("Attribute_LandAnimRate") );
	static const FName NAME_MantleAnimRate( TEXT("Attribute_MantleAnimRate") );
	m_pDisParentAnimTree = Cast<UDishonoredAnimTree>( m_pParentAnimTree );
	ADishonoredPlayerPawn* Player = SkelComponent ? Cast<ADishonoredPlayerPawn>( SkelComponent->GetOwner() ) : NULL;
	if( Player )
	{
		if( m_bScaleByLandAnimRate )
		{
			InRate = Player->GetAttributeValue( NAME_LandAnimRate );
		}
		else if( m_bScaleByMantleAnimRate )
		{
			InRate = Player->GetAttributeValue( NAME_MantleAnimRate );
		}
	}
	if( m_pDisParentAnimTree )
	{
		FDisAnimStateTreeInfo Info;
		m_pDisParentAnimTree->GetAnimStateInfo( &Info );
		UStateSharedActionBase_PlayerOnly* Action = Cast<UStateSharedActionBase_PlayerOnly>( (UObject*)Info.m_pAnimState_AttachToState );
		if( Action && Action->GetCurAction() && Action->GetCurAction()->m_fPlayRateOverride > 0.f )
		{
			InRate = Action->GetCurAction()->m_fPlayRateOverride;
		}
	}
	m_bGotAnimStateExit = FALSE;
	m_bGotAnimStateAdvance = FALSE;
	Super::PlayAnim( bLoop, InRate, StartTime );
}

// 2013 0x69bb30 (337 B): the sequence name re-evaluated, the play on becoming relevant, the parent tree remembered, a playing sequence of
// an active anim state logged in the owner's history
void UDishonoredAnimNodeSeq::OnBecomeRelevant()
{
	UAnimNode::OnBecomeRelevant();
	const UBOOL bPlayerSide = !DisIMSeqOwnedByNPC( this );
	if( m_bRevaluateSeqNameOnRelevant && bPlayerSide )
	{
		const FName NewName = ApplySequenceNameModifiers();
		if( NewName != AnimSeqName || AnimSeq == NULL )
		{
			SetAnim( NewName );
		}
	}
	if( m_bPlayOnBecomeRelevant && bPlayerSide )
	{
		PlayAnim( bLooping, Rate, 0.f );
	}
	m_pDisParentAnimTree = Cast<UDishonoredAnimTree>( m_pParentAnimTree );
	if( bPlaying && m_pDisParentAnimTree && m_pDisParentAnimTree->m_AnimStateInfo.m_bIsAnimStateActive && SkelComponent )
	{
		UDishonoredItemSkeletalComponent* ItemComp = Cast<UDishonoredItemSkeletalComponent>( SkelComponent );
		UObject* OwnerObject = ItemComp ? (UObject*)ItemComp->GetOwningInventoryItem() : (UObject*)SkelComponent->GetOwner();
		IDisAnimStateOwnerInterface* Owner = OwnerObject ? IDisAnimStateOwnerInterface::FromObject( OwnerObject ) : NULL;
		UDisAnimStateComponent* Comp = Owner ? Owner->AnimState_GetAnimStateComp() : NULL;
		if( Comp )
		{
			Comp->LogSeqWithHistory( m_pDisParentAnimTree->m_AnimStateInfo.m_AnimStatePicker, AnimSeqName );
		}
	}
}

// 2013 0x68f1b0 (267 B): inside an active anim state (an NPC's always), the node is the active child of every blend list above it
UBOOL UDishonoredAnimNodeSeq::IsDominantAnimStateSequence() const
{
	if( m_pDisParentAnimTree == NULL || !m_pDisParentAnimTree->IsAnimStateActive() )
	{
		return FALSE;
	}
	UBOOL bDominant = FALSE;
	for( INT ParentIndex = 0; ParentIndex < ParentNodes.Num(); ParentIndex++ )
	{
		UAnimNodeBlendList* List = Cast<UAnimNodeBlendList>( ParentNodes( ParentIndex ) );
		if( List == NULL )
		{
			bDominant = TRUE;
		}
		else if( List->ActiveChildIndex >= 0 && List->ActiveChildIndex < List->Children.Num() && List->Children( List->ActiveChildIndex ).Anim == this )
		{
			bDominant = TRUE;
		}
	}
	return bDominant;
}

/**
 * 2013 0x69bdb0 (378 B). The end of a sequence that carries the exit (m_bContainsAnimStateExit) and has not had its exit notify: for an
 * NPC the master FSM's firing object is the firing interface, for the player the tree's; a dominant sequence of a live owner asks it
 * to leave the state over the time left. The same for the advance (m_bContainsAnimStateAdvance without its notify).
 */
void UDishonoredAnimNodeSeq::OnAnimComplete( FLOAT PlayedTime, FLOAT ExcessTime )
{
	AActor* OwnerActor = SkelComponent ? SkelComponent->GetOwner() : NULL;
	IDisAnimStateOwnerInterface* Owner = OwnerActor ? IDisAnimStateOwnerInterface::FromObject( OwnerActor ) : NULL;
	IDisAnimStateFiringInterface* FiringObj = NULL;
	ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( OwnerActor );
	if( NPC )
	{
		UDishonoredNativeState* LogicalState = NPC->m_pNPCMasterFSM ? NPC->m_pNPCMasterFSM->GetLogicalState() : NULL;
		UObject* Firing = LogicalState ? LogicalState->GetFiringObject() : NULL;
		FiringObj = Firing ? IDisAnimStateFiringInterface::FromObject( Firing ) : NULL;
	}
	else if( m_pDisParentAnimTree )
	{
		FiringObj = m_pDisParentAnimTree->GetAnimStateFireInterface();
	}
	ADishonoredPawn* Pawn = Cast<ADishonoredPawn>( OwnerActor );
	FDisAnimStateTreeInfo Info;
	appMemzero( &Info, sizeof(Info) );
	if( m_bContainsAnimStateExit && !m_bGotAnimStateExit && Owner && !( Pawn && Pawn->IsDead() ) )
	{
		if( NPC || IsDominantAnimStateSequence() )
		{
			if( !NPC )
			{
				m_pDisParentAnimTree->GetAnimStateInfo( &Info );
			}
			Owner->AnimState_TriggerExit( FiringObj, Info.m_AnimStatePicker, (UDishonoredNativeState*)Info.m_pAnimState_AttachToState, GetTimeLeft() );
		}
	}
	if( m_bContainsAnimStateAdvance && !m_bGotAnimStateAdvance && Owner && ( NPC || IsDominantAnimStateSequence() ) )
	{
		Owner->AnimState_Advance( FiringObj, GetTimeLeft() );
	}
}
