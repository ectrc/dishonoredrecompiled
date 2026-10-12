// UDisInterpTrackAllowPlayerLook cpptext: included inside the generated class body (DishonoredGameSoireeClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). UpdateTrack 2012 0x8dc4a0 (2013 0x86eaa0, same 296 B): the last key at or
// before the position hands its yaw/pitch limits to the player camera's AnimDriven influence; no key means no player
// look at all. AddKeyframe 0x8e3030 and GetKeyProperties 0x8dc440 are editor surface and are not ported.
// Body in Src/disinterptrackallowplayerlook.cpp.
public:
	virtual void UpdateTrack( FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump );
