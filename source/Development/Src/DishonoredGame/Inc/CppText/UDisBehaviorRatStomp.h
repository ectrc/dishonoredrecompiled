// UDisBehaviorRatStomp cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent CG. Body in disbehaviorratstomp.cpp.
public:
	virtual void RequestStateExitCallback_TakeActorPosition( class UDishonoredNativeState* _pThisState );
// agent JE (PHASE24 JE): GetDialogAvailability, vtable slot 98 of 2012 ??_7UDisBehaviorRatStomp = UDisBehaviorStolen's body (0x7238a0, ICF-folded):
// a blocking conversation carrying flag 1 is refused.
protected:
	virtual eDisDialogAvailability GetDialogAvailability( eDisConvPriority _Priority, INT _StartDialogFlags ) const { return ( ( _StartDialogFlags & 2 ) && ( _StartDialogFlags & 1 ) ) ? DDA_REJECTED_BY_AI_BEHAVIOR : DDA_AVAILABLE; }
public:
