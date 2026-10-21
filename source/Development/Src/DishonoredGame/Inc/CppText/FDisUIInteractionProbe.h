// FDisUIInteractionProbe cpptext: included inside the generated struct body (DishonoredGameUIClasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). 2013 dropped 2012's timers and Tick (2012 0x80c000): the HUD's PreAdvance runs the five
// checks every frame (2013 0x7b2280). Bodies in Src/disgfxmovieplayerhud.cpp.
public:
	UBOOL FillUIInteractions( FDisUIInteractionContext& _rContext ) const; // 2013 rva 0x78cc60 (2012 0x7f64a0)
	UBOOL CheckMantle();                                                     // 2013 rva 0x78cbf0 (2012 0x7f6430)
	UBOOL CheckAssassinate();                                                // 2013 rva 0x7a50f0 (2012 0x808d20)
	UBOOL CheckDropAssassinate();                                            // 2013 rva 0x7a5420 (2012 0x808e50)
	UBOOL CheckAdrenalineKill();                                             // 2013 rva 0x7a5610 (2012 0x809010)
	UBOOL CheckChoke();                                                      // 2013 rva 0x7a5250 (new in 2013)
