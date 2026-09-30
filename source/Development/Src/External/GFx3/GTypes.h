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
#include <math.h>
#include <new>
#include <intrin.h>

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

// DISHONORED(written): no rva of its own - GAtomicInt is inlined at every use in retail; the type's
// anchor is this header's, 2013 0xdf7374. GAtomicInt is the name of the type, and the interlocked
// increment is the whole point of it. These two operators read `++this->Value` / `--this->Value`,
// which is a plain read-modify-write on a volatile long, and every reference count that crosses
// the game/render thread boundary is one of these: GTexture, GRenderTarget and
// FGFxRendererImpl's element stores are AddRef'd
// on the game thread when a draw is enqueued and Released on the render thread when it executes
// (FGFxDrawBitmapsInternal / FGFxRenderer::DrawBitmaps_RenderThread, SetUIRenderElementStore and its
// transfer command). One lost increment takes the count to zero while an owner still holds the
// pointer, the object is deleted, the heap hands the block back out, and the next virtual call on it
// jumps into whatever overwrote the vptr. That is what killed the main menu on the render thread:
// a wild call at FGFxTexture::Bind out of DrawBitmaps_RenderThread, between one and forty seconds
// after the interface appeared, on the one GTexture the glyph atlas keeps alive for the whole run.
// GAtomicValueBase stays {volatile T Value} - the PDB has no other member and interlocked operations
// need none.
template<class T>
class GAtomicInt : public GAtomicValueBase<T>
{
public:
    GAtomicInt() { this->Value = (T)0; }
    GAtomicInt(T v) { this->Value = v; }
    operator T() const { return this->Value; }
    T operator++()
    {
        static_assert(sizeof(T) == sizeof(long), "GAtomicInt is interlocked on 32-bit values only");
        return (T)_InterlockedIncrement((volatile long*)&this->Value);
    }
    T operator--()
    {
        static_assert(sizeof(T) == sizeof(long), "GAtomicInt is interlocked on 32-bit values only");
        return (T)_InterlockedDecrement((volatile long*)&this->Value);
    }
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
    // DISHONORED(port): 2012 0x9b2530 / 0x9b2550 / 0x9b2570. The length of each axis and the angle
    // of the x axis, which is how AS2's _xscale, _yscale and _rotation are read off a matrix.
    double GetXScale() const { return sqrt((double)(M_[1][0] * M_[1][0] + M_[0][0] * M_[0][0])); }
    double GetYScale() const { return sqrt((double)(M_[1][1] * M_[1][1] + M_[0][1] * M_[0][1])); }
    double GetRotation() const { return atan2((double)M_[1][0], (double)M_[0][0]); }
    bool IsValid() const
    {
        for (int r = 0; r < 2; ++r)
            for (int c = 0; c < 3; ++c)
                if (!(M_[r][c] == M_[r][c]) || M_[r][c] > 3.4e38f || M_[r][c] < -3.4e38f)
                    return false;
        return true;
    }
    // DISHONORED(port): 2013 0x9ab4b0. A singular matrix answers identity with the translation
    // negated, which is retail's own arm and is what keeps a zero-scaled clip from producing
    // infinities in the hit test rather than simply missing.
    void SetInverse(const GMatrix2D& m)
    {
        const float det = m.M_[0][0] * m.M_[1][1] - m.M_[1][0] * m.M_[0][1];
        if (det == 0.f)
        {
            SetIdentity();
            M_[0][2] = -m.M_[0][2];
            M_[1][2] = -m.M_[1][2];
            return;
        }
        const float r = 1.f / det;
        M_[0][0] = r * m.M_[1][1];
        M_[1][1] = r * m.M_[0][0];
        M_[0][1] = -m.M_[0][1] * r;
        M_[1][0] = r * -m.M_[1][0];
        M_[0][2] = -(m.M_[1][2] * M_[0][1] + m.M_[0][2] * M_[0][0]);
        M_[1][2] = -(m.M_[1][2] * M_[1][1] + m.M_[0][2] * M_[1][0]);
    }
    // DISHONORED(port): 2013 0x9ab5d0. this = this * m, i.e. m applies first.
    GMatrix2D& Prepend(const GMatrix2D& m)
    {
        const float a = M_[0][0], b = M_[0][1], c = M_[0][2];
        const float d = M_[1][0], e = M_[1][1], f = M_[1][2];
        M_[0][0] = m.M_[0][0] * a + m.M_[1][0] * b;
        M_[1][0] = m.M_[0][0] * d + m.M_[1][0] * e;
        M_[0][1] = m.M_[0][1] * a + b * m.M_[1][1];
        M_[1][1] = m.M_[0][1] * d + e * m.M_[1][1];
        M_[0][2] = b * m.M_[1][2] + a * m.M_[0][2] + c;
        M_[1][2] = e * m.M_[1][2] + d * m.M_[0][2] + f;
        return *this;
    }
    // DISHONORED(port): 2013 0x9ab750. The point in the space this matrix maps FROM.
    void TransformByInverse(GPoint<float>* out, const GPoint<float>& p) const
    {
        GMatrix2D inv;
        inv.SetInverse(*this);
        out->x = p.x * inv.M_[0][0] + p.y * inv.M_[0][1] + inv.M_[0][2];
        out->y = p.y * inv.M_[1][1] + p.x * inv.M_[1][0] + inv.M_[1][2];
    }
};

// GMatrix3D - float M_[4][4]. The convention is ROW-VECTOR (a point is a row, `p * M`, and the
// translation lives in row 3), and that is read out of retail rather than assumed: GMatrix3D's
// conversion from GMatrix2D (2013 0x9ac010) puts the 2D matrix's translation column M_[0][2] /
// M_[1][2] into M_[3][0] / M_[3][1], and PerspectiveFocalLengthLH (0x9ac800) writes the w-producing
// 1 into M_[2][3]. GMatrix2D is the other way round (`M * p`, translation in column 2), so the
// conversion below is a transpose and not a copy.
class GMatrix3D
{
public:
    float M_[4][4];

    GMatrix3D() { SetIdentity(); }
    // DISHONORED(port): 2013 0x9ac010.
    explicit GMatrix3D(const GMatrix2D& m)
    {
        M_[0][0] = m.M_[0][0]; M_[0][1] = m.M_[1][0]; M_[0][2] = 0.f; M_[0][3] = 0.f;
        M_[1][0] = m.M_[0][1]; M_[1][1] = m.M_[1][1]; M_[1][2] = 0.f; M_[1][3] = 0.f;
        M_[2][0] = 0.f;        M_[2][1] = 0.f;        M_[2][2] = 1.f; M_[2][3] = 0.f;
        M_[3][0] = m.M_[0][2]; M_[3][1] = m.M_[1][2]; M_[3][2] = 0.f; M_[3][3] = 1.f;
    }
    void SetIdentity()
    {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                M_[r][c] = (r == c) ? 1.f : 0.f;
    }
    // DISHONORED(deviation, agent FA): the same conversion with the depth axis scaled by the 2D
    // matrix's own uniform scale instead of left at 1. Retail's body leaves it at 1 because retail's
    // whole display chain and its perspective are in one unit (twips); this tree's chain ends in the
    // twips-to-pixels scale at the root (GFxDisplay.cpp's "Coordinate convention" note) while the
    // perspective is built from the frame rect in pixels, so a z left unscaled would be twenty times
    // too deep and a five-degree y rotation would swing a clip through the camera. Measured: with the
    // literal body the main menu's logo drew at about 2.5x and skewed (fa_apshottime00001.png).
    void SetFrom2DWithDepth(const GMatrix2D& m)
    {
        *this = GMatrix3D(m);
        M_[2][2] = (float)((m.GetXScale() + m.GetYScale()) * 0.5);
    }
    // DISHONORED(port): 2013 0x9ac060. Every element finite and inside the float range; a matrix that
    // fails this is discarded rather than stored, which is what keeps one NaN out of the display list.
    bool IsValid() const
    {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                if (!(M_[r][c] >= -3.402823466e38f && M_[r][c] <= 3.402823466e38f))
                    return false;
        return true;
    }
    // DISHONORED(port): 2013 0x9ac280. this = a * b, row-major, so `a` applies first to a row vector.
    // Written out rather than looped because the aliasing matters: every caller in retail passes a
    // copy of the destination as `a` or `b`.
    void MultiplyMatrix(const GMatrix3D& a, const GMatrix3D& b)
    {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                M_[r][c] = a.M_[r][0] * b.M_[0][c] + a.M_[r][1] * b.M_[1][c]
                         + a.M_[r][2] * b.M_[2][c] + a.M_[r][3] * b.M_[3][c];
    }
    void RotateX(float radians)                                        // 2013 0x9ac6a0
    {
        const float c = (float)cos(radians), s = (float)sin(radians);
        SetIdentity();
        M_[1][1] = c;  M_[1][2] = s;
        M_[2][1] = -s; M_[2][2] = c;
    }
    void RotateY(float radians)                                        // 2013 0x9ac720
    {
        const float c = (float)cos(radians), s = (float)sin(radians);
        SetIdentity();
        M_[0][0] = c; M_[0][2] = -s;
        M_[2][0] = s; M_[2][2] = c;
    }
    // DISHONORED(port): 2013 0x9ac800 / 0x9ac7a0. Focal length rather than a field of view, because
    // that is what MakeViewAndPersp3D has in hand: the eye sits one focal length from the stage plane.
    void PerspectiveFocalLengthLH(float focal, float w, float h, float zn, float zf)
    {
        Zero();
        M_[0][0] = (focal + focal) / w;
        M_[1][1] = (focal + focal) / h;
        M_[2][2] = zf / (zf - zn);
        M_[2][3] = 1.f;
        M_[3][2] = -zn * zf / (zf - zn);
    }
    void PerspectiveFocalLengthRH(float focal, float w, float h, float zn, float zf)
    {
        Zero();
        M_[0][0] = (focal + focal) / w;
        M_[1][1] = (focal + focal) / h;
        M_[2][2] = zf / (zn - zf);
        M_[2][3] = -1.f;
        M_[3][2] = zn * zf / (zn - zf);
    }
    void ViewLH(const GPoint3<float>& eye, const GPoint3<float>& at, const GPoint3<float>& up)
    {
        BuildView(eye, at, up, true);                                  // 2013 0x9aca90
    }
    void ViewRH(const GPoint3<float>& eye, const GPoint3<float>& at, const GPoint3<float>& up)
    {
        BuildView(eye, at, up, false);                                 // 2013 0x9ac860
    }
    // DISHONORED(port): 2013 0x9abf40 with 0x9abdb0 - the adjugate over the determinant, spelled as
    // retail spells it (a cofactor per element). Only the render-target correction needs it.
    void SetInverse(const GMatrix3D& m)
    {
        float cof[4][4];
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                cof[r][c] = m.Cofactor(r, c);
        const float det = m.M_[0][0] * cof[0][0] + m.M_[0][1] * cof[0][1]
                        + m.M_[0][2] * cof[0][2] + m.M_[0][3] * cof[0][3];
        if (det == 0.f)
        {
            SetIdentity();
            return;
        }
        const float inv = 1.f / det;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                M_[r][c] = cof[c][r] * inv;
    }
    static const GMatrix3D& GetIdentity()
    {
        static const GMatrix3D identity;
        return identity;
    }

private:
    void Zero()
    {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                M_[r][c] = 0.f;
    }
    float Cofactor(int row, int col) const                             // 2013 0x9abdb0
    {
        float m3[3][3];
        int dr = 0;
        for (int r = 0; r < 4; ++r)
        {
            if (r == row)
                continue;
            int dc = 0;
            for (int c = 0; c < 4; ++c)
            {
                if (c == col)
                    continue;
                m3[dr][dc++] = M_[r][c];
            }
            ++dr;
        }
        const float minor = m3[0][0] * (m3[1][1] * m3[2][2] - m3[1][2] * m3[2][1])
                          - m3[0][1] * (m3[1][0] * m3[2][2] - m3[1][2] * m3[2][0])
                          + m3[0][2] * (m3[1][0] * m3[2][1] - m3[1][1] * m3[2][0]);
        return ((row + col) & 1) ? -minor : minor;
    }
    // The two view builders differ in one thing only - which way the z axis points - so retail's two
    // bodies are the same arithmetic with `at - eye` and `eye - at` swapped.
    void BuildView(const GPoint3<float>& eye, const GPoint3<float>& at, const GPoint3<float>& up,
                   bool bLeftHanded)
    {
        Zero();
        float zx = bLeftHanded ? (at.x - eye.x) : (eye.x - at.x);
        float zy = bLeftHanded ? (at.y - eye.y) : (eye.y - at.y);
        float zz = bLeftHanded ? (at.z - eye.z) : (eye.z - at.z);
        const float zlen = (float)sqrt((double)(zx * zx + zy * zy + zz * zz));
        zx /= zlen; zy /= zlen; zz /= zlen;
        float xx = up.y * zz - up.z * zy;
        float xy = up.z * zx - zz * up.x;
        float xz = zy * up.x - zx * up.y;
        const float xlen = (float)sqrt((double)(xx * xx + xy * xy + xz * xz));
        xx /= xlen; xy /= xlen; xz /= xlen;
        const float yx = zy * xz - zz * xy;
        const float yy = zz * xx - zx * xz;
        const float yz = zx * xy - zy * xx;
        M_[0][0] = xx; M_[1][0] = xy; M_[2][0] = xz;
        M_[3][0] = -(xx * eye.x + xy * eye.y + xz * eye.z);
        M_[0][1] = yx; M_[1][1] = yy; M_[2][1] = yz;
        M_[3][1] = -(yx * eye.x + yy * eye.y + yz * eye.z);
        M_[0][2] = zx; M_[1][2] = zy; M_[2][2] = zz;
        M_[3][2] = -(zx * eye.x + zy * eye.y + zz * eye.z);
        M_[3][3] = 1.f;
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
