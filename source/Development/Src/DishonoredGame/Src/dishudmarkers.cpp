// DishonoredGame/src/dishudmarkers.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (23):
//   0x822d30  public: virtual class FVector __thiscall FDisHUDMarker::GetLocation(void)const
//   0x822d60  public: virtual class FVector __thiscall FDisTaskMarker::GetLocation(void)const
//   0x822db0  public: virtual void __thiscall FDisAwarenessMarker::Tick(float)
//   0x822df0  public: void __thiscall FDisAwarenessMarker::UpdateWaves(unsigned char, unsigned char, unsigned int)
//   0x822f80  public: void __thiscall FDisAwarenessMarker::ForceFadeOut(void)
//   0x825740  public: virtual void __thiscall FDisHUDMarker::Update(struct FVector2D, struct FVector2D, float, unsigned int)
//   0x8257d0  public: unsigned int __thiscall FDisHUDMarker::SwapDepth(class FDisHUDMarker *)
//   0x8258a0  public: virtual class FVector __thiscall FDisAwarenessMarker::GetLocation(void)const
//   0x825940  public: virtual void __thiscall FDisAwarenessMarker::Update(struct FVector2D, struct FVector2D, float, unsigned int)
//   0x825e00  public: virtual void __thiscall FDisGrenadeMarker::Update(struct FVector2D, struct FVector2D, float, unsigned int)
//   0x826030  void __cdecl ComputeScreenPosition(class FVector, class UCanvas *, unsigned int, unsigned int, struct FVector2D &, unsigned int &)
//   0x8264b0  void __cdecl ComputeMarkerScreenPosition(class FVector, class UCanvas *, float, struct FVector2D, struct FVector2D &, struct FVector2D &, unsigned int &)
//   0x82ac60  public: virtual void __thiscall FDisHUDMarker::CreateGFxInstance(void)
//   0x82aea0  public: virtual void __thiscall FDisHUDMarker::DestroyGFxInstance(void)
//   0x82af60  public: virtual void __thiscall FDisTaskMarker::Update(struct FVector2D, struct FVector2D, float, unsigned int)
//   0x82b2a0  public: virtual void __thiscall FDisAwarenessMarker::CreateGFxInstance(void)
//   0x82b370  public: virtual void __thiscall FDisAwarenessMarker::DestroyGFxInstance(void)
//   0x82b460  public: virtual void __thiscall FDisHeartMarker::Update(struct FVector2D, struct FVector2D, float, unsigned int)
//   0x8311d0  public: virtual __thiscall FDisHUDMarker::~FDisHUDMarker(void)
//   0x831250  public: __thiscall FDisTaskMarker::FDisTaskMarker(struct FDisTaskTarget const *, struct FDisHUDMarkerSettings const *, class FGFxMovie *)
//   0x831530  public: __thiscall FDisAwarenessMarker::FDisAwarenessMarker(class ADishonoredNPCPawn const *, struct FDisHUDMarkerSettings const *, class FGFxMovie *)
//   0x831750  public: __thiscall FDisHeartMarker::FDisHeartMarker(class ADisPickup_Base const *, struct FDisHUDMarkerSettings const *, class FGFxMovie *)
//   0x836c30  public: __thiscall FDisGrenadeMarker::FDisGrenadeMarker(class AActor const *, struct FDisHUDMarkerSettings const *, class FGFxMovie *)

// ---- agent HI (PHASE22 HI): the HUD's task markers ----
// DISHONORED(port): the base marker and the task marker, from the 2013 bodies (rvas in dishudmarkers.h). The awareness,
// grenade and heart markers (0x822db0..0x836c30 above) are not ported: nothing in this tree creates them.
#include "DishonoredGame.h"
#include "dishudmarkers.h"

/** the instance names AttachMovie gives the marker clips: DisInst_<n> */
static INT GDisHUDMarkerInstanceNum = 0;

// DISHONORED(port): 2013 rva 0x7ce380 (no 2012 body of its own: 2012's FDisTaskMarker constructor inlined it)
FDisHUDMarker::FDisHUDMarker( const AActor* _pTrackedActor, const FDisHUDMarkerSettings* _pSettings, FGFxMovie* _pGFxMovie, UBOOL _bCreateGFxInstance )
	: m_pTrackedActor( _pTrackedActor )
	, m_pSettings( _pSettings )
	, m_pGFxMovie( _pGFxMovie )
	, m_Depth( -1 )
	, m_bTrackedActorVisible( TRUE )
	, m_bNeedsSorting( FALSE )
	, m_VisibilityFlags( 0 )
{
	m_GFxInstance.SetNull();
	if( _bCreateGFxInstance )
	{
		CreateGFxInstance();
	}
}

