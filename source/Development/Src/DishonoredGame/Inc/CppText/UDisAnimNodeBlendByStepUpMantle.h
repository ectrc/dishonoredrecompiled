// UDisAnimNodeBlendByStepUpMantle cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). Retail 2012 / 2013 rvas (disanimnodeblendbystepupmantle.cpp): TickAnim 0x6e4520 / 0x6920c0,
// OnStepUpMantle 0x6e4590 / 0x692130, GetMantleSeqNode 0x6d8690 / 0x688ff0. Bodies in Src/disanimnodeblendbystepupmantle.cpp.
public:
	virtual void TickAnim( FLOAT DeltaSeconds );
	void OnStepUpMantle();
private:
	class UAnimNodeSequence* GetMantleSeqNode() const;
public:
