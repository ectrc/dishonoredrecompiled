#pragma once
// Engine/inc/arkcomponentavoidance.h
// PDB functions attributed to this file (4):
//   0x577320  public: virtual unsigned int __thiscall FArkComponentAvoidance::IsOfType(int)const
//   0x577330  public: virtual int __thiscall FArkComponentAvoidance::GetType(void)const
//   0x577340  public: static class FName __cdecl FArkComponentAvoidance::GetInternalName(void)
//   0x5773b0  public: virtual class FName __thiscall FArkComponentAvoidance::GetName(void)const

/*-----------------------------------------------------------------------------
	DISHONORED(port, agent JD): the game's crowd avoidance, one component on the game info (AGameInfo::PostBeginPlay adds
	and starts it). Every UArkAvoidable registers here; each PreAsyncWork tick eases the avoidables' radius and force
	multiplier towards their targets, caches the pairs close enough to matter (m_ProxyCache, read by the locomotion's
	DetectPreAvoidanceCollision and ComputePriorityOfNPCsFromSharedProps) and gives the overlapping pairs opposite forces.
	2012 layout (PDB, 44 bytes): FArkComponentBase @0, IArkComponentPreAsyncWork @16, m_Avoidables @20, m_ProxyCache @32.
	The class is declared here, in its retail header, and defined in DishonoredGame/Src/arkavoidance.cpp: UArkAvoidable's
	only C++ declaration in this tree is DishonoredGame's shim header.
-----------------------------------------------------------------------------*/

#include "arkcomponentbase.h"

class UArkAvoidable;
class AGameInfo;

class FArkComponentAvoidance : public FArkComponentBase, public IArkComponentPreAsyncWork
{
	ARKCOMPONENT_DECLARE_TYPE( ArkCpntType_Avoidance, "ArkCpntType_Avoidance" )
public:
	struct FArkCpntAvoidCacheProps
	{
		UArkAvoidable*	m_pAvoidable1;
		UArkAvoidable*	m_pAvoidable2;
	};

	FArkComponentAvoidance();											// 2012 0x5772e0
	virtual ~FArkComponentAvoidance();									// 2012 0x5773d0
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentAvoidance ); }
	virtual void Starting();											// 2012 0x575340
	virtual void Stopping();											// 2012 0x575370
	virtual DWORD GetAllocatedSize() const { return ( m_Avoidables.Num() + m_Avoidables.GetSlack() ) * sizeof( UArkAvoidable* ); }	// 2012 0x5753b0

	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );					// 2012 0x578ec0
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	static FArkComponentAvoidance* GetAvoidanceComponent( const AGameInfo* const _pGameInfo );	// 2012 0x577430
	void RegisterAvoidable( UArkAvoidable* const _pAvoidable );		// 2012 0x577450
	void UnregisterAvoidable( UArkAvoidable* const _pAvoidable );		// 2012 0x577470
	/** retail reads the member directly; the 2012 accessor is ICF-folded with FNavMeshPolyBase::GetPolyCenter (lea +32) */
	const TArray<FArkCpntAvoidCacheProps>& GetProxyCache() const { return m_ProxyCache; }	// 2012 0x574760
	const TArray<UArkAvoidable*>& GetAvoidables() const { return m_Avoidables; }

private:
	TArray<UArkAvoidable*>				m_Avoidables;					// @20
	TArray<FArkCpntAvoidCacheProps>		m_ProxyCache;					// @32
};
