// AAkAmbientSound cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudioclasses.cpp. 2013 rvas: StartEvent 0x5b3440, StopEvent 0x5b2060,
// StartPlayback 0x5ae910, StopPlayback 0x5ae970, IsSaveable 0x5ae9a0, UpdateComponentsInternal 0x5ae9d0.
// StartPlayback / StopPlayback go through the world's audio system (UWorld::m_pAudioSystem, slots
// RegisterAmbientSound / UnregisterAmbientSound), which is what decides when StartEvent actually fires.
#if DISHONORED_WITH_WWISE
public:
	void StartEvent();
	void StopEvent();
	void StartPlayback();
	void StopPlayback();
#endif // DISHONORED_WITH_WWISE
