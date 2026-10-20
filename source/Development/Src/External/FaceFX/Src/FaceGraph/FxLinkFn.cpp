// FaceFX 1.7.3.1: link functions (fxlinkfn.cpp). Agent FX2, wave 26, ported from the 2012 decompile.
// Startup registers 12 functions in this order: linear, quadratic, cubic, square root, negate, inverse, oneclamp,
// constant, corrective, "custom" and "perlin noise" (both FxNullLinkFn, deprecated), clamped linear. No "null" entry is
// registered, so FindLinkFunctionByType(LFT_Null) answers "custom".
#include "FxLinkFn.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb265c0. */
FxLinkFnParameterDesc::FxLinkFnParameterDesc()
	: parameterDefaultValue(0.0f)
{
}

/** 2012 0xb265e0. */
FxLinkFnParameterDesc::FxLinkFnParameterDesc( const FxChar* name, FxReal defaultValue )
	: parameterName(name)
	, parameterDefaultValue(defaultValue)
{
}

/** 2012 0xb26610: version 0 archives carried a custom curve object, read and discarded. */
FxArchive& operator<<( FxArchive& arc, FxLinkFnParameters& linkFnParams )
{
	FxUInt16 version = arc.SerializeClassVersion("FxLinkFnParameters", FxTrue, 1);
	arc << linkFnParams.parameters;
	if( arc.IsLoading() && !version )
	{
		FxObject* pCustomCurve = 0;
		arc << pCustomCurve;
		if( pCustomCurve )
		{
			delete pCustomCurve;
			pCustomCurve = 0;
		}
	}
	return arc;
}

/** 2012 0xb266b0 (AllocateStaticClassFxLinkFn); ConstructObject 0xb284e0 returns 0 for every link function. */
FX_IMPLEMENT_CLASS(FxLinkFn, 0, FxNamedObject)

FxArray<FxLinkFn*>* FxLinkFn::_linkFnTable = 0;

/** 2012 0xb26740. */
FxLinkFn::FxLinkFn()
	: _isCorrective(FxFalse)
{
}

/** 2012 0xb267c0. */
FxLinkFn::~FxLinkFn()
{
}

/** 2012 0xb26830. */
void FxLinkFn::Startup()
{
	if( _linkFnTable )
	{
		return;
	}
	FxLinkFn::StaticClass();
	_linkFnTable = new(FxAllocate(sizeof(FxArray<FxLinkFn*>))) FxArray<FxLinkFn*>();
	_linkFnTable->Reserve(12);
	FxNullLinkFn::StaticClass();
	FxLinearLinkFn::StaticClass();
	AddLinkFunction(new FxLinearLinkFn());
	FxQuadraticLinkFn::StaticClass();
	AddLinkFunction(new FxQuadraticLinkFn());
	FxCubicLinkFn::StaticClass();
	AddLinkFunction(new FxCubicLinkFn());
	FxSqrtLinkFn::StaticClass();
	AddLinkFunction(new FxSqrtLinkFn());
	FxNegateLinkFn::StaticClass();
	AddLinkFunction(new FxNegateLinkFn());
	FxInverseLinkFn::StaticClass();
	AddLinkFunction(new FxInverseLinkFn());
	FxOneClampLinkFn::StaticClass();
	AddLinkFunction(new FxOneClampLinkFn());
	FxConstantLinkFn::StaticClass();
	AddLinkFunction(new FxConstantLinkFn());
	FxCorrectiveLinkFn::StaticClass();
	AddLinkFunction(new FxCorrectiveLinkFn());
	AddLinkFunction(new FxNullLinkFn("custom"));
	AddLinkFunction(new FxNullLinkFn("perlin noise"));
	FxClampedLinearLinkFn::StaticClass();
	AddLinkFunction(new FxClampedLinearLinkFn());
}

/** 2012 0xb26d30. */
void FxLinkFn::Shutdown()
{
	if( !_linkFnTable )
	{
		return;
	}
	FxSize numLinkFns = _linkFnTable->Length();
	for( FxSize i = 0; i < numLinkFns; ++i )
	{
		delete (*_linkFnTable)[i];
	}
	_linkFnTable->Clear();
	_linkFnTable->~FxArray<FxLinkFn*>();
	FxFree(_linkFnTable, sizeof(FxArray<FxLinkFn*>));
	_linkFnTable = 0;
}

/** 2012 0xb26dd0. */
FxSize FxLinkFn::GetNumLinkFunctions()
{
	return _linkFnTable ? _linkFnTable->Length() : 0;
}

/** 2012 0xb26df0. */
const FxLinkFn* FxLinkFn::GetLinkFunction( FxSize index )
{
	return _linkFnTable ? (*_linkFnTable)[index] : 0;
}

/** 2012 0xb26e20. */
const FxLinkFn* FxLinkFn::FindLinkFunction( const FxName& name )
{
	if( _linkFnTable )
	{
		FxSize numLinkFns = _linkFnTable->Length();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			if( (*_linkFnTable)[i]->GetName() == name )
			{
				return (*_linkFnTable)[i];
			}
		}
	}
	return 0;
}

/** 2012 0xb26ea0. */
const FxLinkFn* FxLinkFn::FindLinkFunctionByType( FxLinkFnType lfType )
{
	if( _linkFnTable )
	{
		FxSize numLinkFns = _linkFnTable->Length();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			if( (*_linkFnTable)[i]->GetType() == lfType )
			{
				return (*_linkFnTable)[i];
			}
		}
	}
	return 0;
}

/** 2012 0xb26f20. */
FxLinkFnType FxLinkFn::FindLinkFunctionType( const FxName& name )
{
	const FxLinkFn* pLinkFn = FindLinkFunction(name);
	return pLinkFn ? pLinkFn->GetType() : LFT_Invalid;
}

/** 2012 0xb26f50: the duplicate lookup's answer is ignored, as in retail. */
void FxLinkFn::AddLinkFunction( FxLinkFn* pLinkFn )
{
	if( pLinkFn )
	{
		FindLinkFunction(pLinkFn->GetName());
		_linkFnTable->PushBack(pLinkFn);
	}
}

/** 2012 0xb26f90 (AllocateStaticClassFxNullLinkFn). */
FX_IMPLEMENT_CLASS(FxNullLinkFn, 0, FxLinkFn)

/** 2012 0xb27020. */
FxNullLinkFn::FxNullLinkFn( const FxChar* name )
	: _fnDesc(" This link function type has been deprecated. ")
{
	SetName(FxName(name));
}

/** 2012 0xb270d0. */
FxNullLinkFn::~FxNullLinkFn()
{
}

