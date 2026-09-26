// USeqAct_AkSetRTPCValue cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudioclasses.cpp. 2013 rvas: Activated 0x5b0a10, UpdateOp 0x5b33d0, SetRTPCValue 0x5b2730.
#if DISHONORED_WITH_WWISE
public:
	virtual void Activated();
	virtual UBOOL UpdateOp( FLOAT DeltaTime );
private:
	void SetRTPCValue();
#endif
