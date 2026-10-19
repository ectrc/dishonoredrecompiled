// IDisAnimStateFiringInterface cpptext: included inside the generated interface body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The object an anim state was fired from (a player action state, an item context, the
// owner itself) hears the state's animation events first: IDisAnimStateOwnerInterface::AnimState_PlayAnim / TriggerExit / Advance
// / AnimNotFound (2012 0x6b1af0..0x6b1bc0) call the firing object's _Derived and fall back to the owner's when it answers FALSE.
// Retail slots are pure; the defaults here answer FALSE (the owner handles it) because most implementing classes are still
// comment-only units. AnimState_LockoutPicker_Derived is the owner's half of IDisAnimStateOwnerInterface::AnimState_LockoutPicker.
public:
	virtual UBOOL AnimState_PlayAnim_Derived( class UAnimNodeSequence* Seq ) { return FALSE; }
	virtual UBOOL AnimState_TriggerExit_Derived( INT StateType, class UDishonoredNativeState* AttachedState, FLOAT TimeToGo ) { return FALSE; }
	virtual UBOOL AnimState_Advance_Derived( FLOAT TimeToGo ) { return FALSE; }
	virtual UBOOL AnimState_AnimNotFound_Derived( const FString& SeqName ) { return FALSE; }
	virtual void AnimState_LockoutPicker_Derived( INT StateType, FLOAT TimeToGo, UBOOL bLocked ) {}
	// DISHONORED(written): UObject::GetInterfaceAddress through the class's VfTable property, except for the player-side classes
	// this package drives (the player pawn, the player action and upper states), which use their C++ base. Body in
	// Src/disanimstateownerinterface.cpp.
	static IDisAnimStateFiringInterface* FromObject( UObject* Object );