/** 2012 0xb27140 (class), 0xb271d0 (ctor), 0xb27300 (dtor). */
FxLinearLinkFn::FxLinearLinkFn()
	: _fnDesc(" y = m * x + b ")
{
	SetName(FxName("linear"));
	_parameters.Reserve(2);
	_parameters.PushBack(FxLinkFnParameterDesc("m", 1.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("b", 0.0f));
}
FxLinearLinkFn::~FxLinearLinkFn() {}
FX_IMPLEMENT_CLASS(FxLinearLinkFn, 0, FxLinkFn)

/** 2012 0xb27370 (class), 0xb27400 (ctor), 0xb274f0 (dtor). */
FxQuadraticLinkFn::FxQuadraticLinkFn()
	: _fnDesc(" y = a * x ^2 ")
{
	SetName(FxName("quadratic"));
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}
FxQuadraticLinkFn::~FxQuadraticLinkFn() {}
FX_IMPLEMENT_CLASS(FxQuadraticLinkFn, 0, FxLinkFn)

/** 2012 0xb27560 (class), 0xb275f0 (ctor), 0xb276e0 (dtor). */
FxCubicLinkFn::FxCubicLinkFn()
	: _fnDesc(" y = a * x ^3 ")
{
	SetName(FxName("cubic"));
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}
FxCubicLinkFn::~FxCubicLinkFn() {}
FX_IMPLEMENT_CLASS(FxCubicLinkFn, 0, FxLinkFn)

/** 2012 0xb27750 (class), 0xb277e0 (ctor), 0xb278d0 (dtor). */
FxSqrtLinkFn::FxSqrtLinkFn()
	: _fnDesc(" y = a * sqrt( x ) ")
{
	SetName(FxName("square root"));
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}
FxSqrtLinkFn::~FxSqrtLinkFn() {}
FX_IMPLEMENT_CLASS(FxSqrtLinkFn, 0, FxLinkFn)

/** 2012 0xb27940 (class), 0xb279d0 (ctor), 0xb27a80 (dtor). */
FxNegateLinkFn::FxNegateLinkFn()
	: _fnDesc(" y = - x ")
{
	SetName(FxName("negate"));
}
FxNegateLinkFn::~FxNegateLinkFn() {}
FX_IMPLEMENT_CLASS(FxNegateLinkFn, 0, FxLinkFn)

/** 2012 0xb27af0 (class), 0xb27b80 (ctor), 0xb27c30 (dtor). */
FxInverseLinkFn::FxInverseLinkFn()
	: _fnDesc(" y = 1 / x ")
{
	SetName(FxName("inverse"));
}
FxInverseLinkFn::~FxInverseLinkFn() {}
FX_IMPLEMENT_CLASS(FxInverseLinkFn, 0, FxLinkFn)

/** 2012 0xb27ca0 (class), 0xb27d30 (ctor), 0xb27de0 (dtor). */
FxOneClampLinkFn::FxOneClampLinkFn()
	: _fnDesc(" y = [1 if x <= 1] or [1 / x if x > 1) ")
{
	SetName(FxName("oneclamp"));
}
FxOneClampLinkFn::~FxOneClampLinkFn() {}
FX_IMPLEMENT_CLASS(FxOneClampLinkFn, 0, FxLinkFn)

/** 2012 0xb27e50 (class), 0xb27ee0 (ctor), 0xb27fd0 (dtor). */
FxConstantLinkFn::FxConstantLinkFn()
	: _fnDesc(" y = c ")
{
	SetName(FxName("constant"));
	_parameters.PushBack(FxLinkFnParameterDesc("c", 1.0f));
}
FxConstantLinkFn::~FxConstantLinkFn() {}
FX_IMPLEMENT_CLASS(FxConstantLinkFn, 0, FxLinkFn)

/** 2012 0xb28040 (class), 0xb280d0 (ctor), 0xb281c0 (dtor). */
FxCorrectiveLinkFn::FxCorrectiveLinkFn()
	: _fnDesc(" x corrects for y ")
{
	SetName(FxName("corrective"));
	_parameters.PushBack(FxLinkFnParameterDesc("Correction Factor", 1.0f));
	_isCorrective = FxTrue;
}
FxCorrectiveLinkFn::~FxCorrectiveLinkFn() {}
FX_IMPLEMENT_CLASS(FxCorrectiveLinkFn, 0, FxLinkFn)

/** 2012 0xb28230 (class), 0xb282c0 (ctor), 0xb28470 (dtor). */
FxClampedLinearLinkFn::FxClampedLinearLinkFn()
	: _fnDesc(" y = m * x ")
{
	SetName(FxName("clamped linear"));
	_parameters.Reserve(4);
	_parameters.PushBack(FxLinkFnParameterDesc("m", 1.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampx", 0.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampy", 0.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampdir", 1.0f));
}
FxClampedLinearLinkFn::~FxClampedLinearLinkFn() {}
FX_IMPLEMENT_CLASS(FxClampedLinearLinkFn, 0, FxLinkFn)

} // namespace Face
} // namespace OC3Ent
