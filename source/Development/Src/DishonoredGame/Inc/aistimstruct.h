#pragma once
// DishonoredGame/inc/aistimstruct.h
// DISHONORED(written): agent CG. The runtime half of the AI stimulus system. The 111 FAIStimStruct types and
// FDisStimRef are already generated from the retail SDK dump into dishonoredgameclasses.h (they are reflected script
// structs); what the generator cannot express is that they are polymorphic and pool-allocated, and that is what this
// header adds:
//
//   * the six-slot dispatch table retail keeps at offset 0 of every stim, and the five base bodies,
//   * the per-EAIStimID type registry (size + creator + table) that UDisStimManager::LoadStim needs,
//   * FDisStimRef's reference count,
//   * UDishonoredAIBrain::HandleAIStim / HandleAIStim_Internal / MultiHandleAIStim_Internal, the entry points every
//     gameplay system raises a stim through.
//
// DISHONORED(bringup): **the dispatch is explicit rather than a C++ vtable, and this is a deliberate deviation with a
// one-line fix.** Retail's FDisSerializableObject derives from Core's FSerializableObject - same 4 bytes, and the
// first two slots of the 2012 PDB's FAIStimStruct_vtbl are exactly FSerializableObject's destructor and
// Serialize(FArchive&) (compare Core/Inc/FSerializableObject.h). Retail's UnrealScript declares a
// `pointer m_VTable_Pointer_Dummy` property to reserve those 4 bytes in the reflected layout.
// resources/tools/symbols/gen_classes_header.py emits that property as a real member (struct_lines(), which unlike
// class_lines() has no CppText hook), so the generated FDisSerializableObject is plain data and no stim struct can
// declare a virtual without changing sizeof and breaking the DishonoredGameLayouts.h asserts. Until the generator
// emits `: public FSerializableObject` with a virtual destructor in place of m_VTable_Pointer_Dummy, this header
// installs its own table of six function pointers into exactly the slot retail puts its vtable pointer in, and every
// call site goes through the DisStim* free functions below. When that hook lands, define
// DISHONORED_HAVE_POLYMORPHIC_DISSERIALIZABLEOBJECT and those free functions become one-line forwards to real virtual
// calls; no call site changes.
//
// The deviation costs one thing: the GC registration. Retail's FAIStimStruct constructor calls
// FSerializableObject::StaticInit() and GObjectSerializer->AddObject(this) (both visible in the 2012 copy
// constructor, 0x777030), so a queued stim's m_pSource and m_pStimManager are reachable by the collector. A
// non-FSerializableObject cannot be handed to UObjectSerializer::AddObject, so that registration is skipped: a stim
// that outlives a collection while holding the only reference to an actor could see that actor collected. In practice
// the queue is drained on the tick it is filled (UDishonoredAIBrain::FlushStimQueue) and every raiser holds its source
// alive, so this has not been observed - it is recorded because it is the real cost.

#ifndef _INC_AISTIMSTRUCT
#define _INC_AISTIMSTRUCT

#include "disdelegate.h"

class UDishonoredGlobalAIManager;
class UDisStimManager;

/** DISHONORED(bringup): hand-over. DisGetGlobalAIManagerUnchecked() and UDishonoredGlobalAIManager::GetStimManager()
    are agent CG's (dishonoredglobalaimanager.cpp / the DishonoredGame utilities); declared here so this header
    compiles on its own, and a redeclaration with the same signature there is harmless. */
UDishonoredGlobalAIManager* DisGetGlobalAIManagerUnchecked();


/** DISHONORED(written): retail spells this FAIStimStruct::EDisPendingStimFilterResult (a nested enum, which the
    generator cannot put inside a generated struct, so it is free here). Its only consumer is
    UDishonoredAIBrain::EnqueueStim (2013 rva 0x71b940, 2012 0x752490) and the four values are read off its switch:
      0 nothing happens and the scan of the queue continues - the value an unbound filter answers
      1 that queued stim is dropped, the new one is still enqueued, the scan continues
      2 the new stim is dropped and every queued stim is kept; the scan stops
      3 the new stim is dropped and that queued stim is dropped too; the scan stops
    DPSFR_KeepBothStims is retail's own name (it survives in the 2012 decompile); the other three are named here from
    those semantics. */
