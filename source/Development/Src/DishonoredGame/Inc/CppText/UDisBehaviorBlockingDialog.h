// UDisBehaviorBlockingDialog cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent ID (PHASE23 ID). IsReadyToSpeak 2013 rva 0x6c2640 (2012 0x722ab0, 79 B), asked by the conversation
// component before a blurb starts. Body in disbehaviorblockingdialog.cpp; the rest of the behaviour is not ported.
public:
	UBOOL IsReadyToSpeak() const;
// agent JE (PHASE24 JE): GetDialogAvailability 2013 0x6d2390 (2012 0x73c9c0, same bytes).
	virtual eDisDialogAvailability GetDialogAvailability( eDisConvPriority _Priority, INT _StartDialogFlags ) const;
