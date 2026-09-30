#pragma once
// DishonoredGame/inc/disaimonitoractorreachability.h
// DISHONORED(port): agent EN (PHASE13 EN). Ark component type 208 (DisCpntType_AIMonitorPawnReachability) - the third
// component of every AI brain's container in Dishonored0.sav, and the one with the longest serialised form of the
// nineteen: forty-four bytes of its own past the base's ten.
//   0x72bb40  public: virtual unsigned int __thiscall FDisAIMonitorPawnReachability::IsOfType(int)const
//   0x72bb60  public: virtual int __thiscall FDisAIMonitorPawnReachability::GetType(void)const
//   0x72bb70  public: static class FName __cdecl FDisAIMonitorPawnReachability::GetInternalName(void)
//   0x72bbe0  public: virtual class FName __thiscall FDisAIMonitorPawnReachability::GetName(void)const

#include "arkcomponentlocomotion.h"	// IArkComponentPreAsyncWorkJustBeforeProceduralAnim

/**
 * DISHONORED(layout): retail sizeof 64 (2013 GetMemoryFootprint 0x728b20 is GetAllocatedSize() + 0x40). Every offset
 * below is the constructor's (0x72bad0) or a body's, and Serialize (0x73b010) walks all eleven DWORDs of the span in
 * order, which is the independent check that there is no gap:
 *
 *   +0x00..0x0F  FArkComponentBase
 *   +0x10        the IArkComponentPreAsyncWorkJustBeforeProceduralAnim vptr
 *   +0x14        m_pOwningBrain                      ctor 0;      Starting takes it from the controller's m_pAIBrain
 *   +0x18        m_fCheckPeriod                      ctor -1.0f;  reloaded into m_fTimeToNextCheck after a check
 *   +0x1C        m_fReachableRecheckPeriod           ctor  1.0f;  reloaded into m_fTimeToNextRecheck when on-navmesh
 *   +0x20        m_pMonitoredPawn                    ctor 0
 *   +0x24        m_fTimeToNextCheck                  ctor  FLT_MAX (0x7f7fc99e); Starting sets it to 0
 *   +0x28        m_fTimeToNextRecheck                ctor  0.0f;   Starting sets it to m_fReachableRecheckPeriod
 *   +0x2C        m_bNeedsMonitoredPawnReset          ctor  1;      ResetWithNewMonitoredPawn clears it
 *   +0x30        m_eCachedReachability               ctor  0;      UpdateCachedReachability (0x747c10) writes it
 *   +0x34..0x3C  m_LastNavMeshCheckLocation          not initialised by the constructor; the reachability check
 *                                                    (0x746580) writes the three floats it path-found to
 */
class FDisAIMonitorPawnReachability : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( DisCpntType_AIMonitorPawnReachability, "DisCpntType_AIMonitorPawnReachability" )

public:
	FDisAIMonitorPawnReachability();

	// ---- FArkComponentBase ----
	virtual void ManageReferences( FGCHelper* _pHelper );			// 2013 0x72bc50
	virtual void Serialize( FArchive& _rArchive );					// 2013 0x73b010
	virtual DWORD GetMemoryFootprint() const { return GetAllocatedSize() + sizeof( FDisAIMonitorPawnReachability ); }	// 2013 0x728b20

	// ---- IArkComponentPreAsyncWorkJustBeforeProceduralAnim ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );				// 2013 0x7480a0, not ported - see the .cpp
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	class UDishonoredAIBrain*	m_pOwningBrain;
	FLOAT						m_fCheckPeriod;
	FLOAT						m_fReachableRecheckPeriod;
	class AActor*				m_pMonitoredPawn;
	FLOAT						m_fTimeToNextCheck;
	FLOAT						m_fTimeToNextRecheck;
	UBOOL						m_bNeedsMonitoredPawnReset;
	eDisReachability			m_eCachedReachability;
	FVector						m_LastNavMeshCheckLocation;

	/** 2013 rva 0x72bc46 / 0x747579: Starting increments it and Stopping decrements it, and PreAsyncWorkTick spreads
	    the checks over that many frames. Serialize's tail increments it too, for a component the save says was
	    already started. */
	static INT	ms_ComponentCount;
};
