#pragma once
// DishonoredGame/inc/disdelegate.h
// DISHONORED(port): agent CG. Arkane's bound-method object, used by the AI stimulus system: a behaviour, a brain
// process or a sub-process hands the brain one of these for a given EAIStimID, and the brain calls it with the
// FAIStimStruct base reference while the bound method takes the concrete stim subclass.
//
// The 2012 PDB attributes 271 functions to this header and every one of them is an instantiation of the two static
// thunk templates below (17 bytes each, all identical-code folded in the shipped exe). Retail declares three
// instantiations of the class, which is the whole of the AI delegate surface:
//   DisDelegate<FAIStimStruct::EDisPendingStimFilterResult, FAIStimStruct>   the pending-stim filter
//   DisDelegate<unsigned int, FAIStimStruct>                                 the Evaluate*/Filter* predicates
//   DisDelegate<void, FAIStimStruct>                                         the SetupFrom* mutators
//
// Line numbers of the retail header, from the PDB attribution, are kept as comments so the shape can be checked:
// operator() is disdelegate.h:76, PrivateDelegatorMemFn :150, PrivateDelegatorMemFnConst :165.

#ifndef _INC_DISDELEGATE
#define _INC_DISDELEGATE

struct FAIStimStruct;

// DISHONORED(layout): 2012 PDB DisDelegate<R,FAIStimStruct>, sizeof 8: @0 m_pObject, @4 m_pDelegator.
// The delegator is a plain __cdecl function pointer, not a pointer to member: retail binds a member function by
// instantiating one of the two static thunks below, so the delegate itself stays two words wide and copyable.
template< class R, class BaseParamType >
class DisDelegate
{
public:
	typedef R ( *DelegatorFnType )( void*, const BaseParamType& );

	void* m_pObject;
	DelegatorFnType m_pDelegator;

	/** DISHONORED(layout): the shared empty delegate every accessor returns when nothing is bound. Call sites compare
	    both words against it rather than testing m_pDelegator for NULL, so it is matched here exactly. */
	static const DisDelegate s_NullDelegate;

	DisDelegate() : m_pObject( NULL ), m_pDelegator( NULL ) {}
	DisDelegate( void* _pObject, DelegatorFnType _pDelegator ) : m_pObject( _pObject ), m_pDelegator( _pDelegator ) {}

	/** DISHONORED(port): 2013 rva 0x6edf50 (2012 0x74ddb0, disdelegate.h:76): an unbound delegate answers R() rather
	    than calling through NULL. A caller that has to tell "not bound" from "bound and answered zero" compares
	    against s_NullDelegate itself instead of using this, which is what UDishonoredAIBrain::ProcessOneStim and
	    EnqueueStim both do. */
	R operator()( const BaseParamType& _rParam ) const
	{
		if( m_pObject == s_NullDelegate.m_pObject && m_pDelegator == s_NullDelegate.m_pDelegator )
		{
			return R();
		}
		return m_pDelegator( m_pObject, _rParam );
	}

	UBOOL IsBound() const
	{
		return !( m_pObject == s_NullDelegate.m_pObject && m_pDelegator == s_NullDelegate.m_pDelegator );
	}

	/** DISHONORED(port): 2013 rva 0x6c2b00 (2012 0x7250c0) and 164 more, disdelegate.h:150. Downcasts the stim to the concrete type
	    the bound method declares and forwards. Retail declares these private and reaches them only through the
	    binding macros below. */
	template< class ObjType, class ParamType, void ( ObjType::*MemFn )( const ParamType& ) >
	static void PrivateDelegatorMemFn( void* _pObject, const BaseParamType& _rParam )
	{
		( ( (ObjType*)_pObject )->*MemFn )( (const ParamType&)_rParam );
	}

	template< class ObjType, class ParamType, R ( ObjType::*MemFn )( const ParamType& ) >
	static R PrivateDelegatorMemFnRet( void* _pObject, const BaseParamType& _rParam )
	{
		return ( ( (ObjType*)_pObject )->*MemFn )( (const ParamType&)_rParam );
	}

	/** DISHONORED(port): 2013 rva 0x6c2ac0 (2012 0x725080) and 33 more, disdelegate.h:165. The const form; the object pointer is
	    `void* const` in the retail signature, which is the same thunk with a const member function. */
	template< class ObjType, class ParamType, R ( ObjType::*MemFn )( const ParamType& ) const >
	static R PrivateDelegatorMemFnConst( void* const _pObject, const BaseParamType& _rParam )
	{
		return ( ( (const ObjType*)_pObject )->*MemFn )( (const ParamType&)_rParam );
	}
};

template< class R, class BaseParamType >
const DisDelegate< R, BaseParamType > DisDelegate< R, BaseParamType >::s_NullDelegate;

/** The three instantiations retail has. EDisPendingStimFilterResult is declared in aistimstruct.h. */
typedef DisDelegate< UBOOL, FAIStimStruct > FDisStimPredicateDelegate;
typedef DisDelegate< void, FAIStimStruct > FDisStimSetupDelegate;

/*-----------------------------------------------------------------------------
	Binding.

	A behaviour binds one line per stim id, which is how retail's ~150 bindings read. The macros exist because the
	member-function pointer is a non-type template argument: it cannot be passed as a value, so the thunk has to be
	instantiated at the binding site.

	  return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorAmbush, FAIStimStruct_AmbushRequest, EvaluateAmbushRequest );
	  return DIS_BIND_STIM_SETUP( UDisBehaviorAmbush, FAIStimStruct_AmbushRequest, SetupFromAmbushRequest );
	  return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoHome, FAIStimStruct_PathingFail, FilterPathingFail );

	Each expands to a delegate bound to `this`, so they are written inside the owning class's accessor.
-----------------------------------------------------------------------------*/

#define DIS_BIND_STIM_PREDICATE_CONST( ObjType, StimType, MemFn ) \
	FDisStimPredicateDelegate( (void*)this, &FDisStimPredicateDelegate::PrivateDelegatorMemFnConst< ObjType, StimType, &ObjType::MemFn > )

#define DIS_BIND_STIM_PREDICATE( ObjType, StimType, MemFn ) \
	FDisStimPredicateDelegate( (void*)this, &FDisStimPredicateDelegate::PrivateDelegatorMemFnRet< ObjType, StimType, &ObjType::MemFn > )

#define DIS_BIND_STIM_SETUP( ObjType, StimType, MemFn ) \
	FDisStimSetupDelegate( (void*)this, &FDisStimSetupDelegate::PrivateDelegatorMemFn< ObjType, StimType, &ObjType::MemFn > )

#define DIS_BIND_STIM_FILTER_CONST( DelegateType, ObjType, StimType, MemFn ) \
	DelegateType( (void*)this, &DelegateType::PrivateDelegatorMemFnConst< ObjType, StimType, &ObjType::MemFn > )

#endif
