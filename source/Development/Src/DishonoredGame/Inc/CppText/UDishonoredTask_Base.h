// UDishonoredTask_Base cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x6cc250 (IsSaveable) and 2012 0x727060, which
	// retail uses for both GameSave and GameLoad. Body in dissavegame.cpp.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const;
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

// ---- agent HI (PHASE22 HI): a task instance. Bodies in dishonoredtask_base.cpp ----
public:
	UBOOL IsComplete() const;                                                       // 2013 rva 0x6c1770
	UBOOL IsFailed() const;                                                         // 2013 rva 0x6c1780
	UBOOL IsHidden() const;                                                         // 2013 rva 0x6c1790
	BYTE GetTaskState() const;                                                      // 2013 rva 0x6c17a0
	void SetTaskState( BYTE _NewState );                                            // 2013 rva 0x6c17b0
	TArray<FDisTaskTarget>& GetTargets();                                           // 2012 rva 0x721fd0
	UBOOL ShouldShowHUDMarkers() const;                                             // 2013 rva 0x6c17f0
	void ShowHUDMarkers( UBOOL _bShow );                                            // 2013 rva 0x6c1800
	void SetTaskHidden( UBOOL _bHidden );                                           // 2013 rva 0x6c63c0
	FString GetStatusString() const;                                                // 2013 rva 0x6cc220
	void OnAdded( FName _TemplateTaskName );                                        // 2013 rva 0x6c6300
	void OnDisSeqAct_UpdateCustomTask( class UDisSeqAct_UpdateCustomTask* _pAction ); // 2012 rva 0x721f40

	// DISHONORED(port): 2013 vtable slots +292 (OnDisSeqAct_UpdateCustomTask_Impl, empty in the base: 0x5cb0c0 is a bare
	// `ret 4`), +296 (OnAdded_Impl, 0x459d10, `ret`) and +300 (GetStatusString_Impl, 0x92d600, an empty FString).
	virtual void OnDisSeqAct_UpdateCustomTask_Impl( class UDisSeqAct_UpdateCustomTask* _pAction ) {}
	virtual FString GetStatusString_Impl() const { return FString(); }
private:
	virtual void OnAdded_Impl() {}
public:
