// UDisSeqAct_DialogInputs cpptext: included inside the generated class body (DishonoredGameKismetClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). Kismet's way into a dialog tree: for every Target speaker, every impulsed
// input link's name becomes the speaker's m_KismetDialogHook and DDH_KISMET_ACTIVATED is fired on it. Activated 2013 rva
// 0x79bf00 (578 B), GetDialogHookInitiator 0x790ac0 (254 B). Bodies in disseqact_dialoginputs.cpp.
public:
	virtual void Activated();
protected:
	class IDisConvSpeakerInterface* GetDialogHookInitiator();
