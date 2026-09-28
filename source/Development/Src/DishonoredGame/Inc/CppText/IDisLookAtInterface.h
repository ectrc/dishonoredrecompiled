// IDisLookAtInterface cpptext: included inside the generated interface body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Something an NPC can look at." An actor that implements it nominates the point the
// look-at should track, which is how a guard looks at another guard's head rather than at his feet, and how a look-at
// follows a moving target without the asker re-stating it.
// DISHONORED(layout): 2012 PDB IDisLookAtInterface_vtbl, 4 slots: +0 destructor, +4 the UObject accessor
// ToScriptInterface<IDisLookAtInterface> calls (2012 rva 0x8fd0a0), +8 the owning actor (read by
// FDisDesireRequest::ReferencesActor, 2013 rva 0x8a9ad0), +12 the location (read by FDisDesireRequest::GetRequestStatus,
// 2013 rva 0x8b3e90). Both readers are in Src/disdesirestructs.cpp.
// DISHONORED(bringup): no class in the tree implements the interface yet - retail's implementors are the pawn, the
// corpse, the interactable and the conversation speaker - so every look-at desire in this wave resolves through its
// actor or its attention proxy instead. The two readers are written against the interface so that the first implementor
// needs no change here.
public:
	virtual class UObject* GetUObjectInterfaceDisLookAtInterface() { return NULL; }
	virtual class AActor* GetLookAtOwnerActor() { return NULL; }
	virtual FVector GetLookAtLocation() { return FVector( 0.f, 0.f, 0.f ); }
