// UDisItemContext cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). The context's interaction fill, vtable +356: the base answers 2013 rva 0x233610
// (`xor eax, eax; retn 4`); UDisItemContext_ThrowGrenade overrides it (2013 0x823a20 / 2012 0x8642a0) in an unported unit.
public:
	virtual UBOOL FillUIInteractions( struct FDisUIInteractionContext& _rContext ) const { return FALSE; }
