// Scaleform GFx 3.3.89 - GFxValue and its object interface, reconstructed from the 2012 Shipping PDB.
// DISHONORED(layout): GFxValue::ObjectInterface 2012 rvas in the comment on each method; the 2013
// bodies are byte-identical (resources/docs/gfx_decision.md 1).
//
// This is the most important type in the whole binding: 1,096 of the 1,350 calls the engine makes
// into libgfx (81 %) are GFxValue / GFxValue::ObjectInterface calls, because the engine's entire
// relationship with the UI is "poke values into ActionScript objects and call ActionScript methods"
// (gfx_decision.md 2.2). Hand-written rather than generated because the accessors are header-inline
// in the SDK and only the ObjectInterface members are real functions.
//
// Layout (PDB): GFxValue is 16 bytes - ObjectInterface* pObjectInterface @0, ValueType Type @4,
// ValueUnion Value @8 (8 bytes, a double). ObjectInterface is 4 bytes - GFxMovieRoot* pMovieRoot @0,
// non-virtual, its methods dispatched statically. DisplayInfo is 232 bytes and is 8-aligned, which
// is what proves GFx was not compiled with UE3's /Zp4.
#ifndef INC_GFX3_GFXVALUE_H
#define INC_GFX3_GFXVALUE_H

#include "GFx3Gen.h"

#pragma pack(push, 8)

class GFxMovieRoot;
class GFxASUserData;

class GFxValue
{
public:
    // Both enums are the PDB's, value for value (GFxValue::ValueType and GFxValue::ValueTypeControl).
    // Note the control bits: the convert bit is 0x80 and the managed bit is 0x40, so the type mask is
    // 0x8F and a convertible type is the plain type with 0x80 set - VT_ConvertNumber is 0x83, not
    // 0x0B. Every retail body masks with 0x8F and tests the managed bit with 0x40; agent BE spotted
    // this against the retail decompiles while these were still hand-guessed here.
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

