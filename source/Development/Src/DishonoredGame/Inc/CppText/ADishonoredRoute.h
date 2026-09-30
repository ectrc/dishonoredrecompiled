// ADishonoredRoute cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent EP. A patrol route: an ARoute whose RouteList is the ordered list of points, plus the
// bookkeeping that lets exactly m_Capacity NPCs adopt it at a time and that ages an unused route so a second NPC
// prefers it. Registered with the map info's UDisPatrolManager by PostBeginPlay, which is the only thing that puts a
// route where UDisBehaviorPatrol can find it. Bodies in Src/dishonoredroute.cpp.
public:
	virtual void PostBeginPlay();
	virtual void PostScriptDestroyed();
	virtual void BeginDestroy();
	virtual void MarkComponentsAsPendingKill( UBOOL _bDetachComponents );
	virtual UBOOL Tick( FLOAT _fDeltaTime, enum ELevelTick _TickType );
	virtual INT ResolveRouteIndex( INT _Idx, BYTE _RouteDirection, BYTE& _rOutComplete, BYTE& _rOutReverse );
	virtual void OnToggle( class USeqAct_Toggle* _pAction );
	virtual void HandleSquadNameChange( const FName& _rOldSquadName, const FName& _rNewSquadName );

	void Adopt();
	void RemoveAdoption();
	UBOOL IsNeglected() const;
	UBOOL IsRouteActive() const;
	UBOOL CanAdopt( const class ADishonoredNPCPawn* const _pPawn, UBOOL _bIgnoreAdoptionRange ) const;
	FVector GetClosestPoint( const FVector& _rFrom ) const;
	INT MoveOntoRoutePath( const FVector& _rFrom );

	/** The route the patrol manager keeps after this one in its single-linked list. */
	class ADishonoredRoute** GetNextRoutePtr() { return &m_pNextRoute; }

	// DISHONORED(port): agent ER, 2013 rvas 0x643890 / 0x6438f0 - five bytes past AActor's: whether the
	// route is active, and how long it has been neglected. Bodies in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
