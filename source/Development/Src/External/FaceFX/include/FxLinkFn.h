// FaceFX 1.7.3.1: link functions (fxlinkfn.h). Agent FX2, wave 26.
// Every concrete link function repeats the parameter block (FX_LINKFN_PARAMETERS, fxlinkfn.h:114): it shadows the base
// FxLinkFn's _fnDesc/_parameters and overrides their accessors, which is why the 2012 classes are 0x3C bytes over a
// 0x24-byte base. The macro name is ours (the PDB keeps only its line).
#ifndef FxLinkFn_H__
#define FxLinkFn_H__

#include "FxFaceGraphPrerequisites.h"

namespace OC3Ent
{
namespace Face
{

enum FxLinkFnType
{
	LFT_Invalid = -1,
	LFT_First = 0,
	LFT_Null = LFT_First,
	LFT_Linear,
	LFT_Quadratic,
	LFT_Cubic,
	LFT_Sqrt,
	LFT_Negate,
	LFT_Inverse,
	LFT_OneClamp,
	LFT_Constant,
	LFT_Corrective,
	LFT_ClampedLinear,
	LFT_Last = LFT_ClampedLinear,
	LFT_NumLinkFnTypes
};

struct FxLinkFnParameterDesc
{
	FxLinkFnParameterDesc();
	FxLinkFnParameterDesc( const FxChar* name, FxReal defaultValue );

	FxName parameterName;
	FxReal parameterDefaultValue;
};

struct FxLinkFnParameters
{
	FxArray<FxReal> parameters;
};

FxArchive& operator<<( FxArchive& arc, FxLinkFnParameters& linkFnParams );

#define FX_LINKFN_PARAMETERS \
public: \
	virtual const FxString& GetFnDesc() const { return _fnDesc; } \
	virtual FxSize GetNumParams() const { return _parameters.Length(); } \
	virtual const FxString& GetParamName( FxSize index ) const { return _parameters[index].parameterName.GetAsString(); } \
	virtual FxReal GetParamDefaultValue( FxSize index ) const { return _parameters[index].parameterDefaultValue; } \
protected: \
	FxString _fnDesc; \
	FxArray<FxLinkFnParameterDesc> _parameters;

class FxLinkFn : public FxNamedObject
{
	FX_DECLARE_ABSTRACT_CLASS(FxLinkFn, FxNamedObject)
	FX_LINKFN_PARAMETERS
public:
	virtual void Serialize( FxArchive& ) {}
	FxLinkFn();
	virtual ~FxLinkFn();

	virtual FxLinkFnType GetType() const = 0;
	FxBool IsCorrective() const { return _isCorrective; }

	static void Startup();
	static void Shutdown();
	static FxSize GetNumLinkFunctions();
	static const FxLinkFn* GetLinkFunction( FxSize index );
	static const FxLinkFn* FindLinkFunction( const FxName& name );
	static const FxLinkFn* FindLinkFunctionByType( FxLinkFnType lfType );
	static FxLinkFnType FindLinkFunctionType( const FxName& name );
	static void AddLinkFunction( FxLinkFn* pLinkFn );

protected:
	FxBool _isCorrective;