// DISHONORED(port): 2013 rva 0x7ce410 (2012 0x8311d0)
FDisHUDMarker::~FDisHUDMarker()
{
	if( !m_GFxInstance.IsNull() )
	{
		DestroyGFxInstance();
	}
}

// DISHONORED(port): 2013 rva 0x7c54f0 (2012 0x82ac60): a clip of the settings' symbol attached to _root.markers_mc at its
// next free depth, hidden until the first Update shows it.
void FDisHUDMarker::CreateGFxInstance()
{
	GFxMovieView* View = m_pGFxMovie ? m_pGFxMovie->pView.GetPtr() : NULL;
	if( View == NULL )
	{
		return;
	}
	ANSICHAR InstanceName[1024];
	appSprintfANSI( InstanceName, "DisInst_%i", GDisHUDMarkerInstanceNum++ );
	GFxValue Container;
	View->GetVariable( &Container, "_root.markers_mc" );
	GFxValue NextHighestDepth;
	Container.Invoke( "getNextHighestDepth", &NextHighestDepth );
	m_Depth = appTrunc( NextHighestDepth.GetNumber() + 0.5 );
	Container.AttachMovie( &m_GFxInstance, TCHAR_TO_ANSI( *m_pSettings->m_SymbolName ), InstanceName, m_Depth );
	m_bNeedsSorting = TRUE;
	GFxValue::DisplayInfo Info;
	Info.SetVisible( false );
	m_GFxInstance.SetDisplayInfo( Info );
}

// DISHONORED(port): 2013 rva 0x7c5730 (2012 0x82aea0)
void FDisHUDMarker::DestroyGFxInstance()
{
	GFxValue Result;
	m_GFxInstance.Invoke( "removeMovieClip", &Result );
	m_Depth = -1;
	m_GFxInstance.SetNull();
	m_bNeedsSorting = FALSE;
	m_VisibilityFlags = 0;
}

// DISHONORED(port): 2013 rva 0x7b9f70 (2012 0x822d30)
FVector FDisHUDMarker::GetLocation() const
{
	return m_pTrackedActor->Location;
}

// DISHONORED(port): 2013 rva 0x7bd430 (2012 0x825740): a visible marker is placed and shown; one that just stopped being
// visible is hidden.
void FDisHUDMarker::Update( FVector2D _Position, FVector2D _TrackedPosition, FLOAT _fDistance, UINT _Flags )
{
	if( m_GFxInstance.IsNull() )
	{
		return;
	}
	GFxValue::DisplayInfo Info;
	if( _Flags & DHMF_Visible )
	{
		Info.SetPosition( _Position.X, _Position.Y );
		Info.SetVisible( true );
		m_GFxInstance.SetDisplayInfo( Info );
	}
	else if( ( _Flags ^ m_VisibilityFlags ) & DHMF_Visible )
	{
		Info.SetVisible( false );
		m_GFxInstance.SetDisplayInfo( Info );
	}
	m_VisibilityFlags = _Flags;
}

