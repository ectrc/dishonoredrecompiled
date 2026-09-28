// FDisBodyIntentionRequest cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Hold this stance, with these items in hand." Unlike the other three requests there is no
// Ark component behind it: the request writes straight into ADishonoredNPCPawn::m_BodyIntention at its own priority, which
// agent CG ported, so this one is complete. Bodies in Src/disdesirestructs.cpp.
public:
	// DISHONORED(port): 2013 rva 0x8b2440 (2012 0x8ffbd0): re-pointing the request at another pawn first withdraws the
	// intention it had placed on the old one.
	void Initialize( class ADishonoredNPCPawn* _pNPCPawn, BYTE _Priority );
	/** DISHONORED(port): 2013 rva 0x8a8050 (2012 0x8f9070). Returns FALSE when an item the caller asked for is not in the
	    NPC's inventory: the stance is still taken, but with that hand empty, and the caller is told so. */
	UBOOL RequestBodyIntention( BYTE _BodyStance, class UClass* _pPrimaryItemClass, class UClass* _pSecondaryItemClass );
	/** DISHONORED(written): the withdraw half, which retail inlines at all six of its call sites
	    (IDisDesiresInterface::ClearBodyIntentionDesire 0x8ae040, StopDesires 0x8b22d0, PauseDesires 0x8ba360,
	    FinalizeDesires 0x8b3cb0, InitializeDesires 0x8ba1f0 and Initialize above). */
	void ClearIntention();
