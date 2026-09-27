// UDisOpacityParameterPpController cpptext: included inside the generated class body (DishonoredGamePostProcessClasses.h).
// DISHONORED(port): IsShown 2013 rva 0x7e7ca0, Tick 0x7eaba0, Update 0x7eac10;
// dispostprocesscontrollers.cpp.
public:
	virtual UBOOL IsShown(const class UArkPpNode* Node);
	virtual UBOOL Tick(FLOAT DeltaTime,enum ELevelTick TickType);
	virtual class UMaterialInterface* Update(const class UArkPpNodeMaterial* Node,UBOOL* bOutOverrideUber,FLOAT* OutWeight,struct FArkUberPpParameters* InOutParams);
