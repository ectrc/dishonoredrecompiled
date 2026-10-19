#pragma once
// Engine/inc/arkcomponentpolicy.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x577fc0  public: static void __cdecl InsertArkComponentToPolicy<class IArkComponentPreAsyncWork, 1>::Insert(class IArkComponentPreAsyncWork *, class TArray<class IArkComponentPreAsyncWork *, class FDefaultAllocator> *)
//   0x578ba0  public: virtual void __thiscall FArkComponentPolicy<class IArkComponentPostAsyncWork, 1>::Register(class IArkComponentPostAsyncWork *)
//   0x578bc0  public: virtual void __thiscall FArkComponentPolicy<class IArkComponentTimeSlicedTick, 1>::Unregister(class IArkComponentTimeSlicedTick *)
//   0x578c50  protected: void __thiscall FArkComponentPolicy<class IArkComponentPreAsyncWorkJustBeforeProceduralAnim, 1>::DoneTicking(void)
//   0x57a910  public: virtual void __thiscall FArkComponentPolicy<class IArkComponentObservable, 0>::Register(class IArkComponentObservable *)


/*-----------------------------------------------------------------------------
	DISHONORED(port): agent HC (PHASE22 HC). The lists FArkComponentManager ticks components from. One policy per tick
	slot, all the same shape in retail (2012 PDB FArkComponentPolicy<T,bSorted>, 28 bytes: vftable { Register,
	Unregister }, m_Components @4, m_IteratingList @16). Ticking copies m_Components into m_IteratingList and walks the
	copy; a component that registers meanwhile goes into the copy, one that unregisters is set to NULL in it, and
	DoneTicking (2012 0x578c50 / 2013 0x537ed0) drops the NULLs and makes the copy the list. The sorted policies keep
	the components ordered by FArkComponentBase::GetType (InsertArkComponentToPolicy, 2012 0x577fc0 / 2013 0x536d30).
-----------------------------------------------------------------------------*/

#include "arkcomponentbase.h"

/** DISHONORED(port): 2012 PDB, 8-byte vftables { <Tick>, GetComponentBase }, as IArkComponentPreAsyncWork's. */
class IArkComponentDuringAsyncWork
{
public:
	virtual void DuringAsyncWorkTick( FLOAT _fTimeStep ) = 0;
	virtual const FArkComponentBase* GetComponentBase() const = 0;
};

class IArkComponentPostAsyncWork
{
public:
	virtual void PostAsyncWorkTick( FLOAT _fTimeStep ) = 0;
	virtual const FArkComponentBase* GetComponentBase() const = 0;
};

/** DISHONORED(port): FDisComponentVision::TimeSlicedTick (2012 0x8c63e0) answers whether it finished inside the budget. */
class IArkComponentTimeSlicedTick
{
public:
	virtual UBOOL TimeSlicedTick( FLOAT _fTimeStep, DOUBLE _fTimeLeft ) = 0;
	virtual const FArkComponentBase* GetComponentBase() const = 0;
};

class IArkComponentObservable;

inline void ArkComponentPolicyTick( IArkComponentPreAsyncWork* _pComponent, FLOAT _fTimeStep ) { _pComponent->PreAsyncWorkTick( _fTimeStep ); }
inline void ArkComponentPolicyTick( IArkComponentDuringAsyncWork* _pComponent, FLOAT _fTimeStep ) { _pComponent->DuringAsyncWorkTick( _fTimeStep ); }
inline void ArkComponentPolicyTick( IArkComponentPostAsyncWork* _pComponent, FLOAT _fTimeStep ) { _pComponent->PostAsyncWorkTick( _fTimeStep ); }

template< class ComponentInterface, UBOOL bSorted >
class FArkComponentPolicy
{
public:
	// DISHONORED(port): 2012 0x578ba0 (sorted) / 0x57a910 (IArkComponentObservable, AddUniqueItem)
	virtual void Register( ComponentInterface* _pComponent )
	{
		TArray<ComponentInterface*>& List = m_IteratingList.Num() ? m_IteratingList : m_Components;
		if constexpr( bSorted )
		{
			Insert( _pComponent, List );
		}
		else
		{
			List.AddUniqueItem( _pComponent );
		}
	}

	// DISHONORED(port): 2012 0x578bc0 (135 B)
	virtual void Unregister( ComponentInterface* _pComponent )
	{
		if( m_IteratingList.Num() )
		{
			const INT Index = m_IteratingList.FindItemIndex( _pComponent );
			if( Index != INDEX_NONE )
			{
				m_IteratingList( Index ) = NULL;
			}
		}
		else
		{
			m_Components.RemoveItem( _pComponent );
		}
	}

	INT Num() const { return m_Components.Num(); }

	/** DISHONORED(diag): agent IB's -ibdiag "in policy" column; not a retail function. */
	UBOOL IsRegistered( ComponentInterface* _pComponent ) const { return m_Components.ContainsItem( _pComponent ) || m_IteratingList.ContainsItem( _pComponent ); }

	/** DISHONORED(port): FArkComponentPreAsyncWorkPolicy::Tick (2012 0x579d80, 160 B), the body all four async-work policies
	    share in retail: each component ticks with its owner's CustomTimeDilation applied. */
	void Tick( FLOAT _fDeltaTime )
	{
		m_IteratingList = m_Components;
		const INT Count = m_IteratingList.Num();
		for( INT Index = 0; Index < Count; Index++ )
		{
			ComponentInterface* Component = m_IteratingList( Index );
			if( Component )
			{
				ArkComponentPolicyTick( Component, Component->GetComponentBase()->m_pOwner->CustomTimeDilation * _fDeltaTime );
			}
		}
		DoneTicking();
	}

protected:
	// DISHONORED(port): 2012 0x577fc0 (218 B). Retail inserts AT the index of the last entry whose type is not greater
	// than the new one (so before it, not after it); kept as retail's.
	static void Insert( ComponentInterface* _pComponent, TArray<ComponentInterface*>& _rList )
	{
		if( _rList.FindItemIndex( _pComponent ) != INDEX_NONE )
		{
			return;
		}
		const INT Type = _pComponent->GetComponentBase()->GetType();
		INT Index = _rList.Num();
		while( Index )
		{
			--Index;
			if( Type >= _rList( Index )->GetComponentBase()->GetType() )
			{
				_rList.InsertItem( _pComponent, Index );
				return;
			}
		}
		_rList.InsertItem( _pComponent, 0 );
	}

	void DoneTicking()
	{
		for( INT Index = m_IteratingList.Num() - 1; Index >= 0; Index-- )
		{
			if( m_IteratingList( Index ) == NULL )
			{
				m_IteratingList.RemoveSwap( Index );
			}
		}
		m_Components = m_IteratingList;
		m_IteratingList.Reset();
	}

	TArray<ComponentInterface*>	m_Components;
	TArray<ComponentInterface*>	m_IteratingList;
};

class FArkComponentPreAsyncWorkPolicy : public FArkComponentPolicy<IArkComponentPreAsyncWork, TRUE> {};
class FArkComponentPreAsyncWorkJustBeforeProceduralAnimPolicy : public FArkComponentPolicy<IArkComponentPreAsyncWorkJustBeforeProceduralAnim, TRUE> {};
class FArkComponentDuringAsyncWorkPolicy : public FArkComponentPolicy<IArkComponentDuringAsyncWork, TRUE> {};
class FArkComponentPostAsyncWorkPolicy : public FArkComponentPolicy<IArkComponentPostAsyncWork, TRUE> {};
class FArkComponentObservablePolicy : public FArkComponentPolicy<IArkComponentObservable, FALSE> {};

