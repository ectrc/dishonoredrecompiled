// UDisPatrolManager cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent EP. One per map, a default subobject of UDishonoredMapInfo. It owns nothing but the head
// of a single-linked list of every ADishonoredRoute in the level, threaded through ADishonoredRoute::m_pNextRoute, and
// the choice of which route an NPC adopts. Bodies in Src/dispatrolmanager.cpp.
public:
	void RegisterPatrolRoute( class ADishonoredRoute* const _pRoute );
	void UnRegisterPatrolRoute( class ADishonoredRoute* const _pRoute );
	UBOOL HasPatrolRoutes() const;
	UBOOL AdoptNewRoute( class ADishonoredNPCPawn* const _pPawn, const FVector& _rFrom,
		class ADishonoredRoute** const _pInOutRoute, INT* const _pOutIndex ) const;
