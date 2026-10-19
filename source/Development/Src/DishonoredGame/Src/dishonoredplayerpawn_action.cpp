// DishonoredGame/src/dishonoredplayerpawn_action.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x6fae20  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::Tick_PlayerAction(float, enum ELevelTick)
//   0x6fae70  public: virtual void __thiscall ADishonoredPlayerPawn::ChangeAnimStateMotionModes(enum ERootMotionMode, enum ERootMotionRotationMode)
//   0x6faf50  public: virtual void __thiscall ADishonoredPlayerPawn::StopRagdolling(void)
//   0x6fb030  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::MAT_IsUnequipped(void)const
//   0x6fe380  public: virtual void __thiscall ADishonoredPlayerPawn::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x6fe530  public: void __thiscall ADishonoredPlayerPawn::Look(class FVector, class FRotator, class ADisDoor *)
//   0x6fe570  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_SetForceInventory(unsigned int)
//   0x704a00  protected: void __thiscall ADishonoredPlayerPawn::GatherAnimStateArrays(enum eAnimState_Picker, struct FDisPawnAnimState_PickerInfo &)
//   0x704ad0  protected: unsigned int __thiscall ADishonoredPlayerPawn::RequestActionHelper(class UDishonoredNativeStateMachine *, unsigned int, struct FStateSharedActionBase_PlayerOnly_Param &, class UObject *, unsigned int)
//   0x704b30  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::AnimState_TriggerExit_Derived(int, class UDishonoredNativeState *, float)
//   0x704be0  protected: virtual void __thiscall ADishonoredPlayerPawn::AnimState_LockoutPicker_Derived(int, float, unsigned int)
//   0x704d00  public: virtual void __thiscall ADishonoredPlayerPawn::AnimState_SyncEnd(enum eAnimState_Picker)
//   0x704d50  public: void __thiscall ADishonoredPlayerPawn::DisableArmFollow(enum EDisEquipUsage, unsigned int, float)
//   0x704db0  public: void __thiscall ADishonoredPlayerPawn::DisableArmOffset(enum EDisEquipUsage, unsigned int, float)
//   0x704e10  public: void __thiscall ADishonoredPlayerPawn::BreakOutOfLean(void)
//   0x704e30  public: void __thiscall ADishonoredPlayerPawn::BreakOutOfLook(void)
//   0x704e50  public: unsigned int __thiscall ADishonoredPlayerPawn::CanApplyWalkFlags(void)const
//   0x704e80  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IgnoreBlockingBy(class AActor const *)const
//   0x704f80  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::InSyncedAction(void)const
//   0x704fd0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::StartRagdolling(void)
//   0x705120  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_BlendOut(class UInterpGroupInst *)
//   0x7051f0  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_FinishPlayerGroup(class UInterpGroupPlayer *, class UInterpGroupInstPlayer *)
//   0x70bb90  public: unsigned int __thiscall ADishonoredPlayerPawn::RequestAction(enum eDisPlayerActionUsage, struct FStateSharedActionBase_PlayerOnly_Param &, class UObject *, unsigned int)
//   0x70bc90  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::RequestItemAction(class UDisItemContext *, struct FStateSharedActionBase_Param &, class UStateSharedActionBase * &, unsigned int)
//   0x70bce0  public: void __thiscall ADishonoredPlayerPawn::Lean(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
//   0x70fc00  protected: virtual void __thiscall ADishonoredPlayerPawn::OnSprintChange_Derived(void)
//   0x70fcc0  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_BeginPlayerGroup(class UInterpGroupPlayer *, class UInterpGroupInstPlayer *)
//   0x720260  public: virtual void __thiscall ADishonoredPlayerPawn::PreBeginPlay_Actions(void)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the player master FSM's life and the player half of the matinee (MAT_*) contract.

	Ported here, with both databases' addresses (2013 rva = VA - 0x400000; a 2013 address marked * is match_2012_2013.csv's
	by the method named, not an independently named function):
	  PreBeginPlay_Actions   2012 0x720260  2013 0x6a4a50  (named in both)
	  Tick_PlayerAction      2012 0x6fae20  2013 0x6a1ca0* (neighbours 0.870, same 70 B)
	  MAT_BeginPlayerGroup   2012 0x70fcc0  2013 0x6ab570* (neighbours 0.862, 257 -> 296 B: 2013 adds the anim-control
	                                                         and leave-camera flags and the +2208 bit 13 write)
	  MAT_FinishPlayerGroup  2012 0x7051f0  2013 0x6ab6a0* (neighbours 0.833)
	  MAT_BlendOut           2012 0x705120  2013 0x6ab4a0  (named in both, same 193 B)
	  MAT_IsUnequipped       2012 0x6fb030  2013 0x6a1e80  (named in both, 27 -> 39 B)
	  MAT_SetForceInventory  2012 0x6fe570  2013 0x6a4d40  (named in both, same 222 B)
	  DisableArmFollow       2012 0x704d50  2013 0x6a1cf0* (callee 0.759; 2013 adds the bWaitToBlendOneFrame argument)
	  CancelAllActivePowers  2012 0x7048c0  (the power components have no bodies here: logged once)
	The MAT_ five are APawn virtuals in retail; here they stay on ADishonoredPlayerPawn and interpgroupplayer.cpp casts.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "gjequipdiag.h"
#include "stateplayermasterparams.h"
#include "fzsoireediag.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PlayerPawnAction()
{
}

/**
 * DISHONORED(port): 2012 0x720260. PreBeginPlay_AnimStates (0x7200e0, the anim-state picker arrays; unported, its
 * component has no bodies), the matinee blender looked up by name in the mesh's anim tree, PreBeginPlay_PlayerCombat
 * (0x71a710, unported), InitNavigationHandle (unported), then the three player FSMs initialised into their default states:
 * the master into UStatePlayerMasterWalk, the upper and the left arm into UStatePlayerUpperIdle.
 * DISHONORED(bringup): the upper and left-arm machines are not initialised here - every UStatePlayerUpper* unit is a
 * skeleton and an InitFSM into a state that has no bodies would run its empty base, which is also what not initialising
 * does; the pointers stay NULL and are tested by every reader. The master FSM is what the soiree needs.
 */
void ADishonoredPlayerPawn::PreBeginPlay_Actions()
{
	if( Mesh && Mesh->Animations )
	{
		UAnimTree* Tree = Cast<UAnimTree>( Mesh->Animations );
		m_pMatineeBlender = Tree ? Cast<UArkAnimNodeBlendPose>( Tree->FindAnimNode( FName(TEXT("PoseBlender")) ) ) : NULL;
	}
	if( !m_pPlayerMasterFSM )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s has no m_pPlayerMasterFSM subobject; the player master FSM cannot be initialised"), *GetName() );
		return;
	}
	FStatePlayerMasterWalk_Param WalkParam;
	m_pPlayerMasterFSM->InitFSM( this, WalkParam, NULL );
	if( DisFZDiagEnabled() )
	{
		FString States;
		for( INT Index = 0; Index < m_pPlayerMasterFSM->m_NativeStates.Num(); Index++ )
		{
			States += m_pPlayerMasterFSM->m_NativeStates(Index) ? m_pPlayerMasterFSM->m_NativeStates(Index)->GetClass()->GetName() : FString(TEXT("null"));
			States += TEXT(" ");
		}
		const FString Current = m_pPlayerMasterFSM->GetCurrentlyActiveState() ? m_pPlayerMasterFSM->GetCurrentlyActiveState()->GetClass()->GetName() : FString(TEXT("none"));
		debugf( TEXT("DISHONORED(bringup): fzdiag master FSM initialised on %s: %d states [%s], current %s, soiree state %s, blender %s"), *GetName(),
			m_pPlayerMasterFSM->m_NativeStates.Num(), *States, *Current,
			m_pPlayerMasterFSM->FindState( UStatePlayerMasterSoiree::StaticClass() ) ? TEXT("present") : TEXT("MISSING"),
			m_pMatineeBlender ? *m_pMatineeBlender->GetName() : TEXT("none") );
	}
}