/** DISHONORED(port): 2012 PDB, 40 bytes: the sorted policy plus the components still owed a slice this round. */
class FArkComponentTimeSlicedTickPolicy : public FArkComponentPolicy<IArkComponentTimeSlicedTick, TRUE>
{
public:
	// DISHONORED(port): 2012 0x578100 (32 B)
	virtual void Unregister( IArkComponentTimeSlicedTick* _pComponent )
	{
		FArkComponentPolicy<IArkComponentTimeSlicedTick, TRUE>::Unregister( _pComponent );
		m_ComponentsToTick.RemoveItem( _pComponent );
	}

	void Tick( FLOAT _fDeltaTime );

private:
	TArray<IArkComponentTimeSlicedTick*>	m_ComponentsToTick;
};

/**
 * DISHONORED(port): agent HC. 2012 PDB FArkComponentManager, 180 bytes: the six policies in this order. UWorld::Init
 * (2012 0x3bbd00) allocates it into UWorld::m_pComponentManager and UWorld::FinishDestroy (0x3b0730) deletes it first;
 * UWorld's tick stages call Tick with the stage (UnLevTic.cpp).
 */
class FArkComponentManager
{
public:
	FArkComponentManager() {}				// 2012 0x57afa0 (202 B): six empty policies
	~FArkComponentManager() {}				// 2012 0x579cf0 (137 B): the six destructors, last first

	void Tick( FLOAT _fTimeStep, BYTE _TickType );

	template< class PolicyType > PolicyType& GetPolicy();

	FArkComponentPreAsyncWorkPolicy							m_PreAsyncWorkPolicy;							// @0
	FArkComponentPreAsyncWorkJustBeforeProceduralAnimPolicy	m_PreAsyncWorkJustBeforeProceduralAnimPolicy;	// @28
	FArkComponentDuringAsyncWorkPolicy						m_DuringAsyncWorkPolicy;						// @56
	FArkComponentPostAsyncWorkPolicy						m_PostAsyncWorkPolicy;							// @84
	FArkComponentObservablePolicy							m_ObservablePolicy;								// @112
	FArkComponentTimeSlicedTickPolicy						m_TimeSlicedTickPolicy;							// @140
};

// DISHONORED(port): the GetPolicy<> instantiations are retail's four-byte `lea eax,[ecx+N]` (2012 0x765610 is the PostAsyncWork
// one, +84; the PreAsyncWork one is ICF-folded into a `mov eax,ecx` that IDA names FStaticLightingVertexMapping::GetVertexMapping,
// 2013 0xc11820, which is what FDisComponentRetargeting::StartRetargeting 2013 0xc88440 calls).
template<> inline FArkComponentPreAsyncWorkPolicy& FArkComponentManager::GetPolicy<FArkComponentPreAsyncWorkPolicy>() { return m_PreAsyncWorkPolicy; }
template<> inline FArkComponentPreAsyncWorkJustBeforeProceduralAnimPolicy& FArkComponentManager::GetPolicy<FArkComponentPreAsyncWorkJustBeforeProceduralAnimPolicy>() { return m_PreAsyncWorkJustBeforeProceduralAnimPolicy; }
template<> inline FArkComponentDuringAsyncWorkPolicy& FArkComponentManager::GetPolicy<FArkComponentDuringAsyncWorkPolicy>() { return m_DuringAsyncWorkPolicy; }
template<> inline FArkComponentPostAsyncWorkPolicy& FArkComponentManager::GetPolicy<FArkComponentPostAsyncWorkPolicy>() { return m_PostAsyncWorkPolicy; }
template<> inline FArkComponentObservablePolicy& FArkComponentManager::GetPolicy<FArkComponentObservablePolicy>() { return m_ObservablePolicy; }
template<> inline FArkComponentTimeSlicedTickPolicy& FArkComponentManager::GetPolicy<FArkComponentTimeSlicedTickPolicy>() { return m_TimeSlicedTickPolicy; }

checkAtCompileTime( sizeof( FArkComponentManager ) == 180, FArkComponentManager_is_180_bytes );
