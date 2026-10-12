// UDisConvGlobalMan cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The game info's conversation manager: every non-immediate dialog hook is
// queued here and fired from the game info's tick in fire-group order with the per-group and global caps the
// conversation tweaks give. 2013 rvas InitGlobalConvMan 0x89caa0* (the 0x8b2e80 agent CG recorded in
// dishonoredgameinfo.cpp is a 26-byte TArray helper), TickGlobalConvMan 0x8a71c0, ManageGlobalDialogHooks 0x8a5b20*,
// QueueFireDialogHook 0x89cc80*, Term_GlobalConv 2012 0x8ef920, GetShortestAudibleDistForSpeaker 2012 0x8e5a00,
// CalcPendingWeights 2012 0x8f2a00 (* = found through the caller, unnamed in the 2013 database). Bodies in
// disconvglobalman.cpp.
public:
	static UDisConvGlobalMan* s_pInstance;
	void InitGlobalConvMan();
	void Term_GlobalConv();
	void TickGlobalConvMan( FLOAT _fDeltaSeconds );
	void QueueFireDialogHook( eDisDialogHook _Hook, INT _iHookSubIndex, class UDisConversationComponent* _pConvComp, class IDisConvSpeakerInterface* _pTarget );
	static FLOAT GetShortestAudibleDistForSpeaker( const class IDisConvSpeakerInterface* _pCurSpeaker, FLOAT _fMaxDistance );
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
	class UDisTweaks_ConvSettings* GetConvSettings() const;
private:
	void ManageGlobalDialogHooks( FLOAT _fDeltaSeconds );
	void CalcPendingWeights( struct FDisQueuedFireDialogHookGroup& _rGroup, eDisQueuedFiringGroupType _GroupType, TArray<struct FDisHookWeight>& _rWeights );
	void CountInitiator( eDisQueuedFiringGroupType _GroupType, const struct FDisQueuedFireDialogHook& _rHook, INT _Delta );
