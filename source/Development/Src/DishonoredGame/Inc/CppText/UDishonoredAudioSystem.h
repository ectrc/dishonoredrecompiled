// UDishonoredAudioSystem cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): the UAudioSystem virtuals of Engine (EngineArkaneClasses.h) in the 2013 vtable order of UDishonoredAudioSystem
// (vftable rva 0xd5c4b0) plus IArkSettingsListenerInterface.
public:
	virtual UObject* GetUObjectInterfaceArkSettingsListenerInterface() { return this; }
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
	virtual void Init();
	virtual void SuspendUpdate();
	virtual void ResumeUpdate();

// DISHONORED(port): agent ID (PHASE23 ID). Update 2013 rva 0x7b3520 (4,734 B) is reached through UAkAudioDevice::Update; agent ID
// ported its end-of-event drain (ConsumeEndOfEventNotifies 0x7a9970, 2012 0x80b800), agent IE the listener cell and the
// per-component propagation cache with GetCellAtPoint 0x792d10 (slot 75). Bodies in dishonoredaudiosystem.cpp.
	virtual AVolume* GetCellAtPoint( const FVector& _rPoint, AVolume* _pCachedCellAtPoint ) const;
	virtual void Update( class UAkAudioDevice* AudioDevice, void* ActiveComponents );
	void ConsumeEndOfEventNotifies( TSet<UAkComponent*>& _rGameObjects );
	virtual void FinishDestroy();
