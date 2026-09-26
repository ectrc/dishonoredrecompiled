// IDisInteractableInterface cpptext: included inside the generated interface body (dishonoredgameclasses.h).
// DISHONORED(written): agent AU. The interface's C++ API, in retail vtable order (offsets from the interface subobject,
// read out of the 2013 bodies below): +4 GetUObjectInterfaceDisInteractableInterface, +8 GetHighlightFlags,
// +56 GetInteractableName, +68 GetUseMessage, +76 GetCannotUseMessage, +80 FormatText,
// +84 GetInteractableTweaks_Derived, +88 ShowHighlight, +92 HideHighlight, +96 AttemptInteract_Derived,
// +108 AttemptCannotUseInteract_Derived.
// Non-virtual bodies in disinteractableinterface.cpp: AttemptInteract 2013 rva 0x63df60 (2012 0x678010),
// AttemptCannotUseInteract 0x63e200 (0x678320), GetInteractableTweaks 0x621550 (0x67c3f0), DoHighlight 0x627ff0,
// UnDoHighlight 0x628060, SetHighlightBit 0x630710, ClearHighlightBit 0x630740, GainCrosshairFocus 0x6307e0,
// WitnessInteraction 0x63b560, and the virtual defaults GetUseMessage 0x628190, GetCannotUseMessage 0x6281f0,
// GetInteractableName 0x6280a0, FormatText 0x61c8c0.
// Every default body is retail's own, so the thirty-odd classes that implement the interface and are still comment-only
// skeleton units keep compiling and behave as retail's base does.
public:
	virtual UObject* GetUObjectInterfaceDisInteractableInterface() { return NULL; }

	/** Retail vtable +8: the implementing actor's highlight-flag word (ADisPickup_Base: &m_HighlightFlags). */
	virtual INT* GetHighlightFlags() { return NULL; }

	virtual UBOOL CanInteract( const struct FCanInteractParams& Params ) const { return TRUE; }
	virtual UBOOL ShouldBlockInteractProbe( const struct FCanInteractParams& Params ) const { return FALSE; }

	virtual const FString& GetInteractableName() const;
	virtual const FString& GetUseMessage() const;
	virtual const FString& GetAltUseMessage() const;
	virtual const FString& GetCannotUseMessage() const;
	virtual const FString& GetCrosshairFocusText() const { return GetInteractableName(); }
	virtual void FormatText( FString& Text ) const;

	virtual const class UDisTweaks_InteractableInterface* GetInteractableTweaks_Derived() const { return NULL; }
	virtual void ShowHighlight( class UMaterialInterface* Material ) {}
	virtual void HideHighlight() {}

	virtual UBOOL AttemptInteract_Derived( class ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed ) { bOutCanBeWitnessed = FALSE; return FALSE; }
	virtual UBOOL AttemptCannotUseInteract_Derived( class ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed ) { bOutCanBeWitnessed = FALSE; return TRUE; }

	virtual void GainCrosshairFocus();
	virtual void LoseCrosshairFocus() {}

	const class UDisTweaks_InteractableInterface& GetInteractableTweaks() const;

	UBOOL AttemptInteract( class ADishonoredPawn* const Pawn );
	UBOOL AttemptCannotUseInteract( class ADishonoredPawn* Pawn );

	void DoHighlight();
	void UnDoHighlight();
	void SetHighlightBit( INT Bit );
	void ClearHighlightBit( INT Bit );

	void WitnessInteraction( class AActor* const InteractedWith, class ADishonoredPawn* const Pawn );
