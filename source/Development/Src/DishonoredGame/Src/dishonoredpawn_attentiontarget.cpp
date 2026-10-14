// DishonoredGame/src/dishonoredpawn_attentiontarget.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (18): all below.
//
// ---- agent GI ports (PHASE20 GI): what a Dishonored pawn answers when an NPC pays attention to it ----
// Resolved by hand in both databases (build/agentGI/idb; decompiles in build/agentGI/decomp2012). Every body here is on the
// IDisAttentionTargetInterface sub-object of the pawn, which is why the 2012 decompiles read the pawn's fields through odd
// offsets (the interface sits at +1216 in 2012, +1224 in 2013). The player's and the NPC's overrides are here too (retail
// keeps them in dishonoredplayerpawn_attentiontarget.cpp and dishonorednpcpawn.cpp).

#include "DishonoredGame.h"
#include "disattentioninfo.h"
#include "dishonoredutilities.h"
#include "aistimstruct.h"

void DishonoredLinkDishonoredPawnAttentionTarget()
{
}

// DISHONORED(port): 2012 rva 0x78e5a0 / 2013 0x7494a0 (7 B): the pawn's observable component (m_pCpntObservable).
// DISHONORED(bringup): FDisComponentObservable is not ported and nothing creates one, so this is NULL for every pawn.
const FDisComponentObservable* ADishonoredPawn::GetObservableComponent() const
{
	return (const FDisComponentObservable*)m_pCpntObservable;
}

// DISHONORED(port): 2012 rvas 0x78e5b0 (36 B, Location), 0x78e5e0 (36 B, Velocity), 0x78e610 / 2013 0x749510 (the cylinder extent)
FVector ADishonoredPawn::GetAttnTargetLocation() const
{
	return Location;
}

FVector ADishonoredPawn::GetAttnTargetVelocity() const
{
	return Velocity;
}

FVector ADishonoredPawn::GetAttnTargetExtent() const
{
	return GetCylinderExtent();
}

// DISHONORED(port): 2012 rva 0x78e630 / 2013 0x749530 (128 B): the focal point is the look-at point (the head bone of the pawn
// tweaks, or the actor origin), cached against the mesh's skeletal tag (USkeletalMeshComponent +736 in 2013).
// DISHONORED(written): the tag is recomputed every call here - this tree's USkeletalMeshComponent has no equivalent of that
// counter - so the cache holds the last value and is always refreshed; the answer is the same, the cost is one bone lookup.
FVector ADishonoredPawn::GetAttnTargetFocalPoint() const
{
	ADishonoredPawn* Self = const_cast<ADishonoredPawn*>( this );
	Self->m_AttnTargetFocalPoint = GetLookAtPoint();
	Self->m_AttnTargetFocalPointSkelMeshTag = Mesh ? Mesh->GetIndex() : 0;
	return m_AttnTargetFocalPoint;
}

// DISHONORED(port): 2012 rva 0x78e6b0 / 2013 0x7495b0 (58 B): the focal point's height above the feet.
FLOAT ADishonoredPawn::GetAttnTargetFocalHeight() const
{
	return GetAttnTargetFocalPoint().Z - DisGetPawnFeet( this ).Z;
}

// DISHONORED(port): 2012 rva 0x78e6f0 / 2013 0x7495f0 (44 B) and 0x78e720 / 0x749620 (17 B)
BYTE ADishonoredPawn::GetAttnTargetIncomingRelationship( const IDisRelationshipInterface* _pRelator ) const
{
	return _pRelator ? _pRelator->DetermineRelationship( *static_cast<const IDisRelationshipInterface*>( this ) ) : (BYTE)Relationship_Neutral;
}

IDisRelationshipInterface* ADishonoredPawn::ToRelationshipInterface() const
{
	return const_cast<ADishonoredPawn*>( this );
}

// DISHONORED(port): 2012 rva 0x78e740 / 2013 0x749640 (10 B): the officially-dead bit (ADishonoredPawn +1288 bit 10 in 2013).
UBOOL ADishonoredPawn::IsAttnTargetDead() const
{
	return m_bIsOfficiallyDead;
}

// DISHONORED(port): 2012 rvas 0x78e750, 0x78e790, 0x78e770 (20 B each): three virtual forwards into the pawn (2012 vtable +1672
// IsInCombat, +1668 CanFight, +1516 IsCarryingCorpse). DISHONORED(bringup): the first two pawn virtuals are the combat manager's
// and are not ported; an NPC answers through its brain's engaged flag and its fighter tweak, the player answers in combat
// never and fights always. IsCarryingCorpse is agent FZ's virtual (FALSE until the item contexts land).
UBOOL ADishonoredPawn::IsAttnTargetInCombat() const
{
	const ADishonoredNPCPawn* NPC = ConstCast<ADishonoredNPCPawn>( this );
	const UDishonoredAIBrain* Brain = NPC ? NPC->GetAttnTargetBrain() : NULL;
	return Brain ? Brain->IsCombatEngaged() : FALSE;
}

