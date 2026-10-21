/*=============================================================================
	stateplayermasterparams.h: agent FZ (PHASE18 FZ) - the FDisNativeStateParam carriers the player master FSM is
	entered with. FStatePlayerMasterSoiree_Param is what ADishonoredPlayerPawn::MAT_BeginPlayerGroup builds on the
	stack (2012 0x70fcc0 / 2013 0x6ab570): the matinee, the blend-in time and the flag word, copied onto the pending
	UStatePlayerMasterSoiree by OnPending (2012 0x6c5f80; the 2013 body is unnamed in the database). The base
	FDisNativeStateParam is DishonoredGameNative.h's.
=============================================================================*/
#ifndef _INC_STATEPLAYERMASTERPARAMS
#define _INC_STATEPLAYERMASTERPARAMS

/**
 * The base of every player master state's parameters: OnPending (2012 0x6959c0, 20 B, ICF-folded with
 * FDisUseState_Base_Param's) writes the machine's managed object - the player pawn - into the pending state's
 * m_pPlayerPawn (UStatePlayerMasterBase +64). Without it no master state knows its pawn.
 */
struct FStatePlayerMasterBase_Param : public FDisNativeStateParam
{
	FStatePlayerMasterBase_Param( UClass* StateClass = NULL ) : FDisNativeStateParam( StateClass ) {}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterBase_Param); }
};

struct FStatePlayerMasterSoiree_Param : public FStatePlayerMasterBase_Param
{
	class USeqAct_Interp* m_pMatinee;
	FLOAT m_fBlendInTime;
	INT m_Flags;

	FStatePlayerMasterSoiree_Param( class USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterSoiree_Param); }
};

/** The default state's parameters: retail's PreBeginPlay_Actions passes the base struct with UStatePlayerMasterWalk's class. */
struct FStatePlayerMasterWalk_Param : public FStatePlayerMasterBase_Param
{
	FStatePlayerMasterWalk_Param();
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterWalk_Param); }
};

/**
 * agent IF (PHASE23 IF). The jump, falling and swim requests: retail builds each on the stack with the base vtable
 * (/OPT:ICF folds all three onto FStatePlayerMasterSwim_Param's, 2012 ctor 0x693cc0 / 2013 0x66dc80) and the state's class.
 */
struct FStatePlayerMasterJump_Param : public FStatePlayerMasterBase_Param
{
	FStatePlayerMasterJump_Param();
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterJump_Param); }
};

struct FStatePlayerMasterFalling_Param : public FStatePlayerMasterBase_Param
{
	FStatePlayerMasterFalling_Param();
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterFalling_Param); }
};

struct FStatePlayerMasterSwim_Param : public FStatePlayerMasterBase_Param
{
	FStatePlayerMasterSwim_Param();
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterSwim_Param); }
};

/**
 * agent IF (PHASE23 IF). The upper-body / left-arm action request ADishonoredPlayerPawn::RequestAction takes: 2012
 * FStateSharedActionBase_PlayerOnly_Param ctor 0x695a20 (the action pointer at +8) and FStatePlayerAction_Param ctor
 * 0x6c4330 / 2013 0xa71690 (UStatePlayerAction's class). agent IM (PHASE23 IM): the OnPendings - 2012 0x695a40 (the pending action
 * state is told its pawn and action; a new action resets the non-repeat helper) and FStatePlayerAction_Param 0x6b4580 (also its
 * player pawn and action machine). Bodies in Src/statesharedactionbase_playeronly.cpp and Src/stateplayeraction.cpp.
 */
struct FStateSharedActionBase_PlayerOnly_Param : public FDisNativeStateParam
{
	const FPawnAction* m_pCurAction;

	FStateSharedActionBase_PlayerOnly_Param( const FPawnAction& Action ) : FDisNativeStateParam( NULL ), m_pCurAction( &Action ) {}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStateSharedActionBase_PlayerOnly_Param); }
};

struct FStatePlayerAction_Param : public FStateSharedActionBase_PlayerOnly_Param
{
	FStatePlayerAction_Param( const FPawnAction& Action ) : FStateSharedActionBase_PlayerOnly_Param( Action )
	{
		m_pStateClass = UStatePlayerAction::StaticClass();
	}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerAction_Param); }
};

/** agent IM (PHASE23 IM). The ready-stance change request (2012 ctor 0x6c6190 / 2013 0x673d40, OnPending 0x6b45b0): the stance to store. */
struct FStatePlayerChangeReadyStance_Param : public FStatePlayerAction_Param
{
	BYTE m_GoToStance;

	FStatePlayerChangeReadyStance_Param( const FPawnAction& Action, BYTE GoToStance );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerChangeReadyStance_Param); }
};

/**
 * agent IM (PHASE23 IM). The upper-body / left-arm machines' idle and navigation states: FStatePlayerUpperBase_Param::OnPending
 * (2012 0x6b4560 / 2013 0x664230) tells the pending state its action machine and pawn; the idle param's ctor is 2012 0x6c3ac0, the
 * navigation one 0x6c3af0 (both use the idle param's vtable). Body in Src/stateplayerupperidle.cpp.
 */
struct FStatePlayerUpperBase_Param : public FDisNativeStateParam
{
	FStatePlayerUpperBase_Param( UClass* StateClass ) : FDisNativeStateParam( StateClass ) {}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerUpperBase_Param); }
};

struct FStatePlayerUpperIdle_Param : public FStatePlayerUpperBase_Param
{
	FStatePlayerUpperIdle_Param() : FStatePlayerUpperBase_Param( UStatePlayerUpperIdle::StaticClass() ) {}
	virtual INT SizeOf() const { return sizeof(FStatePlayerUpperIdle_Param); }
};

struct FStatePlayerActionNav_Param : public FStatePlayerUpperBase_Param
{
	FStatePlayerActionNav_Param() : FStatePlayerUpperBase_Param( UStatePlayerUpperNav::StaticClass() ) {}
	virtual INT SizeOf() const { return sizeof(FStatePlayerActionNav_Param); }
};

/**
 * agent IG (PHASE23 IG). The master FSM's action request (2012 FStatePlayerMasterAction_Param ctor 0x6a5d60, OnPending 0x695c10: the
 * pending action state is told its pawn and action, the non-repeat helper reset when the action changes) and the mantle's
 * (FStatePlayerMasterMantle_Param ctor 0x6c3b20 / 2013 0x673020, OnPending 0x6b3980 / 2013 0x663620 - the 2013 one inlines the
 * action OnPending: the edge height, the start location / yaw as the intro target, the impact / crouch / cancel flags). Bodies in Src/stateplayermastermantle.cpp.
 */
struct FStatePlayerMasterAction_Param : public FStateSharedActionBase_PlayerOnly_Param
{
	FStatePlayerMasterAction_Param( const FPawnAction& Action ) : FStateSharedActionBase_PlayerOnly_Param( Action )
	{
		m_pStateClass = UStatePlayerMasterAction::StaticClass();
	}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterAction_Param); }
};

struct FStatePlayerMasterMantle_Param : public FStatePlayerMasterAction_Param
{
	FLOAT m_fCurrentMantleHeight;
	FVector m_MantleStartLocation;
	INT m_MantleStartYaw;
	UBOOL m_bImpactAnimRequired;
	UBOOL m_bCrouchRequired;
	UBOOL m_bCancelAllowed;

	FStatePlayerMasterMantle_Param( const FPawnAction& Action, FLOAT CurrentMantleHeight, const FVector& MantleStartLocation, INT MantleStartYaw,
		UBOOL bImpactAnimRequired, UBOOL bCrouchRequired, UBOOL bCancelAllowed );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterMantle_Param); }
};

/**
 * agent JE (PHASE24 JE). The dialog request ADishonoredPlayerPawn::StartDialog builds (2013 ctor 0x6715a0: the running instance and,
 * new in 2013, the carry-body flag; 2012 0x6c3e40 had the instance only) and its OnPending (2013 0x6693a0 / 2012 0x6b8ab0): the pawn,
 * a running view track stopped, the instance and the flag stored, no track, the soiree blend-out back to 0.4 s. Body in
 * Src/stateplayermasterindialog.cpp.
 */
struct FStatePlayerMasterInDialog_Param : public FStatePlayerMasterBase_Param
{
	struct FDisDialogRunningInstance* m_pDialogRunningInst;
	UBOOL m_bAllowPlayerToCarryBody;

	FStatePlayerMasterInDialog_Param( struct FDisDialogRunningInstance* DialogRunningInst, UBOOL bAllowPlayerToCarryBody );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FStatePlayerMasterInDialog_Param); }
};

/** The flag word MAT_BeginPlayerGroup builds and FDisSoireeControlState keeps in m_Flags (2013 bits; 2012 had the first three). */
enum EDisSoireeControlFlags
{
	SOIREE_DisablePlayerCollision = 0x2,
	SOIREE_HasMoveTrack = 0x4,
	SOIREE_HasAnimControlTrack = 0x8,
	SOIREE_Reentered = 0x10,
	SOIREE_LeaveCameraToDialogTree = 0x20,
};

#endif
