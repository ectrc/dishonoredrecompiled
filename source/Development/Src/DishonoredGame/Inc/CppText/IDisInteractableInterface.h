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
	virtual void LoseCrosshairFocus();

	const class UDisTweaks_InteractableInterface& GetInteractableTweaks() const;

	UBOOL AttemptInteract( class ADishonoredPawn* const Pawn );
	UBOOL AttemptCannotUseInteract( class ADishonoredPawn* Pawn );

	void DoHighlight();
	void UnDoHighlight();
	void SetHighlightBit( INT Bit );
	void ClearHighlightBit( INT Bit );

	void WitnessInteraction( class AActor* const InteractedWith, class ADishonoredPawn* const Pawn );

	// DISHONORED(port): agent GD (PHASE19 GD), the slots the player's use path calls, read off ADishonoredNPCPawn's interface
	// vtable (2013 0x1141458): +12 GetCrosshairStatus (ADishonoredNPCPawn 2013 0xb59c40 / 2012 0x7c02b0; ADisPickup_Base's own
	// declaration overrides it now), +36 CanAltInteract (NPC 0xb679d0), +40 GetAltInteractTime (base 0xb484d0: 0). The
	// non-virtual AttemptAltInteract and EndInteract (2013 0xa307b0) are named rather than ported: nothing on the first
	// mission's use path alt-interacts, and the latent-interactable list they serve is never filled here.
	virtual enum eCrossHairStatus GetCrosshairStatus( class ADishonoredPawn* Pawn ) const { return CHS_OVER_NEUTRAL; }
	virtual UBOOL CanAltInteract( class ADishonoredPawn* Pawn ) const { return FALSE; }
	virtual FLOAT GetAltInteractTime() const { return 0.f; }
	UBOOL AttemptAltInteract( class ADishonoredPawn* const Pawn );
	void EndInteract( const struct FEndInteractParams& Params );

	// DISHONORED(port): agent JH (PHASE24 JH), the interaction prompt's slots: +60 FillUIInteraction (2013 rva 0x6280d0 / 2012
	// 0x67d190) and +64 FillUIAltInteraction (0x628130 / 0x67d1f0) - the tweaks' interact / alt-interact text as the crosshair line -
	// and +24 LoseCrosshairFocus (0x630810 / 0x681750: clears highlight bit 1). Bodies in disinteractableinterface.cpp.
	virtual void FillUIInteraction( struct FDisUIInteractionContext& _rContext ) const;
	virtual void FillUIAltInteraction( struct FDisUIInteractionContext& _rContext ) const;
