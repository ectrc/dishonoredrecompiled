// ADishonoredMovable cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(port): agent JH (PHASE24 JH). The interactable slots of the movable (2013 interface vtable 0x10f46e0, the interface at
// +1008): +4 GetUObjectInterfaceDisInteractableInterface (2013 rva 0x646bb0), +8 GetHighlightFlags (0x700ad0, folded: +24 of the
// interface, m_HighlightFlags), +84 GetInteractableTweaks_Derived (0x64a230 / 2012 0x671760), +88 ShowHighlight (0x645e90 / 0x66e720),
// +92 HideHighlight (0x6428e0 / 0x66b710); the tweaks interface's +16 GetTweaks_Derived (2012 0x66b670) and +20 SetTweaks_Derived
// (0x66de20) on m_pMovableTweaks. Bodies in dishonoredmovable.cpp. GetCrosshairStatus (+12, 2013 0x64f9b0), CanInteract (+28,
// 0x64f9d0) and AttemptInteract_Derived (+96, 0x64fa10) are not ported: the interface defaults answer.
public:
	virtual UObject* GetUObjectInterfaceDisInteractableInterface() { return this; }
	virtual INT* GetHighlightFlags() { return &m_HighlightFlags; }
	virtual class UDisTweaksBase* GetTweaks_Derived() { return (class UDisTweaksBase*)m_pMovableTweaks; }
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks ) { m_pMovableTweaks = (class UDisTweaks_Movable*)Tweaks; }
	virtual const class UDisTweaks_InteractableInterface* GetInteractableTweaks_Derived() const;
	virtual void ShowHighlight( class UMaterialInterface* _pMaterial );
	virtual void HideHighlight();
