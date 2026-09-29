// UDisAIBrainProcess cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. Bodies in Src/disaibrainprocess.cpp. The _Derived virtuals are retail's own naming
// convention (the non-virtual entry point does the shared work and then calls the subclass's _Derived half), and their
// base bodies are empty in retail because the base process does nothing on its own.
public:
	void InitBrainProcess( class UDishonoredAIBrain* const _pOwningBrain, class UDisTweaks_AIBrainProcess* const _pBrainProcTweaks );
	void TermBrainProcess();
	void TickBrainProcess( FLOAT _fDeltaSeconds );
	void RefreshBrainProcess( FLOAT _fTimeSinceLastThought );
	void PostGameLoad_BrainProcess();
	void OnOtherActorTerminated_AIBrainProcess( const class AActor& _rActor );
	void OnDifficultyChange_AIBrainProcess();
	UBOOL FilterAIStim_BrainProcess( const struct FAIStimStruct& _rStim );

	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );

protected:
	virtual void InitBrainProcess_Derived() {}
	virtual void TermBrainProcess_Derived() {}
	virtual void TickBrainProcess_Derived( FLOAT _fDeltaSeconds ) {}
	virtual void RefreshBrainProcess_Derived( FLOAT _fTimeSinceLastThought ) {}
	virtual void PostGameLoad_BrainProcess_Derived() {}
	virtual void OnOtherActorTerminated_AIBrainProcess_Derived( const class AActor& _rActor ) {}
	virtual void OnDifficultyChange_AIBrainProcess_Derived() {}
	virtual const BYTE* BuildFilterStimMask() { return NULL; }
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_BrainProcess( BYTE _StimID ) { return FDisStimPredicateDelegate(); }
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad, 2013 rva 0x7362c0, retail vtable slot 70. GameSave
// (0x736290, slot 69) is not ported. Body in Src/disaibrainprocess.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
