// DishonoredGame/src/dishonoredroute.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20), 2012 rvas:
//   0x6917f0  public: static void __cdecl ADishonoredRoute::InitializePrivateStaticClassADishonoredRoute(void)
//   0x691810  public: virtual unsigned int __thiscall ADishonoredRoute::Tick(float, enum ELevelTick)
//   0x691850  public: void __thiscall ADishonoredRoute::Adopt(void)
//   0x691870  public: void __thiscall ADishonoredRoute::RemoveAdoption(void)
//   0x691880  public: unsigned int __thiscall ADishonoredRoute::IsNeglected(void)const
//   0x6918a0  public: unsigned int __thiscall ADishonoredRoute::IsRouteActive(void)const
//   0x6918b0  public: virtual void __thiscall ADishonoredRoute::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x691910  public: virtual void __thiscall ADishonoredRoute::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6943d0  public: virtual int __thiscall ADishonoredRoute::ResolveRouteIndex(int, unsigned char, unsigned char &, unsigned char &)
//   0x697bb0  public: virtual void __thiscall ADishonoredRoute::PostBeginPlay(void)
//   0x697c10  public: virtual void __thiscall ADishonoredRoute::PostScriptDestroyed(void)
//   0x697c50  public: virtual void __thiscall ADishonoredRoute::BeginDestroy(void)
//   0x697c90  public: virtual void __thiscall ADishonoredRoute::MarkComponentsAsPendingKill(unsigned int)
//   0x697cd0  public: unsigned int __thiscall ADishonoredRoute::CanAdopt(class ADishonoredNPCPawn * const)const
//   0x697e00  public: class FVector __thiscall ADishonoredRoute::GetClosestPoint(class FVector const &)const
//   0x697f10  public: virtual void __thiscall ADishonoredRoute::OnToggle(class USeqAct_Toggle *)
//   0x697fd0  public: virtual void __thiscall ADishonoredRoute::HandleSquadNameChange(class FName const &, class FName const &)
//   0x698050  public: int __thiscall ADishonoredRoute::MoveOntoRoutePath(class FVector const &)
//   0x6a4f00  public: static class UClass * __cdecl ADishonoredRoute::GetPrivateStaticClassADishonoredRoute(wchar_t const *)
//   0x6a6dc0  public: static class UClass * __cdecl ADishonoredRoute::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent EP (PHASE14 EP): the patrol route.

	A route is an ARoute - RouteType plus RouteList, the ordered list of point actors - with four Dishonored additions:
	a squad filter, a capacity, an adoption range, and a neglect clock. The clock is the interesting one: a route with
	no adopter ages (Tick), and once it is older than m_NeglectThreshold UDisPatrolManager::AdoptNewRoute prefers it
	over any un-neglected route however far away, which is how a level's routes get shared out rather than all NPCs
	crowding onto the nearest one.

	Every body here is measured against retail 2013. The address of each is on the function; ADishonoredRoute's member
	offsets are confirmed twice - once by arithmetic from the generated header's reflected span (612..660) and once
	against resources/docs/types/retail_sdk_layout.json, which puts the IDisSquadInterface vftable at 612 and
	m_SupportedSquads at 616, and every offset the decompiles read falls on a named member of that table.

	Not ported: GameSave (2013 rva 0x643890) and GameLoad (0x6438f0). The writing half of the object layer does not
	exist in this tree (agent ED's reason) and the reading half would need this class on the save class list; a
	restored save therefore brings back a route with its default m_bIsActive and no adopters, which is the state a
	freshly begun level is in anyway.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "dishonoredutilities_saveload.h"

// DISHONORED(port): 2013 rva 0x64bb50 (2012 0x697bb0): the one line that makes a route findable. The route threads
// itself onto the map info's patrol manager list and starts its neglect clock ALREADY expired - m_NeglectThreshold + 1 -
// so the first NPC to look for a route finds every route neglected and takes the nearest one.
void ADishonoredRoute::PostBeginPlay()
{
	Super::PostBeginPlay();

	m_pNextRoute = NULL;
	UDishonoredMapInfo* MapInfo = GWorld && GWorld->GetWorldInfo() ? Cast<UDishonoredMapInfo>( GWorld->GetWorldInfo()->GetMapInfo() ) : NULL;
	if( MapInfo && MapInfo->m_pPatrolManager )
	{
		MapInfo->m_pPatrolManager->RegisterPatrolRoute( this );
	}
	m_NeglectTimer = m_NeglectThreshold + 1.f;
}

/** The three ways a route leaves the world all take it off the manager's list first; retail spells the same four lines
    three times (2013 rvas 0x64bbb0, 0x64bbf0, 0x64bc30) rather than sharing a helper. */
static void DisUnregisterRouteFromPatrolManager( ADishonoredRoute* _pRoute )
{
	if( !GWorld || !GWorld->GetWorldInfo() )
	{
		return;
	}
	UDishonoredMapInfo* MapInfo = Cast<UDishonoredMapInfo>( GWorld->GetWorldInfo()->GetMapInfo() );
	if( MapInfo && MapInfo->m_pPatrolManager )
	{
		MapInfo->m_pPatrolManager->UnRegisterPatrolRoute( _pRoute );
	}
}

// DISHONORED(port): 2013 rva 0x64bbb0 (2012 0x697c10)
void ADishonoredRoute::PostScriptDestroyed()
{
	DisUnregisterRouteFromPatrolManager( this );
	Super::PostScriptDestroyed();
}

// DISHONORED(port): 2013 rva 0x64bbf0 (2012 0x697c50)
void ADishonoredRoute::BeginDestroy()
{
	DisUnregisterRouteFromPatrolManager( this );
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x64bc30 (2012 0x697c90)
void ADishonoredRoute::MarkComponentsAsPendingKill( UBOOL _bDetachComponents )
{
	DisUnregisterRouteFromPatrolManager( this );
	Super::MarkComponentsAsPendingKill( _bDetachComponents );
}

// DISHONORED(port): 2013 rva 0x6437f0 (2012 0x691810): the neglect clock only runs while nobody is on the route.
UBOOL ADishonoredRoute::Tick( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	const UBOOL bResult = Super::Tick( _fDeltaTime, _TickType );
	if( m_NumNPCAdopters < 1 )
	{
		m_NeglectTimer += _fDeltaTime;
	}
	return bResult;
}

// DISHONORED(port): 2013 rva 0x643830 (2012 0x691850)
void ADishonoredRoute::Adopt()
{
	m_NumNPCAdopters++;
	m_NeglectTimer = 0.f;
}

// DISHONORED(port): 2013 rva 0x643850 (2012 0x691870). Retail does not clamp at zero, and neither does this: the
// counter is only ever decremented by the behaviour that incremented it.
void ADishonoredRoute::RemoveAdoption()
{
	m_NumNPCAdopters--;
}

// DISHONORED(port): 2013 rva 0x643860 (2012 0x691880)
UBOOL ADishonoredRoute::IsNeglected() const
{
	return m_NeglectTimer > m_NeglectThreshold;
}

// DISHONORED(port): 2013 rva 0x643880 (2012 0x6918a0)
UBOOL ADishonoredRoute::IsRouteActive() const
{
	return m_bIsActive ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x64bc70 (2012 0x697cd0, where it took no second argument): may this pawn take this
// route. Five tests, in retail's order, and the fifth is the one that matters in practice - a route with any empty
// RouteList entry is refused outright, because FindNextPoint would dereference it.
// DISHONORED(retail): the second parameter is new in 2013 and its only effect is an exception for one map: when it is
// TRUE *and* the current level's package is L_Isl_LowChaos_P the adoption-range test is skipped. It is TRUE only from
// UDisBehaviorPatrol::OnPostGameLoad, i.e. a route the NPC already held across a save is kept however far away it is
// on that one map. The map name is retail's own string constant.
UBOOL ADishonoredRoute::CanAdopt( const ADishonoredNPCPawn* const _pPawn, UBOOL _bIgnoreAdoptionRange ) const
{
	UBOOL bSkipRangeTest = FALSE;
	if( _bIgnoreAdoptionRange )
	{
		ULevel* CurrentLevel = DisGetCurrentLevel();
		if( CurrentLevel )
		{
			bSkipRangeTest = ( CurrentLevel->GetOutermost()->GetFName() == FName( TEXT("L_Isl_LowChaos_P") ) );
		}
	}

	if( !m_bIsActive || RouteList.Num() <= 0 || !_pPawn )
	{
		return FALSE;
	}
	if( !IDisSquadInterface::IsSquadSupported( m_SupportedSquads, _pPawn->m_SpawnerInfo.m_Squad ) )
	{
		return FALSE;
	}
	if( m_Capacity >= 0 && m_NumNPCAdopters >= m_Capacity )
	{
		return FALSE;
	}
	if( !bSkipRangeTest && m_AdoptionRange >= 0.f )
	{
		if( ( _pPawn->Location - Location ).SizeSquared() >= ( m_AdoptionRange * m_AdoptionRange ) )
		{
			return FALSE;
		}
	}
	for( INT Idx = 0; Idx < RouteList.Num(); Idx++ )
	{
		if( !RouteList(Idx).Actor )
		{
			return FALSE;
		}
	}
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x64be30 (2012 0x697e00): the route point nearest a location, in squared distance, with
// the route's own Location as the answer when every entry is empty.
FVector ADishonoredRoute::GetClosestPoint( const FVector& _rFrom ) const
{
	FVector Closest = Location;
	FLOAT BestSq = 3.4028235e38f;
	for( INT Idx = 0; Idx < RouteList.Num(); Idx++ )
	{
		const AActor* Point = RouteList(Idx).Actor;
		if( !Point )
		{
			continue;
		}
		const FLOAT DistSq = ( _rFrom - Point->Location ).SizeSquared();
		if( BestSq > DistSq )
		{
			Closest = Point->Location;
			BestSq = DistSq;
		}
	}
	return Closest;
}

// DISHONORED(port): 2013 rva 0x647130 (2012 0x6943d0): the index one step on from _Idx, wrapped or turned round
// according to the route's type. Retail's switch has five arms; ERouteType has three values, so arms 3 and 4 are
// unreachable with legal data and are kept here only because retail's own grouping is what documents the other three -
// ERT_Circle shares its arm with 3 and ERT_Loop with 4.
//   ERT_Linear  the route ends: out_bComplete, and the index becomes INDEX_NONE
//   ERT_Loop    the route turns round: out_bReverse, and the index steps back inside the range
//   ERT_Circle  the route wraps to the other end
INT ADishonoredRoute::ResolveRouteIndex( INT _Idx, BYTE _RouteDirection, BYTE& _rOutComplete, BYTE& _rOutReverse )
{
	INT Result = _Idx;
	if( _RouteDirection == ERD_Forward )
	{
		if( _Idx >= RouteList.Num() )
		{
			switch( RouteType )
			{
			case ERT_Linear:
				_rOutComplete = 1;
				Result = INDEX_NONE;
				break;
			case ERT_Loop:
			case 4:
				_rOutReverse = 1;
				Result = RouteList.Num() - 2;
				break;
			case ERT_Circle:
			case 3:
				Result = 0;
				break;
			default:
				break;
			}
		}
	}
	else if( _RouteDirection == ERD_Reverse && _Idx < 0 )
	{
		switch( RouteType )
		{
		case ERT_Linear:
			_rOutComplete = 1;
			Result = INDEX_NONE;
			break;
		case ERT_Loop:
		case 4:
			_rOutReverse = 1;
			Result = 1;
			break;
		case ERT_Circle:
		case 3:
			Result = RouteList.Num() - 1;
			break;
		default:
			break;
		}
	}
	return Result;
}

// DISHONORED(port): 2013 rva 0x64c080 (2012 0x698050): where on the route an NPC standing at _rFrom should join it.
// Retail finds the nearest point, then asks for the point AFTER it (plus RouteIndexOffset) and prefers that one when
// the NPC is already closer to it than the two points are to each other - which stops an NPC that has walked past a
// point from turning round to touch it.
INT ADishonoredRoute::MoveOntoRoutePath( const FVector& _rFrom )
{
	INT NearestIdx = INDEX_NONE;
	FLOAT NearestSq = 0.f;
	for( INT Idx = 0; Idx < RouteList.Num(); Idx++ )
	{
		const AActor* Point = RouteList(Idx).Actor;
		if( !Point )
		{
			continue;
		}
		const FLOAT DistSq = ( Point->Location - _rFrom ).SizeSquared();
		if( NearestIdx < 0 || NearestSq > DistSq )
		{
			NearestIdx = Idx;
			NearestSq = DistSq;
		}
	}

	BYTE bComplete = 0;
	BYTE bReverse = 0;
	const INT NextIdx = ResolveRouteIndex( RouteIndexOffset + NearestIdx + 1, ERD_Forward, bComplete, bReverse );
	if( NextIdx < 0 )
	{
		return RouteList.Num() - 1;
	}
	if( NearestIdx < 0 || NextIdx >= RouteList.Num() )
	{
		return NextIdx;
	}
	const AActor* NextPoint = RouteList(NextIdx).Actor;
	const AActor* NearestPoint = RouteList(NearestIdx).Actor;
	if( !NextPoint || !NearestPoint )
	{
		return NextIdx;
	}
	const FLOAT SpanBetweenPoints = ( NearestPoint->Location - NextPoint->Location ).Size();
	if( ( _rFrom - NextPoint->Location ).Size() < SpanBetweenPoints )
	{
		return NextIdx;
	}
	return NearestIdx;
}

// DISHONORED(port): 2013 rva 0x64bf40 (2012 0x697f10): the Kismet Toggle action. Link 0 turns the route on, link 1
// turns it off, and any other impulse flips it.
void ADishonoredRoute::OnToggle( USeqAct_Toggle* _pAction )
{
	if( !_pAction || _pAction->InputLinks.Num() < 1 )
	{
		return;
	}
	if( _pAction->InputLinks(0).bHasImpulse )
	{
		m_bIsActive = TRUE;
	}
	else if( _pAction->InputLinks.Num() > 1 && _pAction->InputLinks(1).bHasImpulse )
	{
		m_bIsActive = FALSE;
	}
	else
	{
		m_bIsActive = m_bIsActive ? FALSE : TRUE;
	}
}

// DISHONORED(port): 2013 rva 0x64c000 (2012 0x697fd0): a squad was renamed, so every entry of this route's filter that
// named the old squad now names the new one.
void ADishonoredRoute::HandleSquadNameChange( const FName& _rOldSquadName, const FName& _rNewSquadName )
{
	for( INT Idx = 0; Idx < m_SupportedSquads.Num(); Idx++ )
	{
		if( m_SupportedSquads(Idx).m_SquadName == _rOldSquadName )
		{
			m_SupportedSquads(Idx).m_SquadName = _rNewSquadName;
		}
	}
}
