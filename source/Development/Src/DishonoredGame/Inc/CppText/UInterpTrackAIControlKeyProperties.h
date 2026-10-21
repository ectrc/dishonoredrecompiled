// UInterpTrackAIControlKeyProperties cpptext: included inside the generated class body (DishonoredGameEngineShims.h).
// DISHONORED(port): agent JC (PHASE24 JC). GetTarget (2013 key-properties vtable +292, 2012 +288): the actor a key aims or fires at;
// none on the base and the body-intention keys. Overrides in the Aim / Fire key properties (interptrackaicontrol_dis.cpp).
public:
	virtual AActor* GetTarget( UInterpTrackInst* const TrInst ) const { return NULL; }
