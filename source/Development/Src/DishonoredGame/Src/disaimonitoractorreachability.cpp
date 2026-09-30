// DishonoredGame/src/disaimonitoractorreachability.cpp
// DISHONORED(port): agent EN (PHASE13 EN). Ark component type 208, DisCpntType_AIMonitorPawnReachability: the class,
// its creator registration and the forty-four bytes its Serialize reads past FArkComponentBase's ten.
//   0x728a90  public: unsigned int __thiscall FDisAIMonitorPawnReachability::DoReachabilityCheck_OnNavmesh(class ADishonoredPawn *)
//   0x72bad0  public: __thiscall FDisAIMonitorPawnReachability::FDisAIMonitorPawnReachability(void)
//   0x72bc00  public: virtual void __thiscall FDisAIMonitorPawnReachability::Starting(void)
//   0x72bc50  public: virtual void __thiscall FDisAIMonitorPawnReachability::ManageReferences(class FArkComponentBase::FGCHelper &)
//   0x73b010  public: virtual void __thiscall FDisAIMonitorPawnReachability::Serialize(class FArchive &)
//   0x73e740  public: unsigned int __thiscall FDisAIMonitorPawnReachability::DoReachabilityCheck_PathFind(class FVector const &)
//   0x73e7f0 / 0x73e890 / 0x73e930  OnBecomeReachable / OnBecomeUnReachable / OnUnReachabilityChange
//   0x73e9d0  public: void __thiscall FDisAIMonitorPawnReachability::OnHideoutEvent(class FArkGameEvent const &)
//   0x745310 / 0x745460  GetActualHideoutThePlayerIsIn / GetKnownHideoutThePlayerIsIn
//   0x746360 / 0x746780  GetPawnKnownReachability / IsPawnReachableBy
//   0x746480  public: unsigned int __thiscall FDisAIMonitorPawnReachability::KnowPlayerIsWithinHideOut(void)
//   0x746580  the reachability check itself (IDA has no name for it: sub_B46580)
//   0x747570  public: virtual void __thiscall FDisAIMonitorPawnReachability::Stopping(void)
//   0x747c10  private: void __thiscall FDisAIMonitorPawnReachability::UpdateCachedReachability(enum eDisReachability)
//   0x7480a0  public: virtual void __thiscall FDisAIMonitorPawnReachability::PreAsyncWorkTick(float)
//   0x7481e0  public: void __thiscall FDisAIMonitorPawnReachability::ResetWithNewMonitoredPawn(class ADishonoredPawn *, enum eDisReachability)

#include "DishonoredGame.h"
#include "disaimonitoractorreachability.h"

// DISHONORED(port): retail's registrant - the PDB names it FDisAIMonitorPawnReachability::s_TypeRegister and attributes
// its dynamic initializer to this unit (0xba93e0 in the 2012 build). Creator: 2013 rva 0x5eb8e0.
ARKCOMPONENT_IMPLEMENT_TYPE( FDisAIMonitorPawnReachability )

INT	FDisAIMonitorPawnReachability::ms_ComponentCount = 0;

// DISHONORED(port): 2013 rva 0x72bad0, member for member.
// DISHONORED(written): m_LastNavMeshCheckLocation is zeroed. Retail leaves the three floats uninitialised because the
// reachability check writes them before anything reads them.
FDisAIMonitorPawnReachability::FDisAIMonitorPawnReachability()
	: m_pOwningBrain( NULL )
	, m_fCheckPeriod( -1.f )
	, m_fReachableRecheckPeriod( 1.f )
	, m_pMonitoredPawn( NULL )
	, m_fTimeToNextCheck( FLT_MAX )
	, m_fTimeToNextRecheck( 0.f )
	, m_bNeedsMonitoredPawnReset( TRUE )
	, m_eCachedReachability( eDisReachability_Unknown )
	, m_LastNavMeshCheckLocation( 0.f, 0.f, 0.f )
{
}

// DISHONORED(port): 2013 rva 0x72bc50. Two references: the brain through the UObject* overload and the monitored pawn
// through the AActor* one, each written back only when the collector moved it.
void FDisAIMonitorPawnReachability::ManageReferences( FGCHelper* _pHelper )
{
	UDishonoredAIBrain* pBrain = _pHelper->manageReference( m_pOwningBrain );
	if( pBrain != m_pOwningBrain )
	{
		m_pOwningBrain = pBrain;
	}
	AActor* pPawn = _pHelper->manageReference( m_pMonitoredPawn );
	if( pPawn != m_pMonitoredPawn )
	{
		m_pMonitoredPawn = pPawn;
	}
}

