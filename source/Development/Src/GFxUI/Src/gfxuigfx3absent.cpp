// GFxUI/Src/gfxuigfx3absent.cpp - the GFx 3.3 entry points and the engine seam, for a build with no GFx
// runtime linked. Agent BE (PHASE8.md package BE item 3: "keep everything behind the existing module
// switch so a build without the GFx runtime still runs").
//
// This is NOT a stub GFx backend in the sense agent AW rejected (gfx_decision.md 3, option A0). It does not
// pretend to be a player: it is the *absence* of one, expressed as link-time symbols so the ported native
// layer compiles and links unchanged. Nothing here is ever called at run time, because GGFxEngine is NULL
// and every wrapped GFxValue stays VT_Undefined, so every ported body takes the early-out retail takes for
// a closed movie. Each entry point therefore returns the neutral answer and nothing logs, which is exactly
// today's observable behaviour: the game stays playable through -newgame and -startmap.
//
// The whole file disappears when DISHONORED_GFXUI_GFX3_RUNTIME is 1 and a real runtime provides these
// symbols instead (see the switch note at the top of Inc/gfxui_gfx3.h). Sources.cmake does not need to
// change for that: the #if is here.
#include "GFxUI.h"
#include "gfxui_gfx3.h"

#if !DISHONORED_GFXUI_GFX3_RUNTIME

/** NULL until the runtime glue creates the engine; every ported body checks it first. With the runtime
    present this global belongs to agent BB's gfxuiengine.cpp instead. */
FGFxEngine* GGFxEngine = NULL;

/*-----------------------------------------------------------------------------
	GRefCountImplCore: the real thing, because our own GFxFunctionHandler
	subclasses are refcounted through it whether or not GFx is present.
-----------------------------------------------------------------------------*/

void GRefCountImplCore::AddRef()
{
	appInterlockedIncrement( (INT*)&RefCount );
}

void GRefCountImplCore::Release()
{
	if( appInterlockedDecrement( (INT*)&RefCount ) == 0 )
	{
		delete this;
	}
}

/*-----------------------------------------------------------------------------
	GFxValue. The copy, assignment and destruction rules are real: they are the
	managed-reference protocol the native layer depends on, and they behave
	correctly with pObjectInterface NULL. Everything that would need a live AS2
	object answers "no".
-----------------------------------------------------------------------------*/

GFxValue::GFxValue( const GFxValue& Other )
	: pObjectInterface( Other.pObjectInterface )
	, Type( Other.Type )
{
	Value = Other.Value;
	if( IsManaged() && pObjectInterface )
	{
		pObjectInterface->ObjectAddRef( this, Value.pData );
	}
}

const GFxValue& GFxValue::operator=( const GFxValue& Other )
{
	if( this != &Other )
	{
		ReleaseManaged();
		pObjectInterface = Other.pObjectInterface;
		Type = Other.Type;
		Value = Other.Value;
		if( IsManaged() && pObjectInterface )
		{
			pObjectInterface->ObjectAddRef( this, Value.pData );
		}
	}
	return *this;
}

GFxValue::~GFxValue()
{
	ReleaseManaged();
}

UBOOL GFxValue::IsObject() const
{
	const ValueType Kind = GetType();
	return Kind == VT_Object || Kind == VT_Array || Kind == VT_DisplayObject;
}

const char* GFxValue::GetString() const
{
	return IsManaged() ? ( Value.pStringManaged ? *Value.pStringManaged : NULL ) : Value.pString;
}

void GFxValue::SetUndefined()
{
	ReleaseManaged();
	Type = VT_Undefined;
	Value.pData = NULL;
}

void GFxValue::SetNull()
{
	ReleaseManaged();
	Type = VT_Null;
	Value.pData = NULL;
}

void GFxValue::SetBoolean( bool In )
{
	ReleaseManaged();
	Type = VT_Boolean;
	Value.BValue = In;
}

void GFxValue::SetNumber( DOUBLE In )
{
	ReleaseManaged();
	Type = VT_Number;
	Value.NValue = In;
}

void GFxValue::SetString( const char* In )
{
	ReleaseManaged();
	Type = VT_String;
	Value.pString = In;
}

void GFxValue::SetStringW( const TCHAR* In )
{
	ReleaseManaged();
	Type = VT_StringW;
	Value.pStringW = In;
}

bool GFxValue::GetMember( const char* Name, GFxValue* Out ) const
{
	return pObjectInterface ? pObjectInterface->GetMember( Value.pData, Name, Out, IsDisplayObject() ) : false;
}

bool GFxValue::SetMember( const char* Name, const GFxValue& In )
{
	return pObjectInterface ? pObjectInterface->SetMember( Value.pData, Name, In, IsDisplayObject() ) : false;
}

bool GFxValue::Invoke( const char* Name, GFxValue* Result, const GFxValue* Args, UINT ArgCount )
{
	return pObjectInterface ? pObjectInterface->Invoke( Value.pData, Result, Name, Args, ArgCount, IsDisplayObject() ) : false;
}

bool GFxValue::Invoke( const char* Name, GFxValue* Result )
{
	return Invoke( Name, Result, NULL, 0 );
}

bool GFxValue::GotoAndPlay( const char* Frame )
{
	return pObjectInterface ? pObjectInterface->GotoAndPlay( Value.pData, Frame, false ) : false;
}

bool GFxValue::GotoAndStop( const char* Frame )
{
	return pObjectInterface ? pObjectInterface->GotoAndPlay( Value.pData, Frame, true ) : false;
}

void GFxValue::VisitMembers( ObjectInterface::ObjVisitor* Visitor ) const
{
	if( pObjectInterface )
	{
		pObjectInterface->VisitMembers( Value.pData, Visitor, IsDisplayObject() );
	}
}

// DISHONORED(port): 2012 rva 0x9ad320 - the whole 2D+3D block in declaration order, every flag set
void GFxValue::DisplayInfo::Set( DOUBLE InX, DOUBLE InY, DOUBLE InRotation, DOUBLE InXScale, DOUBLE InYScale,
	DOUBLE InAlpha, bool bInVisible, DOUBLE InZ, DOUBLE InXRotation, DOUBLE InYRotation, DOUBLE InZScale )
{
	X = InX;
	Y = InY;
	Rotation = InRotation;
	XScale = InXScale;
	YScale = InYScale;
	Alpha = InAlpha;
	Visible = bInVisible;
	Z = InZ;
	XRotation = InXRotation;
	YRotation = InYRotation;
	ZScale = InZScale;
	VarsSet |= V_x | V_y | V_rotation | V_xscale | V_yscale | V_alpha | V_visible | V_z | V_xrotation
		| V_yrotation | V_zscale;
}

/*-----------------------------------------------------------------------------
	GFxValue::ObjectInterface. Unreachable without a runtime: the only way to
	get a non-NULL pObjectInterface is for a live GFxMovieView to have produced
	the value.
-----------------------------------------------------------------------------*/

void GFxValue::ObjectInterface::ObjectAddRef( GFxValue* /*Value*/, void* /*pData*/ ) {}
void GFxValue::ObjectInterface::ObjectRelease( GFxValue* /*Value*/, void* /*pData*/ ) {}
bool GFxValue::ObjectInterface::GetMember( void*, const char*, GFxValue*, bool ) const { return false; }
bool GFxValue::ObjectInterface::SetMember( void*, const char*, const GFxValue&, bool ) { return false; }
bool GFxValue::ObjectInterface::Invoke( void*, GFxValue*, const char*, const GFxValue*, UINT, bool ) { return false; }
UINT GFxValue::ObjectInterface::GetArraySize( void* ) const { return 0; }
bool GFxValue::ObjectInterface::GetElement( void*, UINT, GFxValue* ) const { return false; }
bool GFxValue::ObjectInterface::SetElement( void*, UINT, const GFxValue& ) { return false; }
bool GFxValue::ObjectInterface::PushBack( void*, const GFxValue& ) { return false; }
bool GFxValue::ObjectInterface::GetText( void*, GFxValue*, bool ) const { return false; }
bool GFxValue::ObjectInterface::SetText( void*, const char*, bool ) { return false; }
bool GFxValue::ObjectInterface::SetText( void*, const TCHAR*, bool ) { return false; }
bool GFxValue::ObjectInterface::GetDisplayInfo( void*, DisplayInfo* ) const { return false; }
bool GFxValue::ObjectInterface::SetDisplayInfo( void*, const DisplayInfo& ) { return false; }
bool GFxValue::ObjectInterface::GetDisplayMatrix( void*, GMatrix2D* ) const { return false; }
bool GFxValue::ObjectInterface::SetDisplayMatrix( void*, const GMatrix2D& ) { return false; }
bool GFxValue::ObjectInterface::SetMatrix3D( void*, const GMatrix3D& ) { return false; }
bool GFxValue::ObjectInterface::GetCxform( void*, GRenderer::Cxform* ) const { return false; }
bool GFxValue::ObjectInterface::SetCxform( void*, const GRenderer::Cxform& ) { return false; }
bool GFxValue::ObjectInterface::GotoAndPlay( void*, const char*, bool ) { return false; }
bool GFxValue::ObjectInterface::GotoAndPlay( void*, UINT, bool ) { return false; }
bool GFxValue::ObjectInterface::CreateEmptyMovieClip( void*, GFxValue*, const char*, INT ) { return false; }
bool GFxValue::ObjectInterface::AttachMovie( void*, GFxValue*, const char*, const char*, INT, const GFxValue* ) { return false; }
void GFxValue::ObjectInterface::VisitMembers( void*, ObjVisitor*, bool ) const {}

/*-----------------------------------------------------------------------------
	FGFxMovie / FGFxEngine: the engine seam. FGFxEngine::GetEngine() answering
	NULL is what turns the whole UI off - it is the one check UGFxMoviePlayer::Load
	and ::SetPriority make, and it is agent BB's job to make it answer otherwise.
-----------------------------------------------------------------------------*/

FGFxMovie::FGFxMovie()
	: LastTime( 0.0 )
	, Playing( FALSE )
	, fVisible( TRUE )
	, fUpdate( TRUE )
	, fViewportSet( FALSE )
	, bCanReceiveFocus( FALSE )
	, bCanReceiveInput( FALSE )
	, TimingMode( 0 )
	, pUMovie( NULL )
	, pRenderTexture( NULL )
{
}

FGFxEngine* FGFxEngine::GetEngine()
{
	return GGFxEngine;
}

FGFxMovie* FGFxEngine::LoadMovie( const TCHAR*, UBOOL ) { return NULL; }
void FGFxEngine::StartScene( FGFxMovie*, UTextureRenderTarget2D*, UBOOL, UBOOL ) {}
void FGFxEngine::CloseScene( FGFxMovie*, UBOOL ) {}
void FGFxEngine::InsertMovie( FGFxMovie*, BYTE ) {}
void FGFxEngine::FlushPlayerInput( TSet<INT>* ) {}
FGFxMovie* FGFxEngine::GetTopmostMovie() const { return NULL; }
void FGFxEngine::NotifyGameSessionEnded() {}
void FGFxEngine::ReevaluateFocus() {}
void FGFxEngine::CloseAllMovies( INT ) {}
FGFxMovie* FGFxEngine::GetFocusedMovieFromControllerID( INT ) { return NULL; }

#endif // !DISHONORED_GFXUI_GFX3_RUNTIME