UBOOL ADishonoredPawn::CanAttnTargetFight() const
{
	const ADishonoredNPCPawn* NPC = ConstCast<ADishonoredNPCPawn>( this );
	return NPC ? NPC->IsAFighter() : TRUE;
}

UBOOL ADishonoredPawn::IsAttnTargetCarryingACorpse() const
{
	return IsCarryingCorpse();
}

// DISHONORED(port): 2012 rva 0x78e7b0 / 2013 0x7496b0 (52 B): a pawn whose type flags say "attackable" (AActor +1032 bit 28)
// and is not hidden from attacks (+?, bit 1) can be attacked, else the pawn's own CanBeAttacked virtual (+1168) answers.
// DISHONORED(bringup): neither flag nor virtual is named in this tree; a live pawn can be attacked, a dead one cannot.
UBOOL ADishonoredPawn::CanAttnTargetBeAttacked() const
{
	return !IsDead();
}

// DISHONORED(port): 2012 rva 0x78e7f0 / 2013 0x7496f0 (32 B) and 0x78e830 / 0x749730 (28 B): the global AI manager answers.
void ADishonoredPawn::GetAttentiveBrains( DIS_MEMSTACK_ARRAY( UDishonoredAIBrain* )& _rOut_Brains, BYTE _RequiredRelationship, BYTE _RequiredAttentionLevel ) const
{
	UDishonoredGlobalAIManager* Manager = DisGetGlobalAIManagerUnchecked();
	if( Manager )
	{
		Manager->GetAttentiveBrains( this, _rOut_Brains, _RequiredRelationship, _RequiredAttentionLevel );
	}
}

UBOOL ADishonoredPawn::HasAttentiveBrains( BYTE _RequiredRelationship, BYTE _RequiredAttentionLevel ) const
{
	UDishonoredGlobalAIManager* Manager = DisGetGlobalAIManagerUnchecked();
	return Manager ? Manager->HasAttentiveBrains( this, _RequiredRelationship, _RequiredAttentionLevel ) : FALSE;
}

// DISHONORED(port): 2012 rva 0x794830 / 2013 0x756850 (48 B): the pawn tweaks' attention-target tweaks.
const UDisTweaks_AttentionTarget* ADishonoredPawn::GetAttnTargetTweaks() const
{
	return GetPawnTweaks()->m_pAttentionTargetTweaks;
}

// DISHONORED(port): 2012 rva 0x799b90 / 2013 0x75c960 (371 B): the cached parameter block (the attention-target tweaks, the
// observable, location, velocity, extent, focal point, the dead bit, combatable, the actor), rebuilt when the cache is invalid.
// DISHONORED(written): retail validates the cache once per frame from the pawn tick (m_bCachedAttnParamsValid cleared there);
// nothing in this tree clears it, so the block is rebuilt on every call - the same values, one call later.
void ADishonoredPawn::CollectAttnTargetParams( FDisAttnTargetParams& _rOut_Params )
{
	m_CachedAttnParams.m_pTargetTweaks = (FPointer)GetAttnTargetTweaks();
	m_CachedAttnParams.m_pObservableComponent = m_pCpntObservable;
	m_CachedAttnParams.m_Location = Location;
	m_CachedAttnParams.m_Velocity = Velocity;
	m_CachedAttnParams.m_Extent = GetCylinderExtent();
	m_CachedAttnParams.m_FocalPoint = GetAttnTargetFocalPoint();
	m_CachedAttnParams.m_bIsDead = m_bIsOfficiallyDead;
	m_CachedAttnParams.m_bIsCombatable = 1;
	m_CachedAttnParams.m_pActor = this;
	m_bCachedAttnParamsValid = 1;
	_rOut_Params = m_CachedAttnParams;
}

// DISHONORED(port): 2012 rva 0x794440 / 2013 0x756100 (245 B): the pawn tweaks' look-at bone (an FName at UDisTweaks_Pawn +416
// in 2012, +0x1a0: the retail SDK dump did not type that member, so this tree's UDisTweaks_Pawn has no field for it) on the
// mesh, or the actor origin (the fourth row of LocalToWorld).
// DISHONORED(bringup): the bone name is read as head_jnt, the bone agent FT measured the look-at component driving on every
// NPC skeleton, until the tweak member is typed; a mesh without it answers the origin, as retail does for an unset name.
FVector ADishonoredPawn::GetLookAtPoint() const
{
	static const FName HeadBone( TEXT("head_jnt") );
	if( Mesh && Mesh->MatchRefBone( HeadBone ) != INDEX_NONE )
	{
		return Mesh->GetBoneLocation( HeadBone );
	}
	return LocalToWorld().GetOrigin();
}

