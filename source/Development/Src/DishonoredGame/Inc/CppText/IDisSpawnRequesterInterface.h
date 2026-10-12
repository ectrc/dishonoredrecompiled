// IDisSpawnRequesterInterface cpptext - included by the generated interface body in dishonoredgameclasses.h.
// DISHONORED(port, agent FI): the three the spawner calls through this interface, in retail's own
// order - ADishonoredSpawner::DoSpawnNow (2013 rva 0x65e790) calls interface vtable +8 for the
// allowance (its refusal is reason code 5) and +12 with the new pawn on success, and
// UDisSeqAct_StartSpawn::OnSpawnFailed (0x78acb0) is the third. Every implementer but
// UDisSeqAct_StartSpawn takes these defaults.
// (Agent FX moved them out of the generated header into this file, the mechanism the generator preserves.)
virtual INT GetAllowedSpawnCount() { return 1; }
virtual void OnPawnSpawned( class ADishonoredNPCPawn* Pawn ) {}
virtual void OnSpawnFailed( INT Reason ) {}
