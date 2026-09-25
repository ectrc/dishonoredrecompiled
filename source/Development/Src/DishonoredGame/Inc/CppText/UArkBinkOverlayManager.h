// UArkBinkOverlayManager cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// DISHONORED(port): Engine.ArkBinkOverlayManager is an Engine class in retail (IMPLEMENT_CLASS at disfullscreenmoviebink.cpp:13,
// 2012 rva 0x5744f0). Its five virtuals follow the UObject slots: 2013 vtable of UDisBinkOverlayManager (rva 0xd5d130) +292 Tick,
// +296 OnBinkTick, +300 OnBinkRenderFrame, +304 LoadContentPackage, +308 OnLoadingMovieStopped (the last two are non-virtual
// members of UDisBinkOverlayManager in the 2012 PDB). The Engine side (FDisFullScreenMovieBink, Engine/Bink/Src/
// DisFullScreenMovieBink.h) declares the same class body: keep both identical.
public:
	virtual void Tick() {}
	virtual void OnBinkTick(FLOAT DeltaTime) {}
	virtual void OnBinkRenderFrame(FViewport* Viewport, FCanvas* Canvas) {}
	virtual void LoadContentPackage() {}
	virtual void OnLoadingMovieStopped() {}