enum EDisPendingStimFilterResult
{
	DPSFR_KeepBothStims			= 0,
	DPSFR_DiscardExistingStim	= 1,
	DPSFR_DiscardNewStim		= 2,
	DPSFR_DiscardBothStims		= 3,
	DPSFR_MAX					= 4,
};

typedef DisDelegate< EDisPendingStimFilterResult, FAIStimStruct > FDisStimPendingFilterDelegate;

/** DISHONORED(layout): 2012 PDB FAIStimStruct_vtbl, 24 bytes. The slot order is load-bearing: FDisStimRef's
    destructor calls slot 0, FAIStimStruct::IsSaveable and UDishonoredAIBrain::FlushStimQueue call slot 4,
    EnqueueStim calls slots 8 and 12, and UDisStimManager::NewStim calls slot 20 before it has an object at all. */
struct FAIStimStructVTable
{
	void ( *Destruct )( FAIStimStruct* _pStim );
	void ( *Serialize )( FAIStimStruct* _pStim, FArchive& _rAr );
	FLOAT ( *GetDelay )( const FAIStimStruct* _pStim, class UDishonoredAIBrain* _pBrain );
	FDisStimPendingFilterDelegate ( *GetPendingStimFilterDelegate )( const FAIStimStruct* _pStim, BYTE _StimID );
	FString ( *GetStimDebugString )( const FAIStimStruct* _pStim );
	const UScriptStruct* ( *GetScriptStruct )( const FAIStimStruct* _pStim );
};

/** The base bodies, i.e. FAIStimStruct's own five virtuals. A concrete stim that overrides one supplies its own
    table row (see DIS_STIM_VTABLE below) and reuses these for the slots it does not override. */
void DisStimDestruct_Base( FAIStimStruct* _pStim );
void DisStimSerialize_Base( FAIStimStruct* _pStim, FArchive& _rAr );
FLOAT DisStimGetDelay_Base( const FAIStimStruct* _pStim, class UDishonoredAIBrain* _pBrain );
FDisStimPendingFilterDelegate DisStimGetPendingStimFilterDelegate_Base( const FAIStimStruct* _pStim, BYTE _StimID );
FString DisStimGetStimDebugString_Base( const FAIStimStruct* _pStim );

/** DISHONORED(layout): 2012 PDB FAIStimStruct::FAIStimTypeInfo, 8 bytes {INT m_StimSize_bytes; creator}. One row per
    EAIStimID. The size is what UDisStimManager::AllocateBlock is asked for; the creator is how
    UDisStimManager::LoadStim builds a stim when the save file has given it only an id. m_pVTable is this port's third
    field, holding the dispatch table that retail keeps in the object itself.
    FAIStimStruct::FAIStimTypeInfoRegister (1 byte in the PDB: an empty registrar whose constructor does the work)
    fills the table during static initialisation; DIS_IMPLEMENT_STIM below is that registrar. */
struct FAIStimTypeInfo
{
	INT m_StimSize_bytes;
	FAIStimStruct* ( *m_pCreatorFn )( void* _pBlock );
	const FAIStimStructVTable* m_pVTable;
};

const FAIStimTypeInfo* DisGetStimTypeInfo( BYTE _StimID );

/** Retail's FAIStimTypeInfoRegister: a file-scope object whose constructor writes one row of the table. */
struct FAIStimTypeInfoRegister
{
	FAIStimTypeInfoRegister( BYTE _StimID, const FAIStimTypeInfo& _rInfo );
};

/*-----------------------------------------------------------------------------
	Dispatch. Every AI call site uses these rather than a virtual call; they are the explicit form of the six slots.
-----------------------------------------------------------------------------*/

const FAIStimStructVTable* DisStimVTable( const FAIStimStruct* _pStim );
void DisStimSetVTable( FAIStimStruct* _pStim, const FAIStimStructVTable* _pVTable );

void DisStimDestruct( FAIStimStruct* _pStim );
void DisStimSerialize( FAIStimStruct* _pStim, FArchive& _rAr );
FLOAT DisStimGetDelay( const FAIStimStruct* _pStim, class UDishonoredAIBrain* _pBrain );
FDisStimPendingFilterDelegate DisStimGetPendingStimFilterDelegate( const FAIStimStruct* _pStim, BYTE _StimID );
FString DisStimGetStimDebugString( const FAIStimStruct* _pStim );
const UScriptStruct* DisStimGetScriptStruct( const FAIStimStruct* _pStim );

/** DISHONORED(port): 2013 rva 0x7048d0 (2012 0x76f220): a stim that references a UDisItemContext cannot go into a
    save, because item contexts are rebuilt on load rather than serialized. Retail decides it by running the stim's
    own Serialize through FDisArchiveCheckForItemContexts, an object-reference-collector archive that trips a flag on
    the first UDisItemContext it is handed. */
UBOOL DisStimIsSaveable( const FAIStimStruct* _pStim );

/*-----------------------------------------------------------------------------
	FDisStimRef's reference count (retail's disstimref.cpp).

	DISHONORED(bringup): FDisStimRef is generated as plain data and disstimref.cpp is not one of this package's files,
	so retail's constructor and destructor are free functions here. Whoever takes disstimref.cpp should move them onto
	the struct; every call site in this package already goes through FDisStimRefScope or these two.
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x701800 (2012 0x764ee0): the holder takes a reference. */
void DisStimRefInit( FDisStimRef& _rRef, const FAIStimStruct* _pStim, FLOAT _fDelayTime );

/** DISHONORED(port): 2013 rva 0x70cf70 (2012 0x76fca0): the last reference destructs the stim and returns its block
    to the pool. A stim with no manager is not pool-allocated and is never freed here. */
void DisStimRefRelease( FDisStimRef& _rRef );

/** The RAII form the HandleAIStim templates need: retail keeps a stack FDisStimRef alive across the Enqueue calls so
    the freshly created stim cannot be freed by a queue that rejected it. */
struct FDisStimRefScope
{
	FDisStimRef m_Ref;

	FDisStimRefScope( const FAIStimStruct* _pStim, FLOAT _fDelayTime )
	{
		DisStimRefInit( m_Ref, _pStim, _fDelayTime );
	}
	~FDisStimRefScope()
	{
		DisStimRefRelease( m_Ref );
	}
};


/*-----------------------------------------------------------------------------
	Declaring a concrete stim type.

	One line per stim in a .cpp, which is retail's FAIStimTypeInfoRegister:

	  DIS_IMPLEMENT_STIM( FAIStimStruct_BrainInit, EAIStimID_BrainInit )

	A stim that overrides one of the five virtuals passes its own table instead:

	  static const FAIStimStructVTable GStolenVTable = { &DisStimDestruct_Base, &DisStimSerialize_Stolen,
	      &DisStimGetDelay_Stolen, &DisStimGetPendingStimFilterDelegate_Base, &DisStimGetStimDebugString_Base,
	      &FAIStimStruct_Stolen_GetScriptStruct };
	  DIS_IMPLEMENT_STIM_VTABLE( FAIStimStruct_Stolen, EAIStimID_Stolen, GStolenVTable )
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x706480 (2012 0x76a010) and 110 more instantiations: the UScriptStruct of a native
    struct, found once by name through ANY_PACKAGE and cached in a function-local static.
    DISHONORED(bringup): the 2012 PDB attributes every instantiation to dishonoredutilities_saveload.h, which agent CF
    owns and has not declared it in; the guard lets that package take it over, the way agent AJ's
    DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR lets Engine take over the spawn functor. */
