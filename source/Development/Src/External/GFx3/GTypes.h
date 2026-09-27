// Scaleform GFx 3.3.89 - kernel types, reconstructed from the 2012 Shipping PDB.
// DISHONORED(layout): 2013 rva 0xdf7374 (the "3.3.89" gfxVersion string in Dishonored.exe).
//
// There is no GFx SDK in this tree and none can be obtained: the runtime is 5,635 functions and
// 1.13 MiB inside the retail exe's own .text, with no symbol in any shipped DLL, and 5,235 of those
// functions (92.9 %) are byte-identical between the 2012 QA build and the retail 2013 build
// (resources/docs/gfx_decision.md 1). The 2012 PDB therefore names and lays out the retail runtime
// exactly, and every type in this directory comes out of it through
// resources/tools/pdb/dia_types.py. See resources/docs/agents/agentBB.md.
//
// This header holds the hand-written kernel: the value types, the reference-counting bases, the
// container templates and the string. Everything else is generated into GFx3Gen.h. Layout is
// asserted against the PDB in GFx3Layout.cpp.
//
// Packing: GFx was NOT compiled with UE3's /Zp4. GFxValue::DisplayInfo proves it - `bool Visible`
// sits at @48 and the next `double Z` at @56, i.e. doubles are 8-aligned and the struct is 232
// bytes. Every header here therefore pushes pack(8) so the layout is right whatever the including
// translation unit's /Zp is.
#ifndef INC_GFX3_GTYPES_H
#define INC_GFX3_GTYPES_H

#include <stddef.h>
#include <new>

#pragma pack(push, 8)

// ---------------------------------------------------------------------------------------------
// Scalar typedefs. Widths are the ones the PDB member types imply on Win32 x86.
// ---------------------------------------------------------------------------------------------
typedef signed char        GInt8;
typedef unsigned char      GUInt8;
typedef unsigned char      GUByte;
typedef signed char        GSByte;
typedef short              GInt16;
typedef unsigned short     GUInt16;
typedef int                GInt32;
typedef unsigned int       GUInt32;
typedef __int64            GInt64;
typedef unsigned __int64   GUInt64;
typedef unsigned int       GUInt;
typedef int                GInt;
typedef size_t             GUPInt;
typedef ptrdiff_t          GSPInt;
typedef unsigned int       UPInt;
typedef int                SPInt;

#define GFC_MAX_UINT32  0xFFFFFFFFu
#define GFC_MIN_INT32   (-2147483647 - 1)
#define GFC_MAX_INT32   2147483647

// GFx's own heap identifiers, used as the second template argument of GNewOverrideBase,
// GRefCountBase and the allocators. The values are the ones the PDB instantiates
// (GNewOverrideBase<2>, <3>, <65>, <195>, <322>, <326>, <513>).
// The stat ids the PDB instantiates the templates with. They are values of the generated GStatGroup
// / GStatRenderer / GFxStat* enums in GFx3Enums.h, which is included after this header, so they are
// spelled as literals here with their symbolic name in the comment:
//     2 GStat_Default_Mem, 3 GStat_Image_Mem, 65 (0x41) GStatRender_Mem,
//   195 (0xC3) GFxStatFC_GlyphCache_Mem, 322 (0x142) GFxStatMV_MovieClip_Mem,
//   326 (0x146) GFxStatMV_Other_Mem, 513 (0x201) GFxStatIME_Mem.
#define GFX3_STAT_DEFAULT_MEM 2

// ---------------------------------------------------------------------------------------------
// GNewOverrideBase<SID> - empty (PDB sizeof 1); it only carries the class-scoped operator new.
// ---------------------------------------------------------------------------------------------
class GMemoryHeap;
class GAllocDebugInfo;

template<int SID>
class GNewOverrideBase
{
public:
    enum { StatType = SID };
};

// ---------------------------------------------------------------------------------------------
// Reference counting. GRefCountImplCore is vptr + INT RefCount = 8 bytes (PDB: RefCount @4).
// GRefCountImpl / GRefCountNTSImpl add nothing; GRefCountBaseStatImpl<Impl,SID> adds nothing;
// GRefCountBase<C,SID> / GRefCountBaseNTS<C,SID> add nothing. The whole chain is 8 bytes with one
// vtable slot: the destructor.
// ---------------------------------------------------------------------------------------------
class GRefCountImplCore
{
public:
    int RefCount;

    GRefCountImplCore() : RefCount(1) {}
    virtual ~GRefCountImplCore() {}

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount == 0) delete this; }
    int GetRefCount() const { return RefCount; }
};

class GRefCountImpl : public GRefCountImplCore {};
class GRefCountNTSImpl : public GRefCountImplCore {};

template<class Impl, int SID>
class GRefCountBaseStatImpl : public Impl
{
public:
    enum { StatType = SID };
};

template<class C, int SID>
class GRefCountBase : public GRefCountBaseStatImpl<GRefCountImpl, SID> {};

template<class C, int SID>
class GRefCountBaseNTS : public GRefCountBaseStatImpl<GRefCountNTSImpl, SID> {};

// GRefCountWeakSupportImpl: GRefCountImplCore + one pointer (PDB sizeof 12).
class GWeakPtrProxy;
class GRefCountWeakSupportImpl : public GRefCountImplCore
{
public:
    GWeakPtrProxy* pWeakProxy;
};

// ---------------------------------------------------------------------------------------------
// GPtr<T> - the intrusive smart pointer; one raw pointer (PDB sizeof 4, member pObject @0).
// ---------------------------------------------------------------------------------------------
template<class C>
class GPtr
{
public:
    C* pObject;

    GPtr() : pObject(0) {}
    GPtr(C* p) : pObject(p) { if (pObject) pObject->AddRef(); }
    GPtr(const GPtr<C>& src) : pObject(src.pObject) { if (pObject) pObject->AddRef(); }
    ~GPtr() { if (pObject) pObject->Release(); }

    GPtr<C>& operator=(C* p)
    {
        if (p) p->AddRef();
        if (pObject) pObject->Release();
        pObject = p;
        return *this;
    }
    GPtr<C>& operator=(const GPtr<C>& src) { return operator=(src.pObject); }

    C* GetPtr() const { return pObject; }
    C& operator*() const { return *pObject; }
    C* operator->() const { return pObject; }
    operator C*() const { return pObject; }
    bool operator==(const GPtr<C>& o) const { return pObject == o.pObject; }
    bool operator!=(const GPtr<C>& o) const { return pObject != o.pObject; }
    void Clear() { operator=((C*)0); }
};

// ---------------------------------------------------------------------------------------------
// Intrusive doubly-linked list: GListNode<T> = {T* pPrev; T* pNext;} (8), GList<T> = {Root} (8).
// ---------------------------------------------------------------------------------------------
template<class T>
struct GListNode
{
    union { T* pPrev; void* pVoidPrev; };
    union { T* pNext; void* pVoidNext; };
};

template<class T, class B = T>
class GList
{
public:
    GListNode<B> Root;

    GList() { Root.pPrev = 0; Root.pNext = 0; }
    bool IsEmpty() const { return Root.pNext == 0 || (const void*)Root.pNext == (const void*)&Root; }
};

// ---------------------------------------------------------------------------------------------
// Atomics. GAtomicValueBase<T> = {T Value;} (4), GAtomicInt<T> adds nothing.
// ---------------------------------------------------------------------------------------------
template<class T>
class GAtomicValueBase
{
public:
    volatile T Value;
};

template<class T>
class GAtomicInt : public GAtomicValueBase<T>
{
public:
    GAtomicInt() { this->Value = (T)0; }
    GAtomicInt(T v) { this->Value = v; }
    operator T() const { return this->Value; }
    T operator++() { return ++this->Value; }
    T operator--() { return --this->Value; }
};

// ---------------------------------------------------------------------------------------------
// GArray<T,SID,Policy>. The PDB chain is
//   GArray -> GArrayBase<GArrayData<T,Alloc,Policy>> -> {T* Data; UInt Size; Policy Policy;} = 12.
// GArrayDefaultPolicy is {UInt Capacity;} (4).
// ---------------------------------------------------------------------------------------------
class GArrayDefaultPolicy
{
public:
    unsigned int Capacity;
    GArrayDefaultPolicy() : Capacity(0) {}
    unsigned int GetCapacity() const { return Capacity; }
    void SetCapacity(unsigned int c) { Capacity = c; }
};

template<int MinCapacity, int Granularity, int NeverShrink>
class GArrayConstPolicy
{
public:
    unsigned int Capacity;
    GArrayConstPolicy() : Capacity(0) {}
    unsigned int GetCapacity() const { return Capacity; }
    void SetCapacity(unsigned int c) { Capacity = c; }
};

// Storage-compatible with the runtime's array; growth goes through GMemory (GFx3Memory.cpp).
void* GFx3ArrayAlloc(unsigned int bytes, int statId);
void  GFx3ArrayFree(void* p);

template<class T, int SID = GFX3_STAT_DEFAULT_MEM, class SizePolicy = GArrayDefaultPolicy>
class GArray
{
public:
    T*           Data;
    unsigned int Size;
    SizePolicy   Policy;

    GArray() : Data(0), Size(0) {}
    ~GArray() { Clear(); }

    unsigned int GetSize() const { return Size; }
    unsigned int GetCapacity() const { return Policy.GetCapacity(); }
    T& operator[](unsigned int i) { return Data[i]; }
    const T& operator[](unsigned int i) const { return Data[i]; }

    void Clear()
    {
        for (unsigned int i = 0; i < Size; ++i) Data[i].~T();
        if (Data) GFx3ArrayFree(Data);
        Data = 0;
        Size = 0;
        Policy.SetCapacity(0);
    }

    void Reserve(unsigned int n)
    {
        if (n <= Policy.GetCapacity()) return;
        unsigned int cap = Policy.GetCapacity() ? Policy.GetCapacity() * 2 : 4;
        if (cap < n) cap = n;
        T* fresh = (T*)GFx3ArrayAlloc(cap * sizeof(T), SID);
        for (unsigned int i = 0; i < Size; ++i)
        {
            new (fresh + i) T(Data[i]);
            Data[i].~T();
        }
        if (Data) GFx3ArrayFree(Data);
        Data = fresh;
        Policy.SetCapacity(cap);
    }

    void PushBack(const T& v)
    {
        Reserve(Size + 1);
        new (Data + Size) T(v);
        ++Size;
    }

    void Resize(unsigned int n)
    {
        if (n > Size)
        {
            Reserve(n);
            for (unsigned int i = Size; i < n; ++i) new (Data + i) T();
        }
        else
        {
            for (unsigned int i = n; i < Size; ++i) Data[i].~T();
        }
        Size = n;
    }
};

// ---------------------------------------------------------------------------------------------
// GString - a single pointer whose low bits carry the heap type (PDB: the union of
// DataDesc* pData and UInt HeapTypeBits at @0, sizeof 4). DataDesc is {UInt Size; long RefCount;
// char Data[1];} = 12.
// ---------------------------------------------------------------------------------------------
class GString
{
public:
    struct DataDesc
    {
        unsigned int Size;
        long         RefCount;
        char         Data[1];
    };

    enum HeapType { HT_Global = 0, HT_Local = 1, HT_Dynamic = 2, HT_Mask = 3 };
    enum FlagConstants { Flag_LengthIsSizeShift = 0x1F };

    union
    {
        DataDesc*    pData;
        unsigned int HeapTypeBits;
    };

    GString() : pData(0) {}
    DataDesc* GetData() const { return (DataDesc*)(HeapTypeBits & ~(unsigned int)HT_Mask); }
    const char* ToCStr() const { DataDesc* d = GetData(); return d ? d->Data : ""; }
    unsigned int GetSize() const
    {
        DataDesc* d = GetData();
        return d ? (d->Size & ~(1u << Flag_LengthIsSizeShift)) : 0;
    }
};

class GStringLH : public GString {};