	static FxArray<FxLinkFn*>* _linkFnTable;
};

#define FX_DECLARE_LINKFN(klass, type) \
class klass : public FxLinkFn \
{ \
	FX_DECLARE_ABSTRACT_CLASS(klass, FxLinkFn) \
	FX_LINKFN_PARAMETERS \
public: \
	klass(); \
	virtual ~klass(); \
	virtual FxLinkFnType GetType() const { return type; } \
};

FX_DECLARE_LINKFN(FxLinearLinkFn, LFT_Linear)
FX_DECLARE_LINKFN(FxQuadraticLinkFn, LFT_Quadratic)
FX_DECLARE_LINKFN(FxCubicLinkFn, LFT_Cubic)
FX_DECLARE_LINKFN(FxSqrtLinkFn, LFT_Sqrt)
FX_DECLARE_LINKFN(FxNegateLinkFn, LFT_Negate)
FX_DECLARE_LINKFN(FxInverseLinkFn, LFT_Inverse)
FX_DECLARE_LINKFN(FxOneClampLinkFn, LFT_OneClamp)
FX_DECLARE_LINKFN(FxConstantLinkFn, LFT_Constant)
FX_DECLARE_LINKFN(FxCorrectiveLinkFn, LFT_Corrective)
FX_DECLARE_LINKFN(FxClampedLinearLinkFn, LFT_ClampedLinear)

class FxNullLinkFn : public FxLinkFn
{
	FX_DECLARE_ABSTRACT_CLASS(FxNullLinkFn, FxLinkFn)
	FX_LINKFN_PARAMETERS
public:
	FxNullLinkFn( const FxChar* name );
	virtual ~FxNullLinkFn();
	/** 2012 0xb287f0. */
	virtual FxLinkFnType GetType() const { return LFT_Null; }
};

/** 2012 0xb12500 (fxmath.h:69): the clamped Hermite ease (no slopes) the register interpolation uses. */
inline FxReal FxHermiteInterpolate( FxReal start, FxReal end, FxReal alpha )
{
	FxReal valMin = 0.0f;
	FxReal valMax = 1.0f;
	FxReal a = FxClamp(valMin, alpha, valMax);
	return ((start * 2.0f - end * 2.0f) * a + start * -3.0f + end * 3.0f) * a * a + start;
}

/** 2012 0xb12570 (fxlinkfn.h:434): the compiled graph's link transfer function. */
inline FxReal FxLinkFnEval( FxLinkFnType lfType, FxReal x, const FxLinkFnParameters& params )
{
	const FxArray<FxReal>& p = params.parameters;
	switch( lfType )
	{
	case LFT_Null:
		return 0.0f;
	case LFT_Linear:
		if( p.Length() == 2 )
		{
			return x * p[0] + p[1];
		}
		if( p.Length() == 1 )
		{
			return x * p[0];
		}
		return x;
	case LFT_Quadratic:
		if( p.Length() == 1 )
		{
			return p[0] * (x * x * (x >= 0.0f ? 1.0f : -1.0f));
		}
		return x * x * (x >= 0.0f ? 1.0f : -1.0f);
	case LFT_Cubic:
		if( p.Length() == 1 )
		{
			return p[0] * (x * x * x);
		}
		return x * x * x;
	case LFT_Sqrt:
		if( x == 0.0f )
		{
			return 0.0f;
		}
		if( p.Length() == 1 )
		{
			return p[0] * (FxSqrt(FxAbs(x)) * (x >= 0.0f ? 1.0f : -1.0f));
		}
		return FxSqrt(FxAbs(x)) * (x >= 0.0f ? 1.0f : -1.0f);
	case LFT_Negate:
		return -x;
	case LFT_Inverse:
		if( FxRealEquality(x, 0.0f) )
		{
			return 0.0f;
		}
		return 1.0f / x;
	case LFT_OneClamp:
		return x > 1.0f ? 1.0f / x : 1.0f;
	case LFT_Constant:
		return p.Length() ? p[0] : 1.0f;
	case LFT_Corrective:
		return p.Length() ? p[0] : 0.0f;
	case LFT_ClampedLinear:
		if( p.Length() == 4 )
		{
			FxBool linearAbove = p[3] > 0.0f && p[1] < x;
			FxBool linearBelow = p[3] <= 0.0f && p[1] > x;
			if( !linearAbove && !linearBelow )
			{
				return p[2];
			}
			return p[2] + p[1] * (p[0] * -1.0f) + x * p[0];
		}
		if( p.Length() == 1 )
		{
			return x <= 0.0f ? 0.0f : x * p[0];
		}
		return x;
	default:
		return 0.0f;
	}
}

} // namespace Face
} // namespace OC3Ent

#endif
