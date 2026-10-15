// DishonoredGame/src/disbinkoverlaymanager.cpp
// DISHONORED(port): agent HG (PHASE22 HG). The text drawn over the loading movies: the map name, a rotating hint and LOADING while the
// level loads, then the chapter title, its description and PRESS ANY KEY TO CONTINUE once the movie waits for a key; the
// save-notification text. The 2013 exe is the target (rvas below); the 2012 PDB gives the names.
// PDB functions attributed to this file (16):
//   0x7f40a0  public: static void __cdecl UDisBinkOverlayManager::InitializePrivateStaticClassUDisBinkOverlayManager(void)
//   0x7f40c0  public: void __thiscall UDisBinkOverlayManager::CleanUp(void)
//   0x7f4100  public: void __thiscall UDisBinkOverlayManager::OnLoadingMovieStopped(void)
//   0x7f5710  protected: void __thiscall UDisBinkOverlayManager::DrawTextOverlay(class FCanvas *, struct FDisBinkTextOverlayInfo const &, wchar_t const *, class UFont *, struct FLinearColor const *, struct FVector2D *)
//   0x7f9240  protected: void __thiscall UDisBinkOverlayManager::LoadContentPackage(void)
//   0x7f9570  protected: void __thiscall UDisBinkOverlayManager::OnRenderLoadingMovie(class FViewport *, class FCanvas *)
//   0x7f9ea0  protected: int __thiscall UDisBinkOverlayManager::GetRandomHintIndex(void)const
//   0x8028e0  public: void __thiscall UDisBinkOverlayManager::Initialize(void)
//   0x802a10  public: virtual void __thiscall UDisBinkOverlayManager::Tick(void)
//   0x802b30  public: virtual void __thiscall UDisBinkOverlayManager::OnBinkRenderFrame(class FViewport *, class FCanvas *)
//   0x802b60  protected: void __thiscall UDisBinkOverlayManager::ChangeHint(void)
//   0x80bb20  public: virtual void __thiscall UDisBinkOverlayManager::OnBinkTick(float)
//   0x817f10  public: static class UClass * __cdecl UDisBinkOverlayManager::GetPrivateStaticClassUDisBinkOverlayManager(wchar_t const *)
//   0x81a1b0  public: static class UClass * __cdecl UDisBinkOverlayManager::StaticClassNoInline(void)
//   0x81a1e0  public: void __thiscall UDisBinkOverlayManager::OnPlayLoadingMovie(class FString const &, unsigned int)
//   0x81a9d0  public: void __thiscall UDisBinkOverlayManager::OnPlaySaveNotificationMovie(void)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): agent HG -pbinkdiag: the movie's overlay texts, each with its position, the hint index and the chapter
static UBOOL DisBinkDiag()
{
	static const UBOOL bBinkDiag = ParseParam( appCmdLine(), TEXT("pbinkdiag") );
	return bBinkDiag;
}

// DISHONORED(port): 2013 rva 0x79e440 (2012 0x8028e0)
void UDisBinkOverlayManager::Initialize()
{
	m_LoadingText = m_LoadingText.ToUpper();
	m_PressToContinueText_PC = m_PressToContinueText_PC.ToUpper();
	m_PressToContinueText_Console = m_PressToContinueText_Console.ToUpper();
	m_pBinkMovie = GFullScreenMovie;
	GFullScreenMovie->SetOverlayManager( this );
	m_CurrentHintSetIndex = INDEX_NONE;
	m_CurrentHintIndex = INDEX_NONE;
	m_pCriticalSection = new FCriticalSection();
}

// DISHONORED(port): 2013 rva 0x787530 (2012 0x7f40c0)
void UDisBinkOverlayManager::CleanUp()
{
	if( m_pCriticalSection )
	{
		delete GetCriticalSection();
		m_pCriticalSection = NULL;
	}
	if( m_pBinkMovie )
	{
		GetBinkMovie()->SetOverlayManager( NULL );
	}
}

// DISHONORED(port): 2013 rva 0x787570 (2012 0x7f4100). 2013 also drops the movie player's subtitle font with the normal font and no
// longer clears m_bLoadingStarted (Tick does, after the call).
void UDisBinkOverlayManager::OnLoadingMovieStopped()
{
	if( !m_pContentPackage )
	{
		return;
	}
	m_pContentPackage->RemoveFromRoot();
	m_pContentPackage = NULL;
	if( m_pNormalFont )
	{
		m_pNormalFont->RemoveFromRoot();
		m_pNormalFont = NULL;
		if( GFullScreenMovie )
		{
			GFullScreenMovie->SubtitleFont = NULL;
		}
	}
	if( m_pTitleFont )
	{
		m_pTitleFont->RemoveFromRoot();
		m_pTitleFont = NULL;
	}
	if( m_pButtonTexture )
	{
		m_pButtonTexture->RemoveFromRoot();
		m_pButtonTexture = NULL;
	}
	if( m_pChapterTitleBgTexture )
	{
		m_pChapterTitleBgTexture->RemoveFromRoot();
		m_pChapterTitleBgTexture = NULL;
	}
}

// DISHONORED(port): 2013 rva 0x7934f0 (2012 0x7f9240). Seek-free: <Package>_SF_LOC_INT then <Package>_SF are loaded and the package
// is found by name; 2013 also hands the normal font to the movie player as its subtitle font.
void UDisBinkOverlayManager::LoadContentPackage()
{
	if( m_pContentPackage )
	{
		return;
	}
	if( GUseSeekFreeLoading )
	{
		FString PackageName = FString::Printf( TEXT("%s_SF_LOC_INT"), *m_ContentPackageName );
		UObject::LoadPackage( NULL, *PackageName, LOAD_SeekFree );
		PackageName = PackageName.Left( PackageName.InStr( TEXT("_LOC") ) );
		UObject::LoadPackage( NULL, *PackageName, LOAD_SeekFree );
		// DISHONORED(bringup): retail asks StaticFindObjectChecked; a miss is logged here instead of ending the run
		m_pContentPackage = FindObject<UPackage>( NULL, *m_ContentPackageName, TRUE );
	}
	else
	{
		m_pContentPackage = UObject::LoadPackage( NULL, *m_ContentPackageName, LOAD_None );
	}
	if( DisBinkDiag() )
	{
		debugf( TEXT("DISHONORED(binkdiag): content package '%s' %s"), *m_ContentPackageName, m_pContentPackage ? TEXT("loaded") : TEXT("NOT FOUND") );
	}
	if( !m_pContentPackage )
	{
		return;
	}
	m_pContentPackage->AddToRoot();
	UFont* NormalFont = FindObject<UFont>( m_pContentPackage, *m_NormalFontName, TRUE );
	if( NormalFont )
	{
		NormalFont->AddToRoot();
		m_pNormalFont = NormalFont;
		if( GFullScreenMovie )
		{
			GFullScreenMovie->SubtitleFont = NormalFont;
		}
	}
	UFont* TitleFont = FindObject<UFont>( m_pContentPackage, *m_TitleFontName, TRUE );
	if( TitleFont )
	{
		TitleFont->AddToRoot();
		m_pTitleFont = TitleFont;
	}
	UTexture2D* ButtonTexture = FindObject<UTexture2D>( m_pContentPackage, *m_ButtonTextureName_Xenon, TRUE );
	if( ButtonTexture )
	{
		ButtonTexture->AddToRoot();
		m_pButtonTexture = ButtonTexture;
	}
	UTexture2D* ChapterTitleBgTexture = FindObject<UTexture2D>( m_pContentPackage, *m_ChapterTitleBgTextureName, TRUE );
	if( ChapterTitleBgTexture )
	{
		ChapterTitleBgTexture->AddToRoot();
		m_pChapterTitleBgTexture = ChapterTitleBgTexture;
	}
	if( DisBinkDiag() )
	{
		debugf( TEXT("DISHONORED(binkdiag): fonts normal %s title %s, button %s, chapter title bg %s"),
			m_pNormalFont ? *m_pNormalFont->GetPathName() : TEXT("none"), m_pTitleFont ? *m_pTitleFont->GetPathName() : TEXT("none"),
			m_pButtonTexture ? *m_pButtonTexture->GetPathName() : TEXT("none"), m_pChapterTitleBgTexture ? *m_pChapterTitleBgTexture->GetPathName() : TEXT("none") );
	}
}

