// UDisSeqAct_AIGoToActor cpptext: included inside the generated class body (dishonoredgamekismetclasses.h).
// agent FV (PHASE18 FV): the Kismet go-to that walks an NPC to a mark. Bodies in Src/disseqact_aigotoactor.cpp.
// 2013 rvas: GetDestinationActor 0x78ae40 (2012 0x7cdc20), SetGoToStatus 0x7a1fc0 (0x7d8da0), DeActivated 0x7a2070
// (0x7d8e50), GetDesiredTransitSpeed 0x786360 (0x7cb780), ShouldAlignRotation 0x786370 (0x7cb790).
// 2013 made SetGoToStatus virtual: ADishonoredNPCController::OnAIGoToActor (0x763890) reaches it through the action's
// vtable at +404 where 2012 called it directly; it is virtual here for the same reason.
public:
	class AActor* GetDestinationActor() const;
	virtual void SetGoToStatus( class ADishonoredNPCController* const _pNPCController, BYTE _Status );
	BYTE GetDesiredTransitSpeed() const;
	UBOOL ShouldAlignRotation() const;
	virtual void DeActivated();
