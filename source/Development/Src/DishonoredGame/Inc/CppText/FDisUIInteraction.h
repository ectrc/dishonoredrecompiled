// FDisUIInteraction cpptext: included inside the generated struct body (DishonoredGameUIClasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). 2013 dropped 2012's QTE bit (the struct is 8 bytes: the index and the text).
// Bodies in Src/disgfxmovieplayerhud.cpp.
public:
	// 2013 rva 0x787c90 (2012 0x7f4810): an interaction with its own text
	FDisUIInteraction( BYTE _Index, const FString* _pText );
	// 2013 rva 0x78cb00 (2012 0x7f6330): an interaction with the HUD tweaks' text for its index
	explicit FDisUIInteraction( BYTE _Index );
