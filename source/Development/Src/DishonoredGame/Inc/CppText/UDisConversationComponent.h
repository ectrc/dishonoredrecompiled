// UDisConversationComponent cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent EC (PHASE11 EC), 2013 rva 0x896600 (2012 0x8dab60) - the component's own script
// properties, then which of the bound dialog tree's two running instances was live. Reached from
// ADishonoredNPCPawn::GameLoad_Dialog; body in dissavegame.cpp.
public:
	void SerializeForGameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	/** DISHONORED(written): retail's GameLoad_Dialog reads UActorComponent::bAttached directly; it is
	    protected in this tree and this is the only place outside the component hierarchy that needs it. */
	UBOOL IsAttached() const { return bAttached; }
