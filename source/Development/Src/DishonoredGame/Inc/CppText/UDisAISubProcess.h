// UDisAISubProcess cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): agent CG. A sub-process is a behaviour's always-on background job (barks, head tracking,
// personal space, watch points): the behaviour enables it once and it then ticks and refreshes for as long as the
// behaviour is current. Unlike a sub-state there is one instance per class per behaviour and they do not exclude each
// other. Retail vtable tail (2012 PDB UDisAISubProcess_vtbl +288..+320; every base slot is identical-code folded onto an
// empty body or one that returns zero, which is why none of them has its own PDB entry): ShouldLowerAlertness_Derived,
// InitSubProcess_Derived, BeginSubProcess_Derived, EndSubProcess_Derived, TickSubProcess_Derived,
// RefreshSubProcess_Derived, BuildFilterStimMask, GetFilterStimDelegate_SubProcess, GetDesires.
public:
	// DISHONORED(port): the tweak-interface pair, 2012 vtable UDisAISubProcess{for IDisTweaksInterface} slots 4 and 5
	// (folded onto UDisGrenadeComponent::GetTweaks_Derived 0x78da80 / ADisRatSpawner::SetTweaks_Derived 0x78ff90, both of
	// which are a single load/store at interface+4, i.e. object+60 = m_pSubProcessTweaks). Without this override the base
	// returns NULL and SetTweaks_Derived is a no-op, so every sub-process would read its tweaks class's all-zero class
	// default - the defect agent AU's pickup census caught (agentAU.md, "the one bug that made the first run silently wrong").
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks );

	virtual UBOOL ShouldLowerAlertness_Derived() const { return FALSE; }
	virtual void InitSubProcess_Derived() {}
	virtual void BeginSubProcess_Derived() {}
	virtual void EndSubProcess_Derived( UBOOL bIsBeingTerminated ) {}
	virtual void TickSubProcess_Derived( const FLOAT DeltaTime ) {}
	virtual void RefreshSubProcess_Derived( const FLOAT TimeSinceLastThought ) {}
	virtual const BYTE* BuildFilterStimMask() { return NULL; }
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubProcess( BYTE StimID ) { return FDisStimPredicateDelegate(); }
	virtual class IDisDesiresInterface* GetDesires() { return NULL; }
	virtual void BeginDestroy();

	void InitSubProcess( UDishonoredAIBehavior* const OwningBehavior, UDisTweaks_AISubProcess* const SubProcessTweaks );
	void BeginSubProcess( const FDisBodyIntention& PreviousBodyIntention );
	void EndSubProcess( UBOOL bIsBeingTerminated );
	void TickSubProcess( FLOAT DeltaTime );
	void RefreshSubProcess( FLOAT TimeSinceLastThought );
	void EnableSubProcess_Internal( UBOOL bIsBehaviorPaused );
	void DisableSubProcess_Internal( UBOOL bIsBehaviorPaused );
	void OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention );
	void OnOwningBehaviorPause( UBOOL bIsBeingTerminated );
	UBOOL IsSubProcessEnabled() const { return m_bIsSubProcessEnabled; }
	FDisAttentionProxy GetSubProcessActionTargetProxy() const;
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& Event );
	void PostGameLoad_SubProcess();

protected:
	void SetActionTargetProxy( FDisAttentionProxy ActionTarget );
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad 2013 rva 0x73d580 and PostGameLoad 0x72ae40, retail
// vtable slots 70 and 71; GameSave (0x734d80, slot 69) is not ported. Bodies in Src/disaisubprocess.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
