// UDisAIBehaviorWithDesires cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. As UDisAISubStateWithDesires, one band lower: a behaviour's desire is what the NPC wants
// while no sub-state has an opinion. Bodies in Src/disaisubstatewithdesires.cpp (retail keeps all three WithDesires
// classes in their own units; they are two accessors each, so they share one here).
public:
	// DISHONORED(port): 2013 rvas of the 2012 0x722460 / 0x722470 pair; GetUObjectInterfaceDisDesiresInterface is folded
	// onto another class's identical `return this - <offset>` in both builds.
	virtual class IDisDesiresInterface* GetDesires();
	virtual class UObject* GetUObjectInterfaceDisDesiresInterface();
	virtual class ADishonoredNPCPawn* GetDesiresOwningPawn();

	/** DISHONORED(retail): read off the folded vtable targets, as UDisAISubStateWithDesires: FaceTo 2 (2012 rva 0x4c05d0),
	    Loco 1 (0x948150), LookAt 6 (0x495430), BodyIntention 1 (0x948150) = the four DisXxxPriority_AIBehavior values. */
	virtual BYTE GetDesiresFaceToPriority() const { return DisFaceToPriority_AIBehavior; }
	virtual BYTE GetDesiresLocoPriority() const { return DisLocoPriority_AIBehavior; }
	virtual BYTE GetDesiresLookAtPriority() const { return DisLookAtPriority_AIBehavior; }
	virtual BYTE GetDesiresBodyIntentionPriority() const { return DisBodyIntentionPriority_AIBehavior; }

	// DISHONORED(port): 2012 rvas 0x722480 / 0x722490
	void DesiresFaceToEventCallback( BYTE _FaceToEvent );
	void DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent );
