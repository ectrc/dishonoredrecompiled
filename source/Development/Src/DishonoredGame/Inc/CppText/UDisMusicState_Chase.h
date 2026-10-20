// UDisMusicState_Chase cpptext: included inside the generated class body (DishonoredGameMusicClasses.h).
// DISHONORED(port): agent IH4. 2012 rvas: ChooseChaseIntensity 0x8b8ca0, OnEnterState 0x8bc780, TickState 0x8bc7a0,
// FDisMusicState_Chase_Param::OnPending 0x8b8c80. Bodies in Src/dismusicstate_chase.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void TickState( FLOAT DeltaSeconds );
private:
	void ChooseChaseIntensity();
