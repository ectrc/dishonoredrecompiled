// UArkAnimNodeBlendPose cpptext: included inside the shim class body (DishonoredGameEngineShims.h; the class is an
// Engine-package class the Engine header generation left out, as UInterpGroupPlayer is).
// DISHONORED(port): agent FZ (PHASE18 FZ). The two calls the player MAT_* contract makes on m_pMatineeBlender:
// SetEnable 2012 0x54a250, BlendNow 0x54ffb0 (both unnamed in the 2013 database). TickAnim 0x550080, SetActiveChild
// 0x550360 and InitAnim 0x1c3040 - the blend itself - are not ported, so these two only arm the node.
// Bodies in Src/arkanimnodeblendpose_dis.cpp.
public:
	virtual void SetEnable( UBOOL bEnable );
	void BlendNow( FLOAT BlendTime, UBOOL bWaitForNextFrame );
