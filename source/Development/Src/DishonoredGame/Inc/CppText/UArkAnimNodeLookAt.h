// UArkAnimNodeLookAt cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// DISHONORED(bringup): Engine.ArkAnimNodeLookAt is the NPC anim tree's head- and torso-aim blender. Its Anims array
// holds eighteen additive poses on a 3x3 grid (nine head, nine torso, column-major: UpLeft CenterLeft DownLeft / Up
// Center Down / UpRight CenterRight DownRight) and the only thing in retail that ever writes their weights is
// UArkAnimNodeLookAt::UpdateNodeWeights (2012 rva 0x5532e0, 3,294 bytes), reached from TickAnim (0x5672c0) through
// ComputeHeadAndTorsoAim (0x557b80) and the BlendInfos triangle mapping - and only while m_pConfig is set, which is
// FArkComponentLookat::Starting's job. None of that is ported (Engine/Src/arkanimnodelookat.cpp and
// arkcomponentlookat.cpp are both comment-only stubs, 43 and 51 functions).
//
// So the array keeps the cook's default, entry 0 at weight 1.00, and that entry is ADD_Empty_AimHeadIdle_UpLeft:
// every NPC's head is held craned up and to the left, 63 degrees off its bind pose at head_jnt and 34 at neck_jnt
// while the whole spine stays within 7. That is the tilted head in agent DI's screenshots, measured.
//
// TickAnim below is a STAND-IN, not a port: it drives the same eighteen weights from a normalised aim on the same
// grid by bilinear interpolation, so the rest state (aim 0,0) is the Center pair - which is what retail's own rest
// state is - instead of a corner. It is switched off by -nodislookataim, and the aim itself only moves when
// -dislookatplayer asks for it. The units are NOT retail's: retail's m_Aim is in degrees mapped through
// UArkComponentLookatConfig's per-mode ranges, and this takes [-1,1]. Whoever ports the node deletes this.
public:
	virtual void TickAnim( FLOAT DeltaSeconds );
	/** Weights for the eighteen additive aim poses from a normalised aim; X right, Y up, both in [-1,1]. */
	void DisSetNormalisedAim( const FVector2D& NormalisedAim );
