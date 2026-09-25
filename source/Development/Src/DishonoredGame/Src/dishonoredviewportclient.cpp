// DishonoredGame/src/dishonoredviewportclient.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x62feb0  public: static void __cdecl UDishonoredViewportClient::InitializePrivateStaticClassUDishonoredViewportClient(void)
//   0x62fed0  public: void __thiscall UDishonoredViewportClient::SetListenerLocationOverride(class FVector)
//   0x62ff00  public: void __thiscall UDishonoredViewportClient::EnableListenerLocationOverride(unsigned int)
//   0x62ff20  public: virtual void __thiscall UDishonoredViewportClient::ApplyListenerLocationModifier(class FVector &)
//   0x62ff50  public: virtual void __thiscall UDishonoredViewportClient::Draw(class FViewport *, class FCanvas *)
//   0x6405d0  public: virtual unsigned int __thiscall UDishonoredViewportClient::Exec(wchar_t const *, class FOutputDevice &)
//   0x648890  public: virtual void __thiscall UDishonoredViewportClient::PostRender_Native(class UCanvas *)
//   0x657240  public: static class UClass * __cdecl UDishonoredViewportClient::GetPrivateStaticClassUDishonoredViewportClient(wchar_t const *)
//   0x6585a0  public: static class UClass * __cdecl UDishonoredViewportClient::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x5ea2b0 (2012 0x62fed0, same bytes)
void UDishonoredViewportClient::SetListenerLocationOverride( FVector Location )
{
	m_vListenerLocationOverride = Location;
}

// DISHONORED(written): 2013 rva 0x5ea2e0 (2012 0x62ff00, same bytes)
void UDishonoredViewportClient::EnableListenerLocationOverride( UBOOL bEnable )
{
	m_bListenerLocationOverride = bEnable ? TRUE : FALSE;
}

// DISHONORED(written): 2013 rva 0x5ea300 (2012 0x62ff20, same bytes)
void UDishonoredViewportClient::ApplyListenerLocationModifier( FVector& ListenerLocation )
{
	if( m_bListenerLocationOverride )
	{
		ListenerLocation = m_vListenerLocationOverride;
	}
}

// DISHONORED(written): 2013 rva 0x5ea330 (2012 0x62ff50)
void UDishonoredViewportClient::Draw( FViewport* Viewport, FCanvas* Canvas )
{
	// DISHONORED(bringup): retail first calls UDisGFxMoviePlayerGlobal::PreRender(Viewport) on DisGetGlobalUIManager()->m_pGlobal
	// while that movie is open (UGFxMoviePlayer::bMovieIsOpen); Scaleform is not linked (WITH_GFx=0), so there is no movie
	UGameViewportClient::Draw( Viewport, Canvas );
}

// DISHONORED(written): 2013 rva 0x5fdb90, FExec vtable 0xcd81fc slot 0 (2012 0x6405d0, same bytes)
UBOOL UDishonoredViewportClient::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	UBOOL bHandled = FALSE;
	TCHAR Token[256];
	if( ParseCommand( &Cmd, TEXT("SafeFrame") ) )
	{
		if( ParseToken( Cmd, Token, ARRAY_COUNT(Token), FALSE ) )
		{
			FLOAT TitleSafeFrame = appAtof( Token );
			FLOAT ActionSafeFrame = -1.f;
			if( ParseToken( Cmd, Token, ARRAY_COUNT(Token), FALSE ) )
			{
				ActionSafeFrame = appAtof( Token );
				if( ActionSafeFrame > TitleSafeFrame )
				{
					Exchange( TitleSafeFrame, ActionSafeFrame );
				}
			}
			m_TitleSafeFrame = TitleSafeFrame;
			m_ActionSafeFrame = ActionSafeFrame;
		}
		else if( m_TitleSafeFrame >= 0.f || m_ActionSafeFrame >= 0.f )
		{
			m_TitleSafeFrame = -1.f;
			m_ActionSafeFrame = -1.f;
		}
		else
		{
			m_TitleSafeFrame = 90.f;
			m_ActionSafeFrame = -1.f;
		}
		bHandled = TRUE;
	}
	if( ParseCommand( &Cmd, TEXT("KUWA") ) )
	{
		UArkPpNodeSwitch* KuwaSwitch = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("KuwaSwitch")) ) );
		if( KuwaSwitch )
		{
			KuwaSwitch->m_Selection = !KuwaSwitch->m_Selection;
		}
		return TRUE;
	}
	if( bHandled )
	{
		return TRUE;
	}
	return UGameViewportClient::Exec( Cmd, Ar );
}