// DISHONORED(port): 2012 rva 0x7aaaa0 (35 B): the observable component's WasSpottedBy; without one, unseen.
UBOOL ADishonoredPawn::IsSeenBy( const AActor* _pActor ) const
{
	return FALSE;
}

/*-----------------------------------------------------------------------------
	The NPC pawn
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7c0080 / 2013 0x759a00 (78 / 72 B): the controller's initialized brain, none when dead.
const UDishonoredAIBrain* ADishonoredNPCPawn::GetAttnTargetBrain() const
{
	const ADishonoredNPCController* NPCController = ConstCast<ADishonoredNPCController>( Controller );
	if( IsDead() || !NPCController || !NPCController->GetAIBrain() || !NPCController->GetAIBrain()->IsBrainInitialized() )
	{
		return NULL;
	}
	return NPCController->GetAIBrain();
}

// DISHONORED(port): 2012 rva 0x7ae590 / 2013 0x770650 (44 B): the pawn choking this one, as an attention target.
// DISHONORED(bringup): the choke (the NPC's choker pointer, 2012 +... through the master FSM) is not ported: nobody is murdered.
UBOOL ADishonoredNPCPawn::IsAttnTargetBeingMurdered( IDisAttentionTargetInterface** _ppMurderer ) const
{
	if( _ppMurderer )
	{
		*_ppMurderer = NULL;
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7ab9c0 (39 B)
UBOOL ADishonoredNPCPawn::AreReactionsDisabled() const
{
	return !m_bOverrideReactionDisableFromSoiree && m_iNumReactionDisableRequestsFromSoiree > 0 && Health > 0;
}

/*-----------------------------------------------------------------------------
	The player pawn
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x6fb080 / 2013 0x6a1ed0 (22 B)
void ADishonoredPlayerPawn::CollectAttnTargetParams( FDisAttnTargetParams& _rOut_Params )
{
	ADishonoredPawn::CollectAttnTargetParams( _rOut_Params );
	m_CachedAttnParams.m_bIsPlayer = 1;
	_rOut_Params.m_bIsPlayer = 1;
}

// DISHONORED(port): 2012 rva 0x6fb0a0 (17 B): attention is gained on the player unless he is possessing something.
// DISHONORED(bringup): ADishonoredPawn::IsPossessing (the possession state) is not ported; the player is never possessing here.
UBOOL ADishonoredPlayerPawn::AllowAttentionGain() const
{
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x705250 / 2013 0x6ab710 (49 B): a brain that starts attending the player clears its stealth
// indicator accumulator and is pushed on the player's attentive-brain chain (the head is m_pAttentiveBrainsList, the links the
// brains' m_pNextPlayerAttentiveBrain); 0x6fe650 (113 B) unlinks it.
void ADishonoredPlayerPawn::OnAttentionReceived( UDishonoredAIBrain* _pBrain )
{
	if( !_pBrain )
	{
		return;
	}
	_pBrain->m_PlayerAttentiveInfo.m_Accumulator.Clear();
	if( m_pAttentiveBrainsList )
	{
		_pBrain->m_pNextPlayerAttentiveBrain = m_pAttentiveBrainsList;
	}
	m_pAttentiveBrainsList = _pBrain;
}

void ADishonoredPlayerPawn::OnAttentionLost( UDishonoredAIBrain* _pBrain )
{
	if( !_pBrain )
	{
		return;
	}
	if( m_pAttentiveBrainsList == _pBrain )
	{
		m_pAttentiveBrainsList = _pBrain->m_pNextPlayerAttentiveBrain;
	}
	else
	{
		for( UDishonoredAIBrain* Link = m_pAttentiveBrainsList; Link; Link = Link->m_pNextPlayerAttentiveBrain )
		{
			if( Link->m_pNextPlayerAttentiveBrain == _pBrain )
			{
				Link->m_pNextPlayerAttentiveBrain = _pBrain->m_pNextPlayerAttentiveBrain;
				break;
			}
		}
	}
	_pBrain->m_pNextPlayerAttentiveBrain = NULL;
}

// DISHONORED(port): 2012 rva 0x705290 / 2013 0x6ab750 (115 / 120 B): the pawn's answer, refused while the controller is in a
// state that forbids incoming attacks (its flag +908 bit 15) or the master state (UStatePlayerMasterBase / Action) forbids them.
// DISHONORED(bringup): neither the controller flag nor AreIncomingAttacksAllowed is ported; the base answer stands.
UBOOL ADishonoredPlayerPawn::CanAttnTargetBeAttacked() const
{
	return ADishonoredPawn::CanAttnTargetBeAttacked();
}
