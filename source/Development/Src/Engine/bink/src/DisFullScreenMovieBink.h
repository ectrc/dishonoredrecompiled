/*=============================================================================
	DisFullScreenMovieBink.h: Arkane's Bink movie player (loading movies with an intro, skippable startup movies,
	the overlay manager hooks). Retail source file Engine/Src/DisFullScreenMovieBink.cpp (2012 PDB).
=============================================================================*/

#ifndef _DISFULLSCREENMOVIEBINK_H_
#define _DISFULLSCREENMOVIEBINK_H_

#include "FullScreenMovieBink.h"

class UArkBinkOverlayManager;

/**
 * DISHONORED(port): FDisFullScreenMovieBink, 2012 PDB 324 bytes (FFullScreenMovieBink 248 + the members below), 2013
 * appMalloc(0x158) in StaticInitialize (2013 rva 0x53d470). Behaviour follows the 2013 exe; the 2012 PDB gives the names.
 */
class FDisFullScreenMovieBink : public FFullScreenMovieBink
{
public:
	static FFullScreenMovieSupport* StaticInitialize(UBOOL bUseSound);

	FDisFullScreenMovieBink(UBOOL bUseSound);
	virtual ~FDisFullScreenMovieBink();

	virtual void Tick(FLOAT DeltaTime);
	virtual UBOOL InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event, FLOAT AmountDepressed = 1.f, UBOOL bGamepad = FALSE);
	virtual void GameThreadPlayLoadingMovieAndIntro(EMovieMode InMovieMode, const TCHAR* MovieFilename, const TCHAR* IntroFilename, INT StartFrame = 0, INT InStartOfRenderingMovieFrame = -1, INT InEndOfRenderingMovieFrame = -1);
	virtual void GameThreadPlayMovie(EMovieMode InMovieMode, const TCHAR* MovieFilename, INT StartFrame = 0, INT InStartOfRenderingMovieFrame = -1, INT InEndOfRenderingMovieFrame = -1);
	virtual void GameThreadStopMovie(FLOAT DelayInSeconds = 0.0f, UBOOL bWaitForMovie = TRUE, UBOOL bForceStop = FALSE);
	virtual void GameThreadInitiateStartupSequence();
	virtual void GameThreadRequestDelayedStopMovie();
	virtual void OnBinkTick(FLOAT DeltaTime);
	virtual void OnBinkRenderFrame(FViewport* Viewport, FCanvas* Canvas);

	// DISHONORED(port): agent HG. 2013 FFullScreenMovieSupport slots 7..9 (0x932cc0, 0xc64da0, 0x932d00); 2012 had the first and
	// the last as non-virtual members (0x574630, 0x5746a0) and IsMovieSkippable folded (ICF) into an unrelated getter.
	virtual UBOOL IsLoadingMovie() const
	{
		return m_bIsLoadingMovie;
	}

	virtual UBOOL IsMovieSkippable() const
	{
		return bIsMovieSkippable;
	}

	virtual void SetOverlayManager(UArkBinkOverlayManager* InOverlayMgr)
	{
		m_pOverlayMgr = InOverlayMgr;
	}

private:
	UBOOL OnRequestLoadingMovieExit();
	void OnLoadingMovieSkipped();

	FLOAT m_fLoadingDelay;
	UBOOL m_bAlwaysAutoStart;
	TArray<FString> m_MapsToAutoStart;
	FString m_IntroMovieToPlay;
	UBOOL m_bIsStartupMovie;
	UBOOL m_bIsLoadingMovie;
	FLOAT m_fSkippableTimer;
	FCriticalSection m_CriticalSection;
	UArkBinkOverlayManager* m_pOverlayMgr;
};

#endif
