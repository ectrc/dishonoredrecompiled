// GFx 3.3 GFxLoader: the non-virtual API the engine calls to open a movie, plus the loader-side
// state bag and the movie-data cache behind GFxLoader::pImpl.
//
// Agent BC's report (agentBC.md section 6 item 6) named this as the one thing between the runtime and
// the engine: GFxLoader::CreateMovie (2013 0x9b4030) and GFxLoader::GetMovieInfo (2013 0x9b3fe0) are
// the two functions FGFxEngine::LoadMovieDef (2013 0x594890) calls, and agent BB's generator emits
// only virtuals, so GFx3Gen.h carried GFxLoader's members and its one virtual and nothing else. The
// declarations added to GFx3Gen.h's GFxLoader are those two plus the constructor, Shutdown and the
// four GFxStateBag slots; sizeof(GFxLoader) is unchanged at 16 and GFx3Layout.cpp still passes.
//
// What retail does, and what this reproduces:
//   GFxLoader::GFxLoader (0x9b3d40)   builds a GFxLoaderImpl, which owns the state bag and the
//                                     resource library.
//   GFxLoader::GetMovieInfo (0x9b3fe0) opens the url through the state bag's GFxFileOpener, reads the
//                                     header and fills GFxMovieInfo without instantiating anything.
//   GFxLoader::CreateMovie (0x9b4030)  the same open, then GFxMovieDataDef::Read, then the bind
//                                     process (GFxMovieBindProcess), then a GFxMovieDefImpl over it.
//
// DEVIATION, stated once. Retail's bind process is a task on the loader's GFxTaskManager with its own
// GFxLoadStates clone per import (GFxLoadStates::CloneForImport 0xa240b0) and a weak resource library
// keyed by url and modify time. This loader binds synchronously in CreateMovie, caches the parsed
// GFxMovieDataDef per url in GFxLoaderImpl, and resolves an import by opening it through the same
// file opener. The observable result - every import bound, every dictionary slot real, one parse per
// url - is retail's; the threading and the library's pinning are not.
#ifndef INC_GFX3_GFXLOADERIMPL_H
#define INC_GFX3_GFXLOADERIMPL_H

#include "GFxPlayer.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

// GFxLoaderImpl: what GFxLoader::pImpl points at. Retail's is the loader's whole implementation
// (GFxLoaderImpl, 41 functions in GFxLoader.obj); this is the state bag and the movie cache, which is
// what the engine's two entry points need.
class GFxLoaderImpl : public GFxMovieDataDef::ImportResolver
{
public:
    enum { MaxStates = 40 };

    GFxLoaderImpl();
    virtual ~GFxLoaderImpl();

    void       SetState(GFxState::StateType t, GFxState* s);
    GFxState*  GetStateAddRef(GFxState::StateType t) const;

    // The parse cache. One GFxMovieDataDef per url, kept for the life of the loader, which is what
    // makes an import shared between movies rather than parsed twice (retail's GFxResourceWeakLib).
    GFxMovieDataDef* GetOrReadMovie(const char* url);

    // The ImportResolver the bind process is handed: it is the same GetOrReadMovie, so an import's
    // own imports resolve recursively exactly as GFxMovieBindProcess recurses.
    virtual GFxMovieDataDef* ResolveImportMovie(const char* url);

    // The url an import's relative authoring path resolves to, against the parent's url. The cook's
    // urls mix separators and cases within one file (agentCD.md section 8), so this normalises both
    // and collapses "..": that is GFxURLBuilder::BuildURL plus GFxLoadStates::BuildURL (0xa225c0).
    static void BuildURL(char* out, unsigned int outSize, const char* parentUrl, const char* relative);

    GFile* OpenUrl(const char* url);

    unsigned int GetCachedMovieCount() const { return MovieCount; }
    unsigned int GetImportsBound() const { return ImportsBound; }
    unsigned int GetFontsRegistered() const { return FontsRegistered; }

private:
    struct CacheEntry
    {
        char             Url[256];
        GFxMovieDataDef* pDef;
        unsigned char*   pData;
        unsigned int     Size;
        bool             bBinding;   // guards the self-referential import every lib movie has
    };

    void registerFonts(const unsigned char* data, unsigned int size);

    GFxState*    States[MaxStates];
    CacheEntry*  Movies;
    unsigned int MovieCount;
    unsigned int MovieCapacity;
    char         CurrentUrl[256];    // the parent url BuildURL resolves an import against
    unsigned int ImportsBound;
    unsigned int FontsRegistered;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFX3_GFXLOADERIMPL_H