// DISHONORED(port): 2012 0x6fae20: the three machines tick; retail ticks the upper and left-arm ones too, which are not initialised here
UBOOL ADishonoredPlayerPawn::Tick_PlayerAction( FLOAT DeltaTime, ELevelTick TickType )
{
	if( m_pPlayerMasterFSM && m_pPlayerMasterFSM->GetCurrentlyActiveState() )
	{
		static UBOOL bSaidTicking = FALSE;
		if( !bSaidTicking )
		{
			bSaidTicking = TRUE;
			FZDIAG( TEXT("master FSM ticking on %s (first tick, dt %.3f)"), *GetName(), DeltaTime );
		}
		m_pPlayerMasterFSM->TickStateMachine( DeltaTime );
	}
	if( m_pPlayerUpperFSM && m_pPlayerUpperFSM->GetCurrentlyActiveState() )
	{
		m_pPlayerUpperFSM->TickStateMachine( DeltaTime );
	}
	if( m_pPlayerLeftArmFSM && m_pPlayerLeftArmFSM->GetCurrentlyActiveState() )
	{
		m_pPlayerLeftArmFSM->TickStateMachine( DeltaTime );
	}
	return TRUE;
}

/**
 * DISHONORED(port): 2013 0x6ab570 (the 2012 body lacks the two flags marked 2013). The flag word: bit 1 from
 * m_bDisablePlayerCollision, bit 2 from UInterpGroup::HasMoveTrack, bit 3 (2013) from HasAnimControlTrack, bit 5 (2013)
 * from m_bLeaveCameraControlToDialogTree when the game info is a sub-class of ADishonoredGameInfo (a DLC game; never
 * here). The matinee blender is enabled, sneaking stopped, the body mode set from m_bPutPlayerInWorldDepthGroup
 * (FULL_BODY, else ARMS_ONLY), then the soiree is started: in the dialog state directly, otherwise by requesting
 * UStatePlayerMasterSoiree with the matinee, the stage-mark blend time and the flags.
 * The 2013 body also raises bit 13 of the +2208 flag word first (2013 0x7491a0: the eighteenth bit of the player pawn's
 * bitfield, between m_bClimbingUp and the SDK's next member - unnamed in the dump, so not written here).
 */
void ADishonoredPlayerPawn::MAT_BeginPlayerGroup( UInterpGroupPlayer* GroupPlayer, UInterpGroupInstPlayer* GroupInstPlayer )
{
	if( !GroupPlayer || !GroupInstPlayer )
	{
		return;
	}
	UStatePlayerMasterInDialog* InDialog = m_pPlayerMasterFSM ? Cast<UStatePlayerMasterInDialog>( m_pPlayerMasterFSM->GetLogicalState() ) : NULL;
	INT Flags = 0;
	if( GroupPlayer->m_bDisablePlayerCollision )
	{
		Flags |= SOIREE_DisablePlayerCollision;
	}
	if( GroupPlayer->HasMoveTrack() )
	{
		Flags |= SOIREE_HasMoveTrack;
	}
	if( GroupPlayer->HasAnimControlTrack() )
	{
		Flags |= SOIREE_HasAnimControlTrack;
	}
	if( m_pMatineeBlender )
	{
		m_pMatineeBlender->SetEnable( TRUE );
	}
	StopSneaking( FALSE );
	SetBodyMode( GroupPlayer->m_bPutPlayerInWorldDepthGroup ? BODY_MODE_FULL_BODY : BODY_MODE_ARMS_ONLY );

	USeqAct_Interp* Matinee = Cast<USeqAct_Interp>( GroupInstPlayer->GetOuter() );
	FZDIAG( TEXT("MAT_BeginPlayerGroup %s group '%s': flags 0x%x blendToMark %.2f worldDPG %d, FSM %s, logical state %s"),
		Matinee ? *Matinee->GetName() : TEXT("none"), *GroupPlayer->GroupName.ToString(), Flags, GroupPlayer->m_fTimeToBlendToStageMark,
		(INT)GroupPlayer->m_bPutPlayerInWorldDepthGroup, m_pPlayerMasterFSM ? TEXT("yes") : TEXT("NONE"),
		( m_pPlayerMasterFSM && m_pPlayerMasterFSM->GetLogicalState() ) ? *m_pPlayerMasterFSM->GetLogicalState()->GetClass()->GetName() : TEXT("none") );
	if( InDialog )
	{
		InDialog->StartSoireeControl( Matinee, GroupPlayer->m_fTimeToBlendToStageMark, Flags );
	}
	else if( m_pPlayerMasterFSM )
	{
		FStatePlayerMasterSoiree_Param Param( Matinee, GroupPlayer->m_fTimeToBlendToStageMark, Flags );
		const UBOOL bAccepted = m_pPlayerMasterFSM->RequestStateChange( Param, NULL, FALSE );
		FZDIAG( TEXT("soiree requested: accepted %d, pending %s (id %s), current %s"), bAccepted,
			m_pPlayerMasterFSM->GetPendingState() ? *m_pPlayerMasterFSM->GetPendingState()->GetClass()->GetName() : TEXT("NULL"),
			m_pPlayerMasterFSM->GetPendingStateID() ? *m_pPlayerMasterFSM->GetPendingStateID()->GetName() : TEXT("NULL"),
			m_pPlayerMasterFSM->GetCurrentlyActiveState() ? *m_pPlayerMasterFSM->GetCurrentlyActiveState()->GetClass()->GetName() : TEXT("NULL") );
		if( !bAccepted )
		{
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): the player master FSM refused UStatePlayerMasterSoiree for %s"), Matinee ? *Matinee->GetName() : TEXT("none") );
		}
	}
}

