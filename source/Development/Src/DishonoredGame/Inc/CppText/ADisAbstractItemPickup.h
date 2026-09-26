// ADisAbstractItemPickup cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(written): agent AU. Both slots are identical-COMDAT folded in the 2013 exe. Bodies in disabstractitempickup.cpp.
public:
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	virtual UBOOL ShouldTrace( UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags );
	virtual UBOOL CanBePickedUp( class ADishonoredPlayerPawn* PlayerPawn ) const;
	virtual UBOOL DoInteract_Impl( class ADishonoredPlayerPawn* PlayerPawn );
	virtual const FString& GetUseMessage() const;
	virtual void FormatText( FString& Text ) const;
	virtual void Steal( class ADishonoredPlayerPawn* PlayerPawn );

	class UDisTweaks_AbstractItemPickup* GetAbstractItemTweaks() const;
