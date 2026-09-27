#pragma once
// GFxUI/inc/gfxuiengine.h
// PDB functions attributed to this file (6):
//   0x41630  public: static void __cdecl FGFxEngine::ConvertGFxToUProp(class UProperty *, unsigned char *, class GFxValue const &, class UGFxMoviePlayer *)
//   0x5b6ac0  public: __thiscall FAutoGFxValueArray::FAutoGFxValueArray(unsigned int, void *)
//   0x5b96d0  public: static int __cdecl FGFxEngine::ReplaceCharsInFString(class FString &, wchar_t const *, wchar_t)
//   0x5bf4b0  public: __thiscall FAutoGFxValueArray::~FAutoGFxValueArray(void)
//   0x5e4080  public: static void __cdecl FGFxEngine::ConvertUPropToGFx(class UProperty *, unsigned char *, class GFxValue &, class GFxMovieView *, bool)
//   0x60190  _FGFxEngine::ConvertGFxToUProp_::_49_::ObjVisitor::Visit
//
// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the engine seam - the object that owns the GFx loader, the renderer and the list
// of open movies, and the two per-frame entry points the game drives the interface from.
//
// This is the file agent BE's gfxui_gfx3.h stood in for while there was no runtime: with
// DISHONORED_GFXUI_GFX3_RUNTIME at 0 that header declared FGFxMovie, FAutoGFxValueArray and a
// members-only FGFxEngine, and Src/gfxuigfx3absent.cpp gave every entry point the neutral answer so
// the ported native layer linked. With the switch at 1 the declarations come from here and the bodies
// from Src/gfxuiengine.cpp, which is where the 2012 PDB attributes all 62 of them.
//
// Who calls what, read out of the retail database rather than assumed (build/agentDC/xr.py):
//   UGFxInteraction::Init (2013 0x5a2c40)             -> FGFxEngine::GetEngine
//   UGFxInteraction::Tick (2013 0x57b790)             -> FGFxEngine::Tick
//   UGFxInteraction::SetRenderViewport (0x57b770)     -> FGFxEngine::SetRenderViewport
//   UGFxInteraction::InputKey/InputChar/InputAxis     -> the three input entry points
//   UGameViewportClient::Draw (2013 0x2c59a0)         -> FGFxEngine::RenderUI, FGFxEngine::RenderTextures
//   UGFxMoviePlayer::Load / SetPriority               -> GetEngine, LoadMovie, InsertMovie
// So the per-frame path is an Interaction for the advance and the viewport client for the draw. That
// is worth stating because PHASE9.md's brief said the UI is driven from ADishonoredHUD::PostRender:
// the HUD's PostRender is the *canvas* half and it is a sibling of the RenderUI call, not its caller.
//
// The layout is not reproduced and is not reproducible in the useful sense: FGFxEngine is 0x20C bytes
// in retail (FGFxEngine::GetEngine's appMalloc(0x20C, 8), 2013 0x5a2b60) and nothing serialises it, so
// the members below are the ones the 62 bodies touch, in the order the constructor initialises them,
// without offset assertions. FGFxMovie *is* reproduced: it is 104 bytes (LoadMovie's appMalloc(0x68, 8))
// and the offsets the decompiles read - pView at 52, fVisible at 68, fUpdate at 72, pUMovie at 92 -
// all fall out of the member order below.
#include "gfxuirenderer.h"

class UGFxMoviePlayer;
class UGFxObject;
class UGFxInteraction;
class UTranslationContext;
class FGFxEngine;

// ---------------------------------------------------------------------------------------------
// FGFxMovie: the handle UGFxMoviePlayer::pMovie points at. 104 bytes.
struct FGFxMovie
{
	FString                 FileName;        // @0
	GFxMovieInfo            Info;            // @12
	GPtr<GFxMovieDef>       pDef;            // @48
	GPtr<GFxMovieView>      pView;           // @52
	DOUBLE                  LastTime;        // @56
	UBOOL                   Playing;         // @64
	UBOOL                   fVisible;        // @68
	UBOOL                   fUpdate;         // @72
	UBOOL                   fViewportSet;    // @76
	UBOOL                   bCanReceiveFocus;// @80
	UBOOL                   bCanReceiveInput;// @84
	INT                     TimingMode;      // @88
	UGFxMoviePlayer*        pUMovie;         // @92
	UTextureRenderTarget2D* pRenderTexture;  // @96
	FRenderCommandFence     RenderCmdFence;  // @100

	FGFxMovie();                                                                    // 2012 0x5bf9c0
};

// FGFxLocalPlayerState (ScaleformEngine.h): which movie has the keyboard for one local player.
struct FGFxLocalPlayerState
{
	FGFxMovie* FocusedMovie;

	FGFxLocalPlayerState() : FocusedMovie(NULL) {}
};

// FAutoGFxValueArray (2012 0x5b6ac0 / 0x5bf4b0): the stack-allocated GFxValue run the Invoke paths
// build their argument list in. The retail body places the values in caller stack memory through
// appAlloca; ours keeps the same contract - construct N values, destruct them all at scope end.
class FAutoGFxValueArray
{
public:
	FAutoGFxValueArray(UINT InCount, void* InMemory);
	~FAutoGFxValueArray();

	GFxValue& operator()(UINT Index) { return pValues[Index]; }
	GFxValue* GetValues() const { return pValues; }
	operator GFxValue*() const { return pValues; }

private:
	GFxValue* pValues;
	UINT Count;
};

/** the retail macro of gfxuiengine.h: N GFxValues in the caller's frame, constructed and destructed */
#define AutoGFxValueArray(Name,NumValues) \
	FAutoGFxValueArray Name((NumValues), (NumValues) ? appAlloca(sizeof(GFxValue) * (NumValues)) : NULL)

// ---------------------------------------------------------------------------------------------
// The three GFx states this layer implements that need the engine. The PDB attributes all three to
// this file, which is why they are declared here rather than beside the rest of the seam.
class FGFxURLBuilder : public GFxURLBuilder
{
public:
	virtual void BuildURL(GString* path, const GFxURLBuilder::LocationInfo& loc);   // 2013 0x585a50
};

class FGFxExternalInterface : public GFxExternalInterface
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* MethodName, const GFxValue* Args,
		UINT ArgCount);                                                             // 2013 0x58d510
};

class FGFxFSCommandHandler : public GFxFSCommandHandler
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* Command, const char* Args);// 2013 0x586450
};

// ---------------------------------------------------------------------------------------------
// FGFxEngine.
class FGFxEngineBase
{
public:
	FGFxEngineBase();                                                               // 2013 0x574e80
	virtual ~FGFxEngineBase();                                                      // 2013 0x5720d0
};

class FGFxEngine : public FGFxEngineBase
{
	FGFxEngine();                                                                   // 2013 0x5a2370

public:
	virtual ~FGFxEngine();                                                          // 2013 0x59f190

	/** the process-wide engine, created on demand. NULL never happens once this is linked, which is the
	    difference the runtime switch makes: every ported body in gfxuimovie.cpp checks it first. */
	static FGFxEngine* GetEngine();                                                 // 2013 0x5a2b60

	// --- the per-frame path ---
	void Tick(FLOAT DeltaTime);                                                     // 2013 0x57b140
	void RenderUI(UBOOL bRenderToSceneColor, INT DPG = SDPG_Foreground);            // 2013 0x58dd30
	void RenderTextures();                                                          // 2013 0x58e080
	void SetRenderViewport(FViewport* inViewport);                                   // 2013 0x57af00
	FViewport* GetRenderViewport() const { return HudViewport; }
	GFxLoader* GetLoader() { return &Loader; }

	// --- movies ---
	GFxMovieDef* LoadMovieDef(const TCHAR* Path, GFxMovieInfo& Info);               // 2013 0x594890
	FGFxMovie*   LoadMovie(const TCHAR* Path, UBOOL bInitFirstFrame = TRUE);        // 2012 0x5de3e0
	void StartScene(FGFxMovie* Movie, UTextureRenderTarget2D* RenderTexture = NULL,
		UBOOL fVisible = TRUE, UBOOL fUpdate = TRUE);                               // 2013 0x58e300
	void CloseScene(FGFxMovie* pMovie, UBOOL fDelete);                              // 2013 0x58dab0
	void CloseTopmostScene();                                                       // 2013 0x590ef0
	void CloseAllMovies(INT bOnlyCloseOnLevelChangeMovies);                          // 2013 0x590d90
	void CloseAllTextureMovies();                                                   // 2013 0x590e70
	void NotifyGameSessionEnded();                                                  // 2013 0x594750
	void DeleteQueuedMovies(UBOOL bWait = TRUE);                                    // 2013 0x58dbf0
	void InsertMovie(FGFxMovie* Movie, BYTE DPG);                                    // 2013 0x58d4a0
	void InsertMovieIntoList(FGFxMovie* Movie, TArray<FGFxMovie*>* List);            // 2013 0x585350
	INT  GetNextMovieDPG(INT FirstDPG);
	FGFxMovie* GetTopmostMovie() const;                                             // 2012 0x5bfb90
	FGFxMovie* GetOpenMovie(INT Index) const;                                       // 2013 0x57b2a0
	INT GetNumOpenMovies() const { return OpenMovies.Num(); }
	INT GetNumAllMovies() const { return AllMovies.Num(); }

	// --- focus and size ---
	INT GetLocalPlayerIndexFromControllerID(INT ControllerId, UINT bFailIfMissing = 0);// 2013 0x57b4f0
	FGFxMovie* GetFocusedMovieFromControllerID(INT ControllerId);                    // 2013 0x57b580
	FGFxMovie* GetFocusMovie(INT ControllerId) { return GetFocusedMovieFromControllerID(ControllerId); }
	void ReevaluateFocus();                                                         // 2013 0x586b60
	void ReevaluateSizes();                                                         // 2013 0x586b00
	void AddPlayerState();                                                          // 2012 0x5cf2a0
	void RemovePlayerState(INT Index);
	void SetMovieCanReceiveFocus(FGFxMovie* InMovie, UBOOL bCanReceiveFocus);
	void SetMovieCanReceiveInput(FGFxMovie* InMovie, UBOOL bCanReceiveInput);

	// --- input ---
	UBOOL InputKey(INT ControllerId, FName ukey, EInputEvent uevent);                // 2013 0x591470
	UBOOL InputChar(INT ControllerId, TCHAR Character);                              // 2013 0x5917c0
	UBOOL InputAxis(INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad);// 2013 0x594cc0
	void  FlushPlayerInput(TSet<INT>* pCaptureKeys);                                  // 2013 0x59a240
	void  InitKeyMap();                                                             // 2013 0x59f490
	void  UpdateKeyEmulation(FName A, FName B, FName C, FName D, FName E);            // 2013 0x5a0f30

	// --- the two property converters and the string helper ---
	static void ConvertUPropToGFx(UProperty* Property, BYTE* Address, GFxValue& Value,
		GFxMovieView* Movie, bool bOverwrite = false);                               // 2013 0x584150
	static void ConvertGFxToUProp(UProperty* Property, BYTE* Address, const GFxValue& Value,
		UGFxMoviePlayer* Movie);                                                     // 2013 0x216e10
	static INT ReplaceCharsInFString(FString& Text, const TCHAR* Chars, TCHAR Replacement);// 2013 0x574c20
	static UBOOL GetPackagePath(const char* ppath, FFilename& pkgpath);              // 2013 0x5857f0
	static FFilename CollapseRelativePath(const FFilename& InString);                // 2012 0x5c9240

	/** what the last RenderUI pass submitted, for the census line */
	struct FRenderCensus
	{
		INT Movies;
		INT DisplayObjects;
		INT Sprites;
		INT Shapes;
		INT TextFields;
		INT Images;
		INT Draws;
		INT Triangles;
		INT GlyphDraws;
		INT Glyphs;
		INT Masks;
	};
	const FRenderCensus& GetRenderCensus() const { return RenderCensus; }
	void LogCensus(const TCHAR* Reason);

	/** the render thread's own handle on the target RenderUI binds */
	FGFxRenderTarget* GetHudRenderTarget() const { return HudRenderTarget.GetPtr(); }
	FGFxRenderer* GetRenderer() const { return pRenderer.GetPtr(); }

	FRenderCommandFence RenderCmdFence;
	INT                 GameHasRendered;
	TArray<FGFxMovie*>  OpenMovies;
	TArray<FGFxMovie*>  AllMovies;
	TArray<FGFxLocalPlayerState> PlayerStates;

private:
	static void InitGFxLoaderCommon(GFxLoader& Loader);                              // 2013 0x590ba0
	void InitRenderer();
	void InitLocalization();
	/** -gfxuikey=<drawnframe>:<KeyName>: deliver a press and a release through the real path */
	void TickScriptedKeys();
	void SetMovieSize(FGFxMovie* Movie);                                             // 2013 0x57b300
	UBOOL InputKey(INT ControllerId, FGFxMovie* pFocusMovie, FName ukey, EInputEvent uevent);// 2013 0x590fd0
	UBOOL IsKeyCaptured(FName ukey);                                                 // 2013 0x5916b0

	FGFxEngine& operator=(const FGFxEngine&);
	FGFxEngine(const FGFxEngine&);

	GFxLoader             Loader;
	FViewport*            HudViewport;
	GPtr<FGFxRenderTarget> HudRenderTarget;
	GPtr<FGFxRenderTarget> SceneColorRT;
	GPtr<FGFxRenderer>    pRenderer;
	TArray<FGFxMovie*>    DPGOpenMovies[SDPG_PostProcess + 1];
	TArray<FGFxMovie*>    DeleteMovies;
	TArray<FGFxMovie*>    TextureMovies;
	FLOAT                 CurveError;
	UBOOL                 FontlibInitialized;
	FRenderCensus         RenderCensus;
	UBOOL                 bCensusLogged;
	INT                   DrawnFrames;

	// The Unreal key name -> GFx key code map InitKeyMap builds from [GFxUI.KeyMap] in DefaultInput.ini
	// plus the always-present defaults. Retail keeps two maps, KeyCodes and KeyMap; the second is the
	// one GetInputKey consults and the first is what Exec dumps.
	TMap<NAME_INDEX, INT> KeyMap;
	TMap<INT, TArray<FName> > InitialPressedKeys;
	FIntPoint             MousePos;
};

/** the process-wide GFx engine; created by FGFxEngine::GetEngine */
extern FGFxEngine* GGFxEngine;

/** -nogfxui turns the whole interface off for a run, read on first use (never as a file-scope static:
    a file-scope ParseParam in a static library runs before WinMain sets GCmdLine - PHASE9.md's rule and
    agent CA's measurement). */
extern UBOOL GFxUIIsDisabled();

/*-----------------------------------------------------------------------------
	The DishonoredGame seam, agent DG.

	Retail's UDisGFxMoviePlayerBase overrides three of UGFxMoviePlayer's virtuals and every menu in
	the game depends on what the overrides add:

	  UDisGFxMoviePlayerBase::PreLoad   2012 0x822820  UGFxMoviePlayer::PreLoad, then InitTexts
	                                                   (0x8218a0), which is what fills every string
	                                                   in the interface from the localisation tables
	  UDisGFxMoviePlayerBase::Start     2012 0x7f4200  UGFxMoviePlayer::Start, then
	                                                   UDisGlobalUIManager::OnMovieStackChanged and
	                                                   the player's own PostStart, which is what
	                                                   opens a screen at all
	  UDisGFxMoviePlayerMenuBase::PreAdvance 2012 0x817160 / the main menu's 0x822280, which is the
	                                                   state machine that moves from the start screen
	                                                   to the menu proper

	Declaring those overrides means adding CppText hooks to the DishonoredGame generator and
	regenerating DishonoredGameUIClasses.h - and that regeneration rewrites dishonoredgameclasses.h
	and DishonoredGameNative.h, which agent DF is live in. So the game module installs the three
	bodies here instead and the base calls them at exactly the retail call sites. Same order, same
	arguments; the deviation is the dispatch, and it is agentDG.md deviation 5.
-----------------------------------------------------------------------------*/
typedef void (*FGFxMoviePlayerHook)( class UGFxMoviePlayer* Player );
typedef void (*FGFxMoviePlayerTickHook)( class UGFxMoviePlayer* Player, FLOAT DeltaTime );
typedef UBOOL (*FGFxMoviePlayerInputHook)( class UGFxMoviePlayer* Player, INT ControllerId,
                                           FName Key, BYTE Event, UBOOL& bHandled );

/** after UGFxMoviePlayer::PreLoad succeeded, before Start's StartScene: InitTexts */
extern FGFxMoviePlayerHook     GGFxMoviePlayerPreLoadedHook;
/** after StartScene: OnMovieStackChanged, then PostStart */
extern FGFxMoviePlayerHook     GGFxMoviePlayerStartedHook;
/** at the head of UGFxMoviePlayer::Advance: PreAdvance */
extern FGFxMoviePlayerTickHook GGFxMoviePlayerPreAdvanceHook;
/** UDisGFxMoviePlayerBase::FilterButtonInput and its two overrides */
extern FGFxMoviePlayerInputHook GGFxMoviePlayerFilterButtonHook;