// DISHONORED(port): 2012 0x7051f0: the arms-only body mode back, then the dialog state's StopSoireeControl or the soiree state's OnSoireeFinished
void ADishonoredPlayerPawn::MAT_FinishPlayerGroup( UInterpGroupPlayer* GroupPlayer, UInterpGroupInstPlayer* GroupInstPlayer )
{
	SetBodyMode( BODY_MODE_ARMS_ONLY );
	if( !m_pPlayerMasterFSM )
	{
		return;
	}
	UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( m_pPlayerMasterFSM->GetLogicalState() );
	if( InDialog )
	{
		InDialog->StopSoireeControl( FALSE );
		return;
	}
	UStatePlayerMasterSoiree* Soiree = Cast<UStatePlayerMasterSoiree>( m_pPlayerMasterFSM->GetLogicalState() );
	if( Soiree )
	{
		Soiree->OnSoireeFinished();
	}
}

/**
 * DISHONORED(port): 2012 0x705120. The group instance's blend-out time is handed to whichever soiree carrier is the
 * currently active state, then the matinee blender blends back now unless the group is a folder-only one (retail reads
 * UInterpGroup+108 bits 8/9 - bIsFolder / bIsParented), the time is nil, there is no blender or the pawn is a rigid body,
 * in which case the blender is simply disabled.
 */
void ADishonoredPlayerPawn::MAT_BlendOut( UInterpGroupInst* PlayerGroup )
{
	if( !PlayerGroup )
	{
		return;
	}
	const FLOAT BlendOutTime = PlayerGroup->GetBlendOutTime();
	UInterpGroup* Group = PlayerGroup->Group;
	if( m_pPlayerMasterFSM )
	{
		UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( m_pPlayerMasterFSM->GetCurrentlyActiveState() );
		if( InDialog )
		{
			InDialog->SetSoireeBlendOutTime( BlendOutTime );
		}
		else
		{
			UStatePlayerMasterSoiree* Soiree = Cast<UStatePlayerMasterSoiree>( m_pPlayerMasterFSM->GetCurrentlyActiveState() );
			if( Soiree )
			{
				Soiree->SetSoireeBlendOutTime( BlendOutTime );
			}
		}
	}
	const UBOOL bFolderGroup = Group && ( Group->bIsFolder || Group->bIsParented );
	if( bFolderGroup || BlendOutTime <= 1e-8f || !m_pMatineeBlender || Physics == PHYS_RigidBody )
	{
		if( m_pMatineeBlender )
		{
			m_pMatineeBlender->SetEnable( FALSE );
		}
	}
	else
	{
		m_pMatineeBlender->BlendNow( BlendOutTime, TRUE );
	}
	FZDIAG( TEXT("MAT_BlendOut %s: blendOut %.2f folder %d blender %s"), Group ? *Group->GroupName.ToString() : TEXT("?"), BlendOutTime, bFolderGroup, m_pMatineeBlender ? TEXT("yes") : TEXT("none") );
}

// DISHONORED(port): 2012 0x6fb030
UBOOL ADishonoredPlayerPawn::MAT_IsUnequipped() const
{
	return HasNothingEquipped() && !IsHoldingMovableObject();
}

/**
 * DISHONORED(port): 2012 0x6fe570. When the matinee asks for empty hands and the master FSM is not in the dialog state:
 * the re-equip memory of both hands is set to UDishonoredItemEmpty (UDishonoredInventory::SetReequipItem 0x852a90), a
 * held movable is dropped, and UDishonoredItemEmpty is equipped in both hands (EquipItemByType_KeepTrying 0x7afbe0).
 * Agent GJ (PHASE20 GJ) ported both callees; the body is retail's whole.
 */
void ADishonoredPlayerPawn::MAT_SetForceInventory( UBOOL bShouldForceEmptyHand )
{
	if( DisGJDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): gjdiag MAT_SetForceInventory( %d ) t=%.2f, in dialog %d"), (INT)bShouldForceEmptyHand, GWorld ? GWorld->GetTimeSeconds() : 0.f, (INT)( m_pPlayerMasterFSM && m_pPlayerMasterFSM->IsCurState( UStatePlayerMasterInDialog::StaticClass(), TRUE ) ) );
	}
	if( !bShouldForceEmptyHand )
	{
		return;
	}
	if( m_pPlayerMasterFSM && m_pPlayerMasterFSM->IsCurState( UStatePlayerMasterInDialog::StaticClass(), TRUE ) )
	{
		return;
	}
	// agent GJ (PHASE20 GJ): the two callees exist now (2013 0x6a4d40 checked: the same order - the two re-equip memories, the
	// movable drop, the two keep-trying equips of the empty hand)
	if( m_pInventory )
	{
		m_pInventory->SetReequipItem( EDisEquipUsage_Primary, UDishonoredItemEmpty::StaticClass() );
		m_pInventory->SetReequipItem( EDisEquipUsage_Secondary, UDishonoredItemEmpty::StaticClass() );
	}
	if( IsHoldingMovableObject() )
	{
		DropMovableObject( FALSE );
	}
	EquipItemByType_KeepTrying( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Primary, EEquipItemFlags_None );
	EquipItemByType_KeepTrying( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Secondary, EEquipItemFlags_None );
}

// DISHONORED(port): 2013 0x6a1cf0 (2012 0x704d50 without the last argument): the hand's DisableArmFollow influence weighted 1 or 0
void ADishonoredPlayerPawn::DisableArmFollow( BYTE Usage, UBOOL bDisable, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame )
{
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( !Camera || ( Usage != EDisEquipUsage_Primary && Usage != EDisEquipUsage_Secondary ) )
	{
		return;
	}
	UDishonoredCamera_DisableArmFollow* Influence = Camera->GetCamera_DisableArmFollow( Usage );
	if( Influence )
	{
		Influence->SetWeight( bDisable ? 1.f : 0.f, BlendTime, bWaitToBlendOneFrame );
	}
}

/**
 * DISHONORED(bringup): 2012 0x7048c0 - for every entry of m_ActivePowers, if it IsActive (vtable slot 87) then Cancel
 * (slot 91). m_ActivePowers is the SDK dump's untyped 12-byte gap at 1928 and UDishonoredActivePowerComponent has no
 * bodies, so there is nothing to cancel yet; logged once.
 */
void ADishonoredPlayerPawn::CancelAllActivePowers() const
{
	FZDIAG_ONCE( TEXT("CancelAllActivePowers (2012 0x7048c0) is not ported: the active power components have no bodies") );
}

// ---- agent GF (PHASE19 GF): the player's root-motion mode overrides (the coordinator's message from GB) ----

// DISHONORED(port): 2012 0x6fae70 / 2013 0x6ab1d0. The rotation mode goes straight to the mesh; a mode other than RMM_Translate
// is queued in the mesh's PendingRMM and restores the physics a RMM_Translate parked (m_NextPhysicsMode); RMM_Translate
// remembers the current physics, parks the pawn in PHYS_None and clears the pending mode. 2013 also stops an active
// UDishonoredActivePowerComponent_Blink before parking (the power components have no bodies here - named once).
void ADishonoredPlayerPawn::ChangeAnimStateMotionModes( BYTE _RootMotion, BYTE _RootRotation )
{
	if( !Mesh )
	{
		return;
	}
	Mesh->RootMotionRotationMode = _RootRotation;
	if( _RootMotion != RMM_Translate )
	{
		Mesh->PendingRMM = _RootMotion;
		if( m_NextPhysicsMode != PHYS_None )
		{
			setPhysics( m_NextPhysicsMode, NULL, FVector( 0.f, 0.f, 1.f ) );
			m_NextPhysicsMode = PHYS_None;
		}
	}
	else
	{
		if( Physics != PHYS_None )
		{
			FZDIAG_ONCE( TEXT("ChangeAnimStateMotionModes( RMM_Translate ): the 2013 stop of an active Blink power is not ported (no power component bodies)") );
			m_NextPhysicsMode = Physics;
			setPhysics( PHYS_None, NULL, FVector( 0.f, 0.f, 1.f ) );
		}
		Mesh->PendingRMM = RMM_Translate;
	}
}

// DISHONORED(port): 2012 0x6facb0 / 2013 0x6a1af0 (15 B): the player leaves root motion by ignoring it, not by the base's RMM_Velocity
void ADishonoredPlayerPawn::StopRootMotionMode()
{
	ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore );
}

// ---- agent IF (PHASE23 IF): the physics hook that drives the master FSM, and the action request ----

/**
 * DISHONORED(port): 2012 0x6fe380 / 2013 0x6a4b50 (424 B both). While the mesh is in (or is about to enter) RMM_Translate, or
 * while possessing, a physics change is parked in m_NextPhysicsMode and the pawn keeps its physics (ChangeAnimStateMotionModes
 * restores it). When the physics did change the master FSM is asked for the matching state: Walking -> Walk, Falling ->
 * Falling, Swimming -> Swim.
 * DISHONORED(written): retail asks ADishonoredPawn::IsPossessing (2012 0x7aac60, 10 B), which forwards to the player's
 * DisIsPossessing; the player's own is called here. The master FSM pointer is tested (a pawn whose PreBeginPlay_Actions found no
 * subobject, agent FZ's warning); retail's always exists.
 */
void ADishonoredPlayerPawn::setPhysics( BYTE NewPhysics, AActor* NewFloor, FVector NewFloorV )
{
	const BYTE OldPhysics = Physics;
	UBOOL bPark = FALSE;
	if( Mesh )
	{
		const BYTE RootMotionMode = ( Mesh->PendingRMM != Mesh->OldPendingRMM ) ? Mesh->PendingRMM : Mesh->RootMotionMode;
		bPark = ( RootMotionMode == RMM_Translate );
	}
	if( bPark || ( DisIsPossessing() && NewPhysics != PHYS_None ) )
	{
		m_NextPhysicsMode = NewPhysics;
		Super::setPhysics( Physics, NewFloor, NewFloorV );
	}
	else
	{
		Super::setPhysics( NewPhysics, NewFloor, NewFloorV );
	}
	if( OldPhysics == Physics || m_pPlayerMasterFSM == NULL )
	{
		return;
	}
	if( Physics == PHYS_Walking )
	{
		FStatePlayerMasterWalk_Param MasterParam;
		m_pPlayerMasterFSM->RequestStateChange( MasterParam, NULL, FALSE );
	}
	else if( Physics == PHYS_Falling )
	{
		FStatePlayerMasterFalling_Param MasterParam;
		m_pPlayerMasterFSM->RequestStateChange( MasterParam, NULL, FALSE );
	}
	else if( Physics == PHYS_Swimming )
	{
		FStatePlayerMasterSwim_Param MasterParam;
		m_pPlayerMasterFSM->RequestStateChange( MasterParam, NULL, FALSE );
	}
}

