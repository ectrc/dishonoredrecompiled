// UDisActivePowerComponent_DarkVision cpptext: included inside the generated class body.
// DISHONORED(port): 2013 rva 0x7e7000 (2012 0x824900), byte-identical in both builds - the power's post-process is
// active while it is fading in or running, and the fade-out timer is deliberately not part of the test.
public:
	UBOOL IsPpActive() const { return m_fFadeInTimeRemaining > 0.0f || m_fTimeRemaining > 0.0f; }

	// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x7e8b70 (GameSave) and 0x7f8620 (GameLoad). RETAIL has
	// two bodies here; the 2012 build folds both slots onto one (0x828730), so vtables.csv reports a fold that
	// retail does not have. The two read the same four values, which is why one ported body serves both.
	// Bodies in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
