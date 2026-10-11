// UDisSeqVar_SpawnerPawn cpptext: included inside the generated class body.

// ---- agent FN (PHASE16 FN): the Kismet variable that stands for a spawner's spawned pawns ----
public:
	// DISHONORED(port): agent FN. Bodies in disseqvar_spawnerpawn.cpp.
	//   GetObjectRef    2013 rva 0x7a2f30 (2012 0x807420), 316 bytes
	//   GetObjectValue  2013 rva 0x786c00 (2012 0x7f3c50),  24 bytes
	//   GetValueStr     2013 rva 0x79cc90 (2012 0x807560), 257 bytes
	// Without GetObjectRef the variable reads as its ObjValue - the ADishonoredSpawner itself - and a
	// matinee AI group linked to it binds the spawner as its group actor (agent FK's hand-over item 2,
	// and the "an Actor may only be connected to one Group! [DishonoredSpawner_11]" line before the
	// wave-20 crash). Retail resolves it to the spawner's m_SpawnedPawns, one per index, NULL once past them.
	virtual UObject** GetObjectRef( INT Idx );
	UObject* GetObjectValue();
	virtual FString GetValueStr();
