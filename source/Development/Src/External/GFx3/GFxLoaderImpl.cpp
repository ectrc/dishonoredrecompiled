// GFxLoader's non-virtual API and the loader-side state bag. See GFxLoaderImpl.h for the evidence and
// for the one deviation (the bind process is synchronous here).
//
// DISHONORED(port): 2013 rvas on every body. The three the engine calls are
//   GFxLoader::GFxLoader     0x9b3d40
//   GFxLoader::GetMovieInfo  0x9b3fe0
//   GFxLoader::CreateMovie   0x9b4030
#include "GFxLoaderImpl.h"
#include "GFxGfxFile.h"
#include "GFxCharacterDefs.h"
#include "GFxFont.h"
#include "GFxTextField.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------
static void GFxLoaderCopyStr(char* out, unsigned int outSize, const char* in)
{
    unsigned int n = 0;
    if (outSize == 0)
        return;
    if (in)
    {
        while (in[n] && n + 1 < outSize)
        {
            out[n] = in[n];
            ++n;
        }
    }
    out[n] = 0;
}

static char GFxLoaderLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

// A url comparison that is case- and separator-insensitive, which is what the cook needs: the same
// import is spelled `..\DisFonts\gfxfontlib.swf` in one movie and `../disfonts/gfxfontlib.swf` in
// another (agentCD.md section 8, measured over the whole cook).
static bool GFxLoaderUrlEqual(const char* a, const char* b)
{
    unsigned int i = 0;
    for (;; ++i)
    {
        char ca = GFxLoaderLower(a[i]);
        char cb = GFxLoaderLower(b[i]);
        if (ca == '\\') ca = '/';
        if (cb == '\\') cb = '/';
        if (ca != cb)
            return false;
        if (ca == 0)
            return true;
    }
}

// ---------------------------------------------------------------------------------------------
GFxLoaderImpl::GFxLoaderImpl()
    : Movies(0), MovieCount(0), MovieCapacity(0), ImportsBound(0), FontsRegistered(0)
{
    for (unsigned int i = 0; i < MaxStates; ++i)
        States[i] = 0;
    CurrentUrl[0] = 0;
}

GFxLoaderImpl::~GFxLoaderImpl()
{
    for (unsigned int i = 0; i < MovieCount; ++i)
    {
        // The data def owns nothing of the buffer; the cache owns both.
        delete Movies[i].pDef;
        free(Movies[i].pData);
    }
    free(Movies);
    for (unsigned int i = 0; i < MaxStates; ++i)
    {
        if (States[i])
            States[i]->Release();
    }
}

void GFxLoaderImpl::SetState(GFxState::StateType t, GFxState* s)
{
    if ((unsigned int)t >= MaxStates)
        return;
    if (s)
        s->AddRef();
    if (States[t])
        States[t]->Release();
    States[t] = s;
}

GFxState* GFxLoaderImpl::GetStateAddRef(GFxState::StateType t) const
{
    if ((unsigned int)t >= MaxStates || States[t] == 0)
        return 0;
    States[t]->AddRef();
    return States[t];
}

// GFxURLBuilder::BuildURL's relative arm plus GFxLoadStates::BuildURL (2012 0xa225c0): take the
// parent's directory, append the relative path, normalise the separators and collapse "." and "..".
void GFxLoaderImpl::BuildURL(char* out, unsigned int outSize, const char* parentUrl, const char* relative)
{
    char work[512];
    unsigned int n = 0;

    if (relative == 0)
        relative = "";

    // An absolute path (a leading '/' or a drive letter) replaces the parent entirely, which is
    // GFxURLBuilder::IsPathAbsolute's arm.
    const bool bAbsolute = relative[0] == '/' || relative[0] == '\\'
        || (relative[0] && relative[1] == ':');
    if (!bAbsolute && parentUrl)
    {
        // the parent's directory, i.e. everything up to its last separator
        unsigned int lastSep = 0;
        for (unsigned int i = 0; parentUrl[i] && i < sizeof(work) - 1; ++i)
        {
            if (parentUrl[i] == '/' || parentUrl[i] == '\\')
                lastSep = i + 1;
        }
        for (unsigned int i = 0; i < lastSep && n < sizeof(work) - 1; ++i)
            work[n++] = parentUrl[i] == '\\' ? '/' : parentUrl[i];
    }
    for (unsigned int i = 0; relative[i] && n < sizeof(work) - 1; ++i)
        work[n++] = relative[i] == '\\' ? '/' : relative[i];
    work[n] = 0;

    // Collapse the path components. "/ package/" is one component with a space in it, which is what
    // PACKAGE_PATH is (ScaleformEngine.h:89), so nothing here may split on the space.
    char* parts[64];
    unsigned int lens[64];
    unsigned int count = 0;
    unsigned int at = 0;
    while (work[at] && count < 64)
    {
        unsigned int start = at;
        while (work[at] && work[at] != '/')
            ++at;
        const unsigned int len = at - start;
        if (len == 1 && work[start] == '.')
        {
            // "." drops out
        }
        else if (len == 2 && work[start] == '.' && work[start + 1] == '.')
        {
            if (count)
                --count;
        }
        else if (len)
        {
            parts[count] = work + start;
            lens[count] = len;
            ++count;
        }
        if (work[at] == '/')
            ++at;
    }

    unsigned int w = 0;
    if (outSize == 0)
        return;
    if (work[0] == '/' && w + 1 < outSize)
        out[w++] = '/';
    for (unsigned int i = 0; i < count; ++i)
    {
        if (i && w + 1 < outSize)
            out[w++] = '/';
        for (unsigned int k = 0; k < lens[i] && w + 1 < outSize; ++k)
            out[w++] = parts[i][k];
    }
    out[w] = 0;
}

GFile* GFxLoaderImpl::OpenUrl(const char* url)
{
    GFxState* s = GetStateAddRef(GFxState::State_FileOpener);
    if (s == 0)
        return 0;
    GFxFileOpenerBase* opener = (GFxFileOpenerBase*)s;
    GFile* f = opener->OpenFile(url, GFileConstants::Open_Read, 0);
    s->Release();
    return f;
}

// Every DefineFont/2/3 of a payload, into CB's GFxFontData, registered with the font manager the text
// engine resolves against. This is agent CB's hand-over 2 (agentCB.md section 9) and agent CD's
// hand-over 3, taken here rather than in the tag loader: the tag loader produces a
// GFxFontCharacterDef for the dictionary, which is what a PlaceObject of a font id needs, and the
// *resource* a text format resolves is a different object with a name, which is what the fontlib
// movie's ExportAssets entry supplies. GFxFontLoadFromPayload does exactly that walk.
void GFxLoaderImpl::registerFonts(const unsigned char* data, unsigned int size)
{
    GFxFontManager* mgr = GFxTextGetFontManager();
    if (mgr == 0)
        return;
    char err[128];
    err[0] = 0;
    FontsRegistered += GFxFontLoadFromPayload(mgr, data, size, err, sizeof(err));
}

GFxMovieDataDef* GFxLoaderImpl::GetOrReadMovie(const char* url)
{
    if (url == 0 || url[0] == 0)
        return 0;
    for (unsigned int i = 0; i < MovieCount; ++i)
    {
        if (GFxLoaderUrlEqual(Movies[i].Url, url))
            return Movies[i].bBinding ? 0 : Movies[i].pDef;
    }

    GFile* f = OpenUrl(url);
    if (f == 0)
        return 0;
    const int len = f->GetLength();
    unsigned char* buf = 0;
    if (len > 0)
    {
        buf = (unsigned char*)malloc((unsigned int)len);
        if (buf && f->Read(buf, len) != len)
        {
            free(buf);
            buf = 0;
        }
    }
    f->Release();
    if (buf == 0)
        return 0;

    GFxMovieDataDef* def = new GFxMovieDataDef;
    if (!def->Read(buf, (unsigned int)len))
    {
        delete def;
        free(buf);
        return 0;
    }

    if (MovieCount == MovieCapacity)
    {
        const unsigned int cap = MovieCapacity ? MovieCapacity * 2 : 8;
        CacheEntry* grown = (CacheEntry*)realloc(Movies, cap * sizeof(CacheEntry));
        if (grown == 0)
        {
            delete def;
            free(buf);
            return 0;
        }
        Movies = grown;
        MovieCapacity = cap;
    }
    CacheEntry& e = Movies[MovieCount++];
    GFxLoaderCopyStr(e.Url, sizeof(e.Url), url);
    e.pDef = def;
    e.pData = buf;
    e.Size = (unsigned int)len;
    e.bBinding = true;
    def->SetSourceUrl(url);

    registerFonts(buf, (unsigned int)len);

    // The bind step. The resolver is this, so an import's own imports recurse; bBinding is what stops
    // the recursion when a lib movie imports a symbol from itself, which two of the cook's movies do.
    char savedUrl[256];
    GFxLoaderCopyStr(savedUrl, sizeof(savedUrl), CurrentUrl);
    GFxLoaderCopyStr(CurrentUrl, sizeof(CurrentUrl), url);
    ImportsBound += def->BindImports(this);
    GFxLoaderCopyStr(CurrentUrl, sizeof(CurrentUrl), savedUrl);

    // The entry may have moved if a nested import grew the array.
    for (unsigned int i = 0; i < MovieCount; ++i)
    {
        if (Movies[i].pDef == def)
            Movies[i].bBinding = false;
    }
    return def;
}

GFxMovieDataDef* GFxLoaderImpl::ResolveImportMovie(const char* url)
{
    char resolved[256];
    BuildURL(resolved, sizeof(resolved), CurrentUrl, url);
    return GetOrReadMovie(resolved);
}

// ---------------------------------------------------------------------------------------------
// The image-binding hook. GFxCharacterDefs.cpp calls this to turn a tag-1009 reference into a
// GImageInfoBase; the url is the image's name resolved against the movie's own url, which is exactly
// the shape FGFxImageLoader::LoadImageW (2013 0x586020) turns back into a package path.

static GFxLoaderImpl* GFxLoaderCurrent = 0;

static GPtr<GImageInfoBase> GFxLoaderResolveImage(GFxMovieDataDef* dataDef, const char* name)
{
    GPtr<GImageInfoBase> result;
    if (GFxLoaderCurrent == 0 || name == 0 || name[0] == 0)
        return result;
    GFxState* s = GFxLoaderCurrent->GetStateAddRef(GFxState::State_ImageLoader);
    if (s == 0)
        return result;
    char url[256];
    GFxLoaderImpl::BuildURL(url, sizeof(url), dataDef ? dataDef->GetSourceUrl() : 0, name);
    GImageInfoBase* info = ((GFxImageLoader*)s)->LoadImageW(url);
    s->Release();
    if (info)
    {
        result = info;
        info->Release();      // LoadImageW returns a reference the GPtr now owns
    }
    return result;
}

// ---------------------------------------------------------------------------------------------
// GFxLoader itself.

GFxLoader::GFxLoader()
{
    // DISHONORED(port): 2013 rva 0x9b3d40. Retail's four-argument form takes the file opener, the
    // zlib and jpeg support states and the default load flags; the engine constructs it with all four
    // and then calls SetState for everything else (FGFxEngine::FGFxEngine 2013 0x5a2370), so the
    // states are set the same way either path.
    pImpl = new GFxLoaderImpl;
    pStrongResourceLib = 0;
    DefLoadFlags = 0;
    GFxLoaderCurrent = pImpl;
    GFxImageResolveHook = GFxLoaderResolveImage;
}

void GFxLoader::Shutdown()
{
    // The generated destructor is inline and empty (GFx3Gen.h keeps GFxLoader at its PDB size of 16),
    // so the owner calls this. FGFxEngine's destructor does.
    if (GFxLoaderCurrent == pImpl)
    {
        GFxLoaderCurrent = 0;
        GFxImageResolveHook = 0;
    }
    delete pImpl;
    pImpl = 0;
}

void GFxLoader::SetState(GFxState::StateType t, GFxState* s)
{
    if (pImpl)
        pImpl->SetState(t, s);
}

GFxState* GFxLoader::GetStateAddRef(GFxState::StateType t) const
{
    return pImpl ? pImpl->GetStateAddRef(t) : 0;
}

void GFxLoader::GetStatesAddRef(GFxState** out, const GFxState::StateType* types, unsigned int n) const
{
    for (unsigned int i = 0; i < n; ++i)
        out[i] = GetStateAddRef(types[i]);
}

bool GFxLoader::GetMovieInfo(const char* url, GFxMovieInfo* info, bool getTagCount,
                             unsigned int loadFlags)
{
    // DISHONORED(port): 2013 rva 0x9b3fe0. Retail opens the file, reads the header and fills
    // GFxMovieInfo without building a definition; getTagCount makes it walk the tag stream too.
    (void)loadFlags;
    if (info == 0 || pImpl == 0)
        return false;
    memset(info, 0, sizeof(*info));

    GFile* f = pImpl->OpenUrl(url);
    if (f == 0)
        return false;
    const int len = f->GetLength();
    unsigned char* buf = 0;
    if (len > 0)
    {
        buf = (unsigned char*)malloc((unsigned int)len);
        if (buf && f->Read(buf, len) != len)
        {
            free(buf);
            buf = 0;
        }
    }
    f->Release();
    if (buf == 0)
        return false;

    GFxGfxFileInfo* fi = new GFxGfxFileInfo;
    const bool ok = GFxGfxParseFile(buf, (unsigned int)len, *fi);
    if (ok)
    {
        info->Version = fi->Version;
        info->Flags = 0;
        if (fi->IsCompressed)
            info->Flags |= GFxMovieInfo::SWF_Compressed;
        if (fi->IsGfxExport)
            info->Flags |= GFxMovieInfo::SWF_Stripped;
        info->Width = (int)(fi->FrameWidthPixels + 0.5f);
        info->Height = (int)(fi->FrameHeightPixels + 0.5f);
        info->FPS = fi->FrameRate;
        info->FrameCount = fi->FrameCount;
        info->TagCount = getTagCount ? fi->TagCount : 0;
        info->ExporterVersion = (unsigned short)fi->ExporterInfo.Version;
        info->ExporterFlags = fi->ExporterInfo.ExportFlags;
    }
    delete fi;
    free(buf);
    return ok;
}

GFxMovieDef* GFxLoader::CreateMovie(const char* url, unsigned int loadFlags, unsigned int memoryArena)
{
    // DISHONORED(port): 2013 rva 0x9b4030. Read (cached), bind the imports, wrap in a
    // GFxMovieDefImpl whose state bag falls through to this loader - which is what lets a movie
    // instance reach the render config, the log, the image loader and the font cache the engine set.
    (void)loadFlags;
    (void)memoryArena;
    if (pImpl == 0)
        return 0;
    GFxMovieDataDef* dataDef = pImpl->GetOrReadMovie(url);
    if (dataDef == 0)
        return 0;
    GFxMovieDefImpl* impl = new GFxMovieDefImpl(dataDef);
    impl->SetOwnsDataDef(false);      // the cache owns it; see GFxMovieDefImpl::~GFxMovieDefImpl
    impl->SetStateBagParent(this);
    impl->SetFileURL(url);
    return impl;
}
