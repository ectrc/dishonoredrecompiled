// IDisAnimStateOwnerInterface cpptext: included inside the generated interface body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The owner of an anim-state component (the player pawn, an inventory item). The four
// dispatchers (2012 0x6b1af0 / 0x6b1b30 / 0x6b1b80 / 0x6b1bc0, 2013 0x67d560 / 0x67d5a0 / 0x67d5f0 / 0x67d630) ask the firing
// object first and the owner when it answers FALSE; AnimState_LockoutPicker (2012 0x6c0d40 / 2013 0x691830) sets the picker's
// lockout bit, tells the owner and clears a locked picker. Bodies in Src/disanimstateownerinterface.cpp. AnimState_GetSkeletalComponent
// (slot +28) and AnimState_GetAnimStateComp (slot +32) are pure in retail; defaults here for the comment-only owners.
public:
	virtual class USkeletalMeshComponent* AnimState_GetSkeletalComponent() { return NULL; }
	virtual class UDisAnimStateComponent* AnimState_GetAnimStateComp() { return NULL; }
	UBOOL AnimState_PlayAnim( IDisAnimStateFiringInterface* FiringObj, class UAnimNodeSequence* Seq );
	UBOOL AnimState_TriggerExit( IDisAnimStateFiringInterface* FiringObj, INT StateType, class UDishonoredNativeState* AttachedState, FLOAT TimeToGo );
	UBOOL AnimState_Advance( IDisAnimStateFiringInterface* FiringObj, FLOAT TimeToGo );
	UBOOL AnimState_AnimNotFound( IDisAnimStateFiringInterface* FiringObj, const FString& SeqName );
	void AnimState_LockoutPicker( INT StateType, FLOAT TimeToGo, UBOOL bLocked );
	// DISHONORED(written): as IDisAnimStateFiringInterface::FromObject (the player pawn by its C++ base)
	static IDisAnimStateOwnerInterface* FromObject( UObject* Object );
