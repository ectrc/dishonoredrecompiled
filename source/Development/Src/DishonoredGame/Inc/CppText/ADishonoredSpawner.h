// ADishonoredSpawner cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. Every NPC in Dishonored is spawned by one of these: the census of agentCG.md found 41
// DishonoredSpawner actors and zero placed NPC pawns on L_Tower_P, so without this class the AI brain has nobody to
// think for. A spawner holds the NPC's tweaks (m_pPawnTweaks, which carry the brain tweaks), an actor factory
// sub-object, a three-deep pending-spawn ring and the squad it counts against.
//
// The per-subclass tweaks trap that cost agent AU 147 pickups applies HERE in its sharpest form: DoSpawnNow reaches the
// NPC tweaks through the spawner's IDisTweaksInterface, so without GetTweaks_Derived below it would read the all-zero
// class default of UDisTweaks_NPCPawn, find m_pBrainTweak NULL, and refuse to spawn anything at all - silently.
public:
	virtual void PostBeginPlay();
	virtual UBOOL Tick( FLOAT _fDeltaTime, enum ELevelTick _TickType );
	virtual void OnStartSpawn( class UDisSeqAct_StartSpawn* _pAction );

	UBOOL SpawnOnePawn( const struct FDisSpawnInfo* _pSpawnInfo );
	BYTE DoSpawnNow( const struct FDisSpawnInfo& _rSpawnInfo );
	void ClearPendingSpawns();
	UBOOL IsMinDelaySinceLastSpawnElapsed( FLOAT* const _pOutDelay ) const;

	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );

protected:
	virtual void OnSpawned( class ADishonoredNPCPawn* _pPawn );
