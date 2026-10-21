// UDisBehaviorGuard cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): agent DF. Body in disbehaviorguard.cpp.
public:
	virtual void RequestStateExitCallback_TakePosition( class UDishonoredNativeState* _pThisState );
// agent JE (PHASE24 JE): GetDialogAvailability, vtable slot 98 of 2012 ??_7UDisBehaviorGuard (0x723660, ICF-folded `return 0`): always available.
protected:
	virtual eDisDialogAvailability GetDialogAvailability( eDisConvPriority _Priority, INT _StartDialogFlags ) const { return DDA_AVAILABLE; }
public:
