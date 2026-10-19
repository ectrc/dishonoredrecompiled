// UDisAnimStateComponent cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The anim-state component of a pawn or an item: one FDisAnimState_Picker per state
// picker node (full body, upper body, special), the state each picker holds, the parent / child lockout, the custom-sequence alias
// cache and a twelve-entry history of the sequences played. 2012 / 2013 rvas: BindToAnimStateOwner 0x6b1ab0, InvalidateCurAnimState
// 0x6bd8a0 / 0x691680, GetCurAnimState 0x6bd920 / 0x688870, AddAnimStatePicker 0x6bd970, GetAnimStatePicker 0x6bd980 / 0x688900,
// IsCurAnimStateFullWeight 0x6bd9e0 / 0x691700, FindNodeInCurrentAnimStateByName 0x6bda90 / 0x6917b0, LogSeqWithHistory 0x6bdb10 /
// 0x688960, LockoutChildPickers 0x6c0a80 / 0x691410, ClearAnimState 0x6c0c20 / 0x6915b0, CleanupAnimStates 0x6c3350 / 0x69d850,
// LockPickerChildren 0x6c33d0, UnlockPickerChildren 0x6c3430, UnlockPicker 0x6c3490 / 0x69a130, SetAnimState 0x6cc0d0 / 0x69f610.
// Bodies in Src/disanimstatecomponent.cpp.
public:
	void BindToAnimStateOwner( const TScriptInterface<class IDisAnimStateOwnerInterface>& Owner ) { m_pAnimStateOwner = Owner; }
	void InvalidateCurAnimState( INT StateType );
	const struct FDisAnimState* GetCurAnimState( INT StateType ) const;
	void AddAnimStatePicker( const struct FDisAnimState_Picker& Picker ) { m_StatePickers.AddItem( Picker ); }
	struct FDisAnimState_Picker* GetAnimStatePicker( INT StateType );
	UBOOL IsCurAnimStateFullWeight( INT StateType );
	class UAnimNode* FindNodeInCurrentAnimStateByName( INT StateType, const FName& NodeName ) const;
	void LogSeqWithHistory( INT StateType, const FName& SeqName );
	void ClearAnimState( INT StateType, FLOAT BlendTime );
	void CleanupAnimStates( UBOOL bFullDestroy );
	void LockPickerChildren( INT StateType, FLOAT BlendTime );
	void UnlockPickerChildren( INT StateType, FLOAT BlendTime );
	void UnlockPicker( INT StateType, FLOAT BlendTime );
	BYTE SetAnimState( struct FDisAnimState* State, INT StateType, class UDishonoredNativeState* AttachToState, FLOAT BlendTime, UObject* FiringObj );
private:
	void LockoutChildPickers( struct FDisAnimState_Picker& CurPicker, UBOOL bLocked, FLOAT BlendTime );
	class IDisAnimStateOwnerInterface* GetOwnerInterface() const { return m_pAnimStateOwner.GetObject() ? (class IDisAnimStateOwnerInterface*)m_pAnimStateOwner.GetInterface() : NULL; }
public:
