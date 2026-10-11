// UGFxAction_OpenMovie cpptext: included inside the generated class body (GFxUIUISequenceClasses.h).
// DISHONORED(port): agent FP. Activated is 2013 rva 0x59d3a0 (0x47b bytes; 2012 0x5e51a0, 0x48e), retail vtable
// slot of USequenceOp::Activated. It is how retail opens the MAIN MENU: the menu map's Kismet
// (Dishonored_MainMenu Main_Sequence.DisGFxActionOpenMovie_0) runs it on LevelLoaded. The reference body sat in
// Src/ScaleformInteraction.cpp under WITH_GFx, a unit GFxUI/Sources.cmake excludes, so the class had no override
// at all and the op "activated" into USequenceAction::Activated. Body in Src/gfxuikismet.cpp.
public:
	virtual void Activated();
