// ADisClimbable cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). The interactable slots of the climbable (2013 interface vtable 0x10ec418, the interface at
// +672): +4 GetUObjectInterfaceDisInteractableInterface, +8 GetHighlightFlags (2013 rva 0x700ad0, folded: +24 of the interface,
// m_HighlightFlags), +84 GetInteractableTweaks_Derived (0x62bfa0 / 2012 0x680c70), +88 ShowHighlight (0x61a200 / 0x66ae20), +92
// HideHighlight (0x61a240 / 0x66ae60); the tweaks interface's +16 GetTweaks_Derived (2012 0x8d9800, folded: +0x44 of that interface,
// m_pClimbableTweaks) and +20 SetTweaks_Derived (2012 0x88b160, folded). The climbable glows by swapping its mesh's first material (kept in m_MaterialBackup). Bodies in disclimbable.cpp.
// CanInteract (+28, 2013 0x61e6f0) and AttemptInteract_Derived (+96, 0x623130) are not ported: the interface defaults answer.
public:
	virtual UObject* GetUObjectInterfaceDisInteractableInterface() { return this; }
	virtual INT* GetHighlightFlags() { return &m_HighlightFlags; }
	virtual class UDisTweaksBase* GetTweaks_Derived() { return (class UDisTweaksBase*)m_pClimbableTweaks; }
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks ) { m_pClimbableTweaks = (class UDisTweaks_Climbable*)Tweaks; }
	virtual const class UDisTweaks_InteractableInterface* GetInteractableTweaks_Derived() const;
	virtual void ShowHighlight( class UMaterialInterface* _pMaterial );
	virtual void HideHighlight();