// DISHONORED(written): 2013 rva 0x601f30 (2012 0x648890): the low-framerate warning (Txr_30fps tile and "FPS (Max)" text in the
// bottom-right corner) once m_fLowFPSNeededFrameCount frames in a row ran slower than 28 fps, for m_fLowFPSDisplayDuration seconds
void UDishonoredViewportClient::PostRender_Native( UCanvas* Canvas )
{
	const DOUBLE CurrentTime = appSeconds();
	const FLOAT AverageMS = m_LowFPSData.m_fAverageMS * 0.1f + ( CurrentTime - m_LowFPSData.m_dLastTime ) * 1000.0 * 0.9;
	m_LowFPSData.m_fAverageMS = AverageMS;
	m_LowFPSData.m_dLastTime = CurrentTime;
	if( AverageMS >= 10000.f )
	{
		return;
	}
	const FLOAT AverageSeconds = AverageMS * 0.001f;
	if( AverageSeconds < 1.f / 28.f )
	{
		if( m_LowFPSData.m_fDisplayTimeLeft > 0.f )
		{
			m_LowFPSData.m_fDisplayTimeLeft -= AverageSeconds;
			m_LowFPSData.m_FrameCount = 0;
			if( m_LowFPSData.m_fDisplayTimeLeft <= 0.f )
			{
				m_LowFPSData.m_fBiggestFrameTime = 0.f;
			}
		}
	}
	else
	{
		if( ++m_LowFPSData.m_FrameCount > m_fLowFPSNeededFrameCount )
		{
			m_LowFPSData.m_fDisplayTimeLeft = m_fLowFPSDisplayDuration;
		}
		if( AverageSeconds > m_LowFPSData.m_fBiggestFrameTime )
		{
			m_LowFPSData.m_fBiggestFrameTime = AverageSeconds;
		}
	}
	if( m_LowFPSData.m_fDisplayTimeLeft <= 0.f )
	{
		return;
	}
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	const UBOOL bShowingDebugInfo = PlayerController && PlayerController->myHUD && PlayerController->myHUD->bShowDebugInfo;
	if( m_HideFPSWarningInShipping || bShowingDebugInfo )
	{
		return;
	}
	UTexture2D* WarningTexture = LoadObject<UTexture2D>( NULL, TEXT("DebugMaterials.Textures.Txr_30fps"), NULL, LOAD_None, NULL );
	DrawTile( Canvas->Canvas, Canvas->SizeX - 256, Canvas->SizeY - 256, 224.f, 224.f, 0.f, 0.f, 1.f, 1.f,
		FLinearColor( 1.f, 1.f, 1.f, 0.9f ), WarningTexture ? WarningTexture->Resource : NULL, TRUE );
	const FString Text = FString::Printf( TEXT("%.2f FPS (Max: %.2f ms)"), 1.f / AverageSeconds, m_LowFPSData.m_fBiggestFrameTime * 1000.f );
	DrawShadowedString( Canvas->Canvas, Canvas->SizeX - 256, Canvas->SizeY - 256, *Text, GEngine->MediumFont, FLinearColor( FColor(255, 0, 0, 255) ) );
}

// DISHONORED(written): 2013 rva 0x5ed500 (2012 0x633de0)
void UDishonoredViewportClient::execPostRender_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UCanvas, Canvas);
	P_FINISH;
	PostRender_Native( Canvas );
}
