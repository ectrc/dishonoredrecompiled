// UDishonoredObjectivesComponent cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x72b8c0 - retail folds GameLoad onto it.
	// Body in dissavegame.cpp; the objective list is what pulls every objective into the stream.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent HI (PHASE22 HI): the objective list of the player controller. Bodies in dishonoredobjectivescomponent.cpp ----
public:
	void AddObjective( const UDishonoredObjective* _pPackageObjective );                                // 2013 rva 0x6e0b60
	void RemoveObjective( const UDishonoredObjective* _pPackageObjective );                             // 2013 rva 0x6d4dd0
	void CompleteObjective( const UDishonoredObjective* _pPackageObjective );                           // 2013 rva 0x6d4e70
	void FailObjective( const UDishonoredObjective* _pPackageObjective );                               // 2013 rva 0x6d4ec0
	void SetObjectiveHidden( const UDishonoredObjective* _pPackageObjective, UBOOL _bHidden ) const;    // 2013 rva 0x6d4f00
	UBOOL IsObjectiveComplete( const UDishonoredObjective* _pPackageObjective ) const;                  // 2013 rva 0x6d4f30
	UBOOL HasObjective( const UDishonoredObjective* _pPackageObjective ) const;                         // 2013 rva 0x6d1530
	void ClearObjectives();                                                                             // 2012 rva 0x72d340
	UDishonoredTask_Base* GetTaskInstance( const UDishonoredTask_Base* _pPackageTask ) const;           // 2013 rva 0x6d0dc0
	void FindActiveTaskTargets( TArray<const FDisTaskTarget*, TMemStackAllocator<GMainThreadMemStack> >& _rTaskTargets, UBOOL _bForHUDMarkers ) const; // 2013 rva 0x6d9910
	void OnObjectiveChangedNotify( EDisObjectiveChangedNotifyType _Type, UObject* _pReferenceObject );  // 2013 rva 0x6d4f60
	static UBOOL IsDiagEnabled();

private:
	UDishonoredObjective* FindObjectiveInstance( const UDishonoredObjective* _pPackageObjective, INT* _pOutFoundIndex ) const; // 2013 rva 0x6d0bf0
	void PopInactiveObjectiveInstance( INT _iMoveFromIndex );                                          // 2013 rva 0x6d0cc0
	void OnGotObjectiveNotify( UDishonoredObjective* _pObjective );                                    // 2013 rva 0x6cc050
	void OnCompletedObjectiveNotify( UDishonoredObjective* _pObjective );                              // 2013 rva 0x6d0e00
	void OnFailedObjectiveNotify( UDishonoredObjective* _pObjective );                                 // 2013 rva 0x6cc0e0
	void OnNewTaskNotify( UDishonoredTask_Base* _pTask );                                              // 2013 rva 0x6d1080
	void OnTaskUpdatedNotify( UDishonoredTask_Base* _pTask );                                          // 2013 rva 0x6d1220
	void OnTaskStateChangedNotify( UDishonoredTask_Base* _pTask );                                     // 2013 rva 0x6d1320
public:
