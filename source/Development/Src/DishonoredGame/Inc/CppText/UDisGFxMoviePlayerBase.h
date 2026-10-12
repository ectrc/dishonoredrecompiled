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
