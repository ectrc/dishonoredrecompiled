// Scaleform GFx 3.3.89 - GENERATED, do not edit by hand.
// Produced by build/agentBB_gen_gfx3.py from the 2012 Shipping PDB
// (Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.pdb) through
// resources/tools/pdb/dia_types.py. Every member is at its PDB offset and every virtual at its PDB
// vtable slot; the slot is in the trailing comment. 92.9 % of libgfx is byte-identical between
// that build and the retail 2013 exe (resources/docs/gfx_decision.md 1), so these are the retail
// layouts. Layout assertions: GFx3Layout.cpp.
//
// DISHONORED(layout): 2013 rvas of the three entry points this API exists for -
//   GFxLoader::CreateMovie   0x9b4030
//   GFxLoader::GetMovieInfo  0x9b3fe0
//   FGFxExternalInterface::Callback 0x58d510
//
// MSVC lays a run of consecutive virtual overloads out in REVERSE declaration order, so same-name
// runs below are declared highest-slot-first on purpose (agentAL.md 4). The generator re-applies
// the reversal and asserts every class's slots come out 0..n-1.
#ifndef INC_GFX3_GEN_H
#define INC_GFX3_GEN_H

#include "GTypes.h"

#pragma pack(push, 8)

// Forward declarations. Every class this header defines is declared here first, so a
// pointer or reference to any of them works whatever order the definitions come in.
class GFxValue;      // hand-written, GFxValue.h (it includes this header)
class GASStringContext;
class GAllocDebugInfo;
class GFile;
class GDelegatedFile;
class GBufferedFile;
class GFxASCharacter;
class GFxState;
class GFxActionControl;
class GFxEvent;
class GFxCharEvent;
class GFxExporterInfo;
class GFxExternalInterface;
class GFxFSCommandHandler;
class GFxFileOpenerBase;
class GFxFileOpener;
class GFxFontCacheManager;
class GFxFontCacheManagerImpl;
class GFxFontLib;
class GFxFontLibImpl;
class GFxFontMap;
class GFxFontMapImpl;
class GFxFontPackParams;
class GFxFontProvider;
class GFxFontResource;
class GFxFunctionHandler;
class GFxGlyphCacheVisitor;
class GFxGradientParams;
class GFxResourceKey;
class GFxResource;
class GFxImageCreateInfo;
class GFxImageCreator;
class GFxResourceFileInfo;
class GFxImageFileInfo;
class GFxImageLoader;
class GFxImageSubstProvider;
class GFxImagePackParamsBase;
class GFxImagePackParams;
class GFxImagePacker;
class GFxResourceId;
class GImageBase;
class GTexture;
class GImageInfoBase;
class GFxImageResource;
class GFxStateBag;
class GMemoryHeap;
class GFxMovieDef;
class GFxImportVisitor;
class GFxJpegSupportBase;
class GJPEGSystem;
class GFxJpegSupport;
class GFxSpecialKeysState;
class GFxKeyEvent;
class GFxZlibSupportBase;
class GFxLoader;
class GFxLoaderImpl;
class GFxLogConstants;
class GFxLog;
class GFxMeshCache;
class GFxMeshCacheManager;
class GFxMouseCursorEvent;
class GFxMouseEvent;
class GFxMovie;
class GFxMovieInfo;
class GFxMovieRoot;
class GFxMovieView;
class GFxPNGSupportBase;
class GFxParseControl;
class GFxPlayerLog;
class GFxProgressHandler;
class GRendererEventHandler;
class GRenderer;
class GFxRenderConfig;
class GFxRenderGen;
class GFxRenderStats;
class GFxResourceLibBase;
class GFxResourceWeakLib;
class GFxResourceLib;
class GFxResourceReport;
class GFxSetFocusEvent;
class GFxShapeBase;
class GFxSharedObjectManagerBase;
class GFxStream;
class GFxStyledText;
class GFxSubImageResource;
class GSystem;
class GFxSystem;
class GFxTaskManager;
class GFxWStringBuffer;
class GFxTextClipboard;
class GFxTextureGlyphData;
class GFxTranslator;
class GFxUITranslator;
class GFxURLBuilder;
class GFxUserEventHandler;
class GFxXMLSupportBase;
class GFxZlibSupport;
class GHeapMemVisitor;
class GHeapSegVisitor;
class GImage;
class GImageInfoBaseImpl;
class GImageInfo;
class GJPEGInput;
class GJPEGOutput;
class GMemory;
class GMemoryFile;
class GRenderTarget;
class GRendererNode;
class GRenderTargetImplNode;
class GSubImageInfo;
class GSysAllocBase;
class GSysAlloc;
class GSysAllocPaged;
class GSysAllocMalloc;
class GSysFile;
class GSysMemoryMap;
class GTextureImplNode;

// Reached only through a pointer or reference, and not defined here at all:
class GFxFont;

// GFxKey has no type record of its own in the PDB: it is a tag class whose only
// content is the enum below.
class GFxKey
{
public:
    enum Code
    {
        VoidSymbol = 0x0,
        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A,
        Num0 = 0x30,
        Num1 = 0x31,
        Num2 = 0x32,
        Num3 = 0x33,
        Num4 = 0x34,
        Num5 = 0x35,
        Num6 = 0x36,
        Num7 = 0x37,
        Num8 = 0x38,
        Num9 = 0x39,
        KP_0 = 0x60,
        KP_1 = 0x61,
        KP_2 = 0x62,
        KP_3 = 0x63,
        KP_4 = 0x64,
        KP_5 = 0x65,
        KP_6 = 0x66,
        KP_7 = 0x67,
        KP_8 = 0x68,
        KP_9 = 0x69,
        KP_Multiply = 0x6A,
        KP_Add = 0x6B,
        KP_Enter = 0x6C,
        KP_Subtract = 0x6D,
        KP_Decimal = 0x6E,
        KP_Divide = 0x6F,
        F1 = 0x70,
        F2 = 0x71,
        F3 = 0x72,
        F4 = 0x73,
        F5 = 0x74,
        F6 = 0x75,
        F7 = 0x76,
        F8 = 0x77,
        F9 = 0x78,
        F10 = 0x79,
        F11 = 0x7A,
        F12 = 0x7B,
        F13 = 0x7C,
        F14 = 0x7D,
        F15 = 0x7E,
        Backspace = 0x8,
        Tab = 0x9,
        Clear = 0xC,
        Return = 0xD,
        Shift = 0x10,
        Control = 0x11,
        Alt = 0x12,
        Pause = 0x13,
        CapsLock = 0x14,
        Escape = 0x1B,
        Space = 0x20,
        PageUp = 0x21,
        PageDown = 0x22,
        End = 0x23,
        Home = 0x24,
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,
        Insert = 0x2D,
        Delete = 0x2E,
        Help = 0x2F,
        NumLock = 0x90,
        ScrollLock = 0x91,
        Semicolon = 0xBA,
        Equal = 0xBB,
        Comma = 0xBC,
        Minus = 0xBD,
        Period = 0xBE,
        Slash = 0xBF,
        Bar = 0xC0,
        BracketLeft = 0xDB,
        Backslash = 0xDC,
        BracketRight = 0xDD,
        Quote = 0xDE,
        OEM_AX = 0xE1,
        OEM_102 = 0xE2,
        ICO_HELP = 0xE3,
        ICO_00 = 0xE4,
        KeyCount = 0xE5
    };
};

// GSysAllocStatic has no type record of its own in the PDB: it is a tag class whose only
// content is the enum below.
class GSysAllocStatic
{
public:
    enum
    {
        MaxSegments = 0x4
    };
};

class GASStringContext;

class GAllocDebugInfo
{
public:
    unsigned int StatId;
};

class GFile : public GRefCountBase<GFile, 2>, public GFileConstants
{
public:

    virtual ~GFile() {} // vt[0]
    virtual const char* GetFilePath() = 0; // vt[1]
    virtual bool IsValid() = 0; // vt[2]
    virtual bool IsWritable() = 0; // vt[3]
    virtual int Tell() = 0; // vt[4]
    virtual __int64 LTell() = 0; // vt[5]
    virtual int GetLength() = 0; // vt[6]
    virtual __int64 LGetLength() = 0; // vt[7]
    virtual int GetErrorCode() = 0; // vt[8]
    virtual int Write(const unsigned char* a0, int a1) = 0; // vt[9]
    virtual int Read(unsigned char* a0, int a1) = 0; // vt[10]
    virtual int SkipBytes(int a0) = 0; // vt[11]
    virtual int BytesAvailable() = 0; // vt[12]
    virtual bool Flush() = 0; // vt[13]
    virtual int Seek(int a0, int a1) = 0; // vt[14]
    virtual __int64 LSeek(__int64 a0, int a1) = 0; // vt[15]
    virtual bool ChangeSize(int a0) = 0; // vt[16]
    virtual int CopyFromStream(GFile* a0, int a1) = 0; // vt[17]
    virtual bool Close() = 0; // vt[18]
};

class GDelegatedFile : public GFile
{
public:
    GPtr<GFile> pFile;

    virtual ~GDelegatedFile() {} // vt[0]
};

class GBufferedFile : public GDelegatedFile
{
public:
    enum BufferModeType
    {
        NoBuffer = 0x0,
        ReadBuffer = 0x1,
        WriteBuffer = 0x2
    };
    unsigned char* pBuffer;
    GBufferedFile::BufferModeType BufferMode;
    unsigned int Pos;
    unsigned int DataSize;
    GUByte _pad28[4];
    unsigned __int64 FilePos;

    virtual ~GBufferedFile() {} // vt[0]
};

class GFxASCharacter;

class GFxState : public GRefCountBase<GFxState, 2>
{
public:
    enum StateType
    {
        State_None = 0x0,
        State_RenderConfig = 0x1,
        State_RenderStats = 0x2,
        State_Translator = 0x3,
        State_Log = 0x4,
        State_ImageLoader = 0x5,
        State_ActionControl = 0x6,
        State_UserEventHandler = 0x7,
        State_FSCommandHandler = 0x8,
        State_ExternalInterface = 0x9,
        State_FileOpener = 0xA,
        State_URLBuilder = 0xB,
        State_ImageCreator = 0xC,
        State_ParseControl = 0xD,
        State_ProgressHandler = 0xE,
        State_ImportVisitor = 0xF,
        State_MeshCacheManager = 0x10,
        State_FontPackParams = 0x11,
        State_FontCacheManager = 0x12,
        State_FontLib = 0x13,
        State_FontProvider = 0x14,
        State_FontMap = 0x15,
        State_GradientParams = 0x16,
        State_TaskManager = 0x17,
        State_TextClipboard = 0x18,
        State_TextKeyMap = 0x19,
        State_PreprocessParams = 0x1A,
        State_IMEManager = 0x1B,
        State_XMLSupport = 0x1C,
        State_JpegSupport = 0x1D,
        State_ZlibSupport = 0x1E,
        State_FontCompactorParams = 0x1F,
        State_ImagePackerParams = 0x20,
        State_PNGSupport = 0x21,
        State_Audio = 0x22,
        State_Video = 0x23,
        State_TestStream = 0x24,
        State_SharedObject = 0x25,
        State_LocSupport = 0x26
    };
    GFxState::StateType SType;

