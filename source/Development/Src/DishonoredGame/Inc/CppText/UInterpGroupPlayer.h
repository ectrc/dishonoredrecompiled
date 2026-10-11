// UInterpGroupPlayer cpptext: included inside the generated class body.

// ---- agent FK (PHASE15 FK): the player group of a Dishonored cutscene ----
public:
	/**
	 * DISHONORED(port): agent FK. Retail has no UInterpGroup::UpdateGroup override on this class: it
	 * puts the per-frame blend in UInterpGroupInstPlayer::UpdateGroupInst (2013 rva 0x4fe8c0), and
	 * UInterpGroupInst::UpdateGroupInst does not exist in this tree at all - this engine still has the
	 * reference UE3 shape, UInterpGroup::UpdateGroup( pos, GrInst, bPreview, bJump ), which is what
	 * UInterpGroupDirector and UInterpGroupAI override. Same call, same arguments, same position in the
	 * frame; the only difference is which of the two objects carries the override.
	 * Body in Src/interpgroupplayer.cpp.
	 */
	virtual void UpdateGroup( FLOAT NewPosition, class UInterpGroupInst* GrInst, UBOOL bPreview, UBOOL bJump );

	/**
	 * DISHONORED(port): agent FK, 2013 rva 0x4fc450 (2012 0x53b610). Retail dispatches this through
	 * UInterpGroup's vtable from USeqAct_Interp::NeedWaitToStart (2012 0x230400), and neither the base
	 * virtual nor that caller exists in this tree, so nothing reaches it yet. Ported because it is one
	 * line and because the caller is the next thing a matinee needs to wait on an unequipped player.
	 */
	UBOOL NeedWaitToStart( class UInterpGroupInst* GrInst ) const;
