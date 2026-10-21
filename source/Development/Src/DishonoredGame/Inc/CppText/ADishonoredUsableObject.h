// ADishonoredUsableObject cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). The interactable slots of the usable object (2013 interface vtable 0x10e2090, the interface
// at +664): +4 GetUObjectInterfaceDisInteractableInterface, +8 GetHighlightFlags (2013 rva 0x642290: +100 of the interface,
// m_HighlightFlags), +24 LoseCrosshairFocus (0x657ea0 / 2012 0x68dbf0), +52 GetCrosshairFocusText (0x64e5c0 / 0x683910), +60
// FillUIInteraction (0x653b30 / 0x688c40), +84 GetInteractableTweaks_Derived (0x64e840 / 0x683b90), +88 ShowHighlight (0x645630 /
// 0x66da10), +92 HideHighlight (0x6422c0 / 0x66b370); the tweaks interface's +16 GetTweaks_Derived (2012 0x669bb0, folded:
// m_pUsableObjTweaks) and +20 SetTweaks_Derived (2012 0x66be30); GetActiveStage (2013 0x64eec0 / 2012 0x684440). Bodies in
// dishonoredusableobject.cpp.
public:
	virtual UObject* GetUObjectInterfaceDisInteractableInterface() { return this; }
	virtual INT* GetHighlightFlags() { return &m_HighlightFlags; }
	virtual class UDisTweaksBase* GetTweaks_Derived() { return (class UDisTweaksBase*)m_pUsableObjTweaks; }
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks ) { m_pUsableObjTweaks = (class UDisTweaks_UsableObject*)Tweaks; }
	virtual const class UDisTweaks_InteractableInterface* GetInteractableTweaks_Derived() const;
	virtual const FString& GetCrosshairFocusText() const;
	virtual void FillUIInteraction( struct FDisUIInteractionContext& _rContext ) const;
	virtual void ShowHighlight( class UMaterialInterface* _pMaterial );
	virtual void HideHighlight();
	virtual void LoseCrosshairFocus();
	class UDisTweaks_UsableObject* GetUsableObjectTweaks() const;
	const struct FUsableObjectStage& GetActiveStage( const class UDisTweaks_UsableObject* _pOptionalTweaks ) const;
