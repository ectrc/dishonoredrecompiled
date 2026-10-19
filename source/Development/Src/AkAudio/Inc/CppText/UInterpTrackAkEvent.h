// UInterpTrackAkEvent cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): agent IH3 (fork G). 2012 rvas (2013 in the bodies): PostLoad 0x5f7060, GetKeyProperties 0x5f6780,
// GetAkEventKeyAtPosition 0x5f66c0. Bodies in akaudioclasses.cpp.
#if DISHONORED_WITH_WWISE
public:
	virtual void PostLoad();
	virtual UObject* GetKeyProperties( INT KeyIndex );
	FAkEventTrackKey& GetAkEventKeyAtPosition( FLOAT InPosition );
#endif // DISHONORED_WITH_WWISE
