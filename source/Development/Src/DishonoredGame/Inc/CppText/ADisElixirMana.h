// ADisElixirMana cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(written): agent AU. GetTweaks_Derived / SetTweaks_Derived (2013 rva 0x61d5c0 for the setter),
// CanBePickedUp 0x6222c0, DoInteract_Impl 0x61d590. Bodies in diselixir.cpp.
public:
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	virtual UBOOL CanBePickedUp( class ADishonoredPlayerPawn* PlayerPawn ) const;
	virtual UBOOL DoInteract_Impl( class ADishonoredPlayerPawn* PlayerPawn );
