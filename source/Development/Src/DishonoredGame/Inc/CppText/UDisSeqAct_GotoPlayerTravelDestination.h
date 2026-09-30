// UDisSeqAct_GotoPlayerTravelDestination cpptext: included inside the generated class body.

// ---- agent EL (PHASE12 EL): where the player comes out of a map change ----
public:
	// DISHONORED(port): agent EL, 2013 rva 0x79a940. Finds the ANavigationPoint whose Tag is the destination
	// UDisSeqAct_SetPlayerTravelDestination recorded and teleports the player onto it. Body in
	// dishonoredkismet.cpp.
	//
	// The address is not a symbol either, and the matcher is wrong about this class: it maps the 2012 vtable
	// 0xd420e0 to 2013 0xd52cf0, whose +372 is 0x78f600 - a body that calls ADishonoredPawn::SetMinimumScriptedHeatlh,
	// i.e. UDisSeqAct_LimitPawnMinHealth::Activated. The real 2013 vtable is 0xd4e738 (the operand of the
	// InternalConstructor at 0x7b7e70+18, not the 0x7b7fa0 the matcher names), it is the 300-slot table that
	// ends exactly where UDisSeqAct_SetPlayerTravelDestination's begins at 0xd4ebe8, and its +372 is 0x79a940.
	virtual void Activated();
