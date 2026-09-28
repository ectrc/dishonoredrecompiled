// GFxUI/src/gfxuiengine.cpp
// PDB functions attributed to this file (62); the ones this unit ports are named on their bodies with
// the 2013 rva from resources/docs/symbols/match_2012_2013.csv. Declarations and the evidence for who
// calls what are in Inc/gfxuiengine.h.
//
// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the engine seam that makes the reconstructed interface runtime the game's own
// interface. Five packages built the runtime (BB the API and the seam, BC the AS2 machine, CB the text
// engine, CC the renderer, CD the loaders) and every one of them was driven from a harness. This is the
// unit that owns a GFxLoader, creates the FGFxRenderer, opens a movie out of a cooked USwfMovie, keeps
// the list of open movies, advances them once a frame and draws them once a frame.
//
// Three things about it are worth stating before the code:
//
//  1. The movies are opened as *packages*, not files. Retail's url is "/ package/" + the object path
//     with '.' turned into '/' (LoadMovieDef 2013 0x594890, and PACKAGE_PATH in the reference
//     ScaleformEngine.h:89 spells the prefix with its space), and FGFxFileOpener::OpenFile turns that
//     back into a package path and StaticLoadObjects a USwfMovie whose RawData is the payload. So
//     GFile is a memory file over a package's bytes and nothing ever touches the filesystem.
//  2. The interface is advanced from UGFxInteraction::Tick and drawn from UGameViewportClient::Draw.
//     That is measured (build/agentDC/xr.py over the retail database), not assumed, and it corrects
//     PHASE9.md's brief: ADishonoredHUD::PostRender is the canvas HUD and a sibling of the RenderUI
//     call, not its caller.
//  3. FGFxEngine's layout is not reproduced. It is 0x20C bytes in retail and nothing serialises it, so
//     the members are the ones the 62 bodies touch in the order the constructor writes them, without
//     offset assertions. FGFxMovie *is* reproduced, at its retail 104 bytes.
// GFxUI.h first (the generated class headers), then gfxui_gfx3.h - which is where
// DISHONORED_GFXUI_GFX3_RUNTIME is decided and which includes gfxuiengine.h when it is on. Including
// gfxuiengine.h directly would compile this unit with the switch undefined, i.e. to nothing.
#include "GFxUI.h"
#include "gfxui_gfx3.h"

#if DISHONORED_GFXUI_GFX3_RUNTIME

#include "gfxuifile.h"
#include "gfxuiimageinfo.h"
#include "gfxuirendererimpl.h"
#include "GFxDisplay.h"
#include "GFxLoaderImpl.h"
#include "GFxTextField.h"
#include "GFxAS2Runtime.h"
#include "GFxInput.h"

/** the process-wide GFx engine */
FGFxEngine* GGFxEngine = NULL;

void DishonoredGFxRenderUI();

// DISHONORED(bringup): -gfxuiloadtrace narrates the open path step by step, which is how the null call
// in the first in-game load was localised. Off unless asked for.
static UBOOL GFxUILoadTrace()
{
	static INT Cached = -1;
	if( Cached < 0 )
	{
		Cached = ParseParam( appCmdLine(), TEXT("gfxuiloadtrace") ) ? 1 : 0;
	}
	return Cached != 0;
}
#define GFXUI_LOADTRACE(...) if( GFxUILoadTrace() ) { debugf( TEXT("DISHONORED(bringup): GFx load: ") __VA_ARGS__ ); }

/** retail's own switch, already in Engine/Src/UnEngine.cpp's Exec ("DrawGFx") */
extern UBOOL GDrawGFx;
/** the engine's own screenshot request, consumed at the end of UGameViewportClient::Draw */
extern UBOOL GScreenShotRequest;
extern FString GScreenShotName;

// -nogfxui, read on first use. A file-scope `static UBOOL G = ParseParam(appCmdLine(), ...)` in a static
// library is ALWAYS FALSE: static initialisers run before WinMain sets GCmdLine. That cost agent CA a
// whole wave's worth of wrong measurements (PHASE9.md tracker row CA), so it is read here.
UBOOL GFxUIIsDisabled()
{
	static INT Cached = -1;
	if( Cached < 0 )
	{
		Cached = ParseParam( appCmdLine(), TEXT("nogfxui") ) ? 1 : 0;
	}
	return Cached != 0;
}

// DISHONORED(bringup): -gfxuidumpdl walks the display tree of every open movie once and logs it.
static INT GFxUIDumpDisplayList()
{
	static INT Cached = -2;
	if( Cached == -2 )
	{
		INT Frame = 0;
		if( Parse( appCmdLine(), TEXT("gfxuidumpdl="), Frame ) )
		{
			Cached = Frame;
		}
		else
		{
			Cached = ParseParam( appCmdLine(), TEXT("gfxuidumpdl") ) ? 20 : -1;
		}
	}
	return Cached;
}

static void FGFxDumpCharacter( GFxCharacter* Ch, const GMatrix2D& Parent, INT Indent )
{
	if( Ch == NULL || Indent > 12 )
	{
		return;
	}
	GMatrix2D World;
	GFxDisplayMatrixAppend( &World, Parent, Ch->GetMatrix() );
	const GRect<float> Bounds = Ch->GetBoundsTwips( World );
	GFxASCharacter* AsChar = Ch->IsASCharacter() ? Ch->ToASCharacterDef() : NULL;
	const char* Name = ( AsChar != NULL ) ? AsChar->GetName().ToCStr() : "";
	debugf( TEXT("DISHONORED(bringup): dl %*s%s '%s' depth %d vis %d a %.0f  at (%.0f,%.0f) scale (%.2f,%.2f)")
		TEXT("  box (%.0f,%.0f)-(%.0f,%.0f) skew (%.3f,%.3f) frame %d/%d clip %d"),
		Indent * 2, TEXT(""), ANSI_TO_TCHAR(Ch->GetCharacterTypeName()), ANSI_TO_TCHAR(Name),
		Ch->GetDepth(), Ch->GetVisible() ? 1 : 0,
		Ch->GetCxform().M_[3][0] * 100.f + Ch->GetCxform().M_[3][1],
		World.M_[0][2] * 0.05f, World.M_[1][2] * 0.05f, World.M_[0][0], World.M_[1][1],
		Bounds.Left * 0.05f, Bounds.Top * 0.05f, Bounds.Right * 0.05f, Bounds.Bottom * 0.05f,
		World.M_[0][1], World.M_[1][0],
		Ch->IsASCharacter() && Ch->ToASCharacterDef()->ToSprite()
			? (INT)Ch->ToASCharacterDef()->ToSprite()->GetCurrentFrame() : -1,
		Ch->IsASCharacter() && Ch->ToASCharacterDef()->ToSprite()
			? (INT)Ch->ToASCharacterDef()->ToSprite()->GetFrameCount() : -1,
		Ch->GetClipDepth() );
	GFxSprite* Sprite = Ch->IsASCharacter() ? Ch->ToASCharacterDef()->ToSprite() : NULL;
	if( Sprite != NULL )
	{
		GFxDisplayList& List = Sprite->GetDisplayList();
		for( unsigned int i = 0; i < List.GetCount(); ++i )
		{
			FGFxDumpCharacter( List.GetAt( i ), World, Indent + 1 );
		}
	}
}

// DISHONORED(bringup): -gfxuihide=<name>[,...] hides those children of _level0 every frame.
static const TArray<FString>& GFxUIHiddenClips()
{
	static TArray<FString> Names;
	static UBOOL bParsed = FALSE;
	if( !bParsed )
	{
		bParsed = TRUE;
		FString List;
		if( Parse( appCmdLine(), TEXT("gfxuihide="), List, FALSE ) )
		{
			while( List.Len() > 0 )
			{
				const INT Comma = List.InStr( TEXT(",") );
				Names.AddItem( Comma >= 0 ? List.Left( Comma ) : List );
				List = Comma >= 0 ? List.Mid( Comma + 1 ) : FString();
			}
		}
	}
	return Names;
}

// The hide is by NAME anywhere in the tree, so a subtree of a screen can be bisected too.
static void FGFxHideNamed( GFxSprite* Sprite, const TArray<FString>& Names, INT Depth )
{
	if( Sprite == NULL || Depth > 10 )
	{
		return;
	}
	GFxDisplayList& List = Sprite->GetDisplayList();
	for( unsigned int i = 0; i < List.GetCount(); ++i )
	{
		GFxCharacter* Ch = List.GetAt( i );
		GFxASCharacter* AsChar = ( Ch != NULL && Ch->IsASCharacter() ) ? Ch->ToASCharacterDef() : NULL;
		if( AsChar == NULL )
		{
			continue;
		}
		const FString Name = ANSI_TO_TCHAR( AsChar->GetName().ToCStr() );
		for( INT n = 0; n < Names.Num(); n++ )
		{
			if( Name == Names( n ) )
			{
				Ch->SetVisible( false );
			}
		}
		FGFxHideNamed( AsChar->ToSprite(), Names, Depth + 1 );
	}
}

/** -gfxuicensus: log the per-frame census once a second rather than once */
static UBOOL GFxUICensusVerbose()
{
	static INT Cached = -1;
	if( Cached < 0 )
	{
		Cached = ParseParam( appCmdLine(), TEXT("gfxuicensus") ) ? 1 : 0;
	}
	return Cached != 0;
}

// The hook GFx3's GFxLogf calls: the reconstruction has no engine header, so its diagnostics reach the
// log through a function pointer the host installs (GTypes.h, GFxLogHook).
static void FGFxEngineLogHook( const char* Text )
{
	debugf( TEXT("%s"), ANSI_TO_TCHAR( Text ) );
}

/*-----------------------------------------------------------------------------
	FGFxMovie and FAutoGFxValueArray.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 0x5bf9c0
FGFxMovie::FGFxMovie()
	: LastTime( 0.0 )
	, Playing( FALSE )
	, fVisible( FALSE )
	, fUpdate( FALSE )
	, fViewportSet( FALSE )
	, bCanReceiveFocus( TRUE )
	, bCanReceiveInput( TRUE )
	, TimingMode( 0 )
	, pUMovie( NULL )
	, pRenderTexture( NULL )
{
	appMemzero( &Info, sizeof(Info) );
}

// DISHONORED(port): 2013 0x571f80 (2012 0x5b6ac0)
FAutoGFxValueArray::FAutoGFxValueArray( UINT InCount, void* InMemory )
	: pValues( (GFxValue*)InMemory ), Count( InCount )
{
	for( UINT Index = 0; Index < Count; Index++ )
	{
		new( pValues + Index ) GFxValue();
	}
}

// DISHONORED(port): 2013 0x57ac10 (2012 0x5bf4b0)
FAutoGFxValueArray::~FAutoGFxValueArray()
{
	for( UINT Index = 0; Index < Count; Index++ )
	{
		pValues[Index].~GFxValue();
	}
}

/*-----------------------------------------------------------------------------
	FGFxEngineBase.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x574e80 / 0x5720d0. Retail's base holds a GFxSystem by value, whose
// constructor initialises GMemory's global heap and whose destructor tears it down. GFx3Support.cpp's
// allocator is already live process-wide (GMemory::pGlobalHeap is never NULL there), so the base is
// where the ordering guarantee lives rather than where the work is.
FGFxEngineBase::FGFxEngineBase() {}
FGFxEngineBase::~FGFxEngineBase() {}

/*-----------------------------------------------------------------------------
	The package path helpers. FGFxEngine::GetPackagePath is the one every url goes through.
-----------------------------------------------------------------------------*/

#define GFXUI_PACKAGE_PATH "/ package/"
#define GFXUI_PACKAGE_PATH_U TEXT("/ package/")
#define GFXUI_PACKAGE_PATH_LEN 10

/** the reference's IsPackagePath: the url's tail when it carries the package prefix, else NULL */
static const char* GFxIsPackagePath( const char* ppath )
{
	if( ppath == NULL )
	{
		return NULL;
	}
	static const char Prefix[] = GFXUI_PACKAGE_PATH;
	for( INT Index = 0; Index < GFXUI_PACKAGE_PATH_LEN; Index++ )
	{
		if( ppath[Index] != Prefix[Index] )
		{
			return NULL;
		}
	}
	return ppath + GFXUI_PACKAGE_PATH_LEN;
}

// DISHONORED(port): 2012 0x5c9240
FFilename FGFxEngine::CollapseRelativePath( const FFilename& InString )
{
	FFilename ReturnString = InString;
	FPackageFileCache::NormalizePathSeparators( ReturnString );

	const FString SequenceToCollapse = FString( PATH_SEPARATOR ) + TEXT(".") + PATH_SEPARATOR;
	ReturnString.ReplaceInline( *SequenceToCollapse, PATH_SEPARATOR );

	FString LeftString, RightString;
	const FString SearchString = FString( TEXT("..") ) + PATH_SEPARATOR;
	while( ReturnString.Split( SearchString, &LeftString, &RightString ) )
	{
		INT Index = LeftString.Len() - 1;
		if( Index >= 0 && LeftString[Index] == PATH_SEPARATOR[0] )
		{
			--Index;
		}
		while( Index >= 0 && LeftString[Index] != PATH_SEPARATOR[0] )
		{
			LeftString[Index--] = 0;
		}
		ReturnString = FFilename( *LeftString ) + FString( *RightString );
	}
	return ReturnString;
}

// DISHONORED(port): 2013 0x5857f0 (2012 0x5c9610)
UBOOL FGFxEngine::GetPackagePath( const char* ppath, FFilename& strFilename )
{
	const char* pkgpath = GFxIsPackagePath( ppath );
	if( pkgpath == NULL )
	{
		return FALSE;
	}
	strFilename = FFilename( ANSI_TO_TCHAR( pkgpath ) );

	if( appStricmp( *strFilename.GetExtension(), TEXT("gfx") ) == 0
		|| appStricmp( *strFilename.GetExtension(), TEXT("swf") ) == 0 )
	{
		strFilename = strFilename.GetBaseFilename( FALSE );
	}
	strFilename = CollapseRelativePath( strFilename );
	ReplaceCharsInFString( strFilename, PATH_SEPARATOR, L'.' );
	ReplaceCharsInFString( strFilename, TEXT("\\"), L'.' );
	ReplaceCharsInFString( strFilename, TEXT("/"), L'.' );
	return TRUE;
}

/** The seek-free fallback LoadMovieDef applies and the file opener needs too: the cook renames a UI
    package to <name>_SF, so DisFonts.gfxfontlib is really DisFonts_SF.gfxfontlib. Retail reaches this
    through UObject::LoadPackage in LoadMovieDef (2013 0x594890); the opener needs it as well because an
    *imported* movie never goes through LoadMovieDef. Stated as a deviation in agentDC.md. */
template<class T> static T* GFxLoadCookedObject( const FString& Path )
{
	T* Object = LoadObject<T>( NULL, *Path, NULL, LOAD_NoWarn | LOAD_Quiet, NULL );
	if( Object != NULL )
	{
		return Object;
	}
	const INT Dot = Path.InStr( TEXT(".") );
	if( Dot > 0 )
	{
		const FString SeekFree = Path.Left( Dot ) + TEXT("_SF") + Path.Mid( Dot );
		Object = LoadObject<T>( NULL, *SeekFree, NULL, LOAD_NoWarn | LOAD_Quiet, NULL );
		if( Object != NULL )
		{
			return Object;
		}
		if( GUseSeekFreeLoading )
		{
			UObject::LoadPackage( NULL, *( Path.Left( Dot ) + TEXT("_SF") ), LOAD_NoWarn | LOAD_Quiet );
			Object = FindObject<T>( ANY_PACKAGE, *Path.Mid( Dot + 1 ) );
			if( Object != NULL )
			{
				return Object;
			}
		}
	}
	// Last resort: the object's own name anywhere. A seek-free cook has every UI package resident, and
	// an import url names a directory the cook did not keep.
	const INT LastDot = Path.InStr( TEXT("."), TRUE );
	if( LastDot >= 0 )
	{
		Object = FindObject<T>( ANY_PACKAGE, *Path.Mid( LastDot + 1 ) );
	}
	return Object;
}

/*-----------------------------------------------------------------------------
	FGFxFileOpener - the PDB attributes both slots to this file (gfxuiengine.cpp:251 / :303).
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x58d780 (2012 0x5dda60)
GFile* FGFxFileOpener::OpenFile( const char* Url, INT Flags, INT Mode )
{
	(void)Flags;
	(void)Mode;
	FFilename Filename;
	if( !FGFxEngine::GetPackagePath( Url, Filename ) )
	{
		// Retail opens a real file through GSysFile + GBufferedFile here. Nothing in the retail cook
		// ever reaches it - every url the runtime builds carries the package prefix - so the path is
		// reported rather than implemented.
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): GFx file opener asked for a filesystem path '%s'"),
			ANSI_TO_TCHAR( Url ) );
		return NULL;
	}
	USwfMovie* Movie = GFxLoadCookedObject<USwfMovie>( Filename );
	if( Movie == NULL || Movie->RawData.Num() == 0 )
	{
		return NULL;
	}
	return new FGFxFile( Url, &Movie->RawData(0), Movie->RawData.Num() );
}

// DISHONORED(port): 2013 0x58d9a0. Cooked content never changes under us, so retail's answer of 0 stands.
__int64 FGFxFileOpener::GetFileModifyTime( const char* Url )
{
	(void)Url;
	return 0;
}

GFile* FGFxFileOpener::OpenFileEx( const char* Url, GFxLog* Log, INT Flags, INT Mode )
{
	(void)Log;
	return OpenFile( Url, Flags, Mode );
}

/*-----------------------------------------------------------------------------
	FGFxURLBuilder - gfxuiengine.cpp:334.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x585a50 (2012 0x5c9870). The absolute arm is retail's, including the
// gamedir:// rewrite; the relative arm is GFxURLBuilder's own base behaviour, which the reconstruction
// keeps in GFxLoaderImpl::BuildURL because the import binding needs the same rule.
void FGFxURLBuilder::BuildURL( GString* ppath, const GFxURLBuilder::LocationInfo& loc )
{
	// DISHONORED(bringup): retail's body rewrites gamedir:// against appGameDir and otherwise falls
	// through to GFxURLBuilder's own relative resolution. It cannot be written here for a concrete
	// reason: GTypes.h's GString is the PDB's four-byte handle with ToCStr and GetSize and no
	// allocator, so a GString cannot be *built*. Nothing in this runtime calls the state either -
	// GFxLoaderImpl::ResolveImportMovie applies the same rule with its own buffer
	// (GFxLoaderImpl::BuildURL) because the import binding needs it before any state bag exists. The
	// state is installed so a future caller finds it and so the loader's bag matches retail's.
	(void)ppath;
	(void)loc;
	static UBOOL bWarned = FALSE;
	if( !bWarned )
	{
		bWarned = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FGFxURLBuilder::BuildURL reached for '%s'; ")
			TEXT("the reconstruction resolves import urls in GFxLoaderImpl instead"),
			ANSI_TO_TCHAR( loc.FileName.ToCStr() ) );
	}
}

/*-----------------------------------------------------------------------------
	FGFxImageLoader / FGFxImageCreator - gfxuiengine.cpp:410 / :363.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x586020 (2012 0x5c9e90). The url is the image's name resolved against the
// movie's own url, so it carries the package prefix and GetPackagePath is what turns it into
// <Package>.<TextureName>. Retail's body instead skips to the first '/' run, which is the same result
// for the "protocol://path" urls GFx builds and is recorded in agentDC.md as the one difference.
GImageInfoBase* FGFxImageLoader::LoadImageW( const char* Url )
{
	FFilename Filename;
	if( !FGFxEngine::GetPackagePath( Url, Filename ) )
	{
		// the retail shape: everything after the first run of '/'
		const char* p = Url;
		while( *p && *p != '/' ) ++p;
		while( *p == '/' ) ++p;
		if( *p == 0 )
		{
			return NULL;
		}
		Filename = FFilename( ANSI_TO_TCHAR( p ) );
		FGFxEngine::ReplaceCharsInFString( Filename, TEXT("/"), L'.' );
	}
	UTexture* Texture = GFxLoadCookedObject<UTexture>( Filename );
	UTexture2D* Texture2D = Cast<UTexture2D>( Texture );
	if( Texture2D != NULL )
	{
		// DISHONORED(bringup): the cooked format matters to the interface - an image with no alpha
		// channel is drawn as an opaque rectangle over whatever is under it.
		debugf( TEXT("DISHONORED(bringup): GFx image %s: %dx%d format %d LODGroup %d"),
			*Filename, Texture2D->SizeX, Texture2D->SizeY, (INT)Texture2D->Format,
			(INT)Texture2D->LODGroup );
		FGFxImageInfo* Info = new FGFxImageInfo( Filename, Texture2D->SizeX, Texture2D->SizeY );
		Info->SetEngineTexture( Texture2D );
		return Info;
	}
	UTextureRenderTarget2D* RenderTarget = Cast<UTextureRenderTarget2D>( Texture );
	if( RenderTarget != NULL )
	{
		FGFxImageInfo* Info = new FGFxImageInfo( Filename, RenderTarget->SizeX, RenderTarget->SizeY );
		Info->SetEngineTexture( RenderTarget );
		return Info;
	}
	return NULL;
}

// DISHONORED(port): 2013 0x585d70 (2012 0x5c9be0)
GImageInfoBase* FGFxImageCreator::CreateImage( const GFxImageCreateInfo& Info )
{
	// GFxImageCreateInfo's pImage and pFileInfo are one union slot (GFx3Gen.h says so at the member),
	// so Input_File reads the same pointer as a GFxImageFileInfo. That is the layout, not a cast trick.
	GImageInfo* Result = NULL;
	GFxImageFileInfo* FileInfo = ( Info.Type == GFxImageCreateInfo::Input_File )
		? (GFxImageFileInfo*)Info.pImage : NULL;
	if( FileInfo != NULL )
	{
		FFilename Filename;
		if( FGFxEngine::GetPackagePath( FileInfo->FileName.ToCStr(), Filename ) )
		{
			FGFxImageInfo* FileImage = new FGFxImageInfo( Filename, FileInfo->TargetWidth,
				FileInfo->TargetHeight );
			UTexture2D* Texture2D = GFxLoadCookedObject<UTexture2D>( Filename );
			if( Texture2D != NULL )
			{
				FileImage->SetEngineTexture( Texture2D );
			}
			Result = FileImage;
		}
	}
	if( Result == NULL )
	{
		// GImageInfo is abstract in the reconstruction, so the empty answer is an FGFxImageInfo with no
		// name and no texture - which is what retail's GImageInfo(0, 0) is for this layer's purposes.
		FGFxImageInfo* Empty = new FGFxImageInfo( FString(), 0, 0 );
		Empty->pImage = ( Info.Type == GFxImageCreateInfo::Input_Image ) ? Info.pImage : NULL;
		Result = Empty;
	}
	if( Info.pRenderConfig != NULL )
	{
		Result->GetTexture( Info.pRenderConfig->pRenderer.GetPtr() );
	}
	return Result;
}

/*-----------------------------------------------------------------------------
	FGFxEngine: construction.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x590ba0 (2012 0x5d1600). Every constant here is read out of that body: the
// file opener and url builder states, the player log, and the font cache's texture configuration -
// one 1024x1024 texture, 2-pixel slot padding, 48-pixel maximum slot height, a 128x256 update block,
// the dynamic cache on and a maximum raster scale of 1.25.
void FGFxEngine::InitGFxLoaderCommon( GFxLoader& InLoader )
{
	FGFxFileOpener* Opener = new FGFxFileOpener;
	Opener->SType = GFxState::State_FileOpener;
	InLoader.SetState( GFxState::State_FileOpener, Opener );
	Opener->Release();

	FGFxURLBuilder* UrlBuilder = new FGFxURLBuilder;
	UrlBuilder->SType = GFxState::State_URLBuilder;
	InLoader.SetState( GFxState::State_URLBuilder, UrlBuilder );
	UrlBuilder->Release();

	// The font cache configuration. The cache itself is the process-wide GFxGlyphRasterCache of
	// GFxDisplay.cpp (agent CB's rasteriser packs into it); retail hangs it off a GFxFontCacheManager
	// state, which this reconstruction does not have, so the configuration is applied to the cache
	// directly. Stated in agentDC.md.
	GFxDisplayGetGlyphCache();
}

FGFxEngine::FGFxEngine()
	: GameHasRendered( 0 )
	, HudViewport( NULL )
	, CurveError( 2.0f )
	, FontlibInitialized( FALSE )
	, bCensusLogged( FALSE )
	, DrawnFrames( 0 )
{
	// DISHONORED(port): 2013 0x5a2370 (2012 0x5e2900), in the order retail writes the members.
	appMemzero( &RenderCensus, sizeof(RenderCensus) );
	MousePos.X = 0;
	MousePos.Y = 0;
	GGFxEngine = this;

	// Every AS2 error and every trace line the reconstruction emits reaches Launch.log through this.
	GFxLogHook = &FGFxEngineLogHook;
	GFxMovieRoot::bTraceConstruction = GFxUILoadTrace() ? true : false;
	// Bisect switches for the display walk; see GFxDisplay.h.
	GFxDisplayNoShapes = ParseParam( appCmdLine(), TEXT("gfxuinoshapes") ) ? true : false;
	GFxDisplayNoText = ParseParam( appCmdLine(), TEXT("gfxuinotext") ) ? true : false;
	GFxDisplayNoImages = ParseParam( appCmdLine(), TEXT("gfxuinoimages") ) ? true : false;
	GFxDisplayNoBeginDisplay = ParseParam( appCmdLine(), TEXT("gfxuinodisplay") ) ? true : false;
	GFxDisplayFitFill = ParseParam( appCmdLine(), TEXT("gfxuifitfill") ) ? true : false;
	GFxDisplayNoTextShadow = ParseParam( appCmdLine(), TEXT("gfxuinotextshadow") ) ? true : false;

	InitGFxLoaderCommon( Loader );

	FGFxImageCreator* ImageCreator = new FGFxImageCreator;
	ImageCreator->SType = GFxState::State_ImageCreator;
	Loader.SetState( GFxState::State_ImageCreator, ImageCreator );
	ImageCreator->Release();

	FGFxImageLoader* ImageLoader = new FGFxImageLoader;
	ImageLoader->SType = GFxState::State_ImageLoader;
	Loader.SetState( GFxState::State_ImageLoader, ImageLoader );
	ImageLoader->Release();

	FGFxFSCommandHandler* FSCommands = new FGFxFSCommandHandler;
	FSCommands->SType = GFxState::State_FSCommandHandler;
	Loader.SetState( GFxState::State_FSCommandHandler, FSCommands );
	FSCommands->Release();

	FGFxExternalInterface* ExternalInterface = new FGFxExternalInterface;
	ExternalInterface->SType = GFxState::State_ExternalInterface;
	Loader.SetState( GFxState::State_ExternalInterface, ExternalInterface );
	ExternalInterface->Release();

	InitRenderer();
	InitKeyMap();
}

void FGFxEngine::InitRenderer()
{
	// 2013 0x5a2370's renderer block: one FGFxRenderer (892 bytes), a GFxRenderConfig naming it with
	// RF_EdgeAA | RF_OptimizeTriangles | RF_StrokeNormal (the 0x11 the retail constructor passes) and
	// the curve error, then two render targets - the HUD's and the scene-colour one.
	pRenderer = new FGFxRenderer;
	pRenderer->Release();                 // GPtr took its own reference; RefCount starts at 1

	GFxRenderConfig* Config = new GFxRenderConfig;
	Config->SType = GFxState::State_RenderConfig;
	Config->pRenderer = pRenderer.GetPtr();
	Config->RenderFlags = 0x11;
	Config->StrokerAAWidth = 0.0f;
	Config->RendererCapBits = 0;
	Config->RendererVtxFmts = 0;
	FLOAT Error = CurveError;
	if( Error >= 1000000.0f )
	{
		Error = 1000000.0f;
	}
	else if( Error < 0.000001f )
	{
		Error = 0.000001f;
	}
	Config->MaxCurvePixelError = Error;
	Loader.SetState( GFxState::State_RenderConfig, Config );
	Config->Release();

	HudRenderTarget = pRenderer->CreateRenderTarget();
	HudRenderTarget->Release();
	SceneColorRT = pRenderer->CreateRenderTarget();
	SceneColorRT->Release();
}

void FGFxEngine::InitLocalization()
{
	// DISHONORED(bringup): retail installs a GFxTranslator (FGFxTranslator, gfxuilocalization.cpp) and
	// a font map here, which is how a UI movie's `$strings` and its per-language font substitution
	// work. Neither is reconstructed; the fonts come in through the import binding instead, which is
	// what makes text appear at all. Named in agentDC.md.
	FontlibInitialized = TRUE;
}

FGFxEngine::~FGFxEngine()
{
	// DISHONORED(port): 2013 0x59f190. Close everything, drain the deletion queue with a fence, drop
	// the render targets and the renderer, then the loader's own implementation.
	CloseAllMovies( FALSE );
	CloseAllTextureMovies();
	DeleteQueuedMovies( TRUE );
	GFxDisplayReleaseShapeMeshes();
	GFxDisplayResetGlyphCache();
	HudRenderTarget.Clear();
	SceneColorRT.Clear();
	if( pRenderer.GetPtr() != NULL )
	{
		pRenderer->ReleaseResources();
	}
	pRenderer.Clear();
	Loader.Shutdown();
	if( GGFxEngine == this )
	{
		GGFxEngine = NULL;
	}
}

// DISHONORED(port): 2013 0x5a2b60 (2012 0x5e3450). Retail also creates the UGFxEngine GC manager here
// and roots it; that object exists in this tree (GFxUIClasses.h) and its only job is to keep the
// textures GFx created alive, which agent CC's renderer does with AddToRoot instead (agentCC.md 6.4).
FGFxEngine* FGFxEngine::GetEngine()
{
	if( GGFxEngine != NULL )
	{
		return GGFxEngine;
	}
	if( GFxUIIsDisabled() )
	{
		return NULL;
	}
	GGFxEngine = new FGFxEngine;
	// DISHONORED(bringup): the address of a known symbol, so a crash frame can be mapped back to the
	// link map (the image is /DYNAMICBASE, so the printed addresses are not the map's).
	debugf( TEXT("DISHONORED(bringup): GFx base probe: DishonoredGFxRenderUI at %p"),
		(void*)&DishonoredGFxRenderUI );
	debugf( TEXT("DISHONORED(bringup): GFx engine up: renderer %s, loader %s"),
		GGFxEngine->pRenderer.GetPtr() ? TEXT("created") : TEXT("MISSING"),
		GGFxEngine->Loader.pImpl ? TEXT("created") : TEXT("MISSING") );
	return GGFxEngine;
}

/*-----------------------------------------------------------------------------
	Loading and starting movies.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x594890 (2012 0x5de0a0)
GFxMovieDef* FGFxEngine::LoadMovieDef( const TCHAR* ppath, GFxMovieInfo& Info )
{
	if( ppath == NULL )
	{
		return NULL;
	}
	// The object has to exist before the loader asks the file opener for it, because the opener reads
	// the payload out of the loaded USwfMovie. Retail does the same load here and uses it for one more
	// thing: bUsesFontlib, which is what turns the font library on.
	USwfMovie* MovieAsset = GFxLoadCookedObject<USwfMovie>( FString(ppath) );
	if( MovieAsset != NULL && MovieAsset->bUsesFontlib && !FontlibInitialized )
	{
		InitLocalization();
	}

	FString MoviePath = FString( GFXUI_PACKAGE_PATH_U ) + ppath;
	ReplaceCharsInFString( MoviePath, TEXT("."), L'/' );

	GFXUI_LOADTRACE( TEXT("GetMovieInfo %s"), *MoviePath );
	if( !Loader.GetMovieInfo( TCHAR_TO_ANSI( *MoviePath ), &Info, false, 0 ) )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): GFx LoadMovieDef: no info for %s"), ppath );
		return NULL;
	}
	GFXUI_LOADTRACE( TEXT("CreateMovie %s"), *MoviePath );
	GFxMovieDef* Def = Loader.CreateMovie( TCHAR_TO_ANSI( *MoviePath ), 0, 0 );
	GFXUI_LOADTRACE( TEXT("CreateMovie -> %s"), Def ? TEXT("ok") : TEXT("null") );
	return Def;
}

// DISHONORED(port): 2012 0x5de3e0. sizeof(FGFxMovie) is retail's 104 (appMalloc(0x68, 8)).
FGFxMovie* FGFxEngine::LoadMovie( const TCHAR* ppath, UBOOL fInitFirstFrame )
{
	FGFxMovie* Movie = new FGFxMovie;
	Movie->FileName = ppath;

	Movie->pDef = LoadMovieDef( ppath, Movie->Info );
	if( Movie->pDef.GetPtr() == NULL )
	{
		delete Movie;
		return NULL;
	}
	// The def impl owns the reference CreateMovie returned.
	Movie->pDef->Release();

	GFxMovieDef::MemoryParams MemParams;
	appMemzero( &MemParams, sizeof(MemParams) );
	GFXUI_LOADTRACE( TEXT("CreateInstance") );
	Movie->pView = Movie->pDef->CreateInstance( MemParams, fInitFirstFrame != 0 );
	GFXUI_LOADTRACE( TEXT("CreateInstance -> %s"), Movie->pView.GetPtr() ? TEXT("ok") : TEXT("null") );
	if( Movie->pView.GetPtr() == NULL )
	{
		Movie->pDef.Clear();
		delete Movie;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): GFx LoadMovie: no instance for %s"), ppath );
		return NULL;
	}
	Movie->pView->Release();

	debugf( TEXT("DISHONORED(bringup): GFx movie loaded: %s  %dx%d  %.1f fps  %u frames"),
		ppath, Movie->Info.Width, Movie->Info.Height, Movie->Info.FPS, Movie->Info.FrameCount );
	return Movie;
}

// DISHONORED(port): 2013 0x58e300 (2012 0x5cf0d0)
void FGFxEngine::StartScene( FGFxMovie* Movie, UTextureRenderTarget2D* RenderTexture, UBOOL fVisible,
	UBOOL fUpdate )
{
	if( Movie == NULL )
	{
		return;
	}
	Movie->Playing = TRUE;
	Movie->fVisible = fVisible;
	Movie->fUpdate = fUpdate;
	Movie->pRenderTexture = RenderTexture;

	if( RenderTexture == NULL )
	{
		// A viewport movie draws over whatever is already there, so its own background is transparent.
		GFXUI_LOADTRACE( TEXT("StartScene: background alpha") );
		Movie->pView->SetBackgroundAlpha( 0.0f );
		GFXUI_LOADTRACE( TEXT("StartScene: SetMovieSize") );
		SetMovieSize( Movie );
		GFXUI_LOADTRACE( TEXT("StartScene: cursor") );
		Movie->pView->SetMouseCursorCount( 1 );
		GFXUI_LOADTRACE( TEXT("StartScene: InsertMovie") );
		InsertMovie( Movie, SDPG_Foreground );
		GFXUI_LOADTRACE( TEXT("StartScene: inserted") );
	}
	else
	{
		UINT ViewFlags = 0;
		if( Movie->pUMovie != NULL )
		{
			if( Movie->pUMovie->RenderTextureMode == RTM_AlphaComposite )
			{
				ViewFlags = 2;
			}
			if( Movie->pUMovie->RenderTextureMode == RTM_Alpha
				|| Movie->pUMovie->RenderTextureMode == RTM_AlphaComposite )
			{
				Movie->pView->SetBackgroundAlpha( 0.0f );
			}
			if( !Movie->pUMovie->bEnableGammaCorrection )
			{
				ViewFlags |= 0x1000;
			}
		}
		const INT W = (INT)RenderTexture->GetSurfaceWidth();
		const INT H = (INT)RenderTexture->GetSurfaceHeight();
		GViewport View( W, H, 0, 0, W, H );
		View.Flags = ViewFlags;
		Movie->pView->SetViewport( View );
		TextureMovies.AddItem( Movie );
	}

	Movie->LastTime = ( Movie->TimingMode == TM_Real ) ? GCurrentTime
		: ( GWorld ? GWorld->GetTimeSeconds() : 0.0 );

	if( Movie->pUMovie != NULL && Movie->pUMovie->bPauseGameWhileActive )
	{
		UGameUISceneClient* SceneClient = UUIRoot::GetSceneClient();
		if( SceneClient != NULL )
		{
			for( INT PlayerIndex = 0; PlayerIndex < GEngine->GamePlayers.Num(); PlayerIndex++ )
			{
				if( PlayerIndex == Movie->pUMovie->LocalPlayerOwnerIndex )
				{
					SceneClient->UpdatePausedState( PlayerIndex );
					break;
				}
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): GFx scene started: %s (%s)"), *Movie->FileName,
		RenderTexture ? TEXT("render texture") : TEXT("viewport") );
}

// DISHONORED(port): 2013 0x585350 (2012 0x5c9160)
void FGFxEngine::InsertMovieIntoList( FGFxMovie* Movie, TArray<FGFxMovie*>* List )
{
	List->RemoveItem( Movie );
	INT Index = 0;
	for( ; Index < List->Num(); Index++ )
	{
		FGFxMovie* Other = (*List)( Index );
		const BYTE OtherPriority = Other->pUMovie ? Other->pUMovie->Priority : 0;
		const BYTE MoviePriority = Movie->pUMovie ? Movie->pUMovie->Priority : 0;
		if( OtherPriority > MoviePriority )
		{
			List->InsertItem( Movie, Index );
			break;
		}
	}
	if( Index == List->Num() )
	{
		List->AddItem( Movie );
	}
}

// DISHONORED(port): 2013 0x58d4a0 (2012 0x5ce820)
void FGFxEngine::InsertMovie( FGFxMovie* Movie, BYTE DPG )
{
	INT Index = INDEX_NONE;
	if( !AllMovies.FindItem( Movie, Index ) )
	{
		AllMovies.AddItem( Movie );
	}
	InsertMovieIntoList( Movie, &OpenMovies );
	InsertMovieIntoList( Movie, &DPGOpenMovies[DPG] );
	ReevaluateFocus();
}

INT FGFxEngine::GetNextMovieDPG( INT FirstDPG )
{
	for( INT DPG = FirstDPG; DPG <= SDPG_PostProcess; DPG++ )
	{
		if( DPGOpenMovies[DPG].Num() )
		{
			return DPG;
		}
	}
	return INDEX_NONE;
}

// DISHONORED(port): 2013 0x58dab0 (2012 0x5ce890)
void FGFxEngine::CloseScene( FGFxMovie* pMovie, UBOOL fDelete )
{
	if( pMovie == NULL )
	{
		return;
	}
	pMovie->Playing = FALSE;
	INT Index = INDEX_NONE;
	if( OpenMovies.FindItem( pMovie, Index ) )
	{
		OpenMovies.Remove( Index );
		for( INT DPG = 0; DPG <= SDPG_PostProcess; DPG++ )
		{
			DPGOpenMovies[DPG].RemoveItem( pMovie );
		}
	}
	else if( TextureMovies.FindItem( pMovie, Index ) )
	{
		TextureMovies.Remove( Index );
	}
	if( fDelete || pMovie->pUMovie == NULL )
	{
		DeleteMovies.AddItem( pMovie );
		pMovie->RenderCmdFence.BeginFence();
	}
	ReevaluateFocus();
	if( fDelete && pMovie->pUMovie != NULL )
	{
		pMovie->pUMovie->pMovie = NULL;
		pMovie->pUMovie = NULL;
	}
}

// DISHONORED(port): 2013 0x58dbf0 (2012 0x5ce9c0)
void FGFxEngine::DeleteQueuedMovies( UBOOL bWait )
{
	if( DeleteMovies.Num() == 0 )
	{
		return;
	}
	TArray<FGFxMovie*> Remaining;
	for( INT Index = 0; Index < DeleteMovies.Num(); Index++ )
	{
		FGFxMovie* Movie = DeleteMovies( Index );
		if( Movie == NULL )
		{
			continue;
		}
		if( bWait && Movie->RenderCmdFence.GetNumPendingFences() )
		{
			Remaining.AddItem( Movie );
			continue;
		}
		Movie->pView.Clear();
		Movie->pDef.Clear();
		delete Movie;
	}
	DeleteMovies = Remaining;
}

// DISHONORED(port): 2013 0x590d90 (2012 0x5d17f0)
void FGFxEngine::CloseAllMovies( INT bOnlyCloseOnLevelChangeMovies )
{
	for( INT Index = OpenMovies.Num() - 1; Index >= 0; Index-- )
	{
		FGFxMovie* Movie = OpenMovies( Index );
		if( bOnlyCloseOnLevelChangeMovies && Movie->pUMovie != NULL
			&& !Movie->pUMovie->bCloseOnLevelChange )
		{
			continue;
		}
		if( Movie->pUMovie != NULL )
		{
			Movie->pUMovie->Close( TRUE );
		}
		else
		{
			CloseScene( Movie, TRUE );
		}
	}
}

// DISHONORED(port): 2013 0x590e70 (2012 0x5d18d0)
void FGFxEngine::CloseAllTextureMovies()
{
	for( INT Index = TextureMovies.Num() - 1; Index >= 0; Index-- )
	{
		FGFxMovie* Movie = TextureMovies( Index );
		if( Movie->pUMovie != NULL )
		{
			Movie->pUMovie->Close( TRUE );
		}
		else
		{
			CloseScene( Movie, TRUE );
		}
	}
}

// DISHONORED(port): 2013 0x590ef0 (2012 0x5d1950)
void FGFxEngine::CloseTopmostScene()
{
	FGFxMovie* Movie = GetTopmostMovie();
	if( Movie == NULL )
	{
		return;
	}
	if( Movie->pUMovie != NULL )
	{
		Movie->pUMovie->Close( TRUE );
	}
	else
	{
		CloseScene( Movie, TRUE );
	}
}

// DISHONORED(port): 2013 0x594750 (2012 0x5d49b0)
void FGFxEngine::NotifyGameSessionEnded()
{
	CloseAllMovies( TRUE );
	CloseAllTextureMovies();
	DeleteQueuedMovies( TRUE );
}

// DISHONORED(port): 2012 0x5bfb90
FGFxMovie* FGFxEngine::GetTopmostMovie() const
{
	return OpenMovies.Num() ? OpenMovies( OpenMovies.Num() - 1 ) : NULL;
}

// DISHONORED(port): 2013 0x57b2a0 (2012 0x5bfbe0)
FGFxMovie* FGFxEngine::GetOpenMovie( INT Index ) const
{
	return ( Index >= 0 && Index < OpenMovies.Num() ) ? OpenMovies( Index ) : NULL;
}

/*-----------------------------------------------------------------------------
	Viewport and size.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x57b300 (2012 0x5bfc40). The movie's viewport is the whole window unless the
// player it belongs to has a split-screen sub-rectangle, which is what ULocalPlayer's four normalised
// Origin/Size floats are. The 0x1000 view flag is retail's "no gamma": it is set unless the movie asks
// for gamma correction, which is the bit at +196 the decompile tests with 4.
void FGFxEngine::SetMovieSize( FGFxMovie* Movie )
{
	UINT BaseSizeX = 1280;
	UINT BaseSizeY = 720;
	if( HudViewport != NULL )
	{
		BaseSizeX = HudViewport->GetSizeX();
		BaseSizeY = HudViewport->GetSizeY();
	}
	INT X = 0;
	INT Y = 0;
	INT SizeX = (INT)BaseSizeX;
	INT SizeY = (INT)BaseSizeY;

	if( Movie->pUMovie != NULL && Movie->pUMovie->bOnlyOwnerFocusable
		&& Movie->pUMovie->LocalPlayerOwnerIndex < GEngine->GamePlayers.Num() )
	{
		ULocalPlayer* Player = GEngine->GamePlayers( Movie->pUMovie->LocalPlayerOwnerIndex );
		if( Player != NULL )
		{
			X = appTrunc( Player->Origin.X * (FLOAT)BaseSizeX );
			Y = appTrunc( Player->Origin.Y * (FLOAT)BaseSizeY );
			SizeX = appTrunc( Player->Size.X * (FLOAT)BaseSizeX );
			SizeY = appTrunc( Player->Size.Y * (FLOAT)BaseSizeY );
		}
	}

	UINT ViewFlags = 0x1000;
	if( Movie->pUMovie != NULL && Movie->pUMovie->bEnableGammaCorrection )
	{
		ViewFlags = 0;
	}
	GViewport View( (INT)BaseSizeX, (INT)BaseSizeY, X, Y, SizeX, SizeY );
	View.Flags = ViewFlags;
	Movie->pView->SetViewport( View );
	Movie->fViewportSet = TRUE;
}

// DISHONORED(port): 2013 0x586b00 (2012 0x5ca600)
void FGFxEngine::ReevaluateSizes()
{
	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		SetMovieSize( OpenMovies( Index ) );
	}
}

// DISHONORED(port): 2013 0x57af00 (2012 0x5bf7d0). The HUD render target is re-initialised from the
// viewport here, which is what makes a resize re-create the surfaces rather than draw into the old ones.
void FGFxEngine::SetRenderViewport( FViewport* inViewport )
{
	HudViewport = inViewport;
	if( inViewport == NULL )
	{
		if( HudRenderTarget.GetPtr() != NULL )
		{
			FGFxRenderTargetResource::NativeRenderTarget Native;
			HudRenderTarget->InitRenderTarget( Native );
		}
		return;
	}

	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		FGFxMovie* Movie = OpenMovies( Index );
		if( Movie->fViewportSet )
		{
			// Retail reads the movie's own viewport back, updates the buffer size and writes it again,
			// so a movie that set a sub-rectangle of its own keeps it across a resize.
			GViewport View;
			appMemzero( &View, sizeof(View) );
			View.Width = 1;
			View.Height = 1;
			View.AspectRatio = 1.0f;
			View.Scale = 1.0f;
			Movie->pView->GetViewport( &View );
			View.BufferWidth = inViewport->GetSizeX();
			View.BufferHeight = inViewport->GetSizeY();
			Movie->pView->SetViewport( View );
		}
		else
		{
			SetMovieSize( Movie );
		}
	}

	FGFxRenderTargetResource::NativeRenderTarget Native( inViewport, NULL );
	HudRenderTarget->InitRenderTarget( Native );
}

/*-----------------------------------------------------------------------------
	Focus and player state.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 0x5cf2a0
void FGFxEngine::AddPlayerState()
{
	PlayerStates.AddItem( FGFxLocalPlayerState() );
}

void FGFxEngine::RemovePlayerState( INT Index )
{
	if( Index >= 0 && Index < PlayerStates.Num() )
	{
		PlayerStates.Remove( Index );
	}
}

// DISHONORED(port): 2013 0x57b4f0 (2012 0x5bfe30)
INT FGFxEngine::GetLocalPlayerIndexFromControllerID( INT ControllerId, UINT bFailIfMissing )
{
	for( INT Index = 0; Index < GEngine->GamePlayers.Num(); Index++ )
	{
		ULocalPlayer* Player = GEngine->GamePlayers( Index );
		if( Player != NULL && Player->ControllerId == ControllerId )
		{
			return Index;
		}
	}
	return bFailIfMissing ? INDEX_NONE : 0;
}

// DISHONORED(port): 2013 0x57b580 (2012 0x5bfec0)
FGFxMovie* FGFxEngine::GetFocusedMovieFromControllerID( INT ControllerId )
{
	const INT PlayerIndex = GetLocalPlayerIndexFromControllerID( ControllerId, 1 );
	if( PlayerIndex >= 0 && PlayerIndex < PlayerStates.Num() )
	{
		return PlayerStates( PlayerIndex ).FocusedMovie;
	}
	return NULL;
}

// DISHONORED(port): 2013 0x586b60 (2012 0x5ca660). The topmost movie that can take focus wins, per
// local player, and the movies whose focus changed get the script events. Retail's body is 2,068 bytes
// because it also walks the owner-only case and the controller focus groups.
void FGFxEngine::ReevaluateFocus()
{
	while( PlayerStates.Num() < GEngine->GamePlayers.Num() )
	{
		AddPlayerState();
	}
	if( PlayerStates.Num() == 0 )
	{
		return;
	}

	TArray<FGFxMovie*> Old;
	Old.Add( PlayerStates.Num() );
	for( INT Index = 0; Index < PlayerStates.Num(); Index++ )
	{
		Old( Index ) = PlayerStates( Index ).FocusedMovie;
		PlayerStates( Index ).FocusedMovie = NULL;
	}

	for( INT Index = OpenMovies.Num() - 1; Index >= 0; Index-- )
	{
		FGFxMovie* Movie = OpenMovies( Index );
		UGFxMoviePlayer* Player = Movie->pUMovie;
		if( Player == NULL || !Movie->bCanReceiveFocus )
		{
			continue;
		}
		if( Player->bOnlyOwnerFocusable )
		{
			const INT Owner = Player->LocalPlayerOwnerIndex;
			if( Owner >= 0 && Owner < PlayerStates.Num() && PlayerStates( Owner ).FocusedMovie == NULL )
			{
				PlayerStates( Owner ).FocusedMovie = Movie;
			}
		}
		else
		{
			for( INT PlayerIndex = 0; PlayerIndex < PlayerStates.Num(); PlayerIndex++ )
			{
				if( PlayerStates( PlayerIndex ).FocusedMovie == NULL )
				{
					PlayerStates( PlayerIndex ).FocusedMovie = Movie;
				}
			}
		}
	}

	for( INT Index = 0; Index < PlayerStates.Num(); Index++ )
	{
		FGFxMovie* NewFocus = PlayerStates( Index ).FocusedMovie;
		FGFxMovie* OldFocus = Old( Index );
		if( NewFocus == OldFocus )
		{
			continue;
		}
		// DISHONORED(bringup): the two focus events are script functions, and FindFunctionChecked
		// appErrors when a class does not have one. UDisGFxMoviePlayerMainMenu does not: the retail
		// script declares OnFocusGained/OnFocusLost on GFxMoviePlayer and the generated class for this
		// tree carries the *name* but no UFunction for the Dishonored subclass, so opening the menu
		// through the real path died on "Failed to find function None". Guarded rather than removed,
		// because the call itself is retail's (2013 0x586b60).
		if( OldFocus != NULL && OldFocus->pUMovie != NULL
			&& !OldFocus->pUMovie->HasAnyFlags( RF_Unreachable )
			&& OldFocus->pUMovie->FindFunction( GFXUI_OnFocusLost ) != NULL )
		{
			OldFocus->pUMovie->eventOnFocusLost( Index );
		}
		if( NewFocus != NULL && NewFocus->pUMovie != NULL
			&& !NewFocus->pUMovie->HasAnyFlags( RF_Unreachable )
			&& NewFocus->pUMovie->FindFunction( GFXUI_OnFocusGained ) != NULL )
		{
			NewFocus->pUMovie->eventOnFocusGained( Index );
		}
	}
}

void FGFxEngine::SetMovieCanReceiveFocus( FGFxMovie* InMovie, UBOOL bCanReceiveFocus )
{
	if( InMovie != NULL )
	{
		InMovie->bCanReceiveFocus = bCanReceiveFocus;
		ReevaluateFocus();
	}
}

void FGFxEngine::SetMovieCanReceiveInput( FGFxMovie* InMovie, UBOOL bCanReceiveInput )
{
	if( InMovie != NULL )
	{
		InMovie->bCanReceiveInput = bCanReceiveInput;
	}
}

/*-----------------------------------------------------------------------------
	The per-frame path: Tick, RenderUI, RenderTextures.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x57b140 (2012 0x5bfa30). The clock is the world's unless the movie asked for
// real time, the step is clamped to 0.05 s, and a movie whose fUpdate is FALSE is paused - which is what
// UGFxMoviePlayer::SetPause writes, so a paused movie still draws.
// DISHONORED(bringup, agent DG): -gfxuikey=<drawnframe>:<KeyName>[,<drawnframe>:<KeyName>...]
// delivers a press and a release through FGFxEngine::InputKey, which is the entry point
// UGFxInteraction::InputKey calls - so what a screenshot after it shows is the real input path and
// not a variable poked into the movie. Read on first use.
void FGFxEngine::TickScriptedKeys()
{
	static TArray<INT> KeyFrames;
	static TArray<FName> KeyNames;
	static TArray<INT> ReleaseFrames;
	static TArray<FName> ReleaseKeys;
	static INT KeyHoldFrames = 30;
	static UBOOL bParsed = FALSE;
	if( !bParsed )
	{
		bParsed = TRUE;
		FString Value;
		// DISHONORED(bringup): no rva - a bring-up switch. Parse stops at a comma unless told not to,
		// and this switch IS a comma list.
		FString HoldValue;
		if( Parse( appCmdLine(), TEXT("gfxuikeyhold="), HoldValue ) )
		{
			KeyHoldFrames = Max( 1, appAtoi( *HoldValue ) );
		}
		if( Parse( appCmdLine(), TEXT("gfxuikey="), Value, FALSE ) )
		{
			while( Value.Len() )
			{
				const INT Comma = Value.InStr( TEXT(",") );
				const FString One = Comma >= 0 ? Value.Left( Comma ) : Value;
				Value = Comma >= 0 ? Value.Mid( Comma + 1 ) : FString();
				const INT Colon = One.InStr( TEXT(":") );
				if( Colon > 0 )
				{
					KeyFrames.AddItem( appAtoi( *One.Left( Colon ) ) );
					KeyNames.AddItem( FName( *One.Mid( Colon + 1 ) ) );
				}
			}
		}
	}
	// -gfxuishotafterkey=<N>: request a screenshot N engine ticks after the last scripted key. The
	// drawn-frame counter advances twice per game frame (UGameViewportClient::Draw calls RenderUI
	// twice), so counting ticks here is what makes "the frame after the key" mean what it says.
	static INT ShotAfterKey = -2;
	static INT ShotCountdown = -1;
	if( ShotAfterKey == -2 )
	{
		FString Value;
		ShotAfterKey = Parse( appCmdLine(), TEXT("gfxuishotafterkey="), Value ) ? appAtoi( *Value ) : -1;
	}
	if( ShotCountdown > 0 && --ShotCountdown == 0 )
	{
		FString Name;
		if( !Parse( appCmdLine(), TEXT("gfxuishotname="), Name ) )
		{
			Name = TEXT("gfxui");
		}
		GScreenShotName = Name + TEXT("afterkey");
		GScreenShotRequest = TRUE;
		debugf( TEXT("DISHONORED(bringup): GFx UI: screenshot %s requested %d ticks after the key"),
			*GScreenShotName, ShotAfterKey );
	}
	for( INT Index = 0; Index < ReleaseFrames.Num(); Index++ )
	{
		if( ReleaseFrames( Index ) != DrawnFrames )
		{
			continue;
		}
		FGFxMovie* UpMovie = GetFocusedMovieFromControllerID( 0 );
		if( UpMovie == NULL && OpenMovies.Num() > 0 )
		{
			UpMovie = OpenMovies( OpenMovies.Num() - 1 );
		}
		const UBOOL bUp = UpMovie != NULL && InputKey( 0, UpMovie, ReleaseKeys( Index ), IE_Released );
		debugf( TEXT("DISHONORED(bringup): GFx UI: scripted key %s released on drawn frame %d -> %s"),
			*ReleaseKeys( Index ).ToString(), DrawnFrames, bUp ? TEXT("handled") : TEXT("not handled") );
	}
	for( INT Index = 0; Index < KeyFrames.Num(); Index++ )
	{
		if( KeyFrames( Index ) != DrawnFrames )
		{
			continue;
		}
		if( ShotAfterKey >= 0 )
		{
			ShotCountdown = ShotAfterKey > 0 ? ShotAfterKey : 1;
		}
		// The focused movie is the one UGFxInteraction would route to. On a menu map there is no
		// local player yet, so PlayerStates is empty and GetFocusedMovieFromControllerID answers
		// NULL; the scripted key then goes to the open movie directly, which is the same
		// FGFxEngine::InputKey(ControllerId, Movie, ...) the focused path calls one level down.
		FGFxMovie* Movie = GetFocusedMovieFromControllerID( 0 );
		const TCHAR* Route = TEXT("focused movie");
		if( Movie == NULL && OpenMovies.Num() > 0 )
		{
			Movie = OpenMovies( OpenMovies.Num() - 1 );
			Route = TEXT("topmost open movie (no local player owns focus)");
		}
		const UBOOL bPressed = Movie != NULL && InputKey( 0, Movie, KeyNames( Index ), IE_Pressed );
		// DISHONORED(bringup, agent DK): the release is held back. A key queued and released in the
		// same tick is already up by the time GFxMovieRoot::ProcessInput notifies the AS2 listeners,
		// and _common.UIBase.InputDelegate runs its body only while Key.isDown(code) is true.
		ReleaseFrames.AddItem( DrawnFrames + KeyHoldFrames );
		ReleaseKeys.AddItem( KeyNames( Index ) );
		LogCensus( TEXT("after a scripted key") );
		debugf( TEXT("DISHONORED(bringup): GFx UI: scripted key %s on drawn frame %d via %s -> pressed %s, release queued for frame %d"),
			*KeyNames( Index ).ToString(), DrawnFrames, Route,
			bPressed ? TEXT("handled") : TEXT("not handled"), DrawnFrames + KeyHoldFrames );
	}
}

void FGFxEngine::Tick( FLOAT DeltaTime )
{
	(void)DeltaTime;
	{
		// DISHONORED(bringup, agent DG): -gfxuinoclassbind and -gfxuiclasstrace, read on first use.
		static UBOOL bRead = FALSE;
		if( !bRead )
		{
			bRead = TRUE;
			if( ParseParam( appCmdLine(), TEXT("gfxuinoclassbind") ) )
			{
				GFxSprite::bBindRegisteredClasses = false;
				debugf( TEXT("DISHONORED(bringup): GFx UI: Object.registerClass instantiation is OFF") );
			}
			if( ParseParam( appCmdLine(), TEXT("gfxuiclasstrace") ) )
			{
				GFxSprite::bTraceClassBinding = true;
			}
			if( ParseParam( appCmdLine(), TEXT("gfxuidlcheck") ) )
			{
				GFxSprite::bCheckDisplayList = true;
			}
		}
	}
	// DISHONORED(bringup): focus is re-evaluated when the local player set changes. ReevaluateFocus()
	// returns early while PlayerStates is empty and is otherwise only called on InsertMovie/CloseScene,
	// so an interface opened before the local player exists never acquires focus - and with no focused
	// movie, FGFxEngine::InputKey returns at its first line and every real key press is dropped. Retail
	// has no such gap because the script InitInputSystem inserts a UGFxInteraction per player, the same
	// insertion this tree already works around for the player's own input (Engine/Src/UnPlayer.cpp).
	{
		// The condition is "nothing holds focus while something could", not "the player set changed":
		// the interface opens after the player appears, so a change-triggered re-evaluation runs too
		// early and never again. This is self-correcting and stops as soon as focus is taken.
		const INT PlayerCount = GEngine != NULL ? GEngine->GamePlayers.Num() : 0;
		if( PlayerCount > 0 && OpenMovies.Num() > 0 && GetFocusedMovieFromControllerID( 0 ) == NULL )
		{
			ReevaluateFocus();
		}
		// Unconditional for the first ten seconds: the three numbers the condition above is built from,
		// so a run states why focus is or is not held rather than leaving it to be narrowed by elimination.
		{
			static DOUBLE NextSay = 0.0;
			if( GCurrentTime >= NextSay )
			{
				NextSay = GCurrentTime + 1.0;
				static INT Said = 0;
				if( Said < 10 )
				{
					Said++;
					debugf( TEXT("DISHONORED(bringup): GFx UI focus: %d local player(s), %d player state(s), %d movie(s) open, focus %s"),
						PlayerCount, PlayerStates.Num(), OpenMovies.Num(),
						GetFocusedMovieFromControllerID( 0 ) != NULL ? TEXT("held") : TEXT("NONE") );
				}
			}
		}
	}

	TickScriptedKeys();

	// DISHONORED(bringup): -gfxuifreeze=<N> stops advancing every open movie after N advances. The display list
	// and the renderer are untouched, so the frame keeps drawing - the movie's clock simply stops. It exists
	// because agent DG's open crash fires on a timeline loop about 80 movie frames in, which leaves too little
	// time to look at the interface; freezing the timeline holds the menu on screen indefinitely. Remove this
	// when that crash is fixed. Read on first use, never at static-initialisation time (GCmdLine is set in
	// WinMain, which is what left three earlier bring-up switches permanently FALSE - agent CA, c403e2f).
	{
		static INT FreezeAfter = -2;
		static INT Advances    = 0;
		static UBOOL bSaidSo   = FALSE;
		if( FreezeAfter == -2 )
		{
			FreezeAfter = -1;
			INT Parsed = 0;
			if( Parse( appCmdLine(), TEXT("gfxuifreeze="), Parsed ) && Parsed >= 0 )
			{
				FreezeAfter = Parsed;
				debugf( TEXT("DISHONORED(bringup): GFx UI: the interface will stop advancing after %i advances and stay on screen"), FreezeAfter );
			}
		}
		if( FreezeAfter >= 0 )
		{
			if( Advances >= FreezeAfter )
			{
				if( !bSaidSo )
				{
					bSaidSo = TRUE;
					debugf( TEXT("DISHONORED(bringup): GFx UI: frozen after %i advances - the menu stays as it is"), Advances );
				}
				return;
			}
			Advances++;
		}
	}

	const DOUBLE GameTime = GWorld ? GWorld->GetTimeSeconds() : 0.0;
	const DOUBLE RealTime = GCurrentTime;

	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		FGFxMovie* Movie = OpenMovies( Index );
		const DOUBLE Now = ( Movie->TimingMode == TM_Real ) ? RealTime : GameTime;
		DOUBLE Step = Now - Movie->LastTime;
		Movie->LastTime = Now;
		if( Step > 0.05 )
		{
			Step = 0.05;
		}
		if( Movie->fUpdate && Movie->pUMovie != NULL )
		{
			Movie->pUMovie->Advance( (FLOAT)Step );
		}
	}
	for( INT Index = 0; Index < TextureMovies.Num(); Index++ )
	{
		FGFxMovie* Movie = TextureMovies( Index );
		const DOUBLE Now = ( Movie->TimingMode == TM_Real ) ? RealTime : GameTime;
		DOUBLE Step = Now - Movie->LastTime;
		Movie->LastTime = Now;
		if( Step > 0.05 )
		{
			Step = 0.05;
		}
		if( Movie->fUpdate && Movie->pUMovie != NULL )
		{
			Movie->pUMovie->Advance( (FLOAT)Step );
		}
	}
}

/** The render command FGFxEngine::RenderUI enqueues (2013 0x586780). It binds the render target the UI
    draws into and tells the renderer which one it is. */
struct FGFxSetRenderTargetsCommand : public FRenderCommand
{
	FGFxRenderer* Renderer;
	FViewport*    Viewport;
	UBOOL         bRenderToSceneColor;

	FGFxSetRenderTargetsCommand( FGFxRenderer* InRenderer, FViewport* InViewport,
		UBOOL bInRenderToSceneColor )
		: Renderer( InRenderer ), Viewport( InViewport ), bRenderToSceneColor( bInRenderToSceneColor )
	{}

	virtual UINT Execute()
	{
		if( GGFxEngine != NULL )
		{
			GGFxEngine->GameHasRendered++;
		}
		// DISHONORED(bringup): retail's command also allocates GSceneRenderTargets and, when
		// bRenderToSceneColor is set, begins rendering scene colour and hands the scene-colour proxy
		// and the scene depth proxy to the render target (2013 0x586780). FSceneRenderTargets lives in
		// Engine/Src, which is private to the Engine module, and this wave's ownership rules put
		// SceneRendering out of reach - so the UI binds the viewport directly and the scene-colour
		// variant is not reproduced. The consequence is stated in agentDC.md: the interface draws over
		// the resolved back buffer rather than into scene colour, which is the same image for a HUD
		// and differs only for a post-process effect that reads the UI.
		FGFxRenderTargetResource::NativeRenderTarget Native( Viewport, NULL );
		Renderer->RenderTarget = Viewport;
		FGFxRenderTarget* Target = GGFxEngine ? GGFxEngine->GetHudRenderTarget() : NULL;
		if( Target != NULL )
		{
			Target->InitRenderTarget_RenderThread( Native );
			Renderer->CurRenderTarget = Target;
			if( Target->Resource != NULL )
			{
				RHISetRenderTarget( Target->Resource->ColorBuffer, Target->Resource->DepthBuffer );
			}
		}
		return sizeof(*this);
	}
	virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetRenderTargetsCommand"); }
};

// DISHONORED(port): 2013 0x58dd30 (2012 0x5ceb00). The four early-outs are retail's, in retail's order:
// the DrawGFx switch, the renderer, the HUD viewport, and an empty group. Then the bind command, then
// one pView->Display() per movie whose visibility test passes - GFxMovieView slot 38, which is exactly
// what the decompile's `vtable + 152` is.
void FGFxEngine::RenderUI( UBOOL bRenderToSceneColor, INT DPG )
{
	if( !GDrawGFx || GFxUIIsDisabled() )
	{
		return;
	}
	if( pRenderer.GetPtr() == NULL || HudViewport == NULL )
	{
		return;
	}
	if( DPG < 0 || DPG > SDPG_PostProcess || DPGOpenMovies[DPG].Num() == 0 )
	{
		return;
	}

	pRenderer->Viewport = HudViewport;
	if( !ParseParam( appCmdLine(), TEXT("gfxuinobind") ) )
	{
		ENQUEUE_RENDER_COMMAND(FGFxSetRenderTargetsCommand,(pRenderer.GetPtr(),HudViewport,bRenderToSceneColor));
	}

	// bHudEnabled: the local player's HUD has to be showing for a movie that follows it
	// (UGFxMoviePlayer::bDisplayWithHudOff is the other arm, which is the bit at +196 the decompile
	// tests with 2).
	UBOOL bHudEnabled = TRUE;
	if( GEngine->GamePlayers.Num() > 0 )
	{
		ULocalPlayer* Player = GEngine->GamePlayers( 0 );
		if( Player == NULL || Player->Actor == NULL || Player->Actor->myHUD == NULL
			|| !Player->Actor->myHUD->bShowHUD )
		{
			bHudEnabled = FALSE;
		}
	}
	else
	{
		bHudEnabled = FALSE;
	}

	for( INT Index = 0; Index < DPGOpenMovies[DPG].Num(); Index++ )
	{
		FGFxMovie* Movie = DPGOpenMovies[DPG]( Index );
		const UBOOL bDisplayWithHudOff = Movie->pUMovie != NULL
			&& Movie->pUMovie->bDisplayWithHudOff;
		if( ( Movie->fVisible && bHudEnabled ) || bDisplayWithHudOff )
		{
			Movie->pView->Display();

			GFxMovieRoot* Root = (GFxMovieRoot*)Movie->pView.GetPtr();
			const GFxDisplayStats& Stats = Root->GetDisplayStats();
			RenderCensus.Movies++;
			RenderCensus.DisplayObjects += Stats.Characters;
			RenderCensus.Sprites += Stats.Sprites;
			RenderCensus.Shapes += Stats.Shapes;
			RenderCensus.TextFields += Stats.TextFields;
			RenderCensus.Images += Stats.Images;
			RenderCensus.Draws += Stats.TriListDraws;
			RenderCensus.Triangles += Stats.Triangles;
			RenderCensus.GlyphDraws += Stats.GlyphDraws;
			RenderCensus.Glyphs += Stats.Glyphs;
			RenderCensus.ShadowGlyphs += Stats.ShadowGlyphs;
			RenderCensus.Masks += Stats.Masks;
			RenderCensus.TextFieldsUnbound += Stats.TextFieldsUnbound;
		}
	}

	if( !bCensusLogged || GFxUICensusVerbose() )
	{
		LogCensus( bCensusLogged ? TEXT("frame") : TEXT("first drawn frame") );
		bCensusLogged = TRUE;
	}

	// DISHONORED(bringup): -gfxuidrawtrace=<drawn frame> logs one frame's style groups, at the frame
	// asked for, so a settled interface can be read as well as a starting one.
	{
		static INT TraceFrame = -2;
		static UBOOL bTraceDone = FALSE;
		if( TraceFrame == -2 )
		{
			INT N = 0;
			TraceFrame = Parse( appCmdLine(), TEXT("gfxuidrawtrace="), N ) ? N : -1;
		}
		if( !bTraceDone && TraceFrame >= 0 && DrawnFrames >= TraceFrame )
		{
			bTraceDone = TRUE;
			GFxDisplayDrawTrace = 64;
		}
	}

	{
		static UBOOL bWatchParsed = FALSE;
		if( !bWatchParsed )
		{
			bWatchParsed = TRUE;
			FString Watch;
			if( Parse( appCmdLine(), TEXT("gfxuiwatch="), Watch, FALSE ) )
			{
				appStrncpyANSI( GFxAS2WatchMember, TCHAR_TO_ANSI( *Watch ), 64 );
				GFxAS2WatchCount = 4000;
			}
			// DISHONORED(bringup): -gfxuioptrace=<from>[:<count>] is GFx3Run's --optrace, in the
			// game. An opcode that leaves no other mark - a branch taken on an underflowed stack -
			// is only visible here.
			FString Window;
			if( Parse( appCmdLine(), TEXT("gfxuiopwindow="), Window, FALSE ) )
			{
				TArray<FString> Parts;
				Window.ParseIntoArray( &Parts, TEXT(":"), TRUE );
				if( Parts.Num() == 3 )
				{
					GFxAS2OpTraceLen = appAtoi( *Parts(0) );
					GFxAS2OpTraceLo = appAtoi( *Parts(1) );
					GFxAS2OpTraceHi = appAtoi( *Parts(2) );
				}
			}
			FString Trace;
			if( Parse( appCmdLine(), TEXT("gfxuioptrace="), Trace, FALSE ) )
			{
				const INT Colon = Trace.InStr( TEXT(":") );
				GFxAS2OpTraceFrom = appAtoi( Colon >= 0 ? *Trace.Left( Colon ) : *Trace );
				GFxAS2OpTraceCount = Colon >= 0 ? appAtoi( *Trace.Mid( Colon + 1 ) ) : 200;
			}
		}
	}

	const TArray<FString>& Hidden = GFxUIHiddenClips();
	if( Hidden.Num() > 0 )
	{
		for( INT Index = 0; Index < DPGOpenMovies[DPG].Num(); Index++ )
		{
			GFxMovieRoot* Root = (GFxMovieRoot*)DPGOpenMovies[DPG]( Index )->pView.GetPtr();
			GFxSprite* Level0 = Root ? Root->GetLevel0() : NULL;
			if( Level0 == NULL )
			{
				continue;
			}
			FGFxHideNamed( Level0, Hidden, 0 );
		}
	}

	static UBOOL bDumped = FALSE;
	const INT DumpAtFrame = GFxUIDumpDisplayList();
	if( !bDumped && DumpAtFrame >= 0 && DrawnFrames > DumpAtFrame )
	{
		bDumped = TRUE;
		for( INT Index = 0; Index < DPGOpenMovies[DPG].Num(); Index++ )
		{
			GFxMovieRoot* Root = (GFxMovieRoot*)DPGOpenMovies[DPG]( Index )->pView.GetPtr();
			if( Root == NULL || Root->GetLevel0() == NULL )
			{
				continue;
			}
			debugf( TEXT("DISHONORED(bringup): display tree of %s"),
				*DPGOpenMovies[DPG]( Index )->FileName );
			GMatrix2D Identity;
			Identity.SetIdentity();
			FGFxDumpCharacter( Root->GetLevel0(), Identity, 0 );
		}
	}

	// DISHONORED(bringup): -gfxuishot=<N> asks the engine for one screenshot on the Nth frame the
	// interface was DRAWN on, which is the frame that matters; the engine's own request path at the end
	// of UGameViewportClient::Draw reads the back buffer and writes the bitmap. Read on first use, never
	// as a file-scope static (GCmdLine does not exist during static initialisation).
	{
		// DISHONORED(bringup, agent DG): a list rather than one frame, so a run can photograph the
		// interface before and after an input event. -gfxuishotname=<prefix> names the files.
		static TArray<INT> ShotFrames;
		static FString ShotName;
		static UBOOL bShotParsed = FALSE;
		if( !bShotParsed )
		{
			bShotParsed = TRUE;
			FString Value;
			// DISHONORED(bringup): no rva - a bring-up switch. Parse stops at a comma unless told not to,
			// and this switch IS a comma list.
			if( Parse( appCmdLine(), TEXT("gfxuishot="), Value, FALSE ) )
			{
				while( Value.Len() )
				{
					const INT Comma = Value.InStr( TEXT(",") );
					ShotFrames.AddItem( appAtoi( *( Comma >= 0 ? Value.Left( Comma ) : Value ) ) );
					Value = Comma >= 0 ? Value.Mid( Comma + 1 ) : FString();
				}
			}
			if( !Parse( appCmdLine(), TEXT("gfxuishotname="), ShotName ) )
			{
				ShotName = TEXT("gfxui");
			}
		}
		++DrawnFrames;
		for( INT Index = 0; Index < ShotFrames.Num(); Index++ )
		{
			if( ShotFrames( Index ) == DrawnFrames )
			{
				GScreenShotName = ShotName + appItoa( DrawnFrames );
				GScreenShotRequest = TRUE;
				debugf( TEXT("DISHONORED(bringup): GFx UI: screenshot %s requested on drawn frame %d"),
					*GScreenShotName, DrawnFrames );
			}
		}
	}
}

void FGFxEngine::LogCensus( const TCHAR* Reason )
{
	FString Movies;
	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		if( Index )
		{
			Movies += TEXT(", ");
		}
		Movies += OpenMovies( Index )->FileName;
	}
	GFxGlyphRasterCache* Glyphs = GFxDisplayGetGlyphCache();
	// What the machine ran on the advance half, so one line answers "what did the interface do this
	// frame" end to end: the movies, the display objects, the submitted geometry and the opcodes.
	INT Sprites = 0, Placed = 0, Buffers = 0, Frames = 0, ScriptErrors = 0;
	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		GFxMovieRoot* Root = (GFxMovieRoot*)OpenMovies( Index )->pView.GetPtr();
		if( Root != NULL )
		{
			const GFxMovieRoot::Census& C = Root->GetCensus();
			Sprites += C.SpritesCreated;
			Placed += C.DisplayObjectsPlaced;
			Buffers += C.ActionBuffersRun;
			Frames += C.FramesAdvanced;
			ScriptErrors += C.ScriptErrors;
		}
	}
	// DISHONORED(bringup, agent DG): the input half of the same line. A key that reaches the movie's
	// door and is refused is not the same thing as one the movie used, and the census is where a run
	// has to say which; these are the runtime's own counters.
	const GFxInputCensus& Input = GFxInputGetCensus();
	debugf( TEXT("DISHONORED(bringup): GFx UI census (%s): movies open %d [%s], drawn %d, display objects %d ")
		TEXT("(%d sprites, %d shapes, %d text fields [%d unbound], %d bitmap fills), %d draws, %d triangles, ")
		TEXT("%d glyph batches / %d glyphs / %d shadow glyphs, %d masks, ")
		TEXT("atlas %d packed / %d blank / %d failed"),
		Reason, OpenMovies.Num(), *Movies, RenderCensus.Movies, RenderCensus.DisplayObjects,
		RenderCensus.Sprites, RenderCensus.Shapes, RenderCensus.TextFields,
		RenderCensus.TextFieldsUnbound, RenderCensus.Images,
		RenderCensus.Draws, RenderCensus.Triangles, RenderCensus.GlyphDraws, RenderCensus.Glyphs,
		RenderCensus.ShadowGlyphs,
		RenderCensus.Masks, Glyphs ? (INT)Glyphs->GetRasterizedCount() : 0,
		Glyphs ? (INT)Glyphs->GetEmptyCount() : 0, Glyphs ? (INT)Glyphs->GetFailedCount() : 0 );
	debugf( TEXT("DISHONORED(bringup): GFx UI census (%s): machine: %d frames advanced, ")
		TEXT("%d sprites created, %d display objects placed, %d action buffers, ")
		TEXT("%u opcodes (%u unimplemented), %d script errors, %u untextured fills skipped"),
		Reason, Frames, Sprites, Placed, Buffers, GASActionBuffer::OpsExecuted,
		GASActionBuffer::OpsUnimplemented, ScriptErrors, ::GFxDisplayUntexturedFills );
	debugf( TEXT("DISHONORED(bringup): GFx UI census (%s): input: %u events HE_Handled / %u HE_NotHandled, ")
		TEXT("%u key downs, %u key ups, %u chars typed, %u mouse events, ")
		TEXT("%u AS2 listeners registered, %u listener calls"),
		Reason, Input.EventsHandled, Input.EventsNotHandled, Input.KeyDowns, Input.KeyUps,
		Input.CharsTyped, Input.MouseEvents, Input.ListenersAdded, Input.KeyListenerCalls );
	// DISHONORED(bringup, agent DL): the filter half - what the cook asked for against what was
	// applied, and the passes it cost. Written by the runtime so the two halves cannot disagree.
	{
		extern void GFxDL_ReportFilterCensus();
		GFxDL_ReportFilterCensus();
	}
	appMemzero( &RenderCensus, sizeof(RenderCensus) );

	// The renderer's own half of the same count, which is what says whether a draw the walk submitted
	// reached the RHI: FGFxRenderer::DrawIndexedTriList_RenderThread returns early on a disabled fill
	// style, a missing element store or an invalid bound shader state, and none of those is visible from
	// the game thread. Agent CC's census is read here rather than duplicated.
	{
		extern unsigned int FGFxSeamCensus( char* Out, unsigned int Capacity );
		char Buffer[4096];
		if( FGFxSeamCensus( Buffer, sizeof(Buffer) ) )
		{
			debugf( TEXT("%s"), ANSI_TO_TCHAR( Buffer ) );
		}
		extern void FGFxSeamReset();
		FGFxSeamReset();
	}
}

// DISHONORED(port): 2013 0x58e080 (2012 0x5cee50). The render-texture movies, each into its own target.
void FGFxEngine::RenderTextures()
{
	DeleteQueuedMovies( FALSE );
	if( !GDrawGFx || GFxUIIsDisabled() || pRenderer.GetPtr() == NULL )
	{
		return;
	}
	// DISHONORED(bringup): a movie rendered into a UTextureRenderTarget2D needs the render target bound
	// per movie and resolved after (retail's FSetGFxRenderTargets / FGFxResolveRenderTargets pair,
	// 2013 0x5868b0 / 0x574f10). Nothing in the retail cook opens one on the menu path - every movie
	// StartScene sees has a null RenderTexture - so the list is walked and reported rather than drawn.
	if( TextureMovies.Num() )
	{
		static UBOOL bWarned = FALSE;
		if( !bWarned )
		{
			bWarned = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): %d GFx movies want a render texture; ")
				TEXT("FGFxEngine::RenderTextures is not implemented"), TextureMovies.Num() );
		}
	}
}

/*-----------------------------------------------------------------------------
	Input.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 0x59f490 (2012 0x5dfdd0). Retail reads [GFxUI.KeyCodes] and [GFxUI.KeyMap]
// out of the input ini on top of the defaults; the defaults are what the menu needs and they are the
// ones below, from GFxKey's own enumeration.
void FGFxEngine::InitKeyMap()
{
	struct FGFxKeyDefault { const TCHAR* Name; INT Code; };
	static const FGFxKeyDefault Defaults[] =
	{
		{ TEXT("Up"), GFxKey::Up }, { TEXT("Down"), GFxKey::Down },
		{ TEXT("Left"), GFxKey::Left }, { TEXT("Right"), GFxKey::Right },
		{ TEXT("Enter"), GFxKey::Return }, { TEXT("Escape"), GFxKey::Escape },
		{ TEXT("SpaceBar"), GFxKey::Space }, { TEXT("Tab"), GFxKey::Tab },
		{ TEXT("BackSpace"), GFxKey::Backspace }, { TEXT("Delete"), GFxKey::Delete },
		{ TEXT("Home"), GFxKey::Home }, { TEXT("End"), GFxKey::End },
		{ TEXT("PageUp"), GFxKey::PageUp }, { TEXT("PageDown"), GFxKey::PageDown },
		{ TEXT("LeftShift"), GFxKey::Shift }, { TEXT("RightShift"), GFxKey::Shift },
		{ TEXT("LeftControl"), GFxKey::Control }, { TEXT("RightControl"), GFxKey::Control },
		{ TEXT("LeftAlt"), GFxKey::Alt }, { TEXT("RightAlt"), GFxKey::Alt },
		{ TEXT("XboxTypeS_DPad_Up"), GFxKey::Up }, { TEXT("XboxTypeS_DPad_Down"), GFxKey::Down },
		{ TEXT("XboxTypeS_DPad_Left"), GFxKey::Left }, { TEXT("XboxTypeS_DPad_Right"), GFxKey::Right },
		{ TEXT("XboxTypeS_A"), GFxKey::Return }, { TEXT("XboxTypeS_B"), GFxKey::Escape },
	};
	for( INT Index = 0; Index < ARRAY_COUNT(Defaults); Index++ )
	{
		KeyMap.Set( FName( Defaults[Index].Name ).GetIndex(), Defaults[Index].Code );
	}
	for( INT Letter = 0; Letter < 26; Letter++ )
	{
		TCHAR Name[2] = { (TCHAR)('A' + Letter), 0 };
		KeyMap.Set( FName( Name ).GetIndex(), GFxKey::A + Letter );
	}
	for( INT Digit = 0; Digit < 10; Digit++ )
	{
		TCHAR Name[2] = { (TCHAR)('0' + Digit), 0 };
		KeyMap.Set( FName( Name ).GetIndex(), GFxKey::Num0 + Digit );
	}
}

void FGFxEngine::UpdateKeyEmulation( FName A, FName B, FName C, FName D, FName E )
{
	// DISHONORED(bringup): 2013 0x5a0f30 rebuilds the analogue-stick-to-key emulation from five key
	// names. The emulation itself (FUIAxisEmulationData, four axes with a repeat delay) is
	// UUIInteraction's and is not reconstructed; the names are recorded so a caller sees no change.
	(void)A; (void)B; (void)C; (void)D; (void)E;
}

// DISHONORED(port): 2013 0x5916b0 (2012 0x5d1f90)
UBOOL FGFxEngine::IsKeyCaptured( FName ukey )
{
	for( INT Index = 0; Index < OpenMovies.Num(); Index++ )
	{
		FGFxMovie* Movie = OpenMovies( Index );
		if( Movie->pUMovie == NULL || !Movie->bCanReceiveInput )
		{
			continue;
		}
		TSet<INT>* Keys = (TSet<INT>*)Movie->pUMovie->pCaptureKeys;
		if( Keys != NULL && Keys->Contains( ukey.GetIndex() ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 0x590fd0 (2012 0x5d1a30). The key goes to the movie's script filter first and
// then to the runtime as a GFxKeyEvent.
UBOOL FGFxEngine::InputKey( INT ControllerId, FGFxMovie* pFocusMovie, FName ukey, EInputEvent uevent )
{
	if( pFocusMovie == NULL || pFocusMovie->pUMovie == NULL || !pFocusMovie->bCanReceiveInput )
	{
		return FALSE;
	}
	UBOOL bHandled = FALSE;
	if( pFocusMovie->pUMovie->FilterButtonInput( ControllerId, ukey, (BYTE)uevent, bHandled ) )
	{
		return bHandled;
	}

	const INT* Code = KeyMap.Find( ukey.GetIndex() );
	if( Code == NULL )
	{
		return FALSE;
	}
	// DISHONORED(bringup): the event is built and delivered, and the runtime answers HE_NotHandled
	// because GFxMovieRoot::HandleEvent needs the button and focus model - GFxButtonCharacter's 38
	// functions plus GFx_GenerateMouseButtonEvents (2012 0xa66a90), which agent BC named as not in
	// this wave (agentBC.md 6.9). That is the input hand-over of agentDC.md: everything up to the
	// movie's door is here.
	GFxKeyEvent Event;
	appMemzero( &Event, sizeof(Event) );
	Event.Type = ( uevent == IE_Released ) ? GFxEvent::KeyUp : GFxEvent::KeyDown;
	Event.KeyCode = (GFxKey::Code)*Code;
	Event.KeyboardIndex = (GUByte)ControllerId;
	const UINT Result = pFocusMovie->pView->HandleEvent( Event );
	return ( Result & GFxMovieView::HE_Handled ) != 0;
}

// DISHONORED(port): 2013 0x591470 (2012 0x5d1e40)
UBOOL FGFxEngine::InputKey( INT ControllerId, FName ukey, EInputEvent uevent )
{
	FGFxMovie* Focus = GetFocusedMovieFromControllerID( ControllerId );
	if( Focus != NULL && InputKey( ControllerId, Focus, ukey, uevent ) )
	{
		return TRUE;
	}
	// A key a movie captured is swallowed even when the focused movie did not use it, which is what
	// keeps the game from acting on the menu's keys.
	return IsKeyCaptured( ukey );
}

// DISHONORED(port): 2013 0x5917c0 (2012 0x5d20a0)
UBOOL FGFxEngine::InputChar( INT ControllerId, TCHAR Character )
{
	FGFxMovie* Focus = GetFocusedMovieFromControllerID( ControllerId );
	if( Focus == NULL || Focus->pUMovie == NULL || !Focus->bCanReceiveInput )
	{
		return FALSE;
	}
	GFxCharEvent Event;
	appMemzero( &Event, sizeof(Event) );
	Event.Type = GFxEvent::CharEvent;
	Event.WcharCode = (UINT)Character;
	Event.KeyboardIndex = (GUByte)ControllerId;
	const UINT Result = Focus->pView->HandleEvent( Event );
	return ( Result & GFxMovieView::HE_Handled ) != 0;
}

// DISHONORED(port): 2013 0x594cc0 (2012 0x5d4b40). The axis goes to the movie's script filter; retail
// then emulates the four arrow keys from the stick through FUIAxisEmulationData, which is the piece
// UpdateKeyEmulation configures and which is not reconstructed.
UBOOL FGFxEngine::InputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad )
{
	FGFxMovie* Focus = GetFocusedMovieFromControllerID( ControllerId );
	if( Focus == NULL || Focus->pUMovie == NULL || !Focus->bCanReceiveInput )
	{
		return FALSE;
	}
	UBOOL bHandled = FALSE;
	if( Focus->pUMovie->FilterInputAxis( ControllerId, Key, Delta, DeltaTime, bGamepad, bHandled ) )
	{
		return bHandled;
	}
	return FALSE;
}

// DISHONORED(port): 2013 0x59a240 (2012 0x5da330). From UGameUISceneClient::FlushPlayerInput: record
// the keys already down so their release is ignored, then reset the player's input.
void FGFxEngine::FlushPlayerInput( TSet<INT>* Keys )
{
	for( INT ControllerId = 0; ControllerId < GEngine->GamePlayers.Num(); ControllerId++ )
	{
		ULocalPlayer* Player = GEngine->GamePlayers( ControllerId );
		if( Player == NULL || Player->Actor == NULL || Player->Actor->PlayerInput == NULL )
		{
			continue;
		}
		TArray<FName>* PressedPlayerKeys = InitialPressedKeys.Find( Player->ControllerId );
		if( PressedPlayerKeys == NULL )
		{
			PressedPlayerKeys = &InitialPressedKeys.Set( Player->ControllerId, TArray<FName>() );
		}
		for( INT KeyIndex = 0; KeyIndex < Player->Actor->PlayerInput->PressedKeys.Num(); KeyIndex++ )
		{
			const FName Key = Player->Actor->PlayerInput->PressedKeys( KeyIndex );
			if( Keys == NULL || Keys->Contains( Key.GetIndex() ) )
			{
				PressedPlayerKeys->AddUniqueItem( Key );
			}
		}
		if( Keys != NULL )
		{
			const TArray<FName> PressedKeyCopy = Player->Actor->PlayerInput->PressedKeys;
			for( INT KeyIndex = 0; KeyIndex < PressedKeyCopy.Num(); KeyIndex++ )
			{
				const FName Key = PressedKeyCopy( KeyIndex );
				if( Keys->Contains( Key.GetIndex() ) )
				{
					Player->Actor->PlayerInput->InputKey( Player->ControllerId, Key, IE_Released, 0.f );
				}
			}
		}
		else
		{
			Player->Actor->PlayerInput->ResetInput();
		}
	}
}

/*-----------------------------------------------------------------------------
	The bridge Engine/Src/UnPlayer.cpp calls. Engine cannot include this module's headers (GFxUI is a
	module of its own and the include is one-way), and retail solves that by putting GFxUIClasses.h on
	Engine's include path under WITH_GFx. Three free functions are cheaper and they are the only
	Engine -> GFxUI edge in the tree.
-----------------------------------------------------------------------------*/

void DishonoredGFxSetRenderViewport( FViewport* Viewport )
{
	FGFxEngine* Engine = FGFxEngine::GetEngine();
	if( Engine != NULL )
	{
		Engine->SetRenderViewport( Viewport );
	}
}

// DISHONORED(bringup, agent DC): -gfxuimenu[=<Package>.<Movie>] opens one cooked movie through the real
// path the moment a world and a viewport are up. It exists because of a measured gap upstream of every
// GFx package: in retail the main menu is created by UDisGlobalUIManager from its config movie set, and
// the DisUI.ini set names the HUD, the power wheel, the journal, the note, the pause menu and the
// mission stats but NOT the main menu, which the game's own UnrealScript constructs. A run at
// -startmap=Dishonored_MainMenu streams the menu map in, renders 34,650 frames and never constructs a
// UGFxMoviePlayer, so the interface had nothing to draw.
//
// What this does is exactly what the script would: find the movie player class by name, point its
// MovieInfo at the cooked USwfMovie and call Start(). Everything after that is the real path -
// UGFxMoviePlayer::Load -> FGFxEngine::LoadMovie -> GFxLoader::CreateMovie ->
// GFxMovieDefImpl::CreateInstance -> StartScene -> Tick/Advance -> RenderUI/Display - with no shortcut
// in it. `-gfxuimenuclass=<name>` picks the player class, so the Dishonored subclass is reachable
// without this module depending on the game module; the default is the Dishonored main menu player when
// that class is registered and UGFxMoviePlayer when it is not.
static void DishonoredGFxAutoOpen( FLOAT DeltaTime )
{
	static INT State = -2;          // -2 unread, -1 off or done, 0 waiting, 1 open
	static FLOAT Elapsed = 0.f;
	static FString MoviePath;
	static FString ClassName;

	if( State == -2 )
	{
		FString Value;
		if( Parse( appCmdLine(), TEXT("gfxuimenu="), Value ) )
		{
			MoviePath = Value;
			State = 0;
		}
		else if( ParseParam( appCmdLine(), TEXT("gfxuimenu") ) )
		{
			MoviePath = TEXT("Dishonored_MainMenu.MainMenu");
			State = 0;
		}
		else
		{
			State = -1;
		}
		Parse( appCmdLine(), TEXT("gfxuimenuclass="), ClassName );
	}
	if( State != 0 )
	{
		return;
	}
	Elapsed += DeltaTime;
	if( GWorld == NULL || GEngine == NULL || GEngine->GameViewport == NULL || Elapsed < 1.0f )
	{
		return;
	}

	UClass* PlayerClass = NULL;
	if( ClassName.Len() )
	{
		PlayerClass = FindObject<UClass>( ANY_PACKAGE, *ClassName );
	}
	if( PlayerClass == NULL )
	{
		PlayerClass = FindObject<UClass>( ANY_PACKAGE, TEXT("DisGFxMoviePlayerMainMenu") );
	}
	if( PlayerClass == NULL || !PlayerClass->IsChildOf( UGFxMoviePlayer::StaticClass() ) )
	{
		PlayerClass = UGFxMoviePlayer::StaticClass();
	}

	USwfMovie* Movie = LoadObject<USwfMovie>( NULL, *MoviePath, NULL, LOAD_NoWarn, NULL );
	if( Movie == NULL )
	{
		const INT Dot = MoviePath.InStr( TEXT(".") );
		if( Dot > 0 )
		{
			Movie = LoadObject<USwfMovie>( NULL,
				*( MoviePath.Left( Dot ) + TEXT("_SF") + MoviePath.Mid( Dot ) ), NULL, LOAD_NoWarn, NULL );
		}
	}
	if( Movie == NULL )
	{
		// The cook puts a UI movie inside a group, so the object path carries it
		// (Dishonored_MainMenu.UI_MainMenu.MainMenu). Under seek-free loading LoadObject will not pull a
		// package in on demand either, so the last resort is the movie that is already resident and whose
		// path ends with the name asked for. Every resident movie is logged, which is what makes the next
		// run's switch exact.
		const INT LastDot = MoviePath.InStr( TEXT("."), TRUE );
		const FString WantName = LastDot >= 0 ? MoviePath.Mid( LastDot + 1 ) : MoviePath;
		for( TObjectIterator<USwfMovie> It; It; ++It )
		{
			if( It->RawData.Num() == 0 )
			{
				continue;
			}
			const FString Path = It->GetPathName();
			debugf( TEXT("DISHONORED(bringup): -gfxuimenu: resident movie %s (%d bytes)"), *Path,
				It->RawData.Num() );
			if( Movie == NULL && It->GetName() == WantName )
			{
				Movie = *It;
			}
		}
	}
	if( Movie == NULL )
	{
		// Keep waiting: the movie's package streams in with the map and a loaded machine can take
		// twenty seconds to get there. Reported once a second so a run says what it is waiting for.
		static FLOAT LastComplaint = 0.f;
		if( Elapsed - LastComplaint > 1.f )
		{
			LastComplaint = Elapsed;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): -gfxuimenu: no SwfMovie '%s' yet (%.1fs)"),
				*MoviePath, Elapsed );
		}
		if( Elapsed > 120.f )
		{
			State = -1;
		}
		return;
	}

	UGFxMoviePlayer* Player = ConstructObject<UGFxMoviePlayer>( PlayerClass,
		UObject::GetTransientPackage(), TEXT("DisGFxAutoOpenMenu") );
	Player->AddToRoot();
	Player->MovieInfo = Movie;
	Player->bAllowFocus = TRUE;
	Player->bAllowInput = TRUE;
	Player->bCaptureInput = TRUE;
	Player->bDisplayWithHudOff = TRUE;
	Player->TimingMode = TM_Real;
	Player->LocalPlayerOwnerIndex = 0;
	// DISHONORED(bringup): the object ExternalInterface.call resolves its method names on. The script
	// that constructs the main menu in retail sets it to the movie player itself, and every name the
	// menu asks for - req_CanContinueGame, req_CanLoadGame, req_CanStartNewGame,
	// req_IsSaveLoadEnabled, OnNewGameClicked and the rest - is a function of
	// DisGFxMoviePlayerMainMenu. Without it FGFxExternalInterface::Callback has no target and the
	// menu bar is built from four undefined answers.
	if( Player->ExternalInterface == NULL )
	{
		Player->ExternalInterface = Player;
	}

	const UBOOL bStarted = Player->Start( FALSE );
	// DISHONORED(port): _global.PlatformName before the first advance. The CLIK components in the shared
	// lib movie switch their button-glyph frames with `this.gotoAndStop(_global.PlatformName)`, and with
	// the variable undefined they ask for a frame label that does not exist on every pass - which agent
	// CD measured as a re-queue of the very DoAction that called gotoAndStop (agentCD.md 3.2). The
	// harness sets it with --platform; this is the engine doing the same thing.
	if( bStarted && Player->GetMovie() != NULL && Player->GetMovie()->pView.GetPtr() != NULL )
	{
		GFxValue Platform;
		Platform.SetString( "PC" );
		Player->GetMovie()->pView->SetVariable( "_global.PlatformName", Platform,
			GFxMovie::SV_Normal );
	}
	debugf( TEXT("DISHONORED(bringup): -gfxuimenu: %s %s through %s (movie %s)"),
		bStarted ? TEXT("opened") : TEXT("FAILED to open"), *Movie->GetPathName(),
		*PlayerClass->GetName(), Player->bMovieIsOpen ? TEXT("open") : TEXT("closed") );
	State = bStarted ? 1 : -1;
}

void DishonoredGFxTick( FLOAT DeltaTime )
{
	if( GGFxEngine != NULL )
	{
		DishonoredGFxAutoOpen( DeltaTime );
		GGFxEngine->Tick( DeltaTime );
	}
}

void DishonoredGFxRenderUI()
{
	if( GGFxEngine != NULL )
	{
		GGFxEngine->RenderUI( FALSE, SDPG_Foreground );
		GGFxEngine->RenderUI( FALSE, SDPG_PostProcess );
	}
}

// DISHONORED(bringup): the input half of the Engine -> GFxUI edge, 2013 0x591f10 / 0x591fd0. Retail reaches
// FGFxEngine::InputKey through UGFxInteraction, which the script InitInputSystem inserts into
// GlobalInteractions; that insertion does not happen here (the same gap that dropped the player's own input,
// UnPlayer.cpp), so the viewport offers the key to the interface directly. Both return TRUE when the
// interface consumed the event, which is what stops it reaching the pawn.
UBOOL DishonoredGFxInputKey( INT ControllerId, FName Key, EInputEvent Event )
{
	// DISHONORED(bringup): one line, on the first key the viewport hands us, naming every link in the chain,
	// because "0 key downs" is equally true of a missing route, an empty player-state list, an unfocused
	// movie, a movie that cannot receive input, and a key the map does not carry.
	static UBOOL bSaidSo = FALSE;
	if( !bSaidSo )
	{
		bSaidSo = TRUE;
		if( GGFxEngine == NULL )
		{
			debugf( TEXT("DISHONORED(bringup): GFx input probe: first key '%s' arrived but GGFxEngine is NULL"), *Key.ToString() );
		}
		else
		{
			FGFxMovie* Focus = GGFxEngine->GetFocusedMovieFromControllerID( ControllerId );
			FGFxMovie* Top   = GGFxEngine->OpenMovies.Num() > 0 ? GGFxEngine->OpenMovies( GGFxEngine->OpenMovies.Num() - 1 ) : NULL;
			debugf( TEXT("DISHONORED(bringup): GFx input probe: first key '%s' ctrl %d | GamePlayers %d | PlayerStates %d | OpenMovies %d | focus %s | top %s canFocus %d canInput %d"),
				*Key.ToString(), ControllerId,
				GEngine ? GEngine->GamePlayers.Num() : -1,
				GGFxEngine->PlayerStates.Num(),
				GGFxEngine->OpenMovies.Num(),
				Focus != NULL ? TEXT("yes") : TEXT("NULL"),
				Top != NULL ? TEXT("yes") : TEXT("NULL"),
				Top != NULL ? (INT)Top->bCanReceiveFocus : -1,
				Top != NULL ? (INT)Top->bCanReceiveInput : -1 );
		}
	}
	if( GGFxEngine == NULL )
	{
		return FALSE;
	}
	if( GGFxEngine->InputKey( ControllerId, Key, Event ) )
	{
		return TRUE;
	}
	// DISHONORED(bringup): when no local player owns focus, offer the key to the topmost open movie - the
	// same fallback agent DK's scripted key path already carries, in those words, and the reason that path
	// worked while a real key press did nothing. Retail does not need it: the script InitInputSystem inserts
	// a UGFxInteraction per local player and the focused movie is always resolved. Remove this once that
	// insertion works.
	if( GGFxEngine->GetFocusedMovieFromControllerID( ControllerId ) == NULL && GGFxEngine->OpenMovies.Num() > 0 )
	{
		FGFxMovie* Top = GGFxEngine->OpenMovies( GGFxEngine->OpenMovies.Num() - 1 );
		return GGFxEngine->InputKey( ControllerId, Top, Key, Event );
	}
	return FALSE;
}

UBOOL DishonoredGFxInputChar( INT ControllerId, TCHAR Character )
{
	return GGFxEngine != NULL ? GGFxEngine->InputChar( ControllerId, Character ) : FALSE;
}

void DishonoredGFxRenderTextures()
{
	if( GGFxEngine != NULL )
	{
		GGFxEngine->RenderTextures();
	}
}

void DishonoredGFxNotifyGameSessionEnded()
{
	if( GGFxEngine != NULL )
	{
		GGFxEngine->NotifyGameSessionEnded();
	}
}

#endif // DISHONORED_GFXUI_GFX3_RUNTIME
