// UDisAISubProcessWithDesires cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. The highest of the three bands: a sub-process (head tracking, ambient barks, personal
// space) overrides both its sub-state and its behaviour, which is how an NPC keeps glancing at a distraction while
// walking to a position its sub-state chose. Bodies in Src/disaisubstatewithdesires.cpp.
public:
	// DISHONORED(port): 2012 rvas 0x78d280 / 0x766990 / 0x78d290. The interface sub-object is at +104 here, against +204
	// on a sub-state, which is why the UObject accessor is a different function on each of the three.
	virtual class IDisDesiresInterface* GetDesires();
	virtual class UObject* GetUObjectInterfaceDisDesiresInterface();
	virtual class ADishonoredNPCPawn* GetDesiresOwningPawn();

	/** DISHONORED(retail): read off the folded vtable targets: FaceTo 4 (2012 rva 0x94a6e0), Loco 3 (0x94a6b0),
	    LookAt 8 (0x731c50), BodyIntention 3 (0x94a6b0) = the four DisXxxPriority_AISubProcessBase values. A sub-process
	    that wants to outrank its siblings overrides these again with its own band (…AISubProcessDistractions and friends). */
	virtual BYTE GetDesiresFaceToPriority() const { return DisFaceToPriority_AISubProcessBase; }
	virtual BYTE GetDesiresLocoPriority() const { return DisLocoPriority_AISubProcessBase; }
	virtual BYTE GetDesiresLookAtPriority() const { return DisLookAtPriority_AISubProcessBase; }
	virtual BYTE GetDesiresBodyIntentionPriority() const { return DisBodyIntentionPriority_AISubProcessBase; }

	// DISHONORED(port): 2012 rvas 0x78d2a0 / 0x78d2b0
	void DesiresFaceToEventCallback( BYTE _FaceToEvent );
	void DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent );
