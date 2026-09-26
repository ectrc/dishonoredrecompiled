// UDishonoredAudioSystem cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): the UAudioSystem virtuals of Engine (EngineArkaneClasses.h) in the 2013 vtable order of UDishonoredAudioSystem
// (vftable rva 0xd5c4b0) plus IArkSettingsListenerInterface.
public:
	virtual UObject* GetUObjectInterfaceArkSettingsListenerInterface() { return this; }
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
	virtual void Init();
	virtual void SuspendUpdate();
	virtual void ResumeUpdate();