#ifndef DISHONORED_HAVE_DISHONOREDGETSCRIPTSTRUCT
template< class T >
UScriptStruct* _DishonoredGetScriptStruct( const TCHAR* _pStructName )
{
	static UScriptStruct* s_pStruct = (UScriptStruct*)UObject::StaticFindObjectChecked( UScriptStruct::StaticClass(), ANY_PACKAGE, _pStructName, FALSE );
	return s_pStruct;
}
#endif

#define DIS_IMPLEMENT_STIM_VTABLE( StimType, StimID, VTableRef ) \
	static FAIStimStruct* StimType##_Create( void* _pBlock ) \
	{ \
		return ::new( _pBlock ) StimType(); \
	} \
	static const FAIStimTypeInfo G##StimType##_TypeInfo = { sizeof(StimType), &StimType##_Create, &VTableRef }; \
	static FAIStimTypeInfoRegister G##StimType##_Register( (BYTE)StimID, G##StimType##_TypeInfo );

#define DIS_IMPLEMENT_STIM( StimType, StimID ) \
	static const UScriptStruct* StimType##_GetScriptStruct( const FAIStimStruct* ) \
	{ \
		return _DishonoredGetScriptStruct< StimType >( TEXT(#StimType) + 1 ); \
	} \
	static const FAIStimStructVTable G##StimType##_VTable = \
	{ \
		&DisStimDestruct_Base, &DisStimSerialize_Base, &DisStimGetDelay_Base, \
		&DisStimGetPendingStimFilterDelegate_Base, &DisStimGetStimDebugString_Base, \
		&StimType##_GetScriptStruct \
	}; \
	DIS_IMPLEMENT_STIM_VTABLE( StimType, StimID, G##StimType##_VTable )

/*-----------------------------------------------------------------------------
	Allocating a stim out of the pool.

	DISHONORED(bringup): retail declares this as UDisStimManager::NewStim<T> (a member template; the 2012 PDB attributes
	every instantiation to disstimmanager.cpp line 39). UDisStimManager is a generated class and disstimmanager.cpp
	belongs to another package, so it is a free template here. That package's cpptext can forward in one line:
	    template< class T > T* NewStim( const T& _rStim ) { return DisNewStim( this, _rStim ); }
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x5ffca0 (2012 0x6470b0) and 110 more folded instantiations. Retail asks the TEMPLATE
    stim for its script struct (vtable slot 20, before any object exists), tags the pool block with that struct's
    FName, copy-constructs the stim into the block, re-stamps the concrete type's vtable pointer - the base copy
    constructor leaves the base one there - and finally records the manager that owns the block. A failed allocation
    dereferences NULL in retail (`check`); ours returns NULL and the two callers stop. */
template< class T >
T* DisNewStim( UDisStimManager* _pStimManager, const T& _rStim )
{
	if( !_pStimManager )
	{
		return NULL;
	}
	const UScriptStruct* pScriptStruct = DisStimGetScriptStruct( &_rStim );
	const FName BlockName = pScriptStruct ? pScriptStruct->GetFName() : NAME_None;
	void* pBlock = _pStimManager->AllocateBlock( sizeof(T), BlockName );
	if( !pBlock )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): DisNewStim: the stim pool refused %d bytes for %s"), sizeof(T), *BlockName.ToString() );
		return NULL;
	}
	T* pNewStim = ::new( pBlock ) T( _rStim );
	const FAIStimTypeInfo* pTypeInfo = DisGetStimTypeInfo( pNewStim->m_StimID );
	DisStimSetVTable( pNewStim, pTypeInfo ? pTypeInfo->m_pVTable : DisStimVTable( &_rStim ) );
	pNewStim->m_pStimManager = _pStimManager;
	return pNewStim;
}

/*-----------------------------------------------------------------------------
	Raising a stim.

	DISHONORED(bringup): retail declares these three as member templates of UDishonoredAIBrain (the 2012 PDB attributes
	them to dishonoredaibrain.cpptext lines 106, 165 and 185). UDishonoredAIBrain is a generated class, so the member
	declarations belong in Inc/CppText/UDishonoredAIBrain.h, which agent CG owns; they are free templates here and CG's
	cpptext forwards to them in one line each:
	    template< class T > void HandleAIStim( const T& _rStim, const UObject* _pSource )
	    { DisHandleAIStim( this, _rStim, _pSource ); }
	The bodies are in this header (below the declarations) because they touch UDishonoredAIBrain and UDisStimManager
	members, so this header has to be included after dishonoredgameclasses.h.
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x62ec90 (2012 0x673d40) and 20 more folded instantiations. A stim is allocated out of
    the brain's stim manager, given its source, held by a stack reference across the enqueue so a queue that rejects it
    cannot free it underneath us, and enqueued.
    DISHONORED(written): the 2012 body additionally gates on IsBrainInhibited(). That member, m_BrainInhibitors and
    AddBrainInhibitor/RemoveBrainInhibitor do NOT exist in retail 2013 - none of the three has a match in
    match_2012_2013.csv, m_BrainInhibitors is absent from the 2013 reflected layout, and the 2013 body is 134 bytes
    against 2012's 143. The gate here is the initialized flag alone, which is what retail 2013 does. */
template< class T >
void DisHandleAIStim_Internal( UDishonoredAIBrain* _pBrain, const T& _rStim, UObject* _pStimSource )
{
	if( !_pBrain || !_pBrain->m_bBrainIsInitialized )
	{
		return;
	}
	T* pNewStim = _pBrain->m_pStimManager ? DisNewStim< T >( _pBrain->m_pStimManager, _rStim ) : NULL;
	if( !pNewStim )
	{
		return;
	}
	pNewStim->m_pSource = _pStimSource;

	FDisStimRefScope Holder( pNewStim, 0.f );
	_pBrain->EnqueueStim( pNewStim );
}

/** DISHONORED(port): 2013 rva 0x6570d0 (2012 0x6a0680): the public entry point. A brain that is allowed to be dormant
    does not take the stim at all, which is how an NPC far from the player costs nothing.
    DISHONORED(bringup): the predicate is a 3-byte constant-returning method on the brain, folded in both exes with
    UDisBehaviorTriggerAlarm::CanBeDormant (2013 rva 0x322980), so its name on UDishonoredAIBrain is not recoverable
    from the symbols. It is called through UDishonoredAIBrain::CanBeDormant(), which agent CG declares. */
template< class T >
void DisHandleAIStim( UDishonoredAIBrain* _pBrain, const T& _rStim, const UObject* _pStimSource )
{
	if( _pBrain && !_pBrain->CanBeDormant() )
	{
		DisHandleAIStim_Internal( _pBrain, _rStim, const_cast< UObject* >( _pStimSource ) );
	}
}

/** DISHONORED(port): 2013 rva 0x657100 (2012 0x6a0710) and 20 more folded instantiations: the static broadcast form.
    ONE stim object is created out of the global manager and every initialized brain in the list is handed the same
    pointer, so the stim is shared and its reference count is what keeps it alive for all of them. */
template< class T >
void DisMultiHandleAIStim_Internal( const TArray< UDishonoredAIBrain* >& _rBrains, const T& _rStim, UObject* _pStimSource )
{
	UDishonoredGlobalAIManager* pGlobalAIMan = DisGetGlobalAIManagerUnchecked();
	if( !pGlobalAIMan )
	{
		return;
	}
	UDisStimManager* pStimManager = pGlobalAIMan->GetStimManager();
	if( !pStimManager )
	{
		return;
	}
	T* pNewStim = DisNewStim< T >( pStimManager, _rStim );
	if( !pNewStim )
	{
		return;
	}
	pNewStim->m_pSource = _pStimSource;

	FDisStimRefScope Holder( pNewStim, 0.f );
	for( INT i = 0; i < _rBrains.Num(); ++i )
	{
		UDishonoredAIBrain* pBrain = _rBrains(i);
		if( pBrain && pBrain->m_bBrainIsInitialized )
		{
			pBrain->EnqueueStim( pNewStim );
		}
	}
}

#endif
