// UDisAISubStateMaintainDistance cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. Hold a ring around the focus: close in when outside the ideal distance, back off when
// inside it, look at it either way. Bodies in Src/disaisubstatemaintaindistance.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void OnExitState( class UDishonoredNativeState* NextState );
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

private:
	void SetupDesires();
	void MaintainDistance();
