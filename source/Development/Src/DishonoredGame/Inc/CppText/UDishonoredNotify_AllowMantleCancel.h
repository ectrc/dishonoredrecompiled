// UDishonoredNotify_AllowMantleCancel cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). 2012 0x6d62e0 / 2013 0x684710: the start of the range in which releasing jump cancels
// the mantle. The range's end (Notify_EndRange 0x6d6300 / 0x684730) belongs to Arkane's ranged notifies, which the engine's
// UAnimNodeSequence::IssueNotifies does not have (named, not written). Body in Src/dishonoredanimnotifies.cpp.
public:
	virtual void Notify( class UAnimNodeSequence* NodeSeq );