// DISHONORED(port): 2013 rva 0x7b60e0 (2012 0x81a1e0). 2013 takes whether the map has an m_MapConfig: such a map shows its
// localized [MapNames] entry unless the config sets m_bShowMapNameOnlyOnXboxNoHDD, and no name when it has none; a map without
// a config shows its own name, upper case, '_' as ' '. A binding with an empty hint set keeps the current set (or the first).
void UDisBinkOverlayManager::OnPlayLoadingMovie( const FString& _rMapName, UBOOL _bHasMapConfig, UBOOL _bShowMapNameAndHints )
{
	LoadContentPackage();
	m_fLoadingTime = 0.f;
	m_bShowMapNameAndHints = _bShowMapNameAndHints ? TRUE : FALSE;
	m_bLoadingStarted = TRUE;

	const TCHAR* Language = UObject::GetLanguage();
	TCHAR FilePath[1024];
	const FConfigSection* MapNames = FindLocalizedConfigSection( Language, TEXT("MapNames"), FilePath, ARRAY_COUNT(FilePath) );
	const FString* LocalizedMapName = MapNames ? MapNames->Find( FName( *_rMapName ) ) : NULL;
	UDishonoredEngine* DisEngine = Cast<UDishonoredEngine>( GEngine );
	if( LocalizedMapName )
	{
		FMapConfig* MapConfig = ( _bHasMapConfig && DisEngine ) ? DisEngine->FindMapConfig( _rMapName ) : NULL;
		if( MapConfig && MapConfig->m_bShowMapNameOnlyOnXboxNoHDD )
		{
			m_MapName.Empty();
		}
		else
		{
			m_MapName = LocalizedMapName->ToUpper();
		}
	}
	else if( _bHasMapConfig )
	{
		m_MapName.Empty();
	}
	else
	{
		m_MapName = _rMapName.ToUpper();
		m_MapName.ReplaceInline( TEXT("_"), TEXT(" ") );
	}

	const UBOOL bUsingGamepad = DisEngine && DisEngine->m_bUsingGamepad;
	INT HintSetIndex = INDEX_NONE;
	for( INT BindingIndex = 0; BindingIndex < m_MapHintBindings.Num(); BindingIndex++ )
	{
		if( appStricmp( *m_MapHintBindings(BindingIndex).m_MapName, *_rMapName ) == 0 )
		{
			HintSetIndex = BindingIndex;
			break;
		}
	}
	if( HintSetIndex != INDEX_NONE && m_MapHintBindings(HintSetIndex).m_HintSet.Len() == 0 )
	{
		HintSetIndex = m_CurrentHintSetIndex > 0 ? m_CurrentHintSetIndex : 0;
	}

	if( HintSetIndex == INDEX_NONE )
	{
		m_Hints.Empty();
		new( m_Hints ) FString( TEXT("No hint set defined for this map. No hint set defined for this map.No hint set defined for this map. No hint set defined for this map. No hint set defined for this map.") );
		m_HintOccurrences.Empty();
		m_HintOccurrences.AddItem( 0 );
		m_CurrentHintSetIndex = INDEX_NONE;
	}
	else if( HintSetIndex != m_CurrentHintSetIndex || bUsingGamepad != (UBOOL)m_bCurrentHintSetUsingGamepad )
	{
		const FString SectionName = FString( TEXT("DisBinkOverlayManager_HintSet_") ) + m_MapHintBindings(HintSetIndex).m_HintSet;
		const FConfigSection* HintSet = FindLocalizedConfigSection( Language, *SectionName, FilePath, ARRAY_COUNT(FilePath) );
		m_Hints.Empty();
		if( HintSet )
		{
			static const FName GamepadSpecificHint( TEXT("Hint_Gamepad") );
			static const FName KeyboardSpecificHint( TEXT("Hint_Keyboard") );
			for( FConfigSection::TConstIterator It( *HintSet ); It; ++It )
			{
				const FName SkippedKey = bUsingGamepad ? KeyboardSpecificHint : GamepadSpecificHint;
				if( It.Key() != SkippedKey )
				{
					m_Hints.AddItem( It.Value() );
				}
			}
		}
		m_HintOccurrences.Empty();
		m_HintOccurrences.AddZeroed( m_Hints.Num() );
		m_bCurrentHintSetUsingGamepad = bUsingGamepad;
		m_CurrentHintSetIndex = HintSetIndex;
	}

	ChangeHint();
	m_ChapterTitle.Empty();
	m_ChapterDescription.Empty();
	m_SaveNotificationText.Empty();

	if( DisBinkDiag() )
	{
		debugf( TEXT("DISHONORED(binkdiag): movie open for map %s (config %d, show map name and hints %d): map name '%s', hint set %d of %d hints, hint index %d '%s'"),
			*_rMapName, _bHasMapConfig, _bShowMapNameAndHints, *m_MapName, m_CurrentHintSetIndex, m_Hints.Num(), m_CurrentHintIndex,
			m_Hints.IsValidIndex( m_CurrentHintIndex ) ? *m_Hints(m_CurrentHintIndex) : TEXT("") );
	}
}

// DISHONORED(port): 2013 rva 0x7b69c0 (2012 0x81a9d0): no map name, hints or chapter; the text is the first Text_PC (or unnamed)
// key of [DisBinkOverlayManager_SaveNotificationLoadingScreen]
void UDisBinkOverlayManager::OnPlaySaveNotificationMovie()
{
	LoadContentPackage();
	m_bLoadingStarted = TRUE;
	m_bShowMapNameAndHints = FALSE;
	m_fLoadingTime = 0.f;
	m_MapName.Empty();
	m_Hints.Empty();
	m_HintOccurrences.Empty();
	m_ChapterTitle.Empty();
	m_ChapterDescription.Empty();
	TCHAR FilePath[1024];
	const FConfigSection* Section = FindLocalizedConfigSection( UObject::GetLanguage(), TEXT("DisBinkOverlayManager_SaveNotificationLoadingScreen"), FilePath, ARRAY_COUNT(FilePath) );
	m_SaveNotificationText.Empty();
	if( Section )
	{
		static const FName PlatformKey( TEXT("Text_PC") );
		for( FConfigSection::TConstIterator It( *Section ); It; ++It )
		{
			if( It.Key() == PlatformKey || It.Key() == NAME_None )
			{
				m_SaveNotificationText = It.Value();
				break;
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x79e570 (2012 0x802a10): while the movie plays the chapter title and description are taken once the game
// info has a current chapter; when it has stopped the content is released. 2013 calls OnLoadingMovieStopped through the vtable (+308).
void UDisBinkOverlayManager::Tick()
{
	if( !m_bLoadingStarted )
	{
		return;
	}
	if( GetBinkMovie()->GameThreadIsMoviePlaying( TEXT("") ) )
	{
		if( m_ChapterTitle.Len() == 0 )
		{
			ADishonoredGameInfo* GameInfo = DisGetGameInfo();
			const UDisTweaks_ChapterInfo* CurrentChapter = GameInfo ? GameInfo->GetCurrentChapter() : NULL;
			if( CurrentChapter )
			{
				FScopeLock Lock( GetCriticalSection() );
				m_ChapterTitle = CurrentChapter->m_ChapterTitle.ToUpper();
				m_ChapterDescription = CurrentChapter->m_ChapterDescription;
			}
			if( DisBinkDiag() )
			{
				debugf( TEXT("DISHONORED(binkdiag): chapter %s: title '%s' description '%s'"), GameInfo ? *GameInfo->m_CurrentChapterTag.ToString() : TEXT("(no game info)"),
					*m_ChapterTitle, *m_ChapterDescription );
			}
		}
	}
	else
	{
		OnLoadingMovieStopped();
		m_bLoadingStarted = FALSE;
	}
}

// DISHONORED(port): 2013 rva 0x7a9d30 (2012 0x80bb20): the hint rotates while the loading movie has not yet become skippable
void UDisBinkOverlayManager::OnBinkTick( FLOAT _fDeltaTime )
{
	if( GetBinkMovie()->IsLoadingMovie() && !GetBinkMovie()->IsMovieSkippable() )
	{
		m_fLoadingTime += _fDeltaTime;
		m_fCurrentHintTimer -= _fDeltaTime;
		if( m_fCurrentHintTimer < 0.f )
		{
			ChangeHint();
		}
	}
}

// DISHONORED(port): 2013 rva 0x7b9880 (2012 0x802b30)
void UDisBinkOverlayManager::OnBinkRenderFrame( FViewport* _pViewport, FCanvas* _pCanvas )
{
	if( GetBinkMovie()->IsLoadingMovie() )
	{
		OnRenderLoadingMovie( _pViewport, _pCanvas );
	}
}

// DISHONORED(port): 2013 rva 0x79e6a0 (2012 0x802b60)
void UDisBinkOverlayManager::ChangeHint()
{
	m_fCurrentHintTimer = m_fHintDuration;
	m_CurrentHintIndex = GetRandomHintIndex();
	if( m_CurrentHintIndex != INDEX_NONE )
	{
		m_HintOccurrences(m_CurrentHintIndex)++;
	}
}

// DISHONORED(port): 2013 rva 0x793800 (2012 0x7f9ea0): a random one of the least shown hints, never the current one twice in a row
INT UDisBinkOverlayManager::GetRandomHintIndex() const
{
	const INT HintCount = m_Hints.Num();
	if( HintCount == 0 )
	{
		return INDEX_NONE;
	}
	INT LowestOccurrence = MAXINT;
	INT* LowestOccurrenceHintIndices = (INT*)appAlloca( HintCount * sizeof(INT) );
	INT LowestOccurrenceHintCount = 0;
	for( INT HintIndex = 0; HintIndex < HintCount; HintIndex++ )
	{
		if( m_HintOccurrences(HintIndex) < LowestOccurrence )
		{
			LowestOccurrence = m_HintOccurrences(HintIndex);
			LowestOccurrenceHintIndices[0] = HintIndex;
			LowestOccurrenceHintCount = 1;
		}
		else if( m_HintOccurrences(HintIndex) == LowestOccurrence )
		{
			LowestOccurrenceHintIndices[LowestOccurrenceHintCount++] = HintIndex;
		}
	}
	const INT Pick = appRand() % LowestOccurrenceHintCount;
	if( LowestOccurrenceHintIndices[Pick] == m_CurrentHintIndex )
	{
		return LowestOccurrenceHintIndices[( Pick + 1 ) % LowestOccurrenceHintCount];
	}
	return LowestOccurrenceHintIndices[Pick];
}

// DISHONORED(port): 2013 rva 0x7b8b20 (2012 0x7f9570; the 2012/2013 match ratio is .59 because 2013 sets the render target and a
// depth state first - the canvas flush sets its own, so that preamble is not reproduced). The 1280x720 layout is fitted into the
// viewport (scaled by height when wider than 16:9, by width otherwise, centred). The map name is drawn three times: two passes in
// m_fMapNameShadowColor offset (-3,3) rotated -3.95 and offset (-5,-3) rotated -3.42, then the text itself.
void UDisBinkOverlayManager::OnRenderLoadingMovie( FViewport* _pViewport, FCanvas* _pCanvas )
{
	const FLOAT SizeX = _pViewport->GetSizeX();
	const FLOAT SizeY = _pViewport->GetSizeY();
	FLOAT Scale;
	FVector Offset( 0.f, 0.f, 0.f );
	if( SizeX / SizeY >= 1280.f / 720.f )
	{
		Scale = SizeY / 720.f;
		Offset.X = ( SizeX - Scale * 1280.f ) * 0.5f;
	}
	else
	{
		Scale = SizeX / 1280.f;
		Offset.Y = ( SizeY - Scale * 720.f ) * 0.5f;
	}
	_pCanvas->PushRelativeTransform( FTranslationMatrix( Offset ) );
	_pCanvas->PushRelativeTransform( FScaleMatrix( FVector( Scale, Scale, Scale ) ) );

	if( m_bShowMapNameAndHints )
	{
		FDisBinkTextOverlayInfo ShadowTextInfo = m_MapNameTextInfo;
		ShadowTextInfo.m_Color = m_fMapNameShadowColor;
		ShadowTextInfo.m_fTranslationFromCenter = FVector2D( -3.f, 3.f );
		ShadowTextInfo.m_fRotationFromCenter = -3.95f;
		DrawTextOverlay( _pCanvas, ShadowTextInfo, *m_MapName, m_pTitleFont, NULL, NULL );
		ShadowTextInfo.m_fTranslationFromCenter = FVector2D( -5.f, -3.f );
		ShadowTextInfo.m_fRotationFromCenter = -3.42f;
		DrawTextOverlay( _pCanvas, ShadowTextInfo, *m_MapName, m_pTitleFont, NULL, NULL );
		DrawTextOverlay( _pCanvas, m_MapNameTextInfo, *m_MapName, m_pTitleFont, NULL, NULL );
	}

	if( GetBinkMovie()->IsMovieSkippable() )
	{
		UDishonoredEngine* DisEngine = Cast<UDishonoredEngine>( GEngine );
		if( DisEngine && DisEngine->m_bUsingGamepad )
		{
			FVector2D TextSize( 0.f, 0.f );
			DrawTextOverlay( _pCanvas, m_PressToContinueTextInfo, *m_PressToContinueText_Console, m_pTitleFont, NULL, &TextSize );
			if( m_pButtonTexture )
			{
				const FLOAT ButtonSizeX = m_pButtonTexture->Resource->GetSizeX();
				const FLOAT ButtonSizeY = m_pButtonTexture->Resource->GetSizeY();
				const FLOAT ButtonX = (INT)( m_PressToContinueTextInfo.m_Position.X - TextSize.X - ButtonSizeX + m_ButtonOffset.X );
				const FLOAT ButtonY = (INT)( m_PressToContinueTextInfo.m_Position.Y + TextSize.Y * 0.5f - ButtonSizeY * 0.5f + m_ButtonOffset.Y );
				DrawTile( _pCanvas, ButtonX, ButtonY, ButtonSizeX, ButtonSizeY, 0.f, 0.f, 1.f, 1.f, FLinearColor::White, m_pButtonTexture->Resource, TRUE );
			}
		}
		else
		{
			DrawTextOverlay( _pCanvas, m_PressToContinueTextInfo, *m_PressToContinueText_PC, m_pTitleFont, NULL, NULL );
		}

		FScopeLock Lock( GetCriticalSection() );
		if( m_pChapterTitleBgTexture && m_ChapterTitle.Len() > 0 )
		{
			const FLOAT BgSizeX = m_pChapterTitleBgTexture->Resource->GetSizeX();
			const FLOAT BgSizeY = m_pChapterTitleBgTexture->Resource->GetSizeY();
			_pCanvas->PushRelativeTransform( FTranslationMatrix( FVector( m_ChapterTitleInfo.m_Position.X, m_ChapterTitleInfo.m_Position.Y, 0.f ) ) );
			_pCanvas->PushRelativeTransform( FRotationTranslationMatrix( FRotator( 0, (INT)( m_ChapterTitleInfo.m_fRotation * 182.04445f ), 0 ), FVector::ZeroVector ) );
			DrawTile( _pCanvas, m_ChapterTitleBgOffset.X, m_ChapterTitleBgOffset.Y, BgSizeX, BgSizeY, 0.f, 0.f, 1.f, 1.f, FLinearColor::White, m_pChapterTitleBgTexture->Resource, TRUE );
			_pCanvas->PopTransform();
			_pCanvas->PopTransform();
		}
		DrawTextOverlay( _pCanvas, m_ChapterTitleInfo, *m_ChapterTitle, m_pTitleFont, NULL, NULL );
		DrawTextOverlay( _pCanvas, m_ChapterDescInfo, *m_ChapterDescription, m_pNormalFont, NULL, NULL );
	}
	else
	{
		const FLinearColor Pulse( 1.f, 1.f, 1.f, Abs( appCos( m_fLoadingTime + m_fLoadingTime ) ) );
		DrawTextOverlay( _pCanvas, m_LoadingTextInfo, *m_LoadingText, m_pTitleFont, &Pulse, NULL );
		if( m_bShowMapNameAndHints && m_CurrentHintIndex != INDEX_NONE )
		{
			DrawTextOverlay( _pCanvas, m_HintTextInfo, *m_Hints(m_CurrentHintIndex), m_pNormalFont, NULL, NULL );
		}
		if( m_SaveNotificationText.Len() > 0 )
		{
			DrawTextOverlay( _pCanvas, m_SaveNotificationTextInfo, *m_SaveNotificationText, m_pNormalFont, NULL, NULL );
		}
	}

	_pCanvas->PopTransform();
	_pCanvas->PopTransform();

	if( DisBinkDiag() )
	{
		static INT LastState = -2;
		const INT State = ( GetBinkMovie()->IsMovieSkippable() ? 1 : 0 ) | ( m_ChapterTitle.Len() > 0 ? 2 : 0 ) | ( ( m_CurrentHintIndex + 1 ) << 2 );
		if( State != LastState )
		{
			LastState = State;
			debugf( TEXT("DISHONORED(binkdiag): overlay state: viewport %.0fx%.0f scale %.3f offset (%.1f,%.1f), skippable %d, hint index %d"),
				SizeX, SizeY, Scale, Offset.X, Offset.Y, GetBinkMovie()->IsMovieSkippable(), m_CurrentHintIndex );
			debugf( TEXT("DISHONORED(binkdiag): overlay map name '%s' at (%.0f,%.0f) font %s"), *m_MapName, m_MapNameTextInfo.m_Position.X, m_MapNameTextInfo.m_Position.Y, m_pTitleFont ? *m_pTitleFont->GetName() : TEXT("none") );
			if( GetBinkMovie()->IsMovieSkippable() )
			{
				debugf( TEXT("DISHONORED(binkdiag): overlay press to continue '%s' at (%.0f,%.0f)"), *m_PressToContinueText_PC, m_PressToContinueTextInfo.m_Position.X, m_PressToContinueTextInfo.m_Position.Y );
				debugf( TEXT("DISHONORED(binkdiag): overlay chapter title '%s' at (%.0f,%.0f) bg %d"), *m_ChapterTitle, m_ChapterTitleInfo.m_Position.X, m_ChapterTitleInfo.m_Position.Y, m_pChapterTitleBgTexture != NULL );
				debugf( TEXT("DISHONORED(binkdiag): overlay chapter description '%s' at (%.0f,%.0f) font %s"), *m_ChapterDescription, m_ChapterDescInfo.m_Position.X, m_ChapterDescInfo.m_Position.Y, m_pNormalFont ? *m_pNormalFont->GetName() : TEXT("none") );
			}
			else
			{
				debugf( TEXT("DISHONORED(binkdiag): overlay loading '%s' at (%.0f,%.0f)"), *m_LoadingText, m_LoadingTextInfo.m_Position.X, m_LoadingTextInfo.m_Position.Y );
				debugf( TEXT("DISHONORED(binkdiag): overlay hint %d '%s' at (%.0f,%.0f)"), m_CurrentHintIndex, m_Hints.IsValidIndex( m_CurrentHintIndex ) ? *m_Hints(m_CurrentHintIndex) : TEXT(""), m_HintTextInfo.m_Position.X, m_HintTextInfo.m_Position.Y );
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x7b7130 (2012 0x7f5710, the same code): position, rotation and scale of the info; centring and right
// alignment from the measured size; a rotation about the text's centre (m_fTranslationFromCenter, m_fRotationFromCenter) undoes the
// info's rotation, offsets and rotates again; m_bWrapped wraps to m_Size / m_Scale.
void UDisBinkOverlayManager::DrawTextOverlay( FCanvas* _pCanvas, const FDisBinkTextOverlayInfo& _rInfo, const TCHAR* _pText, UFont* _pFont, const FLinearColor* _pColorMultiplier, FVector2D* _pOutTextSize )
{
	FLinearColor Color( _rInfo.m_Color );
	if( _pColorMultiplier )
	{
		Color.R *= _pColorMultiplier->R;
		Color.G *= _pColorMultiplier->G;
		Color.B *= _pColorMultiplier->B;
		Color.A *= _pColorMultiplier->A;
	}
	_pCanvas->PushRelativeTransform( FTranslationMatrix( FVector( _rInfo.m_Position.X, _rInfo.m_Position.Y, 0.f ) ) );
	_pCanvas->PushRelativeTransform( FRotationTranslationMatrix( FRotator( 0, (INT)( _rInfo.m_fRotation * 182.04445f ), 0 ), FVector::ZeroVector ) );
	_pCanvas->PushRelativeTransform( FScaleMatrix( FVector( _rInfo.m_Scale.X, _rInfo.m_Scale.Y, 1.f ) ) );
	INT TransformCount = 3;

	FVector2D DrawOffset( 0.f, 0.f );
	if( _rInfo.m_bCentered || _rInfo.m_bRightAligned || _rInfo.m_fRotationFromCenter != 0.f || _pOutTextSize )
	{
		FTextSizingParameters Parameters( _pFont, 1.f, 1.f );
		UCanvas::CanvasStringSize( Parameters, _pText, NULL, TRUE );
		const FLOAT TextXL = Parameters.DrawXL;
		const FLOAT TextYL = Parameters.DrawYL;
		if( _rInfo.m_fTranslationFromCenter.X != 0.f || _rInfo.m_fTranslationFromCenter.Y != 0.f || _rInfo.m_fRotationFromCenter != 0.f )
		{
			_pCanvas->PushRelativeTransform( FTranslationMatrix( FVector( TextXL * 0.5f, TextYL * 0.5f, 0.f ) ) );
			_pCanvas->PushRelativeTransform( FRotationTranslationMatrix( FRotator( 0, (INT)( _rInfo.m_fRotation * -182.04445f ), 0 ), FVector::ZeroVector ) );
			_pCanvas->PushRelativeTransform( FTranslationMatrix( FVector( _rInfo.m_fTranslationFromCenter.X / _rInfo.m_Scale.X, _rInfo.m_fTranslationFromCenter.Y / _rInfo.m_Scale.Y, 0.f ) ) );
			_pCanvas->PushRelativeTransform( FRotationTranslationMatrix( FRotator( 0, (INT)( _rInfo.m_fRotationFromCenter * 182.04445f ), 0 ), FVector::ZeroVector ) );
			DrawOffset = FVector2D( -TextXL * 0.5f, -TextYL * 0.5f );
			TransformCount = 7;
		}
		if( _rInfo.m_bCentered )
		{
			DrawOffset.X -= TextXL * 0.5f;
		}
		else if( _rInfo.m_bRightAligned )
		{
			DrawOffset.X -= TextXL;
		}
		if( _pOutTextSize )
		{
			*_pOutTextSize = FVector2D( _rInfo.m_Scale.X * TextXL, _rInfo.m_Scale.Y * TextYL );
		}
	}

	if( _rInfo.m_bWrapped )
	{
		FLOAT WrapXL = _rInfo.m_Size.X / _rInfo.m_Scale.X;
		FLOAT WrapYL = _rInfo.m_Size.Y / _rInfo.m_Scale.Y;
		DrawStringWrapped( _pCanvas, TRUE, DrawOffset.X, DrawOffset.Y, WrapXL, WrapYL, _pFont, _pText, Color );
	}
	else
	{
		DrawString( _pCanvas, DrawOffset.X, DrawOffset.Y, _pText, _pFont, Color, 1.f, 1.f, 0.f, NULL, SE_BLEND_Translucent, TRUE, 0.f, 1.f, FFontRenderInfo( EC_EventParm ) );
	}
	for( ; TransformCount > 0; TransformCount-- )
	{
		_pCanvas->PopTransform();
	}
}
