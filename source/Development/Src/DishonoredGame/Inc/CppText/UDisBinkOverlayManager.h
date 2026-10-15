// UDisBinkOverlayManager cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent HG (PHASE22 HG). The loading-movie overlays (map name, hints, LOADING, PRESS ANY KEY, chapter title and
// description, save notification). 2013 vtable +292 Tick, +296 OnBinkTick, +300 OnBinkRenderFrame, +304 LoadContentPackage,
// +308 OnLoadingMovieStopped (UArkBinkOverlayManager's five virtuals). Bodies in disbinkoverlaymanager.cpp.
public:
	void Initialize();
	void CleanUp();
	void OnPlayLoadingMovie( const FString& _rMapName, UBOOL _bHasMapConfig, UBOOL _bShowMapNameAndHints );
	void OnPlaySaveNotificationMovie();
	virtual void Tick();
	virtual void OnBinkTick( FLOAT _fDeltaTime );
	virtual void OnBinkRenderFrame( FViewport* _pViewport, FCanvas* _pCanvas );
	virtual void LoadContentPackage();
	virtual void OnLoadingMovieStopped();

protected:
	void OnRenderLoadingMovie( FViewport* _pViewport, FCanvas* _pCanvas );
	void DrawTextOverlay( FCanvas* _pCanvas, const FDisBinkTextOverlayInfo& _rInfo, const TCHAR* _pText, UFont* _pFont, const FLinearColor* _pColorMultiplier, FVector2D* _pOutTextSize );
	INT GetRandomHintIndex() const;
	void ChangeHint();
	FFullScreenMovieSupport* GetBinkMovie() const { return (FFullScreenMovieSupport*)m_pBinkMovie; }
	FCriticalSection* GetCriticalSection() const { return (FCriticalSection*)m_pCriticalSection; }
