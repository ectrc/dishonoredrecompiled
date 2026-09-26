// UDisNativeStateTransitionLogic cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable slot after the UObject ones (2012 PDB +288, 2013 +292) InitTransitionLogic 2013 rva 0x67bbe0;
// CanTransition 0x672720 (disnativestatetransitionlogic.cpp).
public:
	virtual void InitTransitionLogic( const TArray<UDishonoredNativeState*>& States );
	UBOOL CanTransition( const UDishonoredNativeState* FromState, UClass* ToStateID ) const;
