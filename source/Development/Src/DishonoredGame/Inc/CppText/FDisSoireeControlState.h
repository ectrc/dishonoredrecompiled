// FDisSoireeControlState cpptext: included inside the generated struct body (DishonoredGameClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The forced-perspective state a soiree puts the player in, shared by
// UStatePlayerMasterSoiree and UStatePlayerMasterInDialog. Retail 2012 rvas (dissoireecontrolstate.cpp): EnterSoireeMode
// 0x69a4b0 (2013 0x666ff0, 591 B - the 2013 body adds the SOIREE_LeaveCameraToDialogTree branch and the equipped items'
// context notification), TickSoireeMode 0x692b80 (22 B, unnamed in 2013), ExitSoireeMode 0x695a70 (2013 0x6628f0).
// Bodies in Src/dissoireecontrolstate.cpp.
void EnterSoireeMode( class USeqAct_Interp* Matinee, class ADishonoredPlayerPawn* PlayerPawn, FLOAT BlendInTime, INT Flags );
void TickSoireeMode( class ADishonoredPlayerPawn* PlayerPawn );
void ExitSoireeMode( class ADishonoredPlayerPawn* PlayerPawn, FLOAT BlendOutTime, UBOOL bInterrupted );
UBOOL IsControlledBySoiree() const { return m_bControlledBySoiree; }