// ---------------------------------------------------------------------------------------------
// DISHONORED(bringup, agent DC): the log hook. This directory includes no engine header, so the AS2
// machine's own diagnostics went to printf - which in a Windows GUI process goes nowhere, so the seven
// script errors of the menu asset were invisible in the game while being visible in the harness. The
// host installs a hook and every one of them reaches Launch.log. GFxLogHook stays null in the harness,
// which keeps printf.
typedef void (*GFxLogHookFn)(const char* text);
extern GFxLogHookFn GFxLogHook;
void GFxLogf(const char* fmt, ...);

class GStringDH
{
public:
    GString      Str;
    GMemoryHeap* pHeap;
};

class GStringDataPtr
{
public:
    const char*  pStr;
    unsigned int Size;

    GStringDataPtr() : pStr(0), Size(0) {}
    GStringDataPtr(const char* s, unsigned int n) : pStr(s), Size(n) {}
};

// GStringBuffer: PDB sizeof 24 - {char* pData; UInt Size; UInt BufferSize; GMemoryHeap* pHeap;
//                                char* pHeapBuffer; UByte ...}
class GStringBuffer
{
public:
    char*        pData;
    unsigned int Size;
    unsigned int BufferSize;
    unsigned int GrowSize;
    bool         LengthIsSize;
    GUByte       _pad17[3];
    GMemoryHeap* pHeap;
};

// ---------------------------------------------------------------------------------------------
// GLock - a Win32 CRITICAL_SECTION by value (PDB: _RTL_CRITICAL_SECTION cs @0, sizeof 24).
// Declared as opaque storage so this header never pulls in windows.h; GFx3Lock.cpp drives it.
// ---------------------------------------------------------------------------------------------
class GLock
{
public:
    void* csStorage[6];

    GLock();
    ~GLock();
    void DoLock();
    void Unlock();

    class Locker
    {
    public:
        GLock* pLock;
        Locker(GLock* l) : pLock(l) { pLock->DoLock(); }
        ~Locker() { pLock->Unlock(); }
    };
};

class GLockSafe
{
public:
    void* csStorage[6];
};

// ---------------------------------------------------------------------------------------------
// Statistics. The Shipping build compiles the stat bodies out: GStat, GStatBag, GStatDesc,
// GMemoryStat, GCounterStat, GTimerStat are all sizeof 1 in the PDB.
// ---------------------------------------------------------------------------------------------
class GStat
{
public:
    enum StatType { Stat_LogicalGroup = 0, Stat_Memory = 1, Stat_Timer = 2, Stat_Counter = 3, Stat_TypeCount = 4 };
    class StatValue
    {
    public:
        enum ValueType { VT_None = 0, VT_Int = 1, VT_Int64 = 2, VT_Float = 3 };
        ValueType    Type;
        const char*  pName;
        union { unsigned int IValue; unsigned __int64 I64Value; float FValue; };
    };
};

class GStatDesc {};
class GStatBag {};
class GMemoryStat {};
class GCounterStat {};
class GTimerStat {};

class GStatInfo : public GStat
{
public:
    class StatInterface;
    unsigned int   StatId;
    StatInterface* pInterface;
    GStat*         pData;
};

// ---------------------------------------------------------------------------------------------
// Geometry. All four are POD with the PDB's exact layout.
// ---------------------------------------------------------------------------------------------
class GColor
{
public:
    struct Rgb24 { GUByte Blue, Green, Red; };
    struct Rgb32 { GUByte Blue, Green, Red, Alpha; };

