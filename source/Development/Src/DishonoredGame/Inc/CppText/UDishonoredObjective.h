// UDishonoredObjective cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x6d0b50 / 0x6d4cd0 / 0x6d4d60. Body in
	// dissavegame.cpp.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const;
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

// ---- agent HI (PHASE22 HI): an objective instance and its tasks. Bodies in dishonoredobjective.cpp ----
public:
	UBOOL IsActive() const;                                                         // 2013 rva 0x6c16c0
	UBOOL IsHidden() const;                                                         // 2013 rva 0x6c16d0
	UBOOL ShouldShowHUDMarkers() const;                                             // 2013 rva 0x6c16e0
	void ShowHUDMarkers( UBOOL _bShow );                                            // 2013 rva 0x6c16f0
	void OnTaskCompletedNotify( UDishonoredTask_Base* _pTask );                     // 2013 rva 0x6cbc70
	void FailObjective();                                                           // 2013 rva 0x6cbd00
	void SetHidden( UBOOL _bHidden );                                               // 2013 rva 0x6cbd80
	UDishonoredTask_Base* GetTaskInstance( const UDishonoredTask_Base* _pPackageTask ) const; // 2013 rva 0x6cbe40
	FName GetTemplateName() const;                                                  // 2013 rva 0x6d0a60
	void OnAdded( const UDishonoredObjective* _pPackageObjective );                 // 2013 rva 0x6dc1c0
