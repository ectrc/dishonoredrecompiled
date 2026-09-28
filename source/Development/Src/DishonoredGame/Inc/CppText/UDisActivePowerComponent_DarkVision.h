// UDisActivePowerComponent_DarkVision cpptext: included inside the generated class body.
// DISHONORED(port): 2013 rva 0x7e7000 (2012 0x824900), byte-identical in both builds - the power's post-process is
// active while it is fading in or running, and the fade-out timer is deliberately not part of the test.
public:
	UBOOL IsPpActive() const { return m_fFadeInTimeRemaining > 0.0f || m_fTimeRemaining > 0.0f; }