/**
 * DISHONORED(port): 2012 0x704ad0 / 2013 0x6aad90 (91 B). The request goes to the machine unless its logical state is a player
 * action that refuses the new action (a death action is never refused).
 * DISHONORED(bringup): two named gaps. The upper and left-arm machines are never initialised in this tree
 * (PreBeginPlay_Actions, agent FZ), and a request to a machine with no state would leave it pending a class it does not
 * hold, so such a request is refused here. UStateSharedActionBase_PlayerOnly::CanTransitionAction (2012 0x692a80) is not
 * ported; no player-action state can be the logical state of an initialised machine here, and if one ever is the request is
 * refused rather than guessed.
 */
UBOOL ADishonoredPlayerPawn::RequestActionHelper( UDishonoredNativeStateMachine* FSM, UBOOL bIsDeathAction, FStateSharedActionBase_PlayerOnly_Param& ActionParam, UObject* FiringObject, UBOOL bCheckOnly )
{
	if( FSM == NULL || FSM->GetCurrentlyActiveState() == NULL )
	{
		IFDIAG( TEXT("action request %s refused: its machine is not initialised"), *ActionParam.m_pCurAction->m_ActionName.ToString() );
		IFDIAG_ONCE( TEXT("RequestAction: the upper-body and left-arm FSMs are not initialised (UStatePlayerUpper*/UStatePlayerAction skeletons); the action is not requested") );
		return FALSE;
	}
	if( Cast<UStateSharedActionBase_PlayerOnly>( FSM->GetLogicalState() ) && !bIsDeathAction )
	{
		IFDIAG_ONCE( TEXT("RequestActionHelper: UStateSharedActionBase_PlayerOnly::CanTransitionAction (2012 0x692a80) is not ported; the request is refused") );
		return FALSE;
	}
	return FSM->RequestStateChange( ActionParam, FiringObject, bCheckOnly );
}

/**
 * DISHONORED(port): 2012 0x70bb90 / 2013 0x6b1d00 (254 B both). A left-hand action is refused while the left hand is disabled.
 * The full-body usage goes to the master machine (a UStatePlayerMasterDead request is a death action), the upper-body and
 * left-hand usages to their machines; a granted request invalidates the matching anim-state picker.
 * DISHONORED(bringup): UDisAnimStateComponent::InvalidateCurAnimState is not ported (agent FZ); named, not called.
 */
UBOOL ADishonoredPlayerPawn::RequestAction( BYTE ActionUsage, FStateSharedActionBase_PlayerOnly_Param& ActionParam, UObject* FiringObject, UBOOL bCheckOnly )
{
	if( ActionParam.m_pCurAction->m_bActionUsesLeftHand && m_bLeftHandDisabled )
	{
		return FALSE;
	}
	UBOOL bResult = FALSE;
	if( ActionUsage == eDisPlayerActionUsage_Fullbody )
	{
		const UBOOL bIsDeathAction = ActionParam.m_pStateClass && ActionParam.m_pStateClass->IsChildOf( UStatePlayerMasterDead::StaticClass() );
		bResult = RequestActionHelper( m_pPlayerMasterFSM, bIsDeathAction, ActionParam, FiringObject, bCheckOnly );
	}
	else if( ActionUsage == eDisPlayerActionUsage_Upperbody || ActionUsage == eDisPlayerActionUsage_LeftHand )
	{
		bResult = RequestActionHelper( ActionUsage == eDisPlayerActionUsage_Upperbody ? m_pPlayerUpperFSM : m_pPlayerLeftArmFSM, FALSE, ActionParam, FiringObject, bCheckOnly );
	}
	if( bResult && !bCheckOnly )
	{
		IFDIAG_ONCE( TEXT("RequestAction: UDisAnimStateComponent::InvalidateCurAnimState is not ported") );
	}
	return bResult;
}
