// UInterpTrackInstAkEvent cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): agent IH3 (fork G). 2012 rvas (2013 in the bodies): InitTrackInst 0x5f5e30, TermTrackInst 0x5f67d0,
// UpdateTrackInst 0x5fb9f0. Bodies in akaudioclasses.cpp.
#if DISHONORED_WITH_WWISE
public:
	virtual void InitTrackInst( UInterpTrack* Track );
	virtual void TermTrackInst( UInterpTrack* Track );
	virtual void UpdateTrackInst( FLOAT NewPosition, UBOOL bJump );
#endif // DISHONORED_WITH_WWISE
