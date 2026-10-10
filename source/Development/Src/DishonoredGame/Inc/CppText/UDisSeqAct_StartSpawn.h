// UDisSeqAct_StartSpawn cpptext: included inside the generated class body.

// ---- agent FI (PHASE14 FI): the Kismet spawn action, and the latent op it leaves hanging ----
public:
	// DISHONORED(port): agent FI. Bodies in disseqact_startspawn.cpp.
	//   Activated      2013 rva 0x79bcc0 (2012 0x7d7480), 305 bytes
	//   UpdateOp       2013 rva 0x790050 (2012 0x7d2dd0), 293 bytes
	//   OnPawnSpawned  2013 rva 0x78aca0 (2012 0x7cdbd0), 15 bytes, IDisSpawnRequesterInterface
	//   OnSpawnFailed  2013 rva 0x78acb0 (2012 0x7cdbe0), 15 bytes, IDisSpawnRequesterInterface
	virtual void Activated();
	virtual UBOOL UpdateOp( FLOAT DeltaTime );
	virtual void OnPawnSpawned( class ADishonoredNPCPawn* Pawn );
	virtual void OnSpawnFailed( INT Reason );