    virtual ~GFxState() {} // vt[0]
};

class GFxActionControl : public GFxState
{
public:
    enum ActionControlFlags
    {
        Action_Verbose = 0x1,
        Action_ErrorSuppress = 0x2,
        Action_LogRootFilenames = 0x4,
        Action_LogChildFilenames = 0x8,
        Action_LogAllFilenames = 0xC,
        Action_LongFilenames = 0x10
    };
    unsigned int ActionFlags;

    virtual ~GFxActionControl() {} // vt[0]
};

class GFxEvent : public GNewOverrideBase<2>
{
public:
    enum EventType
    {
        None = 0x0,
        MouseMove = 0x1,
        MouseDown = 0x2,
        MouseUp = 0x3,
        MouseWheel = 0x4,
        KeyDown = 0x5,
        KeyUp = 0x6,
        SceneResize = 0x7,
        SetFocus = 0x8,
        KillFocus = 0x9,
        DoShowMouse = 0xA,
        DoHideMouse = 0xB,
        DoSetMouseCursor = 0xC,
        CharEvent = 0xD,
        IMEEvent = 0xE
    };
    GFxEvent::EventType Type;
};

class GFxCharEvent : public GFxEvent
{
public:
    unsigned long WcharCode;
    unsigned char KeyboardIndex;
    GUByte _pad9[3];
};

class GFxExporterInfo
{
public:
    enum ExportFlagConstants
    {
        EXF_GlyphTexturesExported = 0x1,
        EXF_GradientTexturesExported = 0x2,
        EXF_GlyphsStripped = 0x10
    };
    GFxFileConstants::FileFormatType Format;
    const char* pPrefix;
    const char* pSWFName;
    unsigned short Version;
    GUByte _pad14[2];
    unsigned int ExportFlags;
};

class GFxExternalInterface : public GFxState
{
public:

    virtual ~GFxExternalInterface() {} // vt[0]
    virtual void Callback(GFxMovieView* a0, const char* a1, const GFxValue* a2, unsigned int a3) = 0; // vt[1]
};

class GFxFSCommandHandler : public GFxState
{
public:

    virtual ~GFxFSCommandHandler() {} // vt[0]
    virtual void Callback(GFxMovieView* a0, const char* a1, const char* a2) = 0; // vt[1]
};

class GFxFileOpenerBase : public GFxState
{
public:

    virtual ~GFxFileOpenerBase() {} // vt[0]
    virtual GFile* OpenFile(const char* a0, int a1, int a2) = 0; // vt[1]
    virtual __int64 GetFileModifyTime(const char* a0) = 0; // vt[2]
    virtual GFile* OpenFileEx(const char* a0, GFxLog* a1, int a2, int a3) = 0; // vt[3]
};

class GFxFileOpener : public GFxFileOpenerBase
{
public:

    virtual ~GFxFileOpener() {} // vt[0]
};

class GFxFontCacheManager : public GFxState
{
public:
    class TextureConfig;
    class TextureConfig
    {
    public:
        unsigned int TextureWidth;
        unsigned int TextureHeight;
        unsigned int MaxNumTextures;
        unsigned int MaxSlotHeight;
        unsigned int SlotPadding;
        unsigned int TexUpdWidth;
        unsigned int TexUpdHeight;
    };
    GFxFontCacheManager::TextureConfig CacheTextureConfig;
    bool DynamicCacheEnabled;
    bool Text3DVectorizationEnabled;
    GUByte _pad42[2];
    float MaxRasterScale;
    unsigned int SteadyCount;
    unsigned int MaxVectorCacheSize;
    float FauxItalicAngle;
    float FauxBoldRatio;
    float OutlineRatio;
    unsigned int NumLockedFrames;
    GFxFontCacheManagerImpl* pCache;

    virtual ~GFxFontCacheManager() {} // vt[0]
};

class GFxFontCacheManagerImpl;

class GFxFontLib : public GFxState
{
public:
    class FontResult;
    class FontResult
    {
    public:
        GFxMovieDef* pMovieDef;
        GFxFontResource* pFontResource;
    };
    GFxFontLibImpl* pImpl;

    virtual ~GFxFontLib() {} // vt[0]
    virtual bool FindFont(GFxFontLib::FontResult* a0, const char* a1, unsigned int a2, GFxMovieDef* a3, GFxStateBag* a4, GFxResourceWeakLib* a5); // vt[1]
};

class GFxFontLibImpl;

class GFxFontMap : public GFxState
{
public:
    enum MapFontFlags
    {
        MFF_Original = 0x10,
        MFF_NoAutoFit = 0x20,
        MFF_Normal = 0x0,
        MFF_Italic = 0x1,
        MFF_Bold = 0x2,
        MFF_BoldItalic = 0x3,
        MFF_FauxItalic = 0x4,
        MFF_FauxBold = 0x8,
        MFF_FauxBoldItalic = 0xC
    };
    class MapEntry;
    class MapEntry
    {
    public:
        GString Name;
        float ScaleFactor;
        float GlyphOffsetX;
        float GlyphOffsetY;
        GFxFontMap::MapFontFlags Flags;
    };
    GFxFontMapImpl* pImpl;

    virtual ~GFxFontMap() {} // vt[0]
};

class GFxFontMapImpl;

class GFxFontPackParams : public GFxState
{
public:
    class TextureConfig;
    class TextureConfig
    {
    public:
        int NominalSize;
        int PadPixels;
        int TextureWidth;
        int TextureHeight;
    };
    GFxFontPackParams::TextureConfig PackTextureConfig;
    bool SeparateTextures;
    GUByte _pad29[3];
    int GlyphCountLimit;

    virtual ~GFxFontPackParams() {} // vt[0]
    virtual GFxState::StateType GetStateType() const; // vt[1]
};

class GFxFontProvider : public GFxState
{
public:

    virtual ~GFxFontProvider() {} // vt[0]
    virtual GFxFont* CreateFontW(const char* a0, unsigned int a1) = 0; // vt[1]
    virtual void LoadFontNames(void* /* GStringHash<GString,GAllocatorGH<GString,2> >& */ a0) = 0; // vt[2]
};

class GFxFontResource;

class GFxFunctionHandler : public GRefCountBase<GFxFunctionHandler, 2>
{
public:
    class Params;
    class Params
    {
    public:
        GFxValue* pRetVal;
        GFxMovieView* pMovie;
        GFxValue* pThis;
        GFxValue* pArgsWithThisRef;
        GFxValue* pArgs;
        unsigned int ArgCount;
        void* pUserData;
    };

    virtual ~GFxFunctionHandler() {} // vt[0]
    virtual void Call(const GFxFunctionHandler::Params& a0) = 0; // vt[1]
};

class GFxGlyphCacheVisitor;

class GFxGradientParams : public GFxState
{
public:
    unsigned int RadialGradientImageSize;
    bool AdaptiveGradients;
    GUByte _pad17[3];

    virtual ~GFxGradientParams() {} // vt[0]
};

class GFxResourceKey
{
public:
    enum KeyType
    {
        Key_None = 0x0,
        Key_Unique = 0x1,
        Key_File = 0x2,
        Key_Gradient = 0x3,
        Key_SubImage = 0x4
    };
    class HashOp;
    class KeyInterface;
    class HashOp
    {
    public:
    };
    class KeyInterface
    {
    public:

        virtual ~KeyInterface() {} // vt[0]
        virtual void AddRef(void* a0) = 0; // vt[1]
        virtual void Release(void* a0) = 0; // vt[2]
        virtual GFxResourceKey::KeyType GetKeyType(void* a0) const = 0; // vt[3]
        virtual unsigned int GetHashCode(void* a0) const = 0; // vt[4]
        virtual bool KeyEquals(void* a0, const GFxResourceKey& a1) = 0; // vt[5]
        virtual const char* GetFileURL(void* a0) const; // vt[6]
    };
    GFxResourceKey::KeyInterface* pKeyInterface;
    void* hKeyData;
};

class GFxResource : public GNewOverrideBase<2>
{
public:
    enum ResourceType
    {
        RT_CharacterDef_Bit = 0x80,
        RT_None = 0x0,
        RT_Image = 0x1,
        RT_Font = 0x2,
        RT_MovieDef = 0x3,
        RT_SoundSample = 0x4,
        RT_MovieDataDef = 0x80,
        RT_ButtonDef = 0x81,
        RT_TextDef = 0x82,
        RT_EditTextDef = 0x83,
        RT_SpriteDef = 0x84,
        RT_ShapeDef = 0x85,
        RT_VideoDef = 0x86,
        RT_TypeCode_Mask = 0xFF00,
        RT_TypeCode_Shift = 0x8
    };
    enum ResourceUse
    {
        Use_None = 0x0,
        Use_Bitmap = 0x1,
        Use_Gradient = 0x2,
        Use_FontTexture = 0x3,
        Use_SoundSample = 0x4,
        Use_TypeCode_Mask = 0xFF
    };
    GAtomicInt<long> RefCount;
    GFxResourceLibBase* pLib;

    // DISHONORED(bringup, agent DC): the refcount starts at ONE. GAtomicInt's default constructor
    // writes 0 (GTypes.h) and retail's GFxResource constructor writes 1, exactly as agent CC measured
    // for GTexture and GRenderTarget (agentCC.md 4.1, FGFxRenderer::CreateTexture 2012 0x5c47d0 writes
    // RefCount.Value = 1 right after the vtable store). Without it the first GPtr that takes and drops a
    // reference frees the object under its creator: FGFxEngine::LoadMovie assigned the movie definition
    // to a GPtr (1) and dropped its own reference (0), the def impl was deleted, and CreateInstance ran
    // on freed memory and jumped through a null vtable - the first in-game instantiation of the runtime,
    // 0.19 s of work and then "Address = 0x0". GFxResource is the third of the three refcounted classes
    // in this header whose default is wrong; the other two are GTexture and GRenderTarget and CC fixed
    // them in the FGFx* constructors. This one has no engine-side constructor to fix it in.
    GFxResource() : pLib(0) { RefCount = 1; }

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount == 0) delete this; }
    int GetRefCount() const { return (int)(long)RefCount; }

    virtual ~GFxResource() {} // vt[0]
    virtual GFxResourceKey GetKey(); // vt[1]
    virtual unsigned int GetResourceTypeCode() const; // vt[2]
    virtual GFxResourceReport* GetResourceReport(); // vt[3]
};

class GFxImageCreateInfo
{
public:
    enum InputType
    {
        Input_None = 0x0,
        Input_Image = 0x1,
        Input_File = 0x2
    };
    GFxImageCreateInfo::InputType Type;
    GFxResource::ResourceUse Use;
    const char* pExportName;
    unsigned int TexUsage;
    GImage* pImage;
    // union alias @16 GFxImageFileInfo* pFileInfo
    GFxFileOpener* pFileOpener;
    GFxRenderConfig* pRenderConfig;
    GFxLog* pLog;
    GFxJpegSupportBase* pJpegSupport;
    GFxPNGSupportBase* pPNGSupport;
    GMemoryHeap* pHeap;
    bool ThreadedLoading;
    GUByte _pad45[3];
};

class GFxImageCreator : public GFxState
{
public:
    bool KeepImageBindData;
    GUByte _pad13[3];

    virtual ~GFxImageCreator() {} // vt[0]
    virtual GImageInfoBase* CreateImage(const GFxImageCreateInfo& a0); // vt[1]
};

class GFxResourceFileInfo : public GRefCountBaseNTS<GFxResourceFileInfo, 2>
{
public:
    GFxFileConstants::FileFormatType Format;
    const GFxExporterInfo* pExporterInfo;
    GString FileName;

    virtual ~GFxResourceFileInfo() {} // vt[0]
};

class GFxImageFileInfo : public GFxResourceFileInfo
{
public:
    enum ImageInfoFlags
    {
        Wrappable = 0x1
    };
    unsigned short TargetWidth;
    unsigned short TargetHeight;
    GFxResource::ResourceUse Use;
    unsigned char Flags;
    GUByte _pad29[3];
    GString ExportName;

    virtual ~GFxImageFileInfo() {} // vt[0]
};

class GFxImageLoader : public GFxState
{
public:

    virtual ~GFxImageLoader() {} // vt[0]
    virtual GImageInfoBase* LoadImageW(const char* a0) = 0; // vt[1]
};

class GFxImageSubstProvider : public GRefCountBase<GFxImageSubstProvider, 2>
{
public:

    virtual ~GFxImageSubstProvider() {} // vt[0]
    virtual GImage* CreateImage(const char* a0) = 0; // vt[1]
};

class GFxImagePackParamsBase : public GFxState
{
public:
    enum SizeOptionType
    {
        PackSize_1 = 0x0,
        PackSize_4 = 0x1,
        PackSize_PowerOf2 = 0x2
    };
    class TextureConfig;
    class TextureConfig
    {
    public:
        int TextureWidth;
        int TextureHeight;
        GFxImagePackParamsBase::SizeOptionType SizeOptions;
    };
    GFxImagePackParamsBase::TextureConfig PackTextureConfig;
    GPtr<GFxImageSubstProvider> pImageSubstProvider;

    virtual ~GFxImagePackParamsBase() {} // vt[0]
    virtual GFxState::StateType GetStateType() const; // vt[1]
    virtual GFxImagePacker* Begin(GFxResourceId* a0, GFxImageCreator* a1, GFxImageCreateInfo* a2) const = 0; // vt[2]
};

class GFxImagePackParams : public GFxImagePackParamsBase
{
public:

    virtual ~GFxImagePackParams() {} // vt[0]
};

class GFxImagePacker;

class GFxResourceId
{
public:
    enum IdType
    {
        IdType_None = 0x0,
        IdType_InternalConstant = 0x10000,
        IdType_GradientImage = 0x50000,
        IdType_DynFontImage = 0x90000,
        IdType_FontImage = 0x60000
    };
    enum IdTypeConstants
    {
        IdType_Bit_IndexMask = 0xFFFF,
        IdType_Bit_TypeMask = 0xFFF0000,
        IdType_Bit_SWF = 0x0,
        IdType_Bit_Static = 0x10000,
        IdType_Bit_Export = 0x20000,
        IdType_Bit_GenMask = 0x30000,
        IdType_Bit_TypeShift = 0x12,
        InvalidId = 0x40000
    };
    class HashOp;
    class HashOp
    {
    public:
    };
    unsigned long Id;
};

class GImageBase
{
public:
    enum ImageFormat
    {
        Image_None = 0x0,
        Image_ARGB_8888 = 0x1,
        Image_RGB_888 = 0x2,
        Image_L_8 = 0x8,
        Image_A_8 = 0x9,
        Image_DXT1 = 0xA,
        Image_DXT3 = 0xB,
        Image_DXT5 = 0xC,
        Image_P_8 = 0x64,
        Image_YUV_822 = 0xC8,
        Image_YUVA_8228 = 0xC9,
        Image_Depth = 0x12C,
        Image_Stencil = 0x12D,
        Image_DepthStencil = 0x12E
    };
    GImageBase::ImageFormat Format;
    unsigned long Width;
    unsigned long Height;
    unsigned long Pitch;
    unsigned char* pData;
    unsigned int DataSize;
    unsigned int MipMapCount;
    GArray<GColor, 2, GArrayDefaultPolicy> ColorMap;
};

class GTexture : public GNewOverrideBase<65>
{
public:
    enum ImageTexUsage
    {
        Usage_Wrap = 0x1,
        Usage_Update = 0x10,
        Usage_Map = 0x20,
        Usage_RenderTarget = 0x40
    };
    enum MapFlags
    {
        Map_KeepOld = 0x1
    };
    class ChangeHandler;
    class MapRect;
    class UpdateRect;
    class ChangeHandler
    {
    public:
        enum EventType
        {
            Event_DataChange = 0x0,
            Event_DataLost = 0x1,
            Event_RendererReleased = 0x2
        };

        virtual ~ChangeHandler() {} // vt[0]
        virtual void OnChange(GRenderer* a0, GTexture::ChangeHandler::EventType a1); // vt[1]
        virtual bool Recreate(GRenderer* a0); // vt[2]
    };
    class MapRect
    {
    public:
        unsigned int width;
        unsigned int height;
        unsigned char* pData;
        unsigned int pitch;
    };
    class UpdateRect
    {
    public:
        GPoint<int> dest;
        GRect<int> src;
    };
    GAtomicInt<long> RefCount;

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount == 0) delete this; }
    int GetRefCount() const { return (int)(long)RefCount; }

    virtual ~GTexture() {} // vt[0]
    virtual bool InitTexture(GImageBase* a0, unsigned int a1) = 0; // vt[1]
    virtual bool InitDynamicTexture(int a0, int a1, GImageBase::ImageFormat a2, int a3, unsigned int a4) = 0; // vt[2]
    virtual void Update(int a0, int a1, const GTexture::UpdateRect* a2, const GImageBase* a3) = 0; // vt[3]
    virtual int Map(int a0, int a1, GTexture::MapRect* a2, int a3) = 0; // vt[4]
    virtual bool Unmap(int a0, int a1, GTexture::MapRect* a2, int a3) = 0; // vt[5]
    virtual GRenderer* GetRenderer() const = 0; // vt[6]
    virtual bool IsDataValid() const = 0; // vt[7]
    virtual void* GetUserData() const = 0; // vt[8]
    virtual void SetUserData(void* a0) = 0; // vt[9]
    virtual void AddChangeHandler(GTexture::ChangeHandler* a0) = 0; // vt[10]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* a0) = 0; // vt[11]
};

class GImageInfoBase : public GRefCountBaseNTS<GImageInfoBase, 2>, public GTexture::ChangeHandler
{
public:
    enum ImageInfoType
    {
        IIT_ImageInfo = 0x0,
        IIT_SubImageInfo = 0x1,
        IIT_Other = 0x2
    };

    virtual ~GImageInfoBase() {} // vt[0]
    virtual unsigned int GetWidth() const = 0; // vt[1]
    virtual unsigned int GetHeight() const = 0; // vt[2]
    virtual GTexture* GetTexture(GRenderer* a0) = 0; // vt[3]
    virtual GImageInfoBase* CreateSubImage(const GRect<int>& a0, GMemoryHeap* a1); // vt[4]
    virtual GRect<int> GetRect() const; // vt[5]
    virtual unsigned int GetImageInfoType() const; // vt[6]
};

class GFxImageResource : public GFxResource
{
public:
    GPtr<GImageInfoBase> pImageInfo;
    GFxResourceKey Key;
    GFxResource::ResourceUse UseType;

    virtual ~GFxImageResource() {} // vt[0]
    virtual GFxResourceId GetBaseImageId(); // vt[4]
    virtual GFxResource::ResourceUse GetImageUse() const; // vt[5]
};

class GFxStateBag : public GFxFileConstants
{
public:

    virtual GFxStateBag* GetStateBagImpl() const; // vt[0]
    virtual ~GFxStateBag() {} // vt[1]
    virtual void SetState(GFxState::StateType a0, GFxState* a1); // vt[2]
    virtual GFxState* GetStateAddRef(GFxState::StateType a0) const; // vt[3]
    virtual void GetStatesAddRef(GFxState** a0, const GFxState::StateType* a1, unsigned int a2) const; // vt[4]
};

class GMemoryHeap : public GListNode<GMemoryHeap>
{
public:
    enum HeapFlags
    {
        Heap_ThreadUnsafe = 0x1,
        Heap_FastTinyBlocks = 0x2,
        Heap_FixedGranularity = 0x4,
        Heap_Root = 0x8,
        Heap_NoDebugInfo = 0x10,
        Heap_UserDebug = 0x1000
    };
    enum MemReportType
    {
        MemReportBrief = 0x0,
        MemReportSummary = 0x1,
        MemReportMedium = 0x2,
        MemReportFull = 0x3,
        MemReportSimple = 0x4,
        MemReportSimpleBrief = 0x5,
        MemReportFileSummary = 0x6,
        MemReportHeapsOnly = 0x7
    };
    enum RootHeapParameters
    {
        RootHeap_MinAlign = 0x10,
        RootHeap_Granularity = 0x4000,
        RootHeap_Reserve = 0x4000,
        RootHeap_Threshold = 0x40000,
        RootHeap_Limit = 0x0
    };
    class HeapDesc;
    class HeapInfo;
    class HeapTracer;
    class HeapVisitor;
    class LimitHandler;
    class RootHeapDesc;
    class RootStats;
    class HeapDesc
    {
    public:
        unsigned int Flags;
        unsigned int MinAlign;
        unsigned int Granularity;
        unsigned int Reserve;
        unsigned int Threshold;
        unsigned int Limit;
        unsigned int HeapId;
        unsigned int Arena;
    };
    class HeapInfo
    {
    public:
        GMemoryHeap::HeapDesc Desc;
        GMemoryHeap* pParent;
        char* pName;
    };
    class HeapTracer
    {
    public:

        virtual ~HeapTracer() {} // vt[0]
        virtual void OnCreateHeap(const GMemoryHeap* a0) = 0; // vt[1]
        virtual void OnDestroyHeap(const GMemoryHeap* a0) = 0; // vt[2]
        virtual void OnAlloc(const GMemoryHeap* a0, unsigned int a1, unsigned int a2, unsigned int a3, const void* a4) = 0; // vt[3]
        virtual void OnRealloc(const GMemoryHeap* a0, const void* a1, unsigned int a2, const void* a3) = 0; // vt[4]
        virtual void OnFree(const GMemoryHeap* a0, const void* a1) = 0; // vt[5]
    };
    class HeapVisitor
    {
    public:

        virtual ~HeapVisitor() {} // vt[0]
        virtual void Visit(GMemoryHeap* a0, GMemoryHeap* a1) = 0; // vt[1]
    };
    class LimitHandler
    {
    public:

        virtual ~LimitHandler() {} // vt[0]
        virtual bool OnExceedLimit(GMemoryHeap* a0, unsigned int a1) = 0; // vt[1]
        virtual void OnFreeSegment(GMemoryHeap* a0, unsigned int a1) = 0; // vt[2]
    };
    class RootHeapDesc : public GMemoryHeap::HeapDesc
    {
    public:
    };
    class RootStats
    {
    public:
        unsigned int SysMemFootprint;
        unsigned int SysMemUsedSpace;
        unsigned int PageMapFootprint;
        unsigned int PageMapUsedSpace;
        unsigned int BookkeepingFootprint;
        unsigned int BookkeepingUsedSpace;
        unsigned int DebugInfoFootprint;
        unsigned int DebugInfoUsedSpace;
        unsigned int UserDebugFootprint;
        unsigned int UserDebugUsedSpace;
    };
    unsigned int SelfSize;
    unsigned int RefCount;
    unsigned int OwnerThreadId;
    void* pAutoRelease;
    GMemoryHeap::HeapInfo Info;
    GList<GMemoryHeap> ChildHeaps;
    GLock HeapLock;
    bool UseLocks;
    bool TrackDebugInfo;
    GUByte _pad102[2];

    virtual ~GMemoryHeap() {} // vt[0]
    virtual void CreateArena(unsigned int a0, GSysAllocPaged* a1) = 0; // vt[1]
    virtual void DestroyArena(unsigned int a0) = 0; // vt[2]
    virtual bool ArenaIsEmpty(unsigned int a0) = 0; // vt[3]
    virtual GMemoryHeap* CreateHeap(const char* a0, const GMemoryHeap::HeapDesc& a1) = 0; // vt[4]
    virtual void SetLimitHandler(GMemoryHeap::LimitHandler* a0) = 0; // vt[5]
    virtual void SetLimit(unsigned int a0) = 0; // vt[6]
    virtual void AddRef() = 0; // vt[7]
    virtual void Release() = 0; // vt[8]
    virtual void* Alloc(unsigned int a0, const GAllocDebugInfo* a1) = 0; // vt[10]
    virtual void* Alloc(unsigned int a0, unsigned int a1, const GAllocDebugInfo* a2) = 0; // vt[9]
    virtual void* Realloc(void* a0, unsigned int a1) = 0; // vt[11]
    virtual void Free(void* a0) = 0; // vt[12]
    virtual void* AllocAutoHeap(const void* a0, unsigned int a1, const GAllocDebugInfo* a2) = 0; // vt[14]
    virtual void* AllocAutoHeap(const void* a0, unsigned int a1, unsigned int a2, const GAllocDebugInfo* a3) = 0; // vt[13]
    virtual GMemoryHeap* GetAllocHeap(const void* a0) = 0; // vt[15]
    virtual unsigned int GetUsableSize(const void* a0) = 0; // vt[16]
    virtual void* AllocSysDirect(unsigned int a0) = 0; // vt[17]
    virtual void FreeSysDirect(void* a0, unsigned int a1) = 0; // vt[18]
    virtual bool GetStats(GStatBag* a0) = 0; // vt[19]
    virtual unsigned int GetFootprint() const = 0; // vt[20]
    virtual unsigned int GetTotalFootprint() const = 0; // vt[21]
    virtual unsigned int GetUsedSpace() const = 0; // vt[22]
    virtual unsigned int GetTotalUsedSpace() const = 0; // vt[23]
    virtual void GetRootStats(GMemoryHeap::RootStats* a0) = 0; // vt[24]
    virtual void VisitMem(GHeapMemVisitor* a0, unsigned int a1) = 0; // vt[25]
    virtual void VisitRootSegments(GHeapSegVisitor* a0) = 0; // vt[26]
    virtual void VisitHeapSegments(GHeapSegVisitor* a0) const = 0; // vt[27]
    virtual void SetTracer(GMemoryHeap::HeapTracer* a0) = 0; // vt[28]
    virtual void destroyItself() = 0; // vt[29]
    virtual void ultimateCheck() = 0; // vt[30]
    virtual void releaseCachedMem() = 0; // vt[31]
    virtual bool dumpMemoryLeaks() = 0; // vt[32]
    virtual void checkIntegrity() const = 0; // vt[33]
    virtual void getUserDebugStats(GMemoryHeap::RootStats* a0) const = 0; // vt[34]
};

class GFxMovieDef : public GFxResource, public GFxStateBag
{
public:
    enum FileAttrFlags
    {
        FileAttr_UseNetwork = 0x1,
        FileAttr_HasMetadata = 0x10
    };
    enum VisitResourceMask
    {
        ResVisit_NestedMovies = 0x8000,
        ResVisit_Fonts = 0x1,
        ResVisit_Bitmaps = 0x2,
        ResVisit_GradientImages = 0x4,
        ResVisit_EditTextFields = 0x8,
        ResVisit_Sounds = 0x10,
        ResVisit_Sprite = 0x20,
        ResVisit_AllLocalImages = 0x6,
        ResVisit_AllImages = 0x8006
    };
    class ImportVisitor;
    class MemoryContext;
    class MemoryParams;
    class ResourceVisitor;
    class ImportVisitor
    {
    public:

        virtual ~ImportVisitor() {} // vt[0]
        virtual void Visit(GFxMovieDef* a0, GFxMovieDef* a1, const char* a2) = 0; // vt[1]
    };
    class MemoryContext : public GRefCountBase<GFxMovieDef::MemoryContext, 2>
    {
    public:

        virtual ~MemoryContext() {} // vt[0]
    };
    class MemoryParams
    {
    public:
        GMemoryHeap::HeapDesc Desc;
        float HeapLimitMultiplier;
        unsigned int MaxCollectionRoots;
        unsigned int FramesBetweenCollections;
    };
    class ResourceVisitor : public GFxFileConstants
    {
    public:

        virtual ~ResourceVisitor() {} // vt[0]
        virtual void Visit(GFxMovieDef* a0, GFxResource* a1, GFxResourceId a2, const char* a3) = 0; // vt[1]
    };

    virtual ~GFxMovieDef() {} // vt[0]
    virtual unsigned int GetVersion() const = 0; // vt[4]
    virtual unsigned int GetLoadingFrame() const = 0; // vt[5]
    virtual float GetWidth() const = 0; // vt[6]
    virtual float GetHeight() const = 0; // vt[7]
    virtual unsigned int GetFrameCount() const = 0; // vt[8]
    virtual float GetFrameRate() const = 0; // vt[9]
    virtual GRect<float> GetFrameRect() const = 0; // vt[10]
    virtual unsigned int GetSWFFlags() const = 0; // vt[11]
    virtual const char* GetFileURL() const = 0; // vt[12]
    virtual void WaitForLoadFinish(bool a0) const = 0; // vt[13]
    virtual void WaitForFrame(unsigned int a0) const = 0; // vt[14]
    virtual unsigned int GetFileAttributesW() const = 0; // vt[15]
    virtual unsigned int GetMetadata(char* a0, unsigned int a1) const = 0; // vt[16]
    virtual GMemoryHeap* GetLoadDataHeap() const = 0; // vt[17]
    virtual GMemoryHeap* GetBindDataHeap() const = 0; // vt[18]
    virtual GMemoryHeap* GetImageHeap() const = 0; // vt[19]
    virtual GFxResource* GetMovieDataResource() const = 0; // vt[20]
    virtual const GFxExporterInfo* GetExporterInfo() const = 0; // vt[21]
    virtual GFxMovieDef::MemoryContext* CreateMemoryContext(const char* a0, const GFxMovieDef::MemoryParams& a1, bool a2) = 0; // vt[22]
    virtual GFxMovieView* CreateInstance(const GFxMovieDef::MemoryParams& a0, bool a1) = 0; // vt[24]
    virtual GFxMovieView* CreateInstance(GFxMovieDef::MemoryContext* a0, bool a1) = 0; // vt[23]
    virtual void VisitImportedMovies(GFxMovieDef::ImportVisitor* a0) = 0; // vt[25]
    virtual void VisitResources(GFxMovieDef::ResourceVisitor* a0, unsigned int a1) = 0; // vt[26]
    virtual GFxResource* GetResource(const char* a0) const = 0; // vt[27]
};

class GFxImportVisitor : public GFxState, public GFxMovieDef::ImportVisitor, public GFxFileConstants
{
public:

    virtual ~GFxImportVisitor() {} // vt[0]
};

class GFxJpegSupportBase : public GFxState
{
public:

    virtual ~GFxJpegSupportBase() {} // vt[0]
    virtual GJPEGInput* CreateSwfJpeg2HeaderOnly(GFile* a0) = 0; // vt[1]
    virtual GImage* ReadJpeg(GFile* a0, GMemoryHeap* a1) = 0; // vt[2]
    virtual GImage* ReadSwfJpeg2(GFile* a0, GMemoryHeap* a1) = 0; // vt[3]
    virtual GImage* ReadSwfJpeg2WithTables(GJPEGInput* a0, GMemoryHeap* a1) = 0; // vt[4]
    virtual GImage* ReadSwfJpeg3(GFile* a0, GMemoryHeap* a1) = 0; // vt[5]
};

class GJPEGSystem : public GRefCountBase<GJPEGSystem, 2>
{
public:

    virtual ~GJPEGSystem() {} // vt[0]
    virtual GJPEGInput* CreateInput(GFile* a0) = 0; // vt[1]
    virtual GJPEGInput* CreateSwfJpeg2HeaderOnly(GFile* a0) = 0; // vt[3]
    virtual GJPEGInput* CreateSwfJpeg2HeaderOnly(const unsigned char* a0, unsigned int a1) = 0; // vt[2]
    virtual GJPEGOutput* CreateOutput(GFile* a0, int a1, int a2, int a3) = 0; // vt[5]
    virtual GJPEGOutput* CreateOutput(GFile* a0) = 0; // vt[4]
};

class GFxJpegSupport : public GFxJpegSupportBase
{
public:
    GPtr<GJPEGSystem> pSystem;

    virtual ~GFxJpegSupport() {} // vt[0]
};

class GFxSpecialKeysState
{
public:
    enum
    {
        Key_ShiftPressed = 0x1,
        Key_CtrlPressed = 0x2,
        Key_AltPressed = 0x4,
        Key_CapsToggled = 0x8,
        Key_NumToggled = 0x10,
        Key_ScrollToggled = 0x20,
        Initialized_Bit = 0x80,
        Initialized_Mask = 0xFF
    };
    unsigned char States;
};

class GFxKeyEvent : public GFxEvent
{
public:
    GFxKey::Code KeyCode;
    unsigned char AsciiCode;
    GUByte _pad9[3];
    unsigned int WcharCode;
    GFxSpecialKeysState SpecialKeysState;
    unsigned char KeyboardIndex;
    GUByte _pad18[2];
};

class GFxZlibSupportBase : public GFxState
{
public:

    virtual ~GFxZlibSupportBase() {} // vt[0]
    virtual GFile* CreateZlibFile(GFile* a0) = 0; // vt[1]
    virtual void InflateWrapper(GFxStream* a0, void* a1, int a2) = 0; // vt[2]
};

class GFxLoader : public GFxStateBag
{
public:
    enum LoadConstants
    {
        LoadAll = 0x0,
        LoadWaitCompletion = 0x1,
        LoadWaitFrame1 = 0x2,
        LoadOrdered = 0x10,
        LoadThreadedBinding = 0x20,
        LoadOnThread = 0x40,
        LoadKeepBindData = 0x80,
        LoadImageFiles = 0x10000,
        LoadDisableSWF = 0x80000,
        LoadDisableImports = 0x100000,
        LoadQuietOpen = 0x200000,
        LoadDebugHeap = 0x10000000
    };
    class LoaderConfig;
    class LoaderConfig
    {
    public:
        unsigned int DefLoadFlags;
        GPtr<GFxFileOpenerBase> pFileOpener;
        GPtr<GFxZlibSupportBase> pZLibSupport;
        GPtr<GFxJpegSupportBase> pJpegSupport;
    };
    GFxLoaderImpl* pImpl;
    GFxResourceLib* pStrongResourceLib;
    unsigned int DefLoadFlags;

    virtual ~GFxLoader() {} // vt[1]
    virtual bool CheckTagLoader(int a0) const; // vt[5]

    // DISHONORED(port): the non-virtual API GFxLoader.h declares and the generator does not emit,
    // because it emits virtuals only - agent BC named this as the one thing between the runtime and the
    // engine (agentBC.md 6.6). The state-bag slots below override GFxStateBag's and add no vtable slot,
    // so sizeof(GFxLoader) is still the PDB's 16 and GFx3Layout.cpp is unchanged. Bodies in
    // GFxLoaderImpl.cpp. THE GENERATOR (build/agentBB_gen_gfx3.py) MUST BE TAUGHT THESE SIX LINES
    // BEFORE IT IS RE-RUN, or a regeneration silently deletes the engine's entry points.
    GFxLoader();                                                             // 2013 0x9b3d40
    void Shutdown();
    bool GetMovieInfo(const char* url, GFxMovieInfo* info, bool getTagCount = false,
                      unsigned int loadFlags = 0);                           // 2013 0x9b3fe0
    GFxMovieDef* CreateMovie(const char* url, unsigned int loadFlags = 0,
                             unsigned int memoryArena = 0);                  // 2013 0x9b4030
    virtual void SetState(GFxState::StateType t, GFxState* s);
    virtual GFxState* GetStateAddRef(GFxState::StateType t) const;
    virtual void GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                 unsigned int n) const;
};

class GFxLoaderImpl;

class GFxLogConstants
{
public:
    enum LogMessageType
    {
        Log_Channel_General = 0x10,
        Log_Channel_Script = 0x20,
        Log_Channel_Parse = 0x30,
        Log_Channel_Action = 0x40,
        Log_Channel_Debug = 0x50,
        Log_Channel_Mask = 0xF0,
        Log_MessageType_Error = 0x0,
        Log_MessageType_Warning = 0x1,
        Log_MessageType_Message = 0x2,
        Log_Error = 0x10,
        Log_Warning = 0x11,
        Log_Message = 0x12,
        Log_ScriptError = 0x20,
        Log_ScriptWarning = 0x21,
        Log_ScriptMessage = 0x22,
        Log_Parse = 0x30,
        Log_ParseShape = 0x31,
        Log_ParseMorphShape = 0x32,
        Log_ParseAction = 0x33,
        Log_Action = 0x40
    };
};

class GFxLog;

// GFxLogBase<T>: PDB sizeof 4 (its own vptr), base GFxLogConstants @4 (empty), two slots.
template<class T>
class GFxLogBase : public GFxLogConstants
{
public:
    virtual ~GFxLogBase() {}                       // vt[0]
    virtual bool IsVerboseActionErrors() const;    // vt[1]
};

class GFxLog : public GFxState, public GFxLogBase<GFxLog>
{
public:

    virtual ~GFxLog() {} // vt[0]
    virtual void LogMessageVarg(GFxLogConstants::LogMessageType a0, const char* a1, char* a2); // vt[1]
};

class GFxMeshCache;

class GFxMeshCacheManager : public GFxState
{
public:
    GMemoryHeap* pHeap;
    GFxRenderGen* pRenderGen;
    GFxMeshCache* pMeshCache;
    GRendererEventHandler* pRenEventHandler;

    virtual ~GFxMeshCacheManager() {} // vt[0]
};

class GFxMouseCursorEvent : public GFxEvent
{
public:
    enum CursorShapeType
    {
        ARROW = 0x0,
        HAND = 0x1,
        IBEAM = 0x2
    };
    unsigned int CursorShape;
    unsigned int MouseIndex;
};

class GFxMouseEvent : public GFxEvent
{
public:
    float x;
    float y;
    float ScrollDelta;
    unsigned int Button;
    unsigned int MouseIndex;
};

class GFxMovie : public GRefCountBase<GFxMovie, 326>
{
public:
    enum PlayState
    {
        Playing = 0x0,
        Stopped = 0x1
    };
    enum SetArrayType
    {
        SA_Int = 0x0,
        SA_Double = 0x1,
        SA_Float = 0x2,
        SA_String = 0x3,
        SA_StringW = 0x4,
        SA_Value = 0x5
    };
    enum SetVarType
    {
        SV_Normal = 0x0,
        SV_Sticky = 0x1,
        SV_Permanent = 0x2
    };

    virtual ~GFxMovie() {} // vt[0]
    virtual GFxMovieDef* GetMovieDef() const = 0; // vt[1]
    virtual unsigned int GetCurrentFrame() const = 0; // vt[2]
    virtual bool HasLooped() const = 0; // vt[3]
    virtual void GotoFrame(unsigned int a0) = 0; // vt[4]
    virtual bool GotoLabeledFrame(const char* a0, int a1) = 0; // vt[5]
    virtual void SetPlayState(GFxMovie::PlayState a0) = 0; // vt[6]
    virtual GFxMovie::PlayState GetPlayState() const = 0; // vt[7]
    virtual void SetVisible(bool a0) = 0; // vt[8]
    virtual bool GetVisible() const = 0; // vt[9]
    virtual bool IsAvailable(const char* a0) const = 0; // vt[10]
    virtual void CreateString(GFxValue* a0, const char* a1) = 0; // vt[11]
    virtual void CreateStringW(GFxValue* a0, const wchar_t* a1) = 0; // vt[12]
    virtual void CreateObject(GFxValue* a0, const char* a1, const GFxValue* a2, unsigned int a3) = 0; // vt[13]
    virtual void CreateArray(GFxValue* a0) = 0; // vt[14]
    virtual void CreateFunction(GFxValue* a0, GFxFunctionHandler* a1, void* a2) = 0; // vt[15]
    virtual bool SetVariable(const char* a0, const GFxValue& a1, GFxMovie::SetVarType a2) = 0; // vt[16]
    virtual bool GetVariable(GFxValue* a0, const char* a1) const = 0; // vt[17]
    virtual bool SetVariableArray(GFxMovie::SetArrayType a0, const char* a1, unsigned int a2, const void* a3, unsigned int a4, GFxMovie::SetVarType a5) = 0; // vt[18]
    virtual bool SetVariableArraySize(const char* a0, unsigned int a1, GFxMovie::SetVarType a2) = 0; // vt[19]
    virtual unsigned int GetVariableArraySize(const char* a0) = 0; // vt[20]
    virtual bool GetVariableArray(GFxMovie::SetArrayType a0, const char* a1, unsigned int a2, void* a3, unsigned int a4) = 0; // vt[21]
    virtual bool Invoke(const char* a0, GFxValue* a1, const GFxValue* a2, unsigned int a3) = 0; // vt[23]
    virtual bool Invoke(const char* a0, GFxValue* a1, const char* a2, ...) = 0; // vt[22]
    virtual bool InvokeArgs(const char* a0, GFxValue* a1, const char* a2, char* a3) = 0; // vt[24]
};

class GFxMovieInfo
{
public:
    enum SWFFlagConstants
    {
        SWF_Compressed = 0x1,
        SWF_Stripped = 0x10
    };
    unsigned int Version;
    unsigned int Flags;
    int Width;
    int Height;
    float FPS;
    unsigned int FrameCount;
    unsigned int TagCount;
    unsigned short ExporterVersion;
    GUByte _pad30[2];
    unsigned long ExporterFlags;
};

class GFxMovieRoot;

class GFxMovieView : public GFxMovie, public GFxStateBag
{
public:
    enum AlignType
    {
        Align_Center = 0x0,
        Align_TopCenter = 0x1,
        Align_BottomCenter = 0x2,
        Align_CenterLeft = 0x3,
        Align_CenterRight = 0x4,
        Align_TopLeft = 0x5,
        Align_TopRight = 0x6,
        Align_BottomLeft = 0x7,
        Align_BottomRight = 0x8
    };
    enum HE_ReturnValueType
    {
        HE_NotHandled = 0x0,
        HE_Handled = 0x1,
        HE_NoDefaultAction = 0x2,
        HE_Completed = 0x3
    };
    enum HitTestType
    {
        HitTest_Bounds = 0x0,
        HitTest_Shapes = 0x1,
        HitTest_ButtonEvents = 0x2,
        HitTest_ShapesNoInvisible = 0x3
    };
    enum ScaleModeType
    {
        SM_NoScale = 0x0,
        SM_ShowAll = 0x1,
        SM_ExactFit = 0x2,
        SM_NoBorder = 0x3
    };

    virtual ~GFxMovieView() {} // vt[0]
    virtual void SetViewport(const GViewport& a0) = 0; // vt[25]
    virtual void GetViewport(GViewport* a0) const = 0; // vt[26]
    virtual void SetViewScaleMode(GFxMovieView::ScaleModeType a0) = 0; // vt[27]
    virtual GFxMovieView::ScaleModeType GetViewScaleMode() const = 0; // vt[28]
    virtual void SetViewAlignment(GFxMovieView::AlignType a0) = 0; // vt[29]
    virtual GFxMovieView::AlignType GetViewAlignment() const = 0; // vt[30]
    virtual GRect<float> GetVisibleFrameRect() const = 0; // vt[31]
    virtual void SetPerspective3D(const GMatrix3D& a0) = 0; // vt[32]
    virtual void SetView3D(const GMatrix3D& a0) = 0; // vt[33]
    virtual GRect<float> GetSafeRect() const = 0; // vt[34]
    virtual void SetSafeRect(const GRect<float>& a0) = 0; // vt[35]
    virtual void Restart() = 0; // vt[36]
    virtual float Advance(float a0, unsigned int a1) = 0; // vt[37]
    virtual void Display() = 0; // vt[38]
    virtual void DisplayPrePass() = 0; // vt[39]
    virtual void SetPause(bool a0) = 0; // vt[40]
    virtual bool IsPaused() const = 0; // vt[41]
    virtual void SetBackgroundColor(const GColor a0) = 0; // vt[42]
    virtual void SetBackgroundAlpha(float a0) = 0; // vt[43]
    virtual float GetBackgroundAlpha() const = 0; // vt[44]
    virtual unsigned int HandleEvent(const GFxEvent& a0) = 0; // vt[45]
    virtual void GetMouseState(unsigned int a0, float* a1, float* a2, unsigned int* a3) = 0; // vt[46]
    virtual void NotifyMouseState(float a0, float a1, unsigned int a2, unsigned int a3) = 0; // vt[47]
    virtual bool HitTest(float a0, float a1, GFxMovieView::HitTestType a2, unsigned int a3) = 0; // vt[48]
    virtual bool HitTest3D(GPoint3<float>* a0, float a1, float a2, unsigned int a3) = 0; // vt[49]
    virtual void SetExternalInterfaceRetVal(const GFxValue& a0) = 0; // vt[50]
    virtual void* GetUserData() const = 0; // vt[51]
    virtual void SetUserData(void* a0) = 0; // vt[52]
    virtual bool AttachDisplayCallback(const char* a0, void* /* void(void*)* */ a1, void* a2) = 0; // vt[53]
    virtual bool IsMovieFocused() const = 0; // vt[54]
    virtual bool GetDirtyFlag(bool a0) = 0; // vt[55]
    virtual void SetMouseCursorCount(unsigned int a0) = 0; // vt[56]
    virtual unsigned int GetMouseCursorCount() const = 0; // vt[57]
    virtual void SetControllerCount(unsigned int a0) = 0; // vt[58]
    virtual unsigned int GetControllerCount() const = 0; // vt[59]
    virtual void GetStats(GStatBag* a0, bool a1) = 0; // vt[60]
    virtual GMemoryHeap* GetHeap() const = 0; // vt[61]
    virtual void ForceCollectGarbage() = 0; // vt[62]
    virtual GPoint<float> TranslateToScreen(const GPoint<float>& a0, GMatrix2D a1) = 0; // vt[64]
    virtual GRect<float> TranslateToScreen(const GRect<float>& a0, GMatrix2D a1) = 0; // vt[63]
    virtual bool TranslateLocalToScreen(const char* a0, const GPoint<float>& a1, GPoint<float>* a2, GMatrix2D a3) = 0; // vt[65]
    virtual bool SetControllerFocusGroup(unsigned int a0, unsigned int a1) = 0; // vt[66]
    virtual unsigned int GetControllerFocusGroup(unsigned int a0) const = 0; // vt[67]
    virtual GFxMovieDef::MemoryContext* GetMemoryContext() const = 0; // vt[68]
    virtual void Release() = 0; // vt[69]
};

class GFxPNGSupportBase : public GFxState
{
public:

    virtual ~GFxPNGSupportBase() {} // vt[0]
    virtual GImage* CreateImage(GFile* a0, GMemoryHeap* a1) = 0; // vt[1]
};

class GFxParseControl : public GFxState
{
public:
    enum VerboseParseConstants
    {
        VerboseParseNone = 0x0,
        VerboseParse = 0x1,
        VerboseParseAction = 0x2,
        VerboseParseShape = 0x10,
        VerboseParseMorphShape = 0x20,
        VerboseParseAllShapes = 0x30,
        VerboseParseAll = 0x33
    };
    unsigned int ParseFlags;

    virtual ~GFxParseControl() {} // vt[0]
};

class GFxPlayerLog : public GFxLog
{
public:

    virtual ~GFxPlayerLog() {} // vt[0]
};

class GFxProgressHandler : public GFxState
{
public:
    class Info;
    class TagInfo;
    class Info
    {
    public:
        GString FileUrl;
        unsigned int BytesLoaded;
        unsigned int TotalBytes;
        unsigned int FrameLoading;
        unsigned int TotalFrames;
    };
    class TagInfo
    {
    public:
        GString FileUrl;
        int TagType;
        int TagOffset;
        int TagLength;
        int TagDataOffset;
    };

    virtual ~GFxProgressHandler() {} // vt[0]
    virtual void ProgressUpdate(const GFxProgressHandler::Info& a0) = 0; // vt[1]
    virtual void LoadTagUpdate(const GFxProgressHandler::TagInfo& a0, bool a1); // vt[2]
};

class GRendererEventHandler : public GListNode<GRendererEventHandler>, public GNewOverrideBase<2>
{
public:
    enum EventType
    {
        Event_EndFrame = 0x0,
        Event_RendererReleased = 0x1
    };
    GRenderer* pRenderer;

    virtual ~GRendererEventHandler() {} // vt[0]
    virtual void OnEvent(GRenderer* a0, GRendererEventHandler::EventType a1); // vt[1]
};

class GRenderer : public GRefCountBase<GRenderer, 65>
{
public:
    enum BitmapSampleMode
    {
        Sample_Point = 0x0,
        Sample_Linear = 0x1
    };
    enum BitmapWrapMode
    {
        Wrap_Repeat = 0x0,
        Wrap_Clamp = 0x1
    };
    enum BlendType
    {
        Blend_None = 0x0,
        Blend_Normal = 0x1,
        Blend_Layer = 0x2,
        Blend_Multiply = 0x3,
        Blend_Screen = 0x4,
        Blend_Lighten = 0x5,
        Blend_Darken = 0x6,
        Blend_Difference = 0x7,
        Blend_Add = 0x8,
        Blend_Subtract = 0x9,
        Blend_Invert = 0xA,
        Blend_Alpha = 0xB,
        Blend_Erase = 0xC,
        Blend_Overlay = 0xD,
        Blend_HardLight = 0xE
    };
    enum CachedDataType
    {
        Cached_Vertex = 0x1,
        Cached_Index = 0x2,
        Cached_BitmapList = 0x3
    };
    enum FilterModes
    {
        Filter_Blur = 0x1,
        Filter_Shadow = 0x2,
        Filter_Highlight = 0x4,
        Filter_Knockout = 0x100,
        Filter_Inner = 0x200,
        Filter_HideObject = 0x400,
        Filter_UserModes = 0xFFFF,
        Filter_SkipLastPass = 0x10000,
        Filter_LastPassOnly = 0x20000
    };
    enum FilterSupport
    {
        FilterSupport_None = 0x0,
        FilterSupport_Ok = 0x1,
        FilterSupport_Multipass = 0x2,
        FilterSupport_Slow = 0x4
    };
    enum GouraudFillType
    {
        GFill_Color = 0x0,
        GFill_1Texture = 0x1,
        GFill_1TextureColor = 0x2,
        GFill_2Texture = 0x3,
        GFill_2TextureColor = 0x4,
        GFill_3Texture = 0x5
    };
    enum IndexFormat
    {
        Index_None = 0x0,
        Index_16 = 0x1,
        Index_32 = 0x2
    };
    enum RenderCapBits
    {
        Cap_CacheDataUse = 0x1,
        Cap_Index16 = 0x4,
        Cap_Index32 = 0x8,
        Cap_RenderStats = 0x10,
        Cap_FillGouraud = 0x100,
        Cap_FillGouraudTex = 0x200,
        Cap_CxformAdd = 0x1000,
        Cap_NestedMasks = 0x2000,
        Cap_TexNonPower2 = 0x4000,
        Cap_TexNonPower2Wrap = 0x8000,
        Cap_TexNonPower2Mip = 0x80000,
        Cap_CanLoseData = 0x10000,
        Cap_KeepVertexData = 0x20000,
        Cap_NoTexOverwrite = 0x40000,
        Cap_ThreadedTextureCreation = 0x100000,
        Cap_RenderTargets = 0x20,
        Cap_RenderTargetPrePass = 0x40,
        Cap_RenderTargetNonPow2 = 0x80,
        Cap_RenderTargetMip = 0x200000,
        Cap_Filter_Blurs = 0x400000,
        Cap_Filter_ColorMatrix = 0x800000
    };
    enum ResizeImageType
    {
        ResizeRgbToRgb = 0x0,
        ResizeRgbaToRgba = 0x1,
        ResizeRgbToRgba = 0x2,
        ResizeGray = 0x3
    };
    enum StereoDisplay
    {
        StereoCenter = 0x0,
        StereoLeft = 0x1,
        StereoRight = 0x2
    };
    enum SubmitMaskMode
    {
        Mask_Clear = 0x0,
        Mask_Increment = 0x1,
        Mask_Decrement = 0x2
    };
    enum UserDataPropertyFlag
    {
        UD_None = 0x0,
        UD_HasString = 0x1,
        UD_HasFloat = 0x2,
        UD_HasMatrix = 0x3
    };
    enum VertexFormat
    {
        Vertex_None = 0x0,
        Vertex_XY16i = 0x1,
        Vertex_XY32f = 0x2,
        Vertex_XY16iC32 = 0x3,
        Vertex_XY16iCF32 = 0x4
    };
    class BitmapDesc;
    class Cxform;
    class BlurFilterParams;
    class CacheProvider;
    class CachedData;
    class DistanceFieldParams;
    class FillTexture;
    class RenderCaps;
    class Stats;
    class StereoParams;
    class UserData;
    class VertexXY16i;
    class VertexXY16iC32;
    class VertexXY16iCF32;
    class BitmapDesc
    {
    public:
        GRect<float> Coords;
        GRect<float> TextureCoords;
        GColor Color;
    };
    class Cxform
    {
    public:
        // DISHONORED(layout): channel-major - M_[channel][0] multiply, M_[channel][1] add, channels
        // R G B A. The PDB spells this [2][4] because DIA reports the innermost extent first (see
        // GMatrix2D in GTypes.h); the source order is the one retail's renderer indexes,
        // M[Channel * 2 + 0/1] in FGFxCxformSetIdentity.
        float M_[4][2];
    };
    class BlurFilterParams
    {
    public:
        unsigned int Mode;
        float BlurX;
        float BlurY;
        unsigned int Passes;
        GPoint<float> Offset;
        GColor Color;
        GColor Color2;
        float Strength;
        GRenderer::Cxform cxform;
    };
    class CacheProvider
    {
    public:
        GRenderer::CachedData* pData;
        bool DiscardSharedData;
        GUByte _pad5[3];
    };
    class CachedData
    {
    public:
        GRenderer* pRenderer;
        void* hData;
    };
    class DistanceFieldParams
    {
    public:
        float Width;
        float ShadowWidth;
        GColor ShadowColor;
        GPoint<float> ShadowOffset;
        GColor GlowColor;
        float GlowSize[4];
    };
    class FillTexture
    {
    public:
        GTexture* pTexture;
        GMatrix2D TextureMatrix;
        GRenderer::BitmapWrapMode WrapMode;
        GRenderer::BitmapSampleMode SampleMode;
    };
    class RenderCaps
    {
    public:
        unsigned long CapBits;
        unsigned long VertexFormats;
        unsigned long BlendModes;
        unsigned long MaxTextureSize;
    };
    class Stats
    {
    public:
        unsigned int Triangles;
        unsigned int Lines;
        unsigned int Primitives;
        unsigned int Masks;
        unsigned int Filters;
    };
    class StereoParams
    {
    public:
        float DisplayWidthCm;
        float Distortion;
        float DisplayDiagInches;
        float DisplayAspectRatio;
        float EyeSeparationCm;
    };
    class UserData
    {
    public:
        const char* pString;
        float* pFloat;
        float* pMatrix;
        unsigned int MatrixSize;
        unsigned char PropFlags;
        GUByte _pad17[3];
    };
    class VertexXY16i
    {
    public:
        short x;
        short y;
    };
    class VertexXY16iC32
    {
    public:
        enum
        {
            VFormat = 0x3
        };
        short x;
        short y;
        unsigned long Color;
    };
    class VertexXY16iCF32
    {
    public:
        enum
        {
            VFormat = 0x4
        };
        short x;
        short y;
        unsigned long Color;
        unsigned long Factors;
    };
    bool bForceDisableFilters;
    GUByte _pad9[3];
    GList<GRendererEventHandler> Handlers;
    GRenderer::StereoParams S3DParams;
    GRenderer::StereoDisplay S3DDisplay;

    virtual ~GRenderer() {} // vt[0]
    virtual void ScopedEventCallback(const char* a0); // vt[1]
    virtual void SaveCurrentRenderTargetContents(); // vt[2]
    virtual void RestoreCurrentRenderTargetContents(); // vt[3]
    virtual bool GetRenderCaps(GRenderer::RenderCaps* a0) = 0; // vt[4]
    virtual GTexture* CreateTexture() = 0; // vt[5]
    virtual GTexture* CreateTextureYUV() = 0; // vt[6]
    virtual void BeginFrame(); // vt[7]
    virtual void EndFrame(); // vt[8]
    virtual GRenderTarget* CreateRenderTarget() = 0; // vt[9]
    virtual void SetDisplayRenderTarget(GRenderTarget* a0, bool a1) = 0; // vt[10]
    virtual void PushRenderTarget(const GRect<float>& a0, GRenderTarget* a1) = 0; // vt[11]
    virtual void PopRenderTarget() = 0; // vt[12]
    virtual GTexture* PushTempRenderTarget(const GRect<float>& a0, unsigned int a1, unsigned int a2, bool a3) = 0; // vt[13]
    virtual void ReleaseTempRenderTargets(unsigned int a0); // vt[14]
    virtual void BeginDisplay(GColor a0, const GViewport& a1, float a2, float a3, float a4, float a5) = 0; // vt[15]
    virtual void EndDisplay() = 0; // vt[16]
    virtual void SetMatrix(const GMatrix2D& a0) = 0; // vt[17]
    virtual void SetUserMatrix(const GMatrix2D& a0) = 0; // vt[18]
    virtual void SetCxform(const GRenderer::Cxform& a0) = 0; // vt[19]
    virtual void PushBlendMode(GRenderer::BlendType a0) = 0; // vt[20]
    virtual void PopBlendMode() = 0; // vt[21]
    virtual bool PushUserData(GRenderer::UserData* a0); // vt[22]
    virtual void PopUserData(); // vt[23]
    virtual void SetPerspective3D(const GMatrix3D& a0) = 0; // vt[24]
    virtual void SetView3D(const GMatrix3D& a0) = 0; // vt[25]
    virtual void SetWorld3D(const GMatrix3D* a0) = 0; // vt[26]
    virtual void MakeViewAndPersp3D(const GRect<float>& a0, GMatrix3D& a1, GMatrix3D& a2, float a3, bool a4); // vt[27]
    virtual void SetStereoParams(GRenderer::StereoParams a0); // vt[28]
    virtual void SetStereoDisplay(GRenderer::StereoDisplay a0, bool a1); // vt[29]
    virtual void SetVertexData(const void* a0, int a1, GRenderer::VertexFormat a2, GRenderer::CacheProvider* a3) = 0; // vt[30]
    virtual void SetIndexData(const void* a0, int a1, GRenderer::IndexFormat a2, GRenderer::CacheProvider* a3) = 0; // vt[31]
    virtual void ReleaseCachedData(GRenderer::CachedData* a0, GRenderer::CachedDataType a1) = 0; // vt[32]
    virtual void DrawIndexedTriList(int a0, int a1, int a2, int a3, int a4) = 0; // vt[33]
    virtual void DrawLineStrip(int a0, int a1) = 0; // vt[34]
    virtual void LineStyleDisable() = 0; // vt[35]
    virtual void LineStyleColor(GColor a0) = 0; // vt[36]
    virtual void FillStyleDisable() = 0; // vt[37]
    virtual void FillStyleColor(GColor a0) = 0; // vt[38]
    virtual void FillStyleBitmap(const GRenderer::FillTexture* a0) = 0; // vt[39]
    virtual void FillStyleGouraud(GRenderer::GouraudFillType a0, const GRenderer::FillTexture* a1, const GRenderer::FillTexture* a2, const GRenderer::FillTexture* a3) = 0; // vt[40]
    virtual void DrawBitmaps(GRenderer::BitmapDesc* a0, int a1, int a2, int a3, const GTexture* a4, const GMatrix2D& a5, GRenderer::CacheProvider* a6) = 0; // vt[41]
    virtual void DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* a0, int a1, int a2, int a3, const GTexture* a4, const GMatrix2D& a5, const GRenderer::DistanceFieldParams& a6, GRenderer::CacheProvider* a7); // vt[42]
    virtual void BeginSubmitMask(GRenderer::SubmitMaskMode a0) = 0; // vt[43]
    virtual void EndSubmitMask() = 0; // vt[44]
    virtual void DisableMask() = 0; // vt[45]
    virtual unsigned int CheckFilterSupport(const GRenderer::BlurFilterParams& a0) = 0; // vt[46]
    virtual void DrawBlurRect(GTexture* a0, const GRect<float>& a1, const GRect<float>& a2, const GRenderer::BlurFilterParams& a3, bool a4) = 0; // vt[47]
    virtual void DrawColorMatrixRect(GTexture* a0, const GRect<float>& a1, const GRect<float>& a2, const float* a3, bool a4) = 0; // vt[48]
    virtual void GetRenderStats(GRenderer::Stats* a0, bool a1) = 0; // vt[49]
    virtual void GetStats(GStatBag* a0, bool a1) = 0; // vt[50]
    virtual void ReleaseResources() = 0; // vt[51]
    virtual bool AddEventHandler(GRendererEventHandler* a0); // vt[52]
    virtual void RemoveEventHandler(GRendererEventHandler* a0); // vt[53]
};

class GFxRenderConfig : public GFxState
{
public:
    enum RenderFlagType
    {
        RF_StrokeCorrect = 0x0,
        RF_StrokeNormal = 0x1,
        RF_StrokeHairline = 0x2,
        RF_StrokeMask = 0x3,
        RF_EdgeAA = 0x10,
        RF_OptimizeTriangles = 0x20,
        RF_NoViewCull = 0x100
    };
    GPtr<GRenderer> pRenderer;
    float MaxCurvePixelError;
    unsigned long RenderFlags;
    float StrokerAAWidth;
    unsigned long RendererCapBits;
    unsigned long RendererVtxFmts;

    virtual ~GFxRenderConfig() {} // vt[0]
};

class GFxRenderGen;

class GFxRenderStats : public GFxState
{
public:
    unsigned long TessTriangles;

    virtual ~GFxRenderStats() {} // vt[0]
};

class GFxResourceLibBase : public GRefCountBase<GFxResourceLibBase, 2>
{
public:

    virtual ~GFxResourceLibBase() {} // vt[0]
    virtual void RemoveResourceOnRelease(GFxResource* a0) = 0; // vt[1]
    virtual void PinResource(GFxResource* a0) = 0; // vt[2]
    virtual void UnpinResource(GFxResource* a0) = 0; // vt[3]
};

class GFxResourceWeakLib : public GFxResourceLibBase
{
public:
    class ResourceNode;
    class ResourceNode
    {
    public:
        enum NodeType
        {
            Node_Resource = 0x0,
            Node_Resolver = 0x1
        };
        class HashOp;
        class HashOp
        {
        public:
        };
        GFxResourceWeakLib::ResourceNode::NodeType Type;
        GUByte pResolver_[4]; // GFxResourceLib::ResourceSlot*
        // union alias @4 GFxResource* pResource
    };
    GFxResourceLib* pStrongLib;
    GLock ResourceLock;
    GUByte Resources_[4]; // GHashSet<GFxResourceWeakLib::ResourceNode,GFxResourceWeakLib::ResourceNode::HashOp,GFxResourceWeakLib::ResourceNode::HashOp,GAllocatorGH<GFxResourceWeakLib::ResourceNode,2>,GHashsetCachedEntry<GFxResourceWeakLib::ResourceNode,GFxResourceWeakLib::ResourceNode::HashOp> >
    GPtr<GMemoryHeap> pImageHeap;

    virtual ~GFxResourceWeakLib() {} // vt[0]
};

class GFxResourceLib : public GRefCountBase<GFxResourceLib, 2>
{
public:
    enum ResolveState
    {
        RS_Unbound = 0x0,
        RS_Available = 0x1,
        RS_WaitingResolve = 0x2,
        RS_NeedsResolve = 0x3,
        RS_Error = 0x4
    };
    class BindHandle;
    class GFxResourcePtrHashFunc;
    class ResourceSlot;
    class BindHandle
    {
    public:
        GFxResourceLib::ResolveState State;
        GFxResource* pResource;
        // union alias @4 GFxResourceLib::ResourceSlot* pSlot
    };
    class GFxResourcePtrHashFunc
    {
    public:
    };
    class ResourceSlot : public GRefCountBase<GFxResourceLib::ResourceSlot, 2>
    {
    public:
        enum ResolveState
        {
            Resolve_InProgress = 0x0,
            Resolve_Success = 0x1,
            Resolve_Fail = 0x2
        };
        GPtr<GFxResourceWeakLib> pLib;
        GFxResourceLib::ResourceSlot::ResolveState State;
        GFxResource* pResource;
        GFxResourceKey Key;
        GString ErrorMessage;
        GUByte ResolveComplete_[44]; // GEvent

        virtual ~ResourceSlot() {} // vt[0]
    };
    GFxResourceWeakLib* pWeakLib;
    GUByte PinSet_[4]; // GHashSetUncached<GFxResource *,GFxResourceLib::GFxResourcePtrHashFunc,GFxResourceLib::GFxResourcePtrHashFunc,GAllocatorGH<GFxResource *,2> >
    bool DebugFlag;
    GUByte _pad17[3];

    virtual ~GFxResourceLib() {} // vt[0]
};

class GFxResourceReport
{
public:

    virtual ~GFxResourceReport() {} // vt[0]
    virtual GString GetResourceName() const; // vt[1]
    virtual GMemoryHeap* GetResourceHeap() const; // vt[2]
    virtual void GetStats(GStatBag* a0, bool a1); // vt[3]
};

class GFxSetFocusEvent : public GFxEvent
{
public:
    GFxSpecialKeysState SpecialKeysStates[4];
};

class GFxShapeBase;

class GFxSharedObjectManagerBase;

class GFxStream;

class GFxStyledText;

class GFxSubImageResource : public GFxImageResource
{
public:
    GRect<int> Rect;
    GFxResourceId BaseImageId;

    virtual ~GFxSubImageResource() {} // vt[0]
};

class GSystem
{
public:
};

class GFxSystem : public GSystem
{
public:
};

class GFxTaskManager;

class GFxWStringBuffer
{
public:
    class ReserveHeader;
    class ReserveHeader
    {
    public:
        wchar_t* pBuffer;
        unsigned int Size;
    };
    wchar_t* pText;
    unsigned int Length;
    GFxWStringBuffer::ReserveHeader Reserved;
};

class GFxTextClipboard : public GFxState
{
public:
    GFxWStringBuffer PlainText;
    GFxStyledText* pStyledText;

    virtual ~GFxTextClipboard() {} // vt[0]
    virtual void OnTextStore(const wchar_t* a0, unsigned int a1); // vt[1]
};

class GFxTextureGlyphData;

class GFxTranslator : public GFxState
{
public:
    enum TranslateCaps
    {
        Cap_ReceiveHtml = 0x1,
        Cap_StripTrailingNewLines = 0x2
    };
    enum WordWrappingTypes
    {
        WWT_Default = 0x0,
        WWT_Asian = 0x1,
        WWT_Prohibition = 0x2,
        WWT_NoHangulWrap = 0x4,
        WWT_Hyphenation = 0x8,
        WWT_Custom = 0x80
    };
    class LineFormatDesc;
    class TranslateInfo;
    class LineFormatDesc
    {
    public:
        enum
        {
            Align_Left = 0x0,
            Align_Right = 0x1,
            Align_Center = 0x2,
            Align_Justify = 0x3
        };
        const wchar_t* pParaText;
        unsigned int ParaTextLen;
        const float* pWidths;
        unsigned int LineStartPos;
        unsigned int NumCharsInLine;
        float VisibleRectWidth;
        float CurrentLineWidth;
        float LineWidthBeforeWordWrap;
        float DashSymbolWidth;
        unsigned char Alignment;
        GUByte _pad37[3];
        unsigned int ProposedWordWrapPoint;
        bool UseHyphenation;
        GUByte _pad45[3];
    };
    class TranslateInfo
    {
    public:
        enum
        {
            Flag_Translated = 0x1,
            Flag_ResultHtml = 0x2,
            Flag_SourceHtml = 0x4
        };
        const wchar_t* pKey;
        GFxWStringBuffer* pResult;
        const char* pInstanceName;
        unsigned char Flags;
        GUByte _pad13[3];
    };
    unsigned int WWMode;

    virtual ~GFxTranslator() {} // vt[0]
    virtual unsigned int GetCaps() const; // vt[1]
    virtual void Translate(GFxTranslator::TranslateInfo* a0); // vt[2]
    virtual bool OnWordWrapping(GFxTranslator::LineFormatDesc* a0); // vt[3]
};

class GFxUITranslator : public GFxTranslator
{
public:

    virtual ~GFxUITranslator() {} // vt[0]
};

class GFxURLBuilder : public GFxState
{
public:
    enum FileUse
    {
        File_Regular = 0x0,
        File_Import = 0x1,
        File_ImageImport = 0x2,
        File_LoadMovie = 0x3,
        File_LoadVars = 0x4,
        File_LoadXML = 0x5,
        File_LoadCSS = 0x6,
        File_Sound = 0x7
    };
    class LocationInfo;
    class LocationInfo
    {
    public:
        GFxURLBuilder::FileUse Use;
        GString FileName;
        GString ParentPath;
    };

    virtual ~GFxURLBuilder() {} // vt[0]
    virtual void BuildURL(GString* a0, const GFxURLBuilder::LocationInfo& a1); // vt[1]
};

class GFxUserEventHandler : public GFxState
{
public:

    virtual ~GFxUserEventHandler() {} // vt[0]
    virtual void HandleEvent(GFxMovieView* a0, const GFxEvent& a1) = 0; // vt[1]
};

class GFxXMLSupportBase;

class GFxZlibSupport : public GFxZlibSupportBase
{
public:

    virtual ~GFxZlibSupport() {} // vt[0]
};

class GHeapMemVisitor;

class GHeapSegVisitor;

class GImage : public GRefCountBaseNTS<GImage, 3>, public GImageBase
{
public:

    virtual ~GImage() {} // vt[0]
};

class GImageInfoBaseImpl : public GImageInfoBase
{
public:
    GPtr<GTexture> pTexture;
    unsigned int TextureUsage;

    virtual ~GImageInfoBaseImpl() {} // vt[0]
};

class GImageInfo : public GImageInfoBaseImpl
{
public:
    GPtr<GImage> pImage;
    unsigned int TargetWidth;
    unsigned int TargetHeight;
    bool ReleaseImage;
    GUByte _pad33[3];

    virtual ~GImageInfo() {} // vt[0]
};

class GJPEGInput : public GNewOverrideBase<2>
{
public:

    virtual ~GJPEGInput() {} // vt[0]
    virtual void DiscardPartialBuffer() = 0; // vt[1]
    virtual int StartImage() = 0; // vt[2]
    virtual int StartRawImage() = 0; // vt[3]
    virtual int FinishImage() = 0; // vt[4]
    virtual unsigned int GetHeight() const = 0; // vt[5]
    virtual unsigned int GetWidth() const = 0; // vt[6]
    virtual int ReadScanline(unsigned char* a0) = 0; // vt[7]
    virtual int ReadRawData(void** a0) = 0; // vt[8]
    virtual void* GetCInfo() = 0; // vt[9]
    virtual bool IsErrorOccurred() const = 0; // vt[10]
};

class GJPEGOutput : public GNewOverrideBase<2>
{
public:

    virtual ~GJPEGOutput() {} // vt[0]
    virtual void WriteScanline(unsigned char* a0) = 0; // vt[1]
    virtual void WriteRawData(const void* a0) = 0; // vt[2]
    virtual void CopyCriticalParams(void* a0) = 0; // vt[3]
    virtual void* GetCInfo() = 0; // vt[4]
};

class GMemory
{
public:
};

class GMemoryFile : public GFile
{
public:
    GString FilePath;
    const unsigned char* FileData;
    int FileSize;
    int FileIndex;
    bool Valid;
    GUByte _pad25[3];

    virtual ~GMemoryFile() {} // vt[0]
};

class GRenderTarget : public GNewOverrideBase<65>
{
public:
    GAtomicInt<long> RefCount;

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount == 0) delete this; }
    int GetRefCount() const { return (int)(long)RefCount; }

    virtual ~GRenderTarget() {} // vt[0]
    virtual bool InitRenderTarget(GTexture* a0, GTexture* a1, GTexture* a2) = 0; // vt[1]
    virtual GRenderer* GetRenderer() const = 0; // vt[2]
    virtual void* GetUserData() const = 0; // vt[3]
    virtual void SetUserData(void* a0) = 0; // vt[4]
    virtual void AddChangeHandler(GTexture::ChangeHandler* a0) = 0; // vt[5]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* a0) = 0; // vt[6]
};

class GRendererNode
{
public:
    GRendererNode* pPrev;
    // union alias @0 GRendererNode* pLast
    GRendererNode* pNext;
    // union alias @4 GRendererNode* pFirst
};

class GRenderTargetImplNode : public GRenderTarget, public GRendererNode
{
public:
    void* UserHandle;
    bool HandlerArrayFlag;
    GUByte _pad21[3];
    GTexture::ChangeHandler* pHandler;
    // union alias @24 GArray<GTexture::ChangeHandler *,2,GArrayDefaultPolicy>* pHandlerArray

    virtual ~GRenderTargetImplNode() {} // vt[0]
};

class GSubImageInfo : public GImageInfoBase
{
public:
    GPtr<GImageInfoBase> pBaseImage;
    GRect<int> Rect;

    virtual ~GSubImageInfo() {} // vt[0]
    virtual GImageInfoBase* GetBaseImage(); // vt[7]
};

class GSysAllocBase
{
public:

    virtual ~GSysAllocBase() {} // vt[0]
    virtual bool initHeapEngine(const void* a0); // vt[1]
    virtual void shutdownHeapEngine(); // vt[2]
};

class GSysAlloc : public GSysAllocBase
{
public:

    virtual ~GSysAlloc() {} // vt[0]
    virtual void* Alloc(unsigned int a0, unsigned int a1) = 0; // vt[3]
    virtual void Free(void* a0, unsigned int a1, unsigned int a2) = 0; // vt[4]
    virtual void* Realloc(void* a0, unsigned int a1, unsigned int a2, unsigned int a3) = 0; // vt[5]
};

class GSysAllocPaged : public GSysAllocBase
{
public:
    class Info;
    class Info
    {
    public:
        unsigned int MinAlign;
        unsigned int MaxAlign;
        unsigned int Granularity;
        unsigned int SysDirectThreshold;
        unsigned int MaxHeapGranularity;
        bool HasRealloc;
        GUByte _pad21[3];
    };

    virtual ~GSysAllocPaged() {} // vt[0]
    virtual void GetInfo(GSysAllocPaged::Info* a0) const = 0; // vt[3]
    virtual void* Alloc(unsigned int a0, unsigned int a1) = 0; // vt[4]
    virtual bool Free(void* a0, unsigned int a1, unsigned int a2) = 0; // vt[5]
    virtual bool ReallocInPlace(void* a0, unsigned int a1, unsigned int a2, unsigned int a3); // vt[6]
    virtual void* AllocSysDirect(unsigned int a0, unsigned int a1, unsigned int* a2, unsigned int* a3); // vt[7]
    virtual bool FreeSysDirect(void* a0, unsigned int a1, unsigned int a2); // vt[8]
    virtual unsigned int GetBase() const; // vt[9]
    virtual unsigned int GetSize() const; // vt[10]
    virtual unsigned int GetFootprint() const; // vt[11]
    virtual unsigned int GetUsedSpace() const; // vt[12]
    virtual void VisitMem(GHeapMemVisitor* a0) const; // vt[13]
    virtual void VisitSegments(GHeapSegVisitor* a0, unsigned int a1, unsigned int a2) const; // vt[14]
};

// GSysAllocBase_SingletonSupport<Allocator, Base>: PDB sizeof 8, member pContainer @4, and a
// nested SysAllocContainer whose size is the allocator's own (20 for FGFxAllocator over
// GSysAllocPaged, 12 for GSysAllocMalloc over GSysAlloc).
template<class D, class B>
class GSysAllocBase_SingletonSupport : public B
{
public:
    class SysAllocContainer;
    SysAllocContainer* pContainer;

    GSysAllocBase_SingletonSupport() : pContainer(0) {}
};

class GSysAllocMalloc : public GSysAllocBase_SingletonSupport<GSysAllocMalloc, GSysAlloc>
{
public:

    virtual ~GSysAllocMalloc() {} // vt[0]
};

class GSysFile : public GDelegatedFile
{
public:

    virtual ~GSysFile() {} // vt[0]
};

class GSysMemoryMap
{
public:

    virtual unsigned int GetPageSize() const = 0; // vt[0]
    virtual void* ReserveAddrSpace(unsigned int a0) = 0; // vt[1]
    virtual bool ReleaseAddrSpace(void* a0, unsigned int a1) = 0; // vt[2]
    virtual void* MapPages(void* a0, unsigned int a1) = 0; // vt[3]
    virtual bool UnmapPages(void* a0, unsigned int a1) = 0; // vt[4]
    virtual ~GSysMemoryMap() {} // vt[5]
};

class GTextureImplNode : public GTexture, public GRendererNode
{
public:
    void* UserHandle;
    bool HandlerArrayFlag;
    GUByte _pad21[3];
    GTexture::ChangeHandler* pHandler;
    // union alias @24 GArray<GTexture::ChangeHandler *,2,GArrayDefaultPolicy>* pHandlerArray

    virtual ~GTextureImplNode() {} // vt[0]
};


#pragma pack(pop)
#endif // INC_GFX3_GEN_H