/*
	DISHONORED(port): 2013 rva 0x73b010. FArkComponentBase::Serialize, then all eleven DWORDs of the component's own
	span in declaration order, then the same policy-membership INT the other two monitors write:

	  FArkComponentBase::Serialize                          m_pOwner (2 bytes), m_bStarted (4), m_bPendingStop (4)
	  [Ar vtable+0x18]( this+0x14 )                         m_pOwningBrain, 2 bytes
	  ByteOrderSerialize( this+0x18, 4 )                    m_fCheckPeriod
	  ByteOrderSerialize( this+0x1C, 4 )                    m_fReachableRecheckPeriod
	  [Ar vtable+0x18]( this+0x20 )                         m_pMonitoredPawn, 2 bytes
	  ByteOrderSerialize( this+0x24, 4 )                    m_fTimeToNextCheck
	  ByteOrderSerialize( this+0x28, 4 )                    m_fTimeToNextRecheck
	  ByteOrderSerialize( this+0x2C, 4 )                    m_bNeedsMonitoredPawnReset
	  INT t = this[12]; ByteOrderSerialize( &t, 4 ); this[12] = t     m_eCachedReachability, through an INT temp
	  ByteOrderSerialize( this+0x34, 4 ) x3                 m_LastNavMeshCheckLocation
	  ContainsItem on the JustBeforeProceduralAnim policy; ByteOrderSerialize( , 4 ); if TRUE, Register
	  if( Ar[+16] ) { if( this[2] ) ++ms_ComponentCount; }   i.e. if( Ar.IsLoading() && m_bStarted )

	Forty-four bytes of its own: 2 + 4 + 4 + 2 + 4 + 4 + 4 + 4 + 12 + 4.
*/
void FDisAIMonitorPawnReachability::Serialize( FArchive& _rArchive )
{
	FArkComponentBase::Serialize( _rArchive );

	_rArchive << *(UObject**)&m_pOwningBrain;
	_rArchive << m_fCheckPeriod;
	_rArchive << m_fReachableRecheckPeriod;
	_rArchive << *(UObject**)&m_pMonitoredPawn;
	_rArchive << m_fTimeToNextCheck;
	_rArchive << m_fTimeToNextRecheck;
	_rArchive << m_bNeedsMonitoredPawnReset;

	// retail's own shape: the member is the enumeration and the wire form is a fixed four bytes.
	INT CachedReachability = m_eCachedReachability;
	_rArchive << CachedReachability;
	m_eCachedReachability = (eDisReachability)CachedReachability;

	_rArchive << m_LastNavMeshCheckLocation;

	// DISHONORED(bringup): as in FDisAIMonitorReaction::Serialize - FArkComponentManager is not ported, so the
	// membership answer is FALSE on a save and the re-registration on a load has no list to join. The four bytes are
	// read either way.
	UBOOL bWasRegisteredWithPolicy = FALSE;
	_rArchive << bWasRegisteredWithPolicy;

	// DISHONORED(port): retail's tail. Starting is what normally increments the counter, and a component the save says
	// was already started never runs Starting again, so the restore has to account for it here.
	if( _rArchive.IsLoading() && m_bStarted )
	{
		ms_ComponentCount++;
	}
}

// DISHONORED(bringup): 2013 rva 0x7480a0 is the reachability monitor's whole behaviour: it runs down
// m_fTimeToNextCheck and m_fTimeToNextRecheck, and when the first expires runs the navmesh-and-path-find check
// (0x746580) and raises an FAIStimStruct_ReachabilityChange on the brain through OnBecomeReachable /
// OnBecomeUnReachable / OnUnReachabilityChange. All three of those go through
// UDishonoredAIBrain::HandleAIStim_Internal<FAIStimStruct_ReachabilityChange> and
// FDisAIKnowledgeComponent::GetMatchingKnowledge<14,FAIKnowledgeRecord_Pawn>, i.e. through the blackboard record
// classes that disaiblackboardrecords_descriptions.cpp does not have. The interface declares the method pure, so the
// body has to exist; it is left empty rather than half-written.
void FDisAIMonitorPawnReachability::PreAsyncWorkTick( FLOAT _fTimeStep )
{
}

/*
	DISHONORED(bringup): everything else this class does in retail is the reachability machinery behind that tick, and
	none of it reads a stream byte:
	  0x746580  the check: DoReachabilityCheck_OnNavmesh on the brain's own pawn and on the monitored pawn, then
	            DisComputeNearestNavMeshLocFromProxy and DoReachabilityCheck_PathFind, writing the result location into
	            m_LastNavMeshCheckLocation. Needs FDisAttentionProxy's proxy-location path and UNavigationHandle.
	  0x728a90 / 0x73e740  the two halves of that check.
	  0x73e7f0 / 0x73e890 / 0x73e930  the three stim raisers.
	  0x745310 / 0x745460 / 0x746360 / 0x746480 / 0x746780  the static knowledge queries, all of which read the
	            owning brain's blackboard through FAIKnowledgeRecord_Hideout / FAIKnowledgeRecord_Pawn.
	  0x747c10  UpdateCachedReachability: writes m_eCachedReachability and mirrors it into the blackboard.
	  0x7481e0  ResetWithNewMonitoredPawn: swaps m_pMonitoredPawn, moves the hideout-event subscription with it and
	            clears m_bNeedsMonitoredPawnReset.
	  0x72bc00 / 0x747570  Starting / Stopping: the policy registration, the brain pointer, the timer reset and
	            ms_ComponentCount. There is no FArkComponentManager here.
*/
