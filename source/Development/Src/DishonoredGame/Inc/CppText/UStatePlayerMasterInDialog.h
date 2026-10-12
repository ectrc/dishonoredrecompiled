// UStatePlayerMasterInDialog cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). Only the soiree hand-over the player MAT_* contract reaches when the
// master FSM is already in dialog: StartSoireeControl 2012 0x6afa80 (2013 0x6696e0), StopSoireeControl 0x6afab0 (2013
// 0x663e20), SetSoireeBlendOutTime 0x6afae0 (2013 0x6609c0). The dialog state itself (OnEnterState 2012 0x6b0c40 and
// the rest of stateplayermasterindialog.cpp) is not ported. Bodies in Src/stateplayermasterindialog.cpp.
public:
	void StartSoireeControl( class USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags );
	void StopSoireeControl( UBOOL bInterrupted );
	void SetSoireeBlendOutTime( FLOAT BlendOutTime ) { m_fSoireeBlendOutTime = BlendOutTime; }
