// FDisSubtitlesInfo cpptext: included inside the generated struct body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent ID (PHASE23 ID). The conversation manager's subtitle lists: main lines (a conversation's own), secondary
// lines (barks heard from afar) and main lines paused with their owner. 2013 rvas: AddSubtitle 0x89cea0*, KillSubtitle
// 0x89d160*, Tick 0x89e660* (TickGlobalConvMan's tail call), the three index finders 0x88d5a0* / 0x88d610* / 0x88d680*
// (2012 AddSubtitle 0x8ecaa0, KillSubtitle 0x8ecd00, Tick 0x8ecd80, findMain/SecondarySubtitleIndex 0x8df3c0 / 0x8df430).
// Bodies in disconvglobalman.cpp.
	void AddSubtitle( class UDisConversationComponent* _pConvComp, const TArray<struct FDisAkCompAndPlayingID>& _rSpeakers, const TCHAR* _pText,
		FLOAT _fVoiceRadius, UBOOL _bMainSubtitle );
	void KillSubtitle( class UDisConversationComponent* _pConvComp );
	void Tick( FLOAT _fDeltaSeconds, const class UDisConversation_InGameData* _pDominantConvo, BYTE _DominantHook );
	INT FindMainSubtitleIndex( const class UDisConversationComponent* _pConvComp ) const;
	INT FindSecondarySubtitleIndex( const class UDisConversationComponent* _pConvComp ) const;
	INT FindPausedSubtitleIndex( const class UDisConversationComponent* _pConvComp ) const;
