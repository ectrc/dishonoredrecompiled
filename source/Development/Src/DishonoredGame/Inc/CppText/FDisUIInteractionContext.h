// FDisUIInteractionContext cpptext: included inside the generated struct body (DishonoredGameUIClasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). Bodies in Src/disgfxmovieplayerhud.cpp.
public:
	// 2013 rva 0x787d20 (2012 0x7f48b0): the interaction's group from its index range (2013 adds the DLC06/DLC07 ranges)
	void Add( const FDisUIInteraction& _rInteraction );
	// 2013 rva 0x787da0 (2012 0x7f48e0): the group's count or any of its texts differs
	UBOOL HasGroupChanged( INT _Group, const FDisUIInteractionContext& _rOther ) const;
