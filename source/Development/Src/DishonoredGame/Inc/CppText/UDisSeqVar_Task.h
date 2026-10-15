// UDisSeqVar_Task cpptext: included inside the generated class body.

// ---- agent HI (PHASE22 HI). Body in disseqvar_task.cpp ----
public:
	// DISHONORED(port): 2013 rvas PostLoad 0x786cd0, GetObjectValue 0x79cda0, GetObjectRef 0x79cdb0, GetValueStr 0x79cdd0,
	// GetTaskPtr 0x792210. The variable names a task of a package objective; it reads as that package task.
	virtual void PostLoad();
	virtual UObject** GetObjectRef( INT Idx );
	UObject* GetObjectValue();
	virtual FString GetValueStr();
private:
	UDishonoredTask_Base* GetTaskPtr() const;
public:
