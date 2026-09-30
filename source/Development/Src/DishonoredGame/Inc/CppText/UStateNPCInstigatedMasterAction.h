// UStateNPCInstigatedMasterAction cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent ER (PHASE14 ER), 2013 rvas 0x664c40 / 0x664c60 (2012 0x6b4ea0 / 0x6b4ec0) - the
// partial state of every instigated NPC action is the instigator, one object reference. Nine state classes
// reach these two through this one, including StateNPCMasterDead_Limp, which has no body of its own: its
// primary vftable's slots 88/89 hold these. Bodies in dissavegame.cpp beside the state machine's dispatcher.
public:
	virtual void SavePartialState( UDishonoredNativeStateMachine* StateMachine, FArchive& Ar, ESaveLoadLocation Location );
	virtual void LoadPartialState( UDishonoredNativeStateMachine* StateMachine, UObject* ManagedObject, FArchive& Ar, ESaveLoadLocation Location );