    // PDB values: the standard colours are 0x00RRGGBB (no alpha) and the alphas are the alpha byte
    // already shifted into place, so a solid red is GColor(Red | Alpha100).
    enum StandardColors
    {
        Black = 0x000000, White = 0xFFFFFF, VeryLightGray = 0xE0E0E0, LightGray = 0xC0C0C0,
        Gray = 0x808080, DarkGray = 0x404040, VeryDarkGray = 0x202020,
        Red = 0xFF0000, LightRed = 0xFF8080, DarkRed = 0x800000, VeryDarkRed = 0x400000,
        Green = 0x00FF00, LightGreen = 0x80FF80, DarkGreen = 0x008000, VeryDarkGreen = 0x004000,
        Blue = 0x0000FF, LightBlue = 0x8080FF, DarkBlue = 0x000080, VeryDarkBlue = 0x000040,
        Cyan = 0x00FFFF, LightCyan = 0x80FFFF, DarkCyan = 0x008080,
        Magenta = 0xFF00FF, LightMagenta = 0xFF80FF, DarkMagenta = 0x800080,
        Yellow = 0xFFFF00, LightYellow = 0xFFFF80, DarkYellow = 0x808000,
        Purple = 0xFF00FF, DarkPurple = 0x800080, Pink = 0xFFC0C0, DarkPink = 0xC08080,
        Beige = 0xFFC080, LightBeige = 0xFFE0C0, DarkBeige = 0xC08040,
        Orange = 0xFF8000, Brown = 0x804000, LightBrown = 0xC06000, DarkBrown = 0x402000
    };
    enum StandardAlphas
    {
        Alpha0   = 0x00000000,
        Alpha25  = 0x40000000,
        Alpha50  = 0x7F000000,
        Alpha75  = 0xBF000000,
        Alpha100 = 0xFF000000
    };

    union
    {
        Rgb32         Channels;
        unsigned long Raw;
    };

    GColor() { Raw = 0; }
    GColor(unsigned long raw) { Raw = raw; }
    GColor(GUByte r, GUByte g, GUByte b, GUByte a = 255)
    {
        Channels.Red = r; Channels.Green = g; Channels.Blue = b; Channels.Alpha = a;
    }

    unsigned long GetRaw() const { return Raw; }
    void  SetRaw(unsigned long v) { Raw = v; }
    GUByte GetRed() const { return Channels.Red; }
    GUByte GetGreen() const { return Channels.Green; }
    GUByte GetBlue() const { return Channels.Blue; }
    GUByte GetAlpha() const { return Channels.Alpha; }
    void   SetAlpha(GUByte a) { Channels.Alpha = a; }
    bool operator==(const GColor& o) const { return Raw == o.Raw; }
    bool operator!=(const GColor& o) const { return Raw != o.Raw; }
};

template<class T>
class GPoint
{
public:
    T x, y;
    enum BoundsType { Min = 0, Max = 1 };
    GPoint() : x((T)0), y((T)0) {}
    GPoint(T ix, T iy) : x(ix), y(iy) {}
    bool operator==(const GPoint<T>& o) const { return x == o.x && y == o.y; }
    GPoint<T> operator+(const GPoint<T>& o) const { return GPoint<T>(x + o.x, y + o.y); }
    GPoint<T> operator-(const GPoint<T>& o) const { return GPoint<T>(x - o.x, y - o.y); }
};

template<class T>
class GPoint3
{
public:
    T x, y, z;
    GPoint3() : x((T)0), y((T)0), z((T)0) {}
    GPoint3(T ix, T iy, T iz) : x(ix), y(iy), z(iz) {}
};

template<class T>
class GSize
{
public:
    T Width, Height;
    enum BoundsType { Min = 0, Max = 1 };
    GSize() : Width((T)0), Height((T)0) {}
    GSize(T w, T h) : Width(w), Height(h) {}
};

template<class T>
class GRect
{
public:
    T Left, Top, Right, Bottom;
    enum BoundsType { Min = 0, Max = 1 };
    GRect() : Left((T)0), Top((T)0), Right((T)0), Bottom((T)0) {}
    GRect(T l, T t, T r, T b) : Left(l), Top(t), Right(r), Bottom(b) {}
    T Width() const { return Right - Left; }
    T Height() const { return Bottom - Top; }
    bool IsEmpty() const { return Left >= Right || Top >= Bottom; }
    bool Contains(T x, T y) const { return x >= Left && x <= Right && y >= Top && y <= Bottom; }
};

// GMatrix2D - float M_[2][3] in the SDK's spelling; the PDB records `float[3][2] M_` because DIA
// reports the innermost extent first. 24 bytes either way; element (row, col) is M_[row][col].
class GMatrix2D
{
public:
    float M_[2][3];

    GMatrix2D() { SetIdentity(); }
    void SetIdentity()
    {
        M_[0][0] = 1.f; M_[0][1] = 0.f; M_[0][2] = 0.f;
        M_[1][0] = 0.f; M_[1][1] = 1.f; M_[1][2] = 0.f;
    }
    void Transform(float* x, float* y) const
    {
        float nx = M_[0][0] * (*x) + M_[0][1] * (*y) + M_[0][2];
        float ny = M_[1][0] * (*x) + M_[1][1] * (*y) + M_[1][2];
        *x = nx; *y = ny;
    }
};

class GMatrix3D
{
public:
    float M_[4][4];

    GMatrix3D() { SetIdentity(); }
    void SetIdentity()
    {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                M_[r][c] = (r == c) ? 1.f : 0.f;
    }
};

class GMatrix3DNewable : public GMatrix3D {};

// GViewport - the render target rectangle GFx draws a movie into.
class GViewport
{
public:
    enum
    {
        View_IsRenderTexture     = 0x1,
        View_AlphaComposite      = 0x2,
        View_UseScissorRect      = 0x4,
        View_NoSetState          = 0x8,
        View_RenderTextureAlpha  = 0x3
    };

    int   BufferWidth, BufferHeight;
    int   Left, Top;
    int   Width, Height;
    int   ScissorLeft, ScissorTop;
    int   ScissorWidth, ScissorHeight;
    float Scale;
    float AspectRatio;
    unsigned int Flags;

    GViewport()
        : BufferWidth(0), BufferHeight(0), Left(0), Top(0), Width(0), Height(0),
          ScissorLeft(0), ScissorTop(0), ScissorWidth(0), ScissorHeight(0),
          Scale(1.f), AspectRatio(1.f), Flags(0) {}
    GViewport(int bw, int bh, int l, int t, int w, int h)
        : BufferWidth(bw), BufferHeight(bh), Left(l), Top(t), Width(w), Height(h),
          ScissorLeft(l), ScissorTop(t), ScissorWidth(w), ScissorHeight(h),
          Scale(1.f), AspectRatio(1.f), Flags(0) {}
};

// ---------------------------------------------------------------------------------------------
// File and string constants: empty tag classes whose only content is their enums.
// ---------------------------------------------------------------------------------------------
class GFileConstants
{
public:
    enum OpenFlags
    {
        Open_Read       = 0x01,
        Open_Write      = 0x02,
        Open_ReadWrite  = 0x03,
        Open_Truncate   = 0x04,
        Open_Create     = 0x08,
        Open_CreateOnly = 0x18,
        Open_Buffered   = 0x20
    };
    enum Modes { Mode_Read = 0x124, Mode_Write = 0x92, Mode_Execute = 0x49, Mode_ReadWrite = 0x1B6 };
    enum SeekOps { Seek_Set = 0, Seek_Cur = 1, Seek_End = 2 };
    enum Errors
    {
        Error_FileNotFound = 0x1001,
        Error_Access       = 0x1002,
        Error_IOError      = 0x1003,
        Error_DiskFull     = 0x1004
    };
};

class GFileStat
{
public:
    GInt64 ModifyTime;
    GInt64 AccessTime;
    GInt64 FileSize;
};

class GFxFileConstants
{
public:
    enum FileFormatType
    {
        File_Unopened  = 0,
        File_Unknown   = 1,
        File_SWF       = 2,
        File_GFX       = 3,
        File_JPEG      = 10,
        File_PNG       = 11,
        File_GIF       = 12,
        File_TGA       = 13,
        File_DDS       = 14,
        File_HDR       = 15,
        File_BMP       = 16,
        File_DIB       = 17,
        File_PFM       = 18,
        File_TIFF      = 19,
        File_WAVE      = 20,
        File_NextAvail = 21,
        File_Original  = 0xFFFF
    };
};

#pragma pack(pop)
#endif // INC_GFX3_GTYPES_H