// DISHONORED(port): 2013 rva 0x7bd4c0 (2012 0x8257d0)
UBOOL FDisHUDMarker::SwapDepth( FDisHUDMarker* _pOther )
{
	if( m_GFxInstance.IsNull() || _pOther->m_GFxInstance.IsNull() )
	{
		return FALSE;
	}
	GFxValue Result;
	m_GFxInstance.Invoke( "swapDepths", &Result, &_pOther->m_GFxInstance, 1 );
	Exchange( m_Depth, _pOther->m_Depth );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x7ce750 (2012 0x831250, which copied a display string where 2013 localises the target's
// m_TargetName from the Kismet action's file and section). The icon and the description clip are children of the
// instance; the description starts hidden. Vanish preset 0 is the settings' alpha variation, 1 and 2 the task-marker
// tweaks' "very close" and "never" presets.
FDisTaskMarker::FDisTaskMarker( const FDisTaskTarget* _pTaskTarget, const FDisHUDMarkerSettings* _pSettings, FGFxMovie* _pGFxMovie )
	: FDisHUDMarker( _pTaskTarget->m_pActor, _pSettings, _pGFxMovie, TRUE )
	, m_pSpecificSettings( NULL )
	, m_pVanishPreset( NULL )
	, m_Distance( -1 )
	, m_FrameNumber( 1 )
{
	m_TargetName = Localize( *_pTaskTarget->m_TargetNameLocSection, TEXT("m_TargetName"), *_pTaskTarget->m_TargetNameLocFile, NULL, FALSE );
	const UDisTweaks_GFxMoviePlayerHUD* Tweaks = (const UDisTweaks_GFxMoviePlayerHUD*)( (UDisGFxMoviePlayerHUD*)_pGFxMovie->pUMovie )->m_pTweaks;
	m_pSpecificSettings = &Tweaks->m_TaskMarkerSpecificSettings;
	switch( _pTaskTarget->m_VanishPreset )
	{
	case 0: m_pVanishPreset = &m_pSettings->m_AlphaVariation; break;
	case 1: m_pVanishPreset = &Tweaks->m_TaskMarkerSpecificSettings.m_VanishPreset_VeryClose; break;
	case 2: m_pVanishPreset = &Tweaks->m_TaskMarkerSpecificSettings.m_VanishPreset_Never; break;
	default: break;
	}
	m_GFxInstance.GetMember( "_icon_mc", &m_GFxTaskIcon );
	m_GFxInstance.GetMember( "_description_mc", &m_GFxTaskDesc );
	m_GFxTaskDesc.GetMember( "txt", &m_GFxTaskDescText );
	GFxValue::DisplayInfo Info;
	Info.SetVisible( false );
	m_GFxTaskDesc.SetDisplayInfo( Info );
	m_GFxTaskDescText.SetText( *m_TargetName );
}

// DISHONORED(port): 2013 rva 0x7ce680 (2012 0x831460)
FDisTaskMarker::~FDisTaskMarker()
{
}

// DISHONORED(port): 2013 rva 0x7b9fa0 (2012 0x822d60): the top of the target's collision cylinder.
FVector FDisTaskMarker::GetLocation() const
{
	const FVector Extent = m_pTrackedActor->GetCylinderExtent();
	return m_pTrackedActor->Location + FVector( 0.f, 0.f, Extent.Z );
}

// DISHONORED(port): 2013 rva 0x7c57f0 (2012 0x82af60). The icon scales and fades with distance (the settings' scale
// variation, the vanish preset), points at its target from the screen border when off screen, and fades by the tweaks'
// combat reduction while the player is in combat; the focused marker shows "<name> [<n>m]" when the tweaks show
// distances. The icon's frame is 1, 2 off screen or focused, 3 focused and on screen.
void FDisTaskMarker::Update( FVector2D _Position, FVector2D _TrackedPosition, FLOAT _fDistance, UINT _Flags )
{
	const UINT FrameNumber = ( ( _Flags & ( DHMF_Focused | DHMF_OffScreen ) ) != 0 ) + ( ( _Flags >> 1 ) & 1 ) + 1;
	FDisHUDMarker::Update( _Position, _TrackedPosition, _fDistance, _Flags );
	if( _Flags & DHMF_Visible )
	{
		const FLOAT Scale = m_pSettings->m_ScaleVariation.ComputeValue( _fDistance );
		FLOAT Alpha = m_pVanishPreset ? m_pVanishPreset->ComputeValue( _fDistance ) : 100.f;
		UBOOL bShowDescription = ( _Flags >> 1 ) & 1;
		FLOAT Rotation = 0.f;
		if( _Flags & DHMF_OffScreen )
		{
			const FVector2D FromCentre = FVector2D( _Position.X - m_pGFxMovie->Info.Width * 0.5f, _Position.Y - m_pGFxMovie->Info.Height * 0.5f ).SafeNormal( 1e-8f );
			Rotation = appAcos( Clamp( FromCentre.Y, -1.f, 1.f ) ) * 57.29578f;
			if( FromCentre.X > 0.f )
			{
				Rotation = 360.f - Rotation;
			}
		}
		const ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
		if( PlayerPawn && PlayerPawn->m_bIsInCombat )
		{
			Alpha *= 1.f - m_pSpecificSettings->m_fOpacityReductionInCombat * 0.01f;
			bShowDescription = FALSE;
		}
		GFxValue::DisplayInfo IconInfo;
		IconInfo.SetRotation( Rotation );
		IconInfo.SetScale( Scale, Scale );
		IconInfo.SetAlpha( Alpha );
		m_GFxTaskIcon.SetDisplayInfo( IconInfo );
		if( m_pSpecificSettings->m_bShowDistance && bShowDescription )
		{
			const INT Meters = (INT)( _fDistance * 0.01f );
			if( m_Distance != Meters )
			{
				m_GFxTaskDescText.SetText( *FString::Printf( TEXT("%s [%sm]"), *m_TargetName, *appItoa( Meters ) ) );
				m_Distance = Meters;
			}
		}
		GFxValue::DisplayInfo DescInfo;
		DescInfo.SetVisible( bShowDescription == 1 );
		m_GFxTaskDesc.SetDisplayInfo( DescInfo );
	}
	if( FrameNumber != m_FrameNumber )
	{
		m_GFxTaskIcon.GotoAndPlay( FrameNumber );
		m_FrameNumber = FrameNumber;
	}
}

// DISHONORED(port): 2013 rva 0x7bdd20 (2012 0x826030). The point through the canvas view's WorldToScreen as -1..1 x and
// y-down; behind the camera the point is mirrored and, when asked, its y is pushed toward the bottom edge by how far
// behind it is, so an off-screen marker slides along the border instead of jumping.
static UBOOL ComputeScreenPosition( const FVector& _Location, UCanvas* _pCanvas, UBOOL _bAdjustInFront, UBOOL _bAdjustBehind, FVector2D& _rOutPosition )
{
	const FSceneView* View = _pCanvas->SceneView;
	const FVector4 Screen = View->WorldToScreen( _Location );
	FVector2D Position( Screen.X * ( 1.f / Screen.W ), -( Screen.Y * ( 1.f / Screen.W ) ) );
	const FVector ToPoint = _Location - FVector( View->ViewOrigin );
	const FVector ViewDir( View->ViewMatrix.M[0][2], View->ViewMatrix.M[1][2], View->ViewMatrix.M[2][2] );
	const UBOOL bInFront = ( ToPoint | ViewDir ) > 0.f;
	if( !bInFront )
	{
		Position = -Position;
	}
	if( bInFront ? _bAdjustInFront : _bAdjustBehind )
	{
		const FLOAT Facing = ToPoint.SafeNormal() | ViewDir.SafeNormal();
		Position.Y = -Facing * ( 1.f - Position.Y ) + Position.Y;
	}
	_rOutPosition = Position;
	return bInFront;
}

// DISHONORED(port): 2013 rva 0x7be1a0 (2012 0x8264b0). Zoom shrinks the spread by 8 % at full zoom; triple-screen mode
// spreads x over the centre screen's two neighbours.
UINT DisComputeMarkerScreenPosition( const FVector& _Location, UCanvas* _pCanvas, FLOAT _fZoom, FLOAT _fAreaRatio, FLOAT _fSizeX, FLOAT _fSizeY,
	FVector2D& _rOutTrackedPosition, FVector2D& _rOutPosition, UBOOL _bTripleScreen )
{
	FVector2D Screen;
	const UBOOL bInFront = ComputeScreenPosition( _Location, _pCanvas, FALSE, TRUE, Screen );
	Screen *= 1.f - _fZoom * 0.08f;
	if( _bTripleScreen )
	{
		Screen.X *= 3.f;
	}
	const FLOAT HalfX = _fSizeX * 0.5f;
	const FLOAT HalfY = _fSizeY * 0.5f;
	_rOutTrackedPosition = FVector2D( HalfX + HalfX * Screen.X, HalfY + HalfY * Screen.Y );
	if( bInFront && _fAreaRatio > Abs( Screen.X ) && _fAreaRatio > Abs( Screen.Y ) )
	{
		_rOutPosition = _rOutTrackedPosition;
		return 0;
	}
	FVector2D Border;
	const FLOAT AbsY = Abs( Screen.Y );
	const FLOAT SlopeXY = AbsY > 0.0001f ? Screen.X / AbsY : 1.f;
	if( Abs( SlopeXY ) < 1.f )
	{
		Border.X = SlopeXY * _fAreaRatio;
		Border.Y = Screen.Y < 0.f ? -_fAreaRatio : _fAreaRatio;
	}
	else
	{
		const FLOAT AbsX = Abs( Screen.X );
		const FLOAT SlopeYX = AbsX > 0.0001f ? Screen.Y / AbsX : 1.f;
		Border.X = Screen.X < 0.f ? -_fAreaRatio : _fAreaRatio;
		Border.Y = SlopeYX * _fAreaRatio;
	}
	_rOutPosition = FVector2D( HalfX + HalfX * Border.X, HalfY + HalfY * Border.Y );
	return 1;
}
