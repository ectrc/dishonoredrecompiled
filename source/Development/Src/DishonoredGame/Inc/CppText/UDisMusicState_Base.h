// UDisMusicState_Base cpptext: included inside the generated class body (DishonoredGameMusicClasses.h).
// DISHONORED(port): agent IH4. 2012 / 2013 rvas (dismusicstate_base.cpp): TickState 0x8af220, OnExitState 0x8af270,
// RegisterDelegate_TickCallback 0x8b1720, FDisMusicState_Base_Param::OnPending 0x8b8b00. Bodies in Src/dismusicstate_base.cpp.
public:
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	void RegisterDelegate_TickCallback( UObject* _pOwner, const FString& _Suffix );