        VT_ConvertBoolean = 0x82,
        VT_ConvertNumber  = 0x83,
        VT_ConvertString  = 0x84,
        VT_ConvertStringW = 0x85
    };
    enum ValueTypeControl
    {
        VTC_ConvertBit    = 0x80,
        VTC_ManagedBit    = 0x40,
        VTC_TypeMask      = 0x8F
    };

    // GFxValue::DisplayInfo - the _x/_y/_rotation/_alpha/... bundle SetDisplayInfo takes.
    // PDB sizeof 232; VarsSet is the bitmask of which fields the caller filled in.
    class DisplayInfo
    {
    public:
        // The PDB's set, exactly: it ends at V_viewMatrix3D and there is no V_projMatrix3D.
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
            V_viewMatrix3D   = 0x2000
        };

        double         X;
        double         Y;
        double         Rotation;
        double         XScale;
        double         YScale;
        double         Alpha;
        bool           Visible;
        GUByte         _pad49[7];
        double         Z;
        double         XRotation;
        double         YRotation;
        double         ZScale;
        double         PerspFOV;
        GMatrix3D      ViewMatrix3D;
        GMatrix3D      PerspectiveMatrix3D;
        unsigned short VarsSet;
        GUByte         _pad226[6];

        DisplayInfo() : VarsSet(0) {}

        void Clear() { VarsSet = 0; }
        void SetX(double x) { X = x; VarsSet |= V_x; }
        void SetY(double y) { Y = y; VarsSet |= V_y; }
        void SetPosition(double x, double y) { SetX(x); SetY(y); }
        void SetRotation(double d) { Rotation = d; VarsSet |= V_rotation; }
        void SetXScale(double s) { XScale = s; VarsSet |= V_xscale; }
        void SetYScale(double s) { YScale = s; VarsSet |= V_yscale; }
        void SetScale(double x, double y) { SetXScale(x); SetYScale(y); }
        void SetAlpha(double a) { Alpha = a; VarsSet |= V_alpha; }
        void SetVisible(bool v) { Visible = v; VarsSet |= V_visible; }
        void SetZ(double z) { Z = z; VarsSet |= V_z; }
        void SetXRotation(double d) { XRotation = d; VarsSet |= V_xrotation; }
        void SetYRotation(double d) { YRotation = d; VarsSet |= V_yrotation; }
        void SetZScale(double s) { ZScale = s; VarsSet |= V_zscale; }
        void SetFOV(double f) { PerspFOV = f; VarsSet |= V_perspFOV; }

        bool IsFlagSet(unsigned f) const { return (VarsSet & f) != 0; }
        double GetX() const { return X; }
        double GetY() const { return Y; }
        double GetRotation() const { return Rotation; }
        double GetXScale() const { return XScale; }
        double GetYScale() const { return YScale; }
        double GetAlpha() const { return Alpha; }
        bool   GetVisible() const { return Visible; }

        // 2012 rva 0x9ad320
        void Set(double x, double y, double rot, double xs, double ys, double alpha, bool visible,
                 double z, double xrot, double yrot, double zscale, double fov);
    };

    // GFxValue::ObjectInterface - the live bridge to an ActionScript object. Non-virtual: every
    // method is a direct call in the retail exe and dispatches on pMovieRoot.
    class ObjectInterface
    {
    public:
        GFxMovieRoot* pMovieRoot;

        // The PDB spells both Visit parameters as `const GFxValue&`; a reference and a pointer are
        // the same thing at the ABI level, so a caller written against either works.
        class ObjVisitor
        {
        public:
            virtual ~ObjVisitor() {}                                              // vt[0]
            virtual void Visit(const char* name, const GFxValue& val) = 0;         // vt[1]
        };
        class ArrVisitor
        {
        public:
            virtual ~ArrVisitor() {}                                              // vt[0]
            virtual void Visit(unsigned idx, const GFxValue& val) = 0;             // vt[1]
        };

        ObjectInterface(GFxMovieRoot* root) : pMovieRoot(root) {}

        // The rva comment on a method means the 2012 exe compiles a function for it. Five of
        // them carry no rva - HasMember, DeleteMember, SetArraySize, VisitElements,
        // RemoveElements - because no call site in Dishonored reaches them, so the linker kept
        // no body. They are part of the 3.3 interface and are declared for completeness;
        // package BC implements them like the rest.

        void ObjectAddRef(GFxValue* val, void* obj);                               // 2012 0x9ae650
        void ObjectRelease(GFxValue* val, void* obj);                              // 2012 0x9aed40

        bool HasMember(void* obj, const char* name, bool isDObj) const;
        bool GetMember(void* obj, const char* name, GFxValue* val, bool isDObj) const; // 0x9b0450
        bool SetMember(void* obj, const char* name, const GFxValue& val, bool isDObj);  // 0x9ad590
        bool Invoke(void* obj, GFxValue* result, const char* name, const GFxValue* args,
                    unsigned nargs, bool isDObj);                                  // 2012 0x9afbb0
        bool DeleteMember(void* obj, const char* name, bool isDObj);
        void VisitMembers(void* obj, ObjVisitor* visitor, bool isDObj) const;       // 2012 0x9ae6c0

        unsigned GetArraySize(void* obj) const;                                    // 2012 0x9ad650
        bool SetArraySize(void* obj, unsigned sz);
        bool GetElement(void* obj, unsigned idx, GFxValue* val) const;             // 2012 0x9b05c0
        bool SetElement(void* obj, unsigned idx, const GFxValue& val);             // 2012 0x9ad670
        bool VisitElements(void* obj, ArrVisitor* visitor, unsigned idx, int count) const;
        bool PushBack(void* obj, const GFxValue& val);                             // 2012 0x9ad6c0
        bool RemoveElements(void* obj, unsigned idx, int count);

        bool GetText(void* obj, GFxValue* val, bool html) const;                   // 2012 0x9b0640
        bool SetText(void* obj, const char* text, bool html);                      // 2012 0x9afdb0
        bool SetText(void* obj, const wchar_t* text, bool html);                    // 2012 0x9afee0

        bool GetDisplayInfo(void* obj, DisplayInfo* info) const;                   // 2012 0x9ad900
        bool SetDisplayInfo(void* obj, const DisplayInfo& info);                   // 2012 0x9adb20
        bool GetDisplayMatrix(void* obj, GMatrix2D* mat) const;                    // 2012 0x9ad710
        bool SetDisplayMatrix(void* obj, const GMatrix2D& mat);                    // 2012 0x9ad7a0
        bool SetMatrix3D(void* obj, const GMatrix3D& mat);                         // 2012 0x9acd30
        bool GetCxform(void* obj, GRenderer::Cxform* cx) const;                    // 2012 0x9ad130
        bool SetCxform(void* obj, const GRenderer::Cxform& cx);                    // 2012 0x9ad160

        bool GotoAndPlay(void* obj, const char* frame, bool stop);                 // 2012 0x9ad050
        bool GotoAndPlay(void* obj, unsigned frame, bool stop);                    // 2012 0x9ad0d0

        bool CreateEmptyMovieClip(void* obj, GFxValue* mc, const char* instanceName,
                                  int depth);                                      // 2012 0x9af6a0
        bool AttachMovie(void* obj, GFxValue* mc, const char* symbolName,
                         const char* instanceName, int depth,
                         const GFxValue* initArgs);                                // 2012 0x9af830
    };

    class ValueUnion
    {
    public:
        union
        {
            double          NValue;
            bool            BValue;
            const char*     pString;
            const char**    pStringManaged;
            const wchar_t*  pStringW;
            void*           pData;
        };
    };

    ObjectInterface* pObjectInterface;
    ValueType        Type;
    ValueUnion       Value;

    GFxValue() : pObjectInterface(0), Type(VT_Undefined) { Value.NValue = 0.0; }
    GFxValue(ValueType type) : pObjectInterface(0), Type(type) { Value.NValue = 0.0; }
    GFxValue(double v) : pObjectInterface(0), Type(VT_Number) { Value.NValue = v; }
    GFxValue(bool v) : pObjectInterface(0), Type(VT_Boolean) { Value.BValue = v; }
    GFxValue(const char* v) : pObjectInterface(0), Type(VT_String) { Value.pString = v; }
    GFxValue(const wchar_t* v) : pObjectInterface(0), Type(VT_StringW) { Value.pStringW = v; }

    GFxValue(const GFxValue& src)
        : pObjectInterface(src.pObjectInterface), Type(src.Type)
    {
        Value = src.Value;
        if (IsManagedValue()) AcquireManagedValue(src);
    }
    ~GFxValue() { if (IsManagedValue()) ReleaseManagedValue(); }

    GFxValue& operator=(const GFxValue& src)
    {
        if (this == &src) return *this;
        if (IsManagedValue()) ReleaseManagedValue();
        pObjectInterface = src.pObjectInterface;
        Type = src.Type;
        Value = src.Value;
        if (IsManagedValue()) AcquireManagedValue(src);
        return *this;
    }

    ValueType GetType() const { return (ValueType)(Type & VTC_TypeMask); }
    bool IsUndefined() const { return GetType() == VT_Undefined; }
    bool IsNull() const { return GetType() == VT_Null; }
    bool IsBool() const { return GetType() == VT_Boolean; }
    bool IsNumber() const { return GetType() == VT_Number; }
    bool IsString() const { return GetType() == VT_String; }
    bool IsStringW() const { return GetType() == VT_StringW; }
    bool IsObject() const
    {
        ValueType t = GetType();
        return t == VT_Object || t == VT_Array || t == VT_DisplayObject;
    }
    bool IsArray() const { return GetType() == VT_Array; }
    bool IsDisplayObject() const { return GetType() == VT_DisplayObject; }
    bool IsManagedValue() const { return (Type & VTC_ManagedBit) != 0; }
    bool IsConvertibleType() const { return (Type & VTC_ConvertBit) != 0; }

    bool        GetBool() const { return Value.BValue; }
    double      GetNumber() const { return Value.NValue; }
    const char* GetString() const
    {
        return IsManagedValue() ? *Value.pStringManaged : Value.pString;
    }
    const wchar_t* GetStringW() const { return Value.pStringW; }

    void SetUndefined() { ChangeType(VT_Undefined); }
    void SetNull() { ChangeType(VT_Null); }
    void SetBoolean(bool v) { ChangeType(VT_Boolean); Value.BValue = v; }
    void SetNumber(double v) { ChangeType(VT_Number); Value.NValue = v; }
    void SetString(const char* v) { ChangeType(VT_String); Value.pString = v; }
    void SetStringW(const wchar_t* v) { ChangeType(VT_StringW); Value.pStringW = v; }

    // The object-side API: every one of these forwards to the ObjectInterface the value was
    // created by. `mIsDisplayObj` selects the display-object path in the runtime.
    bool HasMember(const char* name) const
    {
        return IsObject() && pObjectInterface->HasMember(Value.pData, name, IsDisplayObject());
    }
    bool GetMember(const char* name, GFxValue* val) const
    {
        return IsObject() && pObjectInterface->GetMember(Value.pData, name, val, IsDisplayObject());
    }
    bool SetMember(const char* name, const GFxValue& val)
    {
        return IsObject() && pObjectInterface->SetMember(Value.pData, name, val, IsDisplayObject());
    }
    bool DeleteMember(const char* name)
    {
        return IsObject() && pObjectInterface->DeleteMember(Value.pData, name, IsDisplayObject());
    }
    void VisitMembers(ObjectInterface::ObjVisitor* v) const
    {
        if (IsObject()) pObjectInterface->VisitMembers(Value.pData, v, IsDisplayObject());
    }
    bool Invoke(const char* name, GFxValue* result, const GFxValue* args, unsigned nargs)
    {
        return IsObject()
            && pObjectInterface->Invoke(Value.pData, result, name, args, nargs, IsDisplayObject());
    }
    bool Invoke(const char* name, GFxValue* result = 0) { return Invoke(name, result, 0, 0); }

    unsigned GetArraySize() const
    {
        return IsArray() ? pObjectInterface->GetArraySize(Value.pData) : 0;
    }
    bool SetArraySize(unsigned sz)
    {
        return IsArray() && pObjectInterface->SetArraySize(Value.pData, sz);
    }
    bool GetElement(unsigned idx, GFxValue* val) const
    {
        return IsArray() && pObjectInterface->GetElement(Value.pData, idx, val);
    }
    bool SetElement(unsigned idx, const GFxValue& val)
    {
        return IsArray() && pObjectInterface->SetElement(Value.pData, idx, val);
    }
    bool PushBack(const GFxValue& val)
    {
        return IsArray() && pObjectInterface->PushBack(Value.pData, val);
    }
    bool RemoveElements(unsigned idx, int count = -1)
    {
        return IsArray() && pObjectInterface->RemoveElements(Value.pData, idx, count);
    }
    bool ClearElements() { return RemoveElements(0, -1); }

    bool GetText(GFxValue* val) const
    {
        return IsDisplayObject() && pObjectInterface->GetText(Value.pData, val, false);
    }
    bool SetText(const char* text)
    {
        return IsDisplayObject() && pObjectInterface->SetText(Value.pData, text, false);
    }
    bool SetText(const wchar_t* text)
    {
        return IsDisplayObject() && pObjectInterface->SetText(Value.pData, text, false);
    }
    bool SetTextHTML(const char* text)
    {
        return IsDisplayObject() && pObjectInterface->SetText(Value.pData, text, true);
    }

    bool GetDisplayInfo(DisplayInfo* info) const
    {
        return IsDisplayObject() && pObjectInterface->GetDisplayInfo(Value.pData, info);
    }
    bool SetDisplayInfo(const DisplayInfo& info)
    {
        return IsDisplayObject() && pObjectInterface->SetDisplayInfo(Value.pData, info);
    }
    bool GetDisplayMatrix(GMatrix2D* mat) const
    {
        return IsDisplayObject() && pObjectInterface->GetDisplayMatrix(Value.pData, mat);
    }
    bool SetDisplayMatrix(const GMatrix2D& mat)
    {
        return IsDisplayObject() && pObjectInterface->SetDisplayMatrix(Value.pData, mat);
    }
    bool SetMatrix3D(const GMatrix3D& mat)
    {
        return IsDisplayObject() && pObjectInterface->SetMatrix3D(Value.pData, mat);
    }
    bool GetColorTransform(GRenderer::Cxform* cx) const
    {
        return IsDisplayObject() && pObjectInterface->GetCxform(Value.pData, cx);
    }
    bool SetColorTransform(const GRenderer::Cxform& cx)
    {
        return IsDisplayObject() && pObjectInterface->SetCxform(Value.pData, cx);
    }
    bool GotoAndPlay(const char* frame)
    {
        return IsDisplayObject() && pObjectInterface->GotoAndPlay(Value.pData, frame, false);
    }
    bool GotoAndStop(const char* frame)
    {
        return IsDisplayObject() && pObjectInterface->GotoAndPlay(Value.pData, frame, true);
    }
    bool GotoAndPlay(unsigned frame)
    {
        return IsDisplayObject() && pObjectInterface->GotoAndPlay(Value.pData, frame, false);
    }
    bool GotoAndStop(unsigned frame)
    {
        return IsDisplayObject() && pObjectInterface->GotoAndPlay(Value.pData, frame, true);
    }
    bool AttachMovie(GFxValue* mc, const char* symbolName, const char* instanceName,
                     int depth = -1, const GFxValue* initArgs = 0)
    {
        return IsDisplayObject()
            && pObjectInterface->AttachMovie(Value.pData, mc, symbolName, instanceName, depth,
                                             initArgs);
    }
    bool CreateEmptyMovieClip(GFxValue* mc, const char* instanceName, int depth = -1)
    {
        return IsDisplayObject()
            && pObjectInterface->CreateEmptyMovieClip(Value.pData, mc, instanceName, depth);
    }

private:
    void ChangeType(ValueType t)
    {
        if (IsManagedValue()) ReleaseManagedValue();
        Type = t;
    }
    void AcquireManagedValue(const GFxValue& src)
    {
        pObjectInterface->ObjectAddRef(this, src.Value.pData);
    }
    void ReleaseManagedValue()
    {
        pObjectInterface->ObjectRelease(this, Value.pData);
        pObjectInterface = 0;
        Type = VT_Undefined;
    }
};

#pragma pack(pop)
#endif // INC_GFX3_GFXVALUE_H
