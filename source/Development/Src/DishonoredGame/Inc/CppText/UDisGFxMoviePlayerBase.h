// UDisGFxMoviePlayerBase cpptext: included inside the generated class body.

// ---- agent EI (PHASE12 EI): the message box's id, its timer and the game event that answers it ----
public:
	// DISHONORED(port): agent EI, 2013 rva 0x7a4550. Asks the global UI for a box, keeps the id it is given and
	// subscribes to DisGameEventType_MessageBoxResult. Body in disgfxmovieplayerbase.cpp.
	void ShowMessageBox( const FString& _rMessage, const FString& _rButton0, const FString& _rButton1, const FString& _rButton2 );
	// DISHONORED(port): agent EI, 2013 rva 0x787780
	void HideMessageBox();
	// DISHONORED(port): agent EI, 2013 rva 0x787730
	void AddMessageBoxTimer( FLOAT _fDuration );
	// DISHONORED(port): agent EI, 2013 rva 0x793c70. The answer: when the id matches, the movie's own
	// _root.MessageBoxInvoke.OnMessageBoxClosed(<button>) runs the callback the content stored.
	virtual void OnMessageBoxResult( const class FArkGameEvent& _rEvent );
	// DISHONORED(port): agent EI, 2013 rvas 0x7877b0 and 0x79e820. The two attribute setters the message box
	// drives the global movie with; both flush the player's input when the movie gains what it did not have.
	void AllowFocus( UBOOL _bAllow );
	void AllowInput( UBOOL _bAllowInput, UBOOL _bCaptureInput );
	// DISHONORED(port): agent EI, 2013 rva 0x7a45c0 - drops the message-box subscription with the object
	virtual void BeginDestroy();

// ---- agent FP (PHASE16 FP): the movie package, loaded synchronously ----
public:
	// DISHONORED(port): 2013 rva 0x7a3fd0 (2012 0x81d0c0). The path, or the tweaks' package and movie name when
	// none is given; the package through GPackageFileCache and LoadPackage with the seek-free suffix; the
	// SwfMovie into MovieInfo, rooted, and its path into m_DynamicallyLoadedMoviePath. Body in
	// disgfxmovieplayerbase.cpp.
	UBOOL LoadMoviePackage( const TCHAR* _pMoviePath );
	// DISHONORED(port): 2013 rva 0x787720 (2012 0x7f42b0), retail vtable +436: the negation of m_bIsLoadingMoviePackage
	UBOOL HasFinishedAsyncLoading() const { return m_bIsLoadingMoviePackage ? FALSE : TRUE; }

// ---- agent FU (PHASE17 FU): the screen-to-movie map every PreRender uses ----
public:
	// DISHONORED(port): 2013 rva 0x787860 (2012 0x7f4390, same bytes). The movie at its authored size, the screen
	// letterboxed or pillarboxed around it. Body in disgfxmovieplayerbase.cpp.
	void ComputeMovieSpaceInfo( INT _ScreenSizeX, INT _ScreenSizeY, struct FDisMovieSpaceInfo& _rInfo );

// ---- agent HF (PHASE22 HF): the bound-key substitution every displayed hint goes through ----
public:
	// DISHONORED(port): 2013 rva 0x7a4620 (2012 0x8085b0, which had no force flags). `GBA_<action>`, `KEY_<key>` and
	// `k in the text become the key bound to the action. Body in disgfxmovieplayerbase.cpp.
	static void FormatInteractionText( FString& _rText, UBOOL _bForcePC, UBOOL _bForceGamepad );
	// DISHONORED(port): 2013 rva 0x78c590 (2012 0x7f5f30, same bytes)
	static void FormatGamepadKeyName( const TCHAR* _pKeyName, FString& _rOut );
	// DISHONORED(port): 2013 rva 0x78c630 (2012 0x7f5fd0 took the key's name; 2013 takes the key)
	static void FormatMouseKeyName( FName _Key, FString& _rOut );
	// DISHONORED(port): 2013 rva 0x793dc0 (2012 0x7fa350)
	static FString LocalizeKeyboardKeyName( FName _Key, UBOOL _bInGame );

// ---- agent JB (PHASE24 JB): the IDisTweaksInterface slots; ApplyTweakChanges_Derived fills SoundThemes ----
public:
	// DISHONORED(port): 2012 rva 0x81c4b0
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	// DISHONORED(port): 2012 vtable 0xd528c8 (a folded `return this->m_pTweaks`), 2012 rva 0x7f6030, 2012 rva 0x808a20.
	// Bodies in disgfxmovieplayerbase.cpp.
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	virtual void ApplyTweakChanges_Derived();
