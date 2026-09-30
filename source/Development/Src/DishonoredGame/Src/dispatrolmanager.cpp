// DishonoredGame/src/dispatrolmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8), 2012 rvas:
//   0x8af180  public: static void __cdecl UDisPatrolManager::InitializePrivateStaticClassUDisPatrolManager(void)
//   0x8af1a0  GetNextRoutePtr
//   0x8af1b0  public: unsigned int __thiscall UDisPatrolManager::HasPatrolRoutes(void)const
//   0x8b1210  public: unsigned int __thiscall UDisPatrolManager::AdoptNewRoute(class ADishonoredNPCPawn * const, class FVector const &, class ADishonoredRoute * * const, int * const)const
//   0x8b81b0  public: static class UClass * __cdecl UDisPatrolManager::GetPrivateStaticClassUDisPatrolManager(wchar_t const *)
//   0x8b8240  public: void __thiscall UDisPatrolManager::RegisterPatrolRoute(class ADishonoredRoute * const)
//   0x8b8270  public: void __thiscall UDisPatrolManager::UnRegisterPatrolRoute(class ADishonoredRoute * const)
//   0x8bc640  public: static class UClass * __cdecl UDisPatrolManager::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent EP (PHASE14 EP): who owns the level's routes, and which one an NPC takes.

	The manager holds one pointer, m_pRouteList, the head of a list threaded through ADishonoredRoute::m_pNextRoute; it
	is a default subobject of UDishonoredMapInfo, so it already exists in every map this tree loads. Measured on
	L_Tower_P before this package: "map info yes, patrol manager yes (route list empty); 6 routes" - the manager was
	there and empty, because ADishonoredRoute::PostBeginPlay was the unported half.

	AdoptNewRoute is the whole policy and it is two nearest-point searches rather than one: the nearest NEGLECTED route
	wins outright, and the nearest un-neglected route is the answer only when no neglected route exists AND the NPC
	holds no route yet. So an NPC already on a route will move to a neglected one but never swap to a merely closer one.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x847d10 (2012 0x8b8240): push onto the head of the list.
void UDisPatrolManager::RegisterPatrolRoute( ADishonoredRoute* const _pRoute )
{
	if( !_pRoute )
	{
		return;
	}
	if( m_pRouteList )
	{
		_pRoute->m_pNextRoute = m_pRouteList;
	}
	m_pRouteList = _pRoute;
}

// DISHONORED(port): 2013 rva 0x847d40 (2012 0x8b8270): unlink, and clear the route's own link whatever happened - which
// is what makes this safe to call three times for one route, as the three destruction paths of ADishonoredRoute do.
void UDisPatrolManager::UnRegisterPatrolRoute( ADishonoredRoute* const _pRoute )
{
	if( !_pRoute )
	{
		return;
	}
	if( m_pRouteList == _pRoute )
	{
		m_pRouteList = _pRoute->m_pNextRoute;
	}
	else
	{
		for( ADishonoredRoute* Route = m_pRouteList; Route; Route = Route->m_pNextRoute )
		{
			if( Route->m_pNextRoute == _pRoute )
			{
				Route->m_pNextRoute = _pRoute->m_pNextRoute;
				break;
			}
		}
	}
	_pRoute->m_pNextRoute = NULL;
}

// DISHONORED(port): 2013 rva 0x83df20 (2012 0x8af1b0)
UBOOL UDisPatrolManager::HasPatrolRoutes() const
{
	return m_pRouteList != NULL;
}

// DISHONORED(port): 2013 rva 0x841590 (2012 0x8b1210): pick a route for this pawn and say where on it to join.
// _pInOutRoute is read as well as written: a route the NPC already holds is excluded from the search, and when only
// un-neglected routes are available an NPC that already holds one keeps it.
UBOOL UDisPatrolManager::AdoptNewRoute( ADishonoredNPCPawn* const _pPawn, const FVector& _rFrom,
	ADishonoredRoute** const _pInOutRoute, INT* const _pOutIndex ) const
{
	if( !m_pRouteList || !_pInOutRoute || !_pOutIndex )
	{
		return FALSE;
	}

	ADishonoredRoute* NearestUnneglected = NULL;
	ADishonoredRoute* NearestNeglected = NULL;
	FLOAT NearestUnneglectedSq = 3.4028235e38f;
	FLOAT NearestNeglectedSq = 3.4028235e38f;

	for( ADishonoredRoute* Route = m_pRouteList; Route; Route = Route->m_pNextRoute )
	{
		if( Route == *_pInOutRoute || !Route->CanAdopt( _pPawn, FALSE ) )
		{
			continue;
		}
		const FLOAT DistSq = ( _rFrom - Route->GetClosestPoint( _rFrom ) ).SizeSquared();
		if( Route->IsNeglected() )
		{
			if( NearestNeglectedSq > DistSq )
			{
				NearestNeglectedSq = DistSq;
				NearestNeglected = Route;
			}
		}
		else if( NearestUnneglectedSq > DistSq )
		{
			NearestUnneglectedSq = DistSq;
			NearestUnneglected = Route;
		}
	}

	ADishonoredRoute* Chosen = NearestNeglected;
	if( Chosen )
	{
		if( *_pInOutRoute )
		{
			(*_pInOutRoute)->RemoveAdoption();
		}
	}
	else
	{
		Chosen = NearestUnneglected;
		if( !Chosen || *_pInOutRoute )
		{
			return FALSE;
		}
	}

	Chosen->Adopt();
	*_pInOutRoute = Chosen;
	const INT Index = Chosen->MoveOntoRoutePath( _rFrom );
	*_pOutIndex = ( Index == INDEX_NONE ) ? 0 : Index;
	return TRUE;
}
