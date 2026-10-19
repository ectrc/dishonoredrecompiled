// UDishonoredAudioSystem cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): the UAudioSystem virtuals of Engine (EngineArkaneClasses.h) in the 2013 vtable order of UDishonoredAudioSystem
// (vftable rva 0xd5c4b0) plus IArkSettingsListenerInterface.
public:
	virtual UObject* GetUObjectInterfaceArkSettingsListenerInterface() { return this; }
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
	virtual void Init();
	virtual void SuspendUpdate();
	virtual void ResumeUpdate();

// DISHONORED(port): agent ID (PHASE23 ID). Update 2013 rva 0x7b3520 (4,734 B) is reached through UAkAudioDevice::Update; only its
// end-of-event drain is ported (ConsumeEndOfEventNotifies 0x7a9970, 2012 0x80b800). Bodies in dishonoredaudiosystem.cpp.
	virtual void Update( class UAkAudioDevice* AudioDevice, void* ActiveComponents );
	void ConsumeEndOfEventNotifies( TSet<UAkComponent*>& _rGameObjects );
	virtual void FinishDestroy();
