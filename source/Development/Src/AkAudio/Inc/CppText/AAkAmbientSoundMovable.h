// AAkAmbientSoundMovable cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudioclasses.cpp, PostBeginPlay 2013 rva 0x5aea60 (a movable ambient sound starts
// playing as soon as it is in the world instead of waiting for the audio system's cell walk).
#if DISHONORED_WITH_WWISE
public:
	virtual void PostBeginPlay();
#endif
