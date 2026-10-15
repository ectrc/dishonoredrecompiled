// UDisSeqAct_ObjectiveAction cpptext: included inside the generated class body.

// ---- agent HI (PHASE22 HI). Body in disseqact_objectiveaction.cpp ----
public:
	// DISHONORED(port): 2013 rva 0x7a81d0 (2012 0x7dd5d0): the package objectives linked to the action's variables.
	void GetObjectives( TArray<UDishonoredObjective*, TMemStackAllocator<GMainThreadMemStack> >& _rObjectives ) const;
	// DISHONORED(port): 2013 vtable +400, what ADishonoredPlayerController::OnObjectiveAction calls per objective.
	virtual void HandlePackageObjective( class UDishonoredObjectivesComponent* _pObjComp, const UDishonoredObjective* _pPackageObjective ) {}
