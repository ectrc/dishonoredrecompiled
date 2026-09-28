// UDisAISubStateWithDesires cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. The sub-state that has desires of its own. It adds no data - it only says "yes, I am an
// IDisDesiresInterface", answers which pawn the desires belong to, and claims the AISubState priority band, which is
// higher than its behaviour's and lower than any sub-process's. Twenty of the twenty-four concrete sub-states derive
// from it; the four that do not (Init, GenericAction and their kin) state no desires at all.
// Bodies in Src/disaisubstatewithdesires.cpp.
public:
	// DISHONORED(port): 2013 rva 0x728150 (2012 0x765ac0) / 0x702800 (0x766b80) / 0x728160 (0x765ad0): the sub-state IS the
	// interface, and the pawn is its brain's pawn (m_pOwningBrain @80 -> m_pOwningPawn @348, which is what the 2013 body
	// `*(*(this - 31) + 348)` spells out from the interface sub-object at +204).
	virtual class IDisDesiresInterface* GetDesires();
	virtual class UObject* GetUObjectInterfaceDisDesiresInterface();
	virtual class ADishonoredNPCPawn* GetDesiresOwningPawn();

	/** DISHONORED(retail): the four priorities are all identical-code-folded in both builds, so they were read off the
	    vtable's folded targets and cross-checked against EDis*Priority: FaceTo 3 (2012 rva 0x94a6b0 `return 3`),
	    Loco 2 (0x4c05d0), LookAt 7 (0x965d20), BodyIntention 2 (0x4c05d0) - i.e. exactly DisFaceToPriority_AISubState,
	    DisLocoPriority_AISubState, DisLookAtPriority_AISubState and DisBodyIntentionPriority_AISubState. All eight
	    constants of the three WithDesires classes land on their own enumerator, which is what settles the declaration
	    order of these four slots. */
	virtual BYTE GetDesiresFaceToPriority() const { return DisFaceToPriority_AISubState; }
	virtual BYTE GetDesiresLocoPriority() const { return DisLocoPriority_AISubState; }
	virtual BYTE GetDesiresLookAtPriority() const { return DisLookAtPriority_AISubState; }
	virtual BYTE GetDesiresBodyIntentionPriority() const { return DisBodyIntentionPriority_AISubState; }

	// DISHONORED(port): 2013 rvas 0x728170 / 0x728180 (2012 0x765ae0 / 0x765af0): what the components call back into.
	void DesiresFaceToEventCallback( BYTE _FaceToEvent );
	void DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent );
