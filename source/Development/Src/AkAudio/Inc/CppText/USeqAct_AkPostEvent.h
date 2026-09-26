// USeqAct_AkPostEvent cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudioclasses.cpp. 2013 rvas: Activated 0x5b3810, UpdateOp 0x5b3860,
// FinishDestroy 0x5afac0, PlayEventOnTargets 0x5b31b0, GameSave 0x5ae8a0.
#if DISHONORED_WITH_WWISE
public:
	virtual void Activated();
	virtual UBOOL UpdateOp( FLOAT DeltaTime );
	virtual void FinishDestroy();
private:
	void PlayEventOnTargets();
#endif
