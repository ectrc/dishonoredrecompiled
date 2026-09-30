// UDisSeqAct_SetPlayerTravelDestination cpptext: included inside the generated class body.

// ---- agent EL (PHASE12 EL): where the player comes out of a map change ----
public:
	// DISHONORED(port): agent EL, 2013 rva 0x78a0a0. Records m_Tag and the package name of the level this
	// action ran in on ADishonoredPlayerController, so that the level the map change brings up can place the
	// player with UDisSeqAct_GotoPlayerTravelDestination. Body in dishonoredkismet.cpp.
	//
	// The address is not a symbol: ?Activated@UDisSeqAct_SetPlayerTravelDestination@@UAEXXZ is unmatched in
	// match_2012_2013.csv, so it is read out of the class vtable ??_7UDisSeqAct_SetPlayerTravelDestination@@6B@
	// (2013 0xd4ebe8, the operand of InternalConstructor at 0x7b82c0+18) slot +372, the Activated slot -
	// USequenceOp::DeActivated is +376 and retail's save layer inserted UObject::IsRefSaveable at +368.
	virtual void Activated();
