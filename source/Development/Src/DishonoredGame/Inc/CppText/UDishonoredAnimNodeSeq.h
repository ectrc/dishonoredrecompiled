// UDishonoredAnimNodeSeq cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The Arkane sequence node: the item-prefixed sequence name, the land / mantle / action
// play rates, and the end of an anim state's dominant sequence reported to the owner (AnimState_TriggerExit / AnimState_Advance).
// 2012 / 2013 rvas: ApplySequenceNameModifiers 0x6e03b0 / 0x697ca0, PlayAnim 0x6e0530 / 0x697e30, OnBecomeRelevant 0x6e0870 /
// 0x69bb30, IsDominantAnimStateSequence 0x6e09b0 / 0x68f1b0, OnAnimComplete 0x6e0df0 / 0x69bdb0. Bodies in Src/dishonoredanimnodeseq.cpp.
public:
	virtual FName ApplySequenceNameModifiers();
	virtual void PlayAnim( UBOOL bLoop = FALSE, FLOAT InRate = 1.000000, FLOAT StartTime = 0.000000 );
	virtual void OnBecomeRelevant();
	virtual void OnAnimComplete( FLOAT PlayedTime, FLOAT ExcessTime );
	UBOOL IsDominantAnimStateSequence() const;
