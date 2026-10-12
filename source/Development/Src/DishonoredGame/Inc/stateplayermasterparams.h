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
