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

// DISHONORED(port): agent IH3 (fork G). 2013 rvas (2012 in the bodies): RegisterAmbientSound (2012 0x7f9160) and
// UnregisterAmbientSound 0x7a3ea0 (the UAudioSystem slots take AActor* here), SetRTPC 0x7a32a0, UpdateRTPCs 0x7a3580,
// UpdateAudioPropagationCache 0x7a3700, CleanUp; the Wwise ids Init resolves are the retail class statics.
	virtual void RegisterAmbientSound( AActor* AmbientSound );
	virtual void UnregisterAmbientSound( AActor* AmbientSound );
	void SetRTPC( FName _Parameter, FLOAT _fValue, FLOAT _fChangeRate );
	void CleanUp();
private:
	void UpdateRTPCs( FLOAT _fDeltaTime );
	UBOOL UpdateAudioPropagationCache( UDisAudioPropagationInfo* _pAudioPropagationInfo, FDisAudioPropagationCache& _rPropagationCache, FLOAT _fRawDistSq,
		UBOOL _bForAINoise, TArray<FDisAudioDebugPath>* _pDebugPaths );
public:
	static DWORD ms_TimeDilationRTPC_ID;
	static DWORD ms_FreezeEvent_ID;
	static DWORD ms_UnfreezeEvent_ID;
	static DWORD ms_PauseEvent_ID;
	static DWORD ms_ResumeEvent_ID;
	static DWORD ms_AllStateNoneEvent_ID;
	static DWORD ms_ScaleRTPC_ID;
	static DWORD ms_GlobalVolumeRTPC_ID;
	static DWORD ms_MusicVolumeRTPC_ID;
	static DWORD ms_SFXVolumeRTPC_ID;
	static DWORD ms_VoicesVolumeRTPC_ID;
	static DWORD ms_AmbientVolumeRTPC_ID;
	static DWORD ms_MenuVolumeRTPC_ID;
