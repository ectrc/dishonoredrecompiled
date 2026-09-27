// GFxUI/Inc/gfxui_gfx3.h - the Scaleform GFx 3.3.89 surface the GFxUI native layer talks to, plus the
// engine-side seam types that layer shares with the runtime glue. This is agent BE's ONE header of
// assumptions (PHASE8.md package BE): agent BB reconstructs the runtime itself into
// source/Development/Src/External/GFx3, and when its headers land DISHONORED_WITH_GFX3 becomes 1 and this
// file forwards to them instead of declaring anything - every .cpp of the layer keeps compiling unchanged.
//
// Nothing here is guessed. Every type, member order, enumerator value and function signature below comes
// from the 2012 symbolized build (resources/docs/types/all_types.h for the layouts and vtables,
// resources/docs/symbols/functions.csv for the demangled signatures and rvas), and agent AW measured that
// 92.9 % of libgfx is byte-identical 2012 -> 2013, so those names describe the retail runtime exactly
// (resources/docs/gfx_decision.md section 1). 2012 rvas are marked; 2013 rvas come from
// match_2012_2013.csv.
//
// DISHONORED(layout): GFxValue 16 bytes, GFxValue::DisplayInfo 8-aligned 120 bytes, GMatrix2D 24,
// GMatrix3D 64, GRenderer::Cxform 32, GViewport 40 - all 2012 PDB sizes, asserted at the end of the file.
#ifndef _INC_GFXUI_GFX3
#define _INC_GFXUI_GFX3

// The switch. It is deliberately NOT agent BB's DISHONORED_WITH_GFX3: that one says "the reconstructed GFx
// 3.3 headers and the renderer/file/image seam are in the build", which is already true, while this one says
// "something implements GFxValue::ObjectInterface and GFxMovieView", which nothing does yet - BB's
// External/GFx3 is headers plus layout assertions and has no .cpp for either (package BC is writing the
// ActionScript machine that will). So this layer keeps its own declarations and its own link-time bodies
// until then, and the module builds and links identically with DISHONORED_WITH_GFX3 on and off.
//
// Switching over, when BC's runtime exists, is three things and nothing else:
//   1. define DISHONORED_GFXUI_GFX3_RUNTIME=1, which makes this header include BB's GFx3.h instead of
//      declaring anything and drops Src/gfxuigfx3absent.cpp out of the build;
//   2. BB must fix three things in GFxValue.h first, all of which this header has right and which are
//      checkable against the 2012 PDB enum dump (resources/docs/types/all_types.h) and against the retail
//      decompiles in build/agentBE/dec2012:
//        * VTC_ConvertBit is 0x80, VTC_ManagedBit is 0x40 and VTC_TypeMask is 0x8F, not 0x08 / 0x10 / 0x0F;
//          every retail body masks the type with 0x8F and tests the managed bit with 0x40
//          (UGFxObject::GetElementFloat, 2012 0x5ba1a0, is the shortest example). The convertible types
//          follow: VT_ConvertNumber is 0x83, not 0x0B;
//        * GFxValue::ObjectInterface::ObjVisitor::Visit takes const GFxValue* , not const GFxValue&
//          (all_types.h, GFxValue::ObjectInterface::ObjVisitor_vtbl);
//        * GFxValue::DisplayInfo::Flags has no V_projMatrix3D; the PDB's set ends at V_viewMatrix3D
//          = 0x2000, and V_perspFOV / V_perspMatrix3D are the names of 0x800 / 0x1000;
//   3. the engine-side seam below (FGFxMovie, FGFxEngine, the two Callbacks, FAutoGFxValueArray) moves into
//      BB's Inc/gfxuiengine.h, which is where retail has it; it is still a comment-only skeleton today.
#ifndef DISHONORED_GFXUI_GFX3_RUNTIME
	#define DISHONORED_GFXUI_GFX3_RUNTIME 0
#endif

#if DISHONORED_GFXUI_GFX3_RUNTIME

// agent BB's reconstruction; one umbrella header, include path from cmake/GFx.cmake
#include "GFx3.h"

#else // !DISHONORED_GFXUI_GFX3_RUNTIME

// DISHONORED(layout): GFx is not built with UE3's /Zp4, and the proof is in the PDB: GFxValue::DisplayInfo
// has `bool Visible` at 48 and the next `double Z` at 56, and the whole struct is 232 bytes. Everything from
// here to the matching pop is therefore 8-packed, exactly as agent BB's GFxValue.h does it.
#pragma pack(push, 8)

class GFxMovieDef;
class GFxMovieView;
class GFxFunctionHandler;
class GMemoryHeap;
class GStatBag;
class GFxEvent;
struct GFxExporterInfo;

// GRefCountImplCore (2012 all_types.h): the base of every GFx refcounted object. GPtr<> below only ever
// needs AddRef/Release, which GFx spells as members of GRefCountImpl.
class GRefCountImplCore
{
public:
	virtual ~GRefCountImplCore() {}
	volatile INT RefCount;

	void AddRef();
	void Release();
};

// GPtr<T> (GFx 3.3 GRefCount.h): an intrusive smart pointer whose one member is the raw pointer, which is
// why FGFxMovie's pDef/pView read as plain pointers in the decompiles.
template<class T> class GPtr
{
public:
	T* pObject;

	GPtr() : pObject(NULL) {}
	GPtr(T* p) : pObject(p) { if( pObject ) pObject->AddRef(); }
	GPtr(const GPtr& o) : pObject(o.pObject) { if( pObject ) pObject->AddRef(); }
	~GPtr() { if( pObject ) pObject->Release(); }
	GPtr& operator=(T* p)
	{
		if( p ) p->AddRef();
		if( pObject ) pObject->Release();
		pObject = p;
		return *this;
	}
	GPtr& operator=(const GPtr& o) { return operator=(o.pObject); }
	T* operator->() const { return pObject; }
	T& operator*() const { return *pObject; }
	operator T*() const { return pObject; }
	UBOOL operator!() const { return pObject == NULL; }
	T* GetPtr() const { return pObject; }
};

// GMatrix2D / GMatrix3D / GRenderer::Cxform / GViewport (2012 all_types.h). GFx stores 2D transforms
// row-major 2x3 and 3D 4x4; the Cxform is the 4x2 multiply/add colour transform UGFxObject exposes as
// FASColorTransform.
class GMatrix2D
{
public:
	FLOAT M_[2][3];

	GMatrix2D() { SetIdentity(); }
	void SetIdentity()
	{
		M_[0][0] = 1.f; M_[0][1] = 0.f; M_[0][2] = 0.f;
		M_[1][0] = 0.f; M_[1][1] = 1.f; M_[1][2] = 0.f;
	}
};

class GMatrix3D
{
public:
	FLOAT M_[4][4];

	GMatrix3D() { SetIdentity(); }
	void SetIdentity()
	{
		for( INT Row = 0; Row < 4; Row++ )
		{
			for( INT Col = 0; Col < 4; Col++ )
			{
				M_[Row][Col] = Row == Col ? 1.f : 0.f;
			}
		}
	}
};

struct GViewport
{
	INT BufferWidth;
	INT BufferHeight;
	INT Left;
	INT Top;
	INT Width;
	INT Height;
	INT ScissorLeft;
	INT ScissorTop;
	INT ScissorWidth;
	INT ScissorHeight;
	FLOAT Scale;
	FLOAT AspectRatio;
	UINT Flags;

	GViewport()
		: BufferWidth(0), BufferHeight(0), Left(0), Top(0), Width(0), Height(0)
		, ScissorLeft(0), ScissorTop(0), ScissorWidth(0), ScissorHeight(0)
		, Scale(1.f), AspectRatio(1.f), Flags(0)
	{}
	GViewport(INT InBufferWidth, INT InBufferHeight, INT InLeft, INT InTop, INT InWidth, INT InHeight)
		: BufferWidth(InBufferWidth), BufferHeight(InBufferHeight), Left(InLeft), Top(InTop)
		, Width(InWidth), Height(InHeight)
		, ScissorLeft(InLeft), ScissorTop(InTop), ScissorWidth(InWidth), ScissorHeight(InHeight)
		, Scale(1.f), AspectRatio(1.f), Flags(0)
	{}
};

class GRenderer
{
public:
	struct Cxform
	{
		FLOAT M_[4][2];
	};
};

// GFxValue (2012 all_types.h, methods from functions.csv). The layout matters twice over: UGFxObject
// keeps one inline in its reflected INT Value[12] and the decompiles index it as Value[0]=pObjectInterface,
// Value[1]=Type, Value[2]=Value.pData.
class GFxValue
{
public:
	enum ValueType
	{
		VT_Undefined      = 0x00,
		VT_Null           = 0x01,
		VT_Boolean        = 0x02,
		VT_Number         = 0x03,
		VT_String         = 0x04,
		VT_StringW        = 0x05,
		VT_Object         = 0x06,
		VT_Array          = 0x07,
		VT_DisplayObject  = 0x08,

		VTC_ConvertBit    = 0x80,
		VTC_ManagedBit    = 0x40,
		VTC_TypeMask      = 0x8F,

		VT_ConvertBoolean = VTC_ConvertBit | VT_Boolean,
		VT_ConvertNumber  = VTC_ConvertBit | VT_Number,
		VT_ConvertString  = VTC_ConvertBit | VT_String,
		VT_ConvertStringW = VTC_ConvertBit | VT_StringW,
	};

	// GFxValue::DisplayInfo (2012 all_types.h, __declspec(align(8))): every field is a double and VarsSet
	// is the Flags bitmask of the fields that were actually assigned.
	struct DisplayInfo
	{
		enum Flags
		{
			V_x              = 0x0001,
			V_y              = 0x0002,
			V_rotation       = 0x0004,
			V_xscale         = 0x0008,
			V_yscale         = 0x0010,
			V_alpha          = 0x0020,
			V_visible        = 0x0040,
			V_z              = 0x0080,
			V_xrotation      = 0x0100,
			V_yrotation      = 0x0200,
			V_zscale         = 0x0400,
			V_perspFOV       = 0x0800,
			V_perspMatrix3D  = 0x1000,
			V_viewMatrix3D   = 0x2000,
		};

		DOUBLE X;
		DOUBLE Y;
		DOUBLE Rotation;
		DOUBLE XScale;
		DOUBLE YScale;
		DOUBLE Alpha;
		bool Visible;
		DOUBLE Z;
		DOUBLE XRotation;
		DOUBLE YRotation;
		DOUBLE ZScale;
		DOUBLE PerspFOV;
		GMatrix3D ViewMatrix3D;
		GMatrix3D PerspectiveMatrix3D;
		WORD VarsSet;

		DisplayInfo() : VarsSet(0) {}

		void SetX(DOUBLE In)        { X = In;        VarsSet |= V_x; }
		void SetY(DOUBLE In)        { Y = In;        VarsSet |= V_y; }
		void SetZ(DOUBLE In)        { Z = In;        VarsSet |= V_z; }
		void SetRotation(DOUBLE In) { Rotation = In; VarsSet |= V_rotation; }
		void SetXScale(DOUBLE In)   { XScale = In;   VarsSet |= V_xscale; }
		void SetYScale(DOUBLE In)   { YScale = In;   VarsSet |= V_yscale; }
		void SetZScale(DOUBLE In)   { ZScale = In;   VarsSet |= V_zscale; }
		void SetAlpha(DOUBLE In)    { Alpha = In;    VarsSet |= V_alpha; }
		void SetVisible(bool In)    { Visible = In;  VarsSet |= V_visible; }
		void SetPosition(DOUBLE InX, DOUBLE InY) { SetX(InX); SetY(InY); }
		// GFxValue::DisplayInfo::Set (2012 rva 0x9ad320): the whole 2D+3D block in declaration order
		void Set(DOUBLE InX, DOUBLE InY, DOUBLE InRotation, DOUBLE InXScale, DOUBLE InYScale, DOUBLE InAlpha,
			bool bInVisible, DOUBLE InZ, DOUBLE InXRotation, DOUBLE InYRotation, DOUBLE InZScale);
	};

	// GFxValue::ObjectInterface (2012 all_types.h nested forward declarations + functions.csv): not a
	// virtual interface - the AS2 runtime's object bridge, whose methods all take the value's pData as
	// their first argument. 1,096 of the engine's 1,350 calls into libgfx land here (gfx_decision.md 2.2).
	class ObjectInterface
	{
	public:
		class ObjVisitor
		{
		public:
			virtual ~ObjVisitor() {}
			virtual void Visit(const char* Name, const GFxValue* Value) = 0;
		};
		class ArrVisitor
		{
		public:
			virtual ~ArrVisitor() {}
			virtual void Visit(UINT Index, const GFxValue* Value) = 0;
		};

		void ObjectAddRef(GFxValue* Value, void* pData);                                                          // 2012 0x9ae650
		void ObjectRelease(GFxValue* Value, void* pData);                                                          // 2012 0x9aed40
		bool GetMember(void* pData, const char* Name, GFxValue* Out, bool bIsDisplayObject) const;                 // 2012 0x9b0450
		bool SetMember(void* pData, const char* Name, const GFxValue& Value, bool bIsDisplayObject);               // 2012 0x9ad590
		bool Invoke(void* pData, GFxValue* Result, const char* Name, const GFxValue* Args, UINT ArgCount,
			bool bIsDisplayObject);                                                                                // 2012 0x9afbb0
		UINT GetArraySize(void* pData) const;                                                                      // 2012 0x9ad650
		bool GetElement(void* pData, UINT Index, GFxValue* Out) const;                                             // 2012 0x9b05c0
		bool SetElement(void* pData, UINT Index, const GFxValue& Value);                                           // 2012 0x9ad670
		bool PushBack(void* pData, const GFxValue& Value);                                                         // 2012 0x9ad6c0
		bool GetText(void* pData, GFxValue* Out, bool bIsHtml) const;                                              // 2012 0x9b0640
		bool SetText(void* pData, const char* Text, bool bIsHtml);                                                 // 2012 0x9afdb0
		bool SetText(void* pData, const TCHAR* Text, bool bIsHtml);                                                // 2012 0x9afee0
		bool GetDisplayInfo(void* pData, DisplayInfo* Out) const;                                                  // 2012 0x9ad900
		bool SetDisplayInfo(void* pData, const DisplayInfo& Info);                                                 // 2012 0x9adb20
		bool GetDisplayMatrix(void* pData, GMatrix2D* Out) const;                                                  // 2012 0x9ad710
		bool SetDisplayMatrix(void* pData, const GMatrix2D& Matrix);                                               // 2012 0x9ad7a0
		bool SetMatrix3D(void* pData, const GMatrix3D& Matrix);                                                    // 2012 0x9acd30
		bool GetCxform(void* pData, GRenderer::Cxform* Out) const;                                                 // 2012 0x9ad130
		bool SetCxform(void* pData, const GRenderer::Cxform& Cxform);                                              // 2012 0x9ad160
		bool GotoAndPlay(void* pData, const char* Frame, bool bStop);                                              // 2012 0x9ad050
		bool GotoAndPlay(void* pData, UINT Frame, bool bStop);                                                     // 2012 0x9ad0d0
		bool CreateEmptyMovieClip(void* pData, GFxValue* Out, const char* InstanceName, INT Depth);                // 2012 0x9af6a0
		bool AttachMovie(void* pData, GFxValue* Out, const char* SymbolName, const char* InstanceName, INT Depth,
			const GFxValue* InitObject);                                                                           // 2012 0x9af830
		void VisitMembers(void* pData, ObjVisitor* Visitor, bool bIsDisplayObject) const;                          // 2012 0x9ae6c0
	};

	union ValueUnion
	{
		DOUBLE NValue;
		bool BValue;
		const char* pString;
		const char* const* pStringManaged;
		const TCHAR* pStringW;
		void* pData;
	};

	ObjectInterface* pObjectInterface;
	ValueType Type;
	ValueUnion Value;

	GFxValue() : pObjectInterface(NULL), Type(VT_Undefined) { Value.pData = NULL; }
	explicit GFxValue(ValueType InType) : pObjectInterface(NULL), Type(InType) { Value.pData = NULL; }
	GFxValue(const GFxValue& Other);                                                                              // 2012 0x5b9620
	~GFxValue();                                                                                                   // 2012 0x20f20
	const GFxValue& operator=(const GFxValue& Other);                                                              // 2012 0x5b9670

	ValueType GetType() const { return (ValueType)(Type & VTC_TypeMask); }
	UBOOL IsManaged() const { return (Type & VTC_ManagedBit) != 0; }
	UBOOL IsUndefined() const { return GetType() == VT_Undefined; }
	UBOOL IsNull() const { return GetType() == VT_Null; }
	UBOOL IsBool() const { return GetType() == VT_Boolean; }
	UBOOL IsNumber() const { return GetType() == VT_Number; }
	UBOOL IsString() const { return GetType() == VT_String; }
	UBOOL IsStringW() const { return GetType() == VT_StringW; }
	UBOOL IsObject() const;                                                                                        // 2012 0x167d0
	UBOOL IsArray() const { return GetType() == VT_Array; }
	UBOOL IsDisplayObject() const { return GetType() == VT_DisplayObject; }

	bool GetBool() const { return Value.BValue; }
	DOUBLE GetNumber() const { return Value.NValue; }
	const char* GetString() const;                                                                                 // 2012 0x20f50
	const TCHAR* GetStringW() const { return Value.pStringW; }

	void SetUndefined();                                                                                           // 2012 0x5bf330
	void SetNull();                                                                                                // 2012 0x5bf360
	void SetBoolean(bool In);                                                                                      // 2012 0x5bf390
	void SetNumber(DOUBLE In);                                                                                     // 2012 0x5bf3e0
	void SetString(const char* In);                                                                                // 2012 0x7fa030
	void SetStringW(const TCHAR* In);                                                                              // 2012 0x5bf420

	// The convenience forwarders GFx 3.3 declares inline on GFxValue; the retail build inlined them, which
	// is why the decompiles show the ObjectInterface call directly.
	bool GetMember(const char* Name, GFxValue* Out) const;                                                         // 2012 0x5b69a0
	bool SetMember(const char* Name, const GFxValue& Value);                                                       // 2012 0x5b69d0
	bool Invoke(const char* Name, GFxValue* Result, const GFxValue* Args, UINT ArgCount);                          // 2012 0x5b6a00
	bool Invoke(const char* Name, GFxValue* Result);                                                               // 2012 0x5b6a40
	bool GotoAndPlay(const char* Frame);                                                                           // 2012 0x5b6a80
	bool GotoAndStop(const char* Frame);                                                                           // 2012 0x5b6aa0
	void VisitMembers(ObjectInterface::ObjVisitor* Visitor) const;                                                 // 2012 0x167f0

	UINT GetArraySize() const { return pObjectInterface ? pObjectInterface->GetArraySize(Value.pData) : 0; }
	bool GetElement(UINT Index, GFxValue* Out) const { return pObjectInterface ? pObjectInterface->GetElement(Value.pData, Index, Out) : false; }
	bool SetElement(UINT Index, const GFxValue& In) { return pObjectInterface ? pObjectInterface->SetElement(Value.pData, Index, In) : false; }
	bool PushBack(const GFxValue& In) { return pObjectInterface ? pObjectInterface->PushBack(Value.pData, In) : false; }
	bool GetText(GFxValue* Out) const { return pObjectInterface ? pObjectInterface->GetText(Value.pData, Out, false) : false; }
	bool SetText(const TCHAR* In) { return pObjectInterface ? pObjectInterface->SetText(Value.pData, In, false) : false; }
	bool SetTextHTML(const TCHAR* In) { return pObjectInterface ? pObjectInterface->SetText(Value.pData, In, true) : false; }
	bool GetDisplayInfo(DisplayInfo* Out) const { return pObjectInterface ? pObjectInterface->GetDisplayInfo(Value.pData, Out) : false; }
	bool SetDisplayInfo(const DisplayInfo& In) { return pObjectInterface ? pObjectInterface->SetDisplayInfo(Value.pData, In) : false; }
	bool GetDisplayMatrix(GMatrix2D* Out) const { return pObjectInterface ? pObjectInterface->GetDisplayMatrix(Value.pData, Out) : false; }
	bool SetDisplayMatrix(const GMatrix2D& In) { return pObjectInterface ? pObjectInterface->SetDisplayMatrix(Value.pData, In) : false; }
	bool SetMatrix3D(const GMatrix3D& In) { return pObjectInterface ? pObjectInterface->SetMatrix3D(Value.pData, In) : false; }
	bool GetCxform(GRenderer::Cxform* Out) const { return pObjectInterface ? pObjectInterface->GetCxform(Value.pData, Out) : false; }
	bool SetCxform(const GRenderer::Cxform& In) { return pObjectInterface ? pObjectInterface->SetCxform(Value.pData, In) : false; }
	bool GotoAndPlayFrame(UINT Frame) { return pObjectInterface ? pObjectInterface->GotoAndPlay(Value.pData, Frame, false) : false; }
	bool GotoAndStopFrame(UINT Frame) { return pObjectInterface ? pObjectInterface->GotoAndPlay(Value.pData, Frame, true) : false; }
	bool CreateEmptyMovieClip(GFxValue* Out, const char* InstanceName, INT Depth) { return pObjectInterface ? pObjectInterface->CreateEmptyMovieClip(Value.pData, Out, InstanceName, Depth) : false; }
	bool AttachMovie(GFxValue* Out, const char* SymbolName, const char* InstanceName, INT Depth) { return pObjectInterface ? pObjectInterface->AttachMovie(Value.pData, Out, SymbolName, InstanceName, Depth, NULL) : false; }

	/** the managed-value release every ported body performs before the value leaves scope */
	void ReleaseManaged()
	{
		if( IsManaged() && pObjectInterface )
		{
			pObjectInterface->ObjectRelease(this, Value.pData);
			pObjectInterface = NULL;
		}
	}
};

// GFxFunctionHandler (2012 all_types.h + its 2-slot vtable): the AS2 -> C++ closure the engine installs
// with GFxMovieView::CreateFunction, used by UGFxObject::SetFunction / execActionScriptSetFunction*.
class GFxFunctionHandler : public GRefCountImplCore
{
public:
	struct Params
	{
		GFxValue* pRetVal;
		GFxMovieView* pMovie;
		GFxValue* pThis;
		GFxValue* pArgsWithThisRef;
		GFxValue* pArgs;
		UINT ArgCount;
		void* pUserData;
	};

	virtual ~GFxFunctionHandler() {}
	virtual void Call(const Params& InParams) = 0;
};

// GFxState / GFxStateBag (2012 all_types.h). Only the state types the native layer installs are listed;
// the full enumeration is in all_types.h and belongs to agent BB's header.
class GFxState : public GRefCountImplCore
{
public:
	enum StateType
	{
		State_None              = 0,
		State_Translator        = 3,
		State_FSCommandHandler  = 8,
		State_ExternalInterface = 9,
		State_FileOpener        = 10,
	};
	virtual ~GFxState() {}
};

class GFxExternalInterface : public GFxState
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* MethodName, const GFxValue* Args, UINT ArgCount) = 0;
};

class GFxFSCommandHandler : public GFxState
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* Command, const char* Args) = 0;
};

// GFxMovieInfo (GFx 3.3 GFxLoader.h): what GFxLoader::GetMovieInfo (2013 0x9b3fe0) fills in.
struct GFxMovieInfo
{
	enum SWFFlagConstants
	{
		SWF_Compressed = 0x01,
		SWF_Stripped   = 0x10,
	};

	UINT Version;
	UINT Flags;
	INT Width;
	INT Height;
	FLOAT FPS;
	UINT FrameCount;
	UINT TagCount;
	WORD ExporterVersion;
	UINT ExporterFlags;

	GFxMovieInfo()
		: Version(0), Flags(0), Width(0), Height(0), FPS(0.f), FrameCount(0), TagCount(0)
		, ExporterVersion(0), ExporterFlags(0)
	{}
	UBOOL IsStripped() const { return (Flags & SWF_Stripped) != 0; }
};

// GFxMovie / GFxMovieView (2012 all_types.h GFxMovieView_vtbl, 73 slots). Only the slots the native layer
// calls are declared, in vtable order, because the layer dispatches virtually: adding or reordering a slot
// here would call the wrong function once BB's runtime is behind it. The two Invoke overloads sit in the
// table as [varargs, (GFxValue*,UINT)] and MSVC lays consecutive overloads out in REVERSE declaration
// order (agent AL's PhysX trap, PHASE8.md BB item 1), so they are declared the other way round.
class GFxMovie : public GRefCountImplCore
{
public:
	enum PlayState
	{
		Playing = 0,
		Stopped = 1,
	};
	enum SetVarType
	{
		SV_Normal    = 0,
		SV_Sticky    = 1,
		SV_Permanent = 2,
	};
	enum SetArrayType
	{
		SA_Int     = 0,
		SA_Double  = 1,
		SA_Float   = 2,
		SA_String  = 3,
		SA_StringW = 4,
		SA_Value   = 5,
	};

	virtual ~GFxMovie() {}
	virtual GFxMovieDef* GetMovieDef() = 0;
	virtual UINT GetCurrentFrame() = 0;
	virtual bool HasLooped() = 0;
	virtual void GotoFrame(UINT Frame) = 0;
	virtual bool GotoLabeledFrame(const char* Label, INT Offset) = 0;
	virtual void SetPlayState(PlayState State) = 0;
	virtual PlayState GetPlayState() = 0;
	virtual void SetVisible(bool bVisible) = 0;
	virtual bool GetVisible() = 0;
	virtual bool IsAvailable(const char* Path) = 0;
	virtual void CreateString(GFxValue* Out, const char* Text) = 0;
	virtual void CreateStringW(GFxValue* Out, const TCHAR* Text) = 0;
	virtual void CreateObject(GFxValue* Out, const char* ClassName = NULL, const GFxValue* Args = NULL, UINT ArgCount = 0) = 0;
	virtual void CreateArray(GFxValue* Out) = 0;
	virtual void CreateFunction(GFxValue* Out, GFxFunctionHandler* Handler, void* UserData = NULL) = 0;
	virtual bool SetVariable(const char* Path, const GFxValue& Value, SetVarType Type = SV_Sticky) = 0;
	virtual bool GetVariable(GFxValue* Out, const char* Path) = 0;
	virtual bool SetVariableArray(SetArrayType Type, const char* Path, UINT Index, const void* Data, UINT Count, SetVarType SetType = SV_Sticky) = 0;
	virtual bool SetVariableArraySize(const char* Path, UINT Count, SetVarType SetType = SV_Sticky) = 0;
	virtual UINT GetVariableArraySize(const char* Path) = 0;
	virtual bool GetVariableArray(SetArrayType Type, const char* Path, UINT Index, void* Data, UINT Count) = 0;
	virtual bool Invoke(const char* Path, GFxValue* Result, const GFxValue* Args, UINT ArgCount) = 0;
	virtual bool Invoke(const char* Path, GFxValue* Result, const char* ArgFormat, ...) = 0;
	virtual bool InvokeArgs(const char* Path, GFxValue* Result, const char* ArgFormat, char* Args) = 0;
};

class GFxStateBag
{
public:
	virtual GFxStateBag* GetStateBagImpl() = 0;
	virtual ~GFxStateBag() {}
	virtual void SetState(GFxState::StateType Type, GFxState* State) = 0;
	virtual GFxState* GetStateAddRef(GFxState::StateType Type) = 0;
	virtual void GetStatesAddRef(GFxState** States, const GFxState::StateType* Types, UINT Count) = 0;
};

class GFxMovieView : public GFxMovie, public GFxStateBag
{
public:
	enum ScaleModeType
	{
		SM_NoScale  = 0,
		SM_ShowAll  = 1,
		SM_ExactFit = 2,
		SM_NoBorder = 3,
	};
	// DISHONORED(layout): the 9-value viewport alignment, NOT the 4-value Left/Right/Center/Justify enum
	// IDA prints for GFxMovieView::AlignType in all_types.h - that one is the text alignment of
	// GFxTranslator::LineFormatDesc, which IDA's type dedup collapsed onto this name (the typedef line
	// beside it says so). The evidence for these nine: UGFxMoviePlayer::execSetAlignment (2012 0x5bdde0)
	// casts the script GFxAlign byte straight to GFxMovieView::AlignType, and script GFxAlign is
	// Align_Center .. Align_BottomRight in exactly this order (script_classes_2013.json).
	enum AlignType
	{
		Align_Center       = 0,
		Align_TopCenter    = 1,
		Align_BottomCenter = 2,
		Align_CenterLeft   = 3,
		Align_CenterRight  = 4,
		Align_TopLeft      = 5,
		Align_TopRight     = 6,
		Align_BottomLeft   = 7,
		Align_BottomRight  = 8,
	};
	enum HE_ReturnValueType
	{
		HE_NotHandled      = 0,
		HE_Handled         = 1,
		HE_NoDefaultAction = 2,
		HE_Completed       = 3,
	};

	virtual void SetViewport(const GViewport& Viewport) = 0;
	virtual void GetViewport(GViewport* Out) = 0;
	virtual void SetViewScaleMode(ScaleModeType Mode) = 0;
	virtual ScaleModeType GetViewScaleMode() = 0;
	virtual void SetViewAlignment(AlignType Align) = 0;
	virtual AlignType GetViewAlignment() = 0;
	virtual void GetVisibleFrameRect(FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY) = 0;
	virtual void SetPerspective3D(const GMatrix3D& Matrix) = 0;
	virtual void SetView3D(const GMatrix3D& Matrix) = 0;
	virtual void GetSafeRect(FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY) = 0;
	virtual void SetSafeRect(FLOAT MinX, FLOAT MinY, FLOAT MaxX, FLOAT MaxY) = 0;
	virtual void Restart() = 0;
	virtual FLOAT Advance(FLOAT DeltaTime, UINT FrameCatchUpCount) = 0;
	virtual void Display() = 0;
	virtual void DisplayPrePass() = 0;
	virtual void SetPause(bool bPause) = 0;
	virtual bool IsPaused() = 0;
	virtual void SetBackgroundColor(DWORD Color) = 0;
	virtual void SetBackgroundAlpha(FLOAT Alpha) = 0;
	virtual FLOAT GetBackgroundAlpha() = 0;
	virtual UINT HandleEvent(const GFxEvent& Event) = 0;
	virtual void GetMouseState(UINT MouseIndex, FLOAT* X, FLOAT* Y, UINT* Buttons) = 0;
	virtual void NotifyMouseState(FLOAT X, FLOAT Y, UINT Buttons, UINT MouseIndex) = 0;
	virtual bool HitTest(FLOAT X, FLOAT Y, HE_ReturnValueType TestType, UINT ControllerIndex) = 0;
	virtual bool HitTest3D(void* Point, FLOAT X, FLOAT Y, UINT ControllerIndex) = 0;
	virtual void SetExternalInterfaceRetVal(const GFxValue& Value) = 0;
	virtual void* GetUserData() = 0;
	virtual void SetUserData(void* UserData) = 0;
	virtual bool AttachDisplayCallback(const char* Path, void (*Callback)(void*), void* UserData) = 0;
	virtual bool IsMovieFocused() = 0;
	virtual bool GetDirtyFlag(bool bClear) = 0;
	virtual void SetMouseCursorCount(UINT Count) = 0;
	virtual UINT GetMouseCursorCount() = 0;
	virtual void SetControllerCount(UINT Count) = 0;
	virtual UINT GetControllerCount() = 0;
	virtual void GetStats(GStatBag* Bag, bool bReset) = 0;
	virtual GMemoryHeap* GetHeap() = 0;
	virtual void ForceCollectGarbage() = 0;
};

// GFxMovieDef (2012 all_types.h GFxMovieDef_vtbl, 29 slots) - only the queries the native layer and its
// test harness read.
class GFxMovieDef : public GRefCountImplCore
{
public:
	virtual ~GFxMovieDef() {}
	virtual UINT GetVersion() = 0;
	virtual UINT GetLoadingFrame() = 0;
	virtual FLOAT GetWidth() = 0;
	virtual FLOAT GetHeight() = 0;
	virtual UINT GetFrameCount() = 0;
	virtual FLOAT GetFrameRate() = 0;
	virtual UINT GetSWFFlags() = 0;
	virtual const char* GetFileURL() = 0;
	virtual GFxMovieView* CreateInstance(bool bInitFirstFrame) = 0;
};

#pragma pack(pop)

#endif // !DISHONORED_GFXUI_GFX3_RUNTIME

// The GFxUI script enums this layer uses - ASType (AS_Undefined .. AS_Boolean), GFxScaleMode, GFxAlign,
// GFxRenderTextureMode, GFxTimingMode - are NOT declared here: gen_classes_header.py --sdk already emits
// them into Inc/GFxUIEngineShims.h with the retail 2013 values. Worth stating because the reference GFx 4
// GFxUI's ASType inserts AS_Int at 3 and shifts AS_String / AS_Boolean to 4 / 5, and its ASValue carries an
// extra int member; taking the wrong one silently swaps strings and booleans in every ASValue that crosses
// the boundary (UGFxMoviePlayer::GetVariable, 2012 0x5c0b70, maps VT_Boolean -> 4 and VT_String -> 3).

// ---------------------------------------------------------------------------------------------------
// The engine-side seam. These are OUR types (module gfxui in the PDB), not GFx's, and they are shared
// between this native layer and the runtime glue: retail declares them in gfxui/inc/gfxuiengine.h, which
// is agent BB's file, so when the runtime is present they come from there and none of the declarations
// below is made. Until then this is the layer's single source of them. Division of labour: BB owns
// FGFxEngine's body, the renderer and the loader; this package owns the two Callbacks, the two property
// converters, FAutoGFxValueArray and every consumer of FGFxMovie.
// ---------------------------------------------------------------------------------------------------

#if DISHONORED_GFXUI_GFX3_RUNTIME

#include "gfxuiengine.h"

#else // !DISHONORED_GFXUI_GFX3_RUNTIME

class UGFxMoviePlayer;
class UGFxObject;
class UTranslationContext;

// FGFxMovie (2012 all_types.h): the movie handle UGFxMoviePlayer::pMovie points at. Member order and
// therefore offsets are the PDB's.
struct FGFxMovie
{
	FString FileName;
	GFxMovieInfo Info;
	GPtr<GFxMovieDef> pDef;
	GPtr<GFxMovieView> pView;
	DOUBLE LastTime;
	UBOOL Playing;
	UBOOL fVisible;
	UBOOL fUpdate;
	UBOOL fViewportSet;
	UBOOL bCanReceiveFocus;
	UBOOL bCanReceiveInput;
	INT TimingMode;
	UGFxMoviePlayer* pUMovie;
	UTextureRenderTarget2D* pRenderTexture;
	FRenderCommandFence RenderCmdFence;

	FGFxMovie();                                                                                                   // 2012 0x5bf9c0
};

// FAutoGFxValueArray (2012 0x5b6ac0 / 0x5bf4b0, gfxuiengine.h): the stack-allocated GFxValue run the
// Invoke paths build their argument list in. The retail body places the values in caller stack memory
// through appAlloca; ours keeps the same contract - construct N values, destruct them all at scope end.
class FAutoGFxValueArray
{
public:
	FAutoGFxValueArray(UINT InCount, void* InMemory);
	~FAutoGFxValueArray();

	GFxValue& operator()(UINT Index) { return pValues[Index]; }
	GFxValue* GetValues() const { return pValues; }
	operator GFxValue*() const { return pValues; }

private:
	GFxValue* pValues;
	UINT Count;
};

/** the retail macro of gfxuiengine.h: N GFxValues in the caller's frame, constructed and destructed */
#define AutoGFxValueArray(Name,NumValues) \
	FAutoGFxValueArray Name((NumValues), (NumValues) ? appAlloca(sizeof(GFxValue) * (NumValues)) : NULL)

// FGFxEngine: agent BB's class. Only the members and statics this native layer calls are declared; the
// definition stays in the runtime glue. GGFxEngine is NULL in a build without the runtime, which is what
// every ported body checks before it touches a movie.
class FGFxEngine
{
public:
	static void ConvertUPropToGFx(UProperty* Property, BYTE* Address, GFxValue& Value, GFxMovieView* Movie, bool bOverwrite = false);   // 2013 0x58c8f0 (2012 0x5e4080)
	static void ConvertGFxToUProp(UProperty* Property, BYTE* Address, const GFxValue& Value, UGFxMoviePlayer* Movie);                   // 2012 0x41630
	static INT ReplaceCharsInFString(FString& Text, const TCHAR* Chars, TCHAR Replacement);                                            // 2012 0x5b96d0

	/** the process-wide engine, created on demand by the runtime glue; NULL when no runtime is linked */
	static FGFxEngine* GetEngine();                                                                                                     // 2012 0x5e3450

	FGFxMovie* LoadMovie(const TCHAR* Filename, UBOOL bInitFirstFrame);                                                                 // 2012 0x5de3e0
	void StartScene(FGFxMovie* pMovie, UTextureRenderTarget2D* pRenderTexture, UBOOL bCaptureInput, UBOOL bPlay);                       // 2013 0x58e300 (2012 0x5cf0d0)
	void CloseScene(FGFxMovie* pMovie, UBOOL bUnload);                                                                                  // 2012 0x5ce890
	void InsertMovie(FGFxMovie* pMovie, BYTE Priority);                                                                                 // 2012 0x5ce820
	void FlushPlayerInput(TSet<INT>* pCaptureKeys);                                                                                     // 2012 0x5da330
	FGFxMovie* GetTopmostMovie() const;                                                                                                 // 2012 0x5bfb90
	void NotifyGameSessionEnded();                                                                                                      // 2012 0x5d49b0
	void ReevaluateFocus();                                                                                                             // 2012 0x5ca660
	void CloseAllMovies(INT LocalPlayerIndex);                                                                                          // 2012 0x5d17f0
	FGFxMovie* GetFocusedMovieFromControllerID(INT ControllerId);                                                                       // 2012 0x5bfec0
};

/** the process-wide GFx engine; NULL when no runtime is linked (Src/gfxuigfx3absent.cpp) */
extern FGFxEngine* GGFxEngine;

// FGFxExternalInterface / FGFxFSCommandHandler (gfxuiengine.cpp:499 / :444): AS2 -> UnrealScript. The
// bodies are this package's, in Src/gfxuiexternalinterface.cpp.
class FGFxExternalInterface : public GFxExternalInterface
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* MethodName, const GFxValue* Args, UINT ArgCount);        // 2013 0x58d510 (2012 0x5e4880)
};

class FGFxFSCommandHandler : public GFxFSCommandHandler
{
public:
	virtual void Callback(GFxMovieView* pMovie, const char* Command, const char* Args);                              // 2013 0x586450 (2012 0x5e3190)
};

#endif // !DISHONORED_GFXUI_GFX3_RUNTIME

#if !DISHONORED_GFXUI_GFX3_RUNTIME
// DISHONORED(layout): the 2012 PDB sizes of the types this header declares itself. When BB's headers take
// over they carry their own assertions against the same PDB, so the two cannot silently disagree.
static_assert(sizeof(GFxValue) == 16, "GFxValue: 2012 PDB size is 16");
static_assert(sizeof(GMatrix2D) == 24, "GMatrix2D: 2012 PDB size is 24");
static_assert(sizeof(GMatrix3D) == 64, "GMatrix3D: 2012 PDB size is 64");
static_assert(sizeof(GRenderer::Cxform) == 32, "GRenderer::Cxform: 2012 PDB size is 32");
static_assert(sizeof(GViewport) == 52, "GViewport: 2012 PDB size is 52");
static_assert(sizeof(GFxMovieInfo) == 36, "GFxMovieInfo: 2012 PDB size is 36");
static_assert(sizeof(GFxValue::DisplayInfo) == 232, "GFxValue::DisplayInfo: 2012 PDB size is 232");
#endif

#endif // _INC_GFXUI_GFX3
