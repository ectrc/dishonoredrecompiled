// UAkComponent cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudioclasses.cpp. A UAkComponent IS the Wwise game object: retail passes the component
// pointer itself as the AkGameObjectID (UAkComponent::Attach 2013 rva 0x5b3d20 calls RegisterGameObj(this),
// Detach 0x5b3980 UnregisterGameObj(this)), which is why AkGameObjectID is AkUIntPtr and not AkUInt32.
#if DISHONORED_WITH_WWISE
public:
	/** 0x5b3d20 / 0x5b3980 / 0x5b39e0 / 0x5b3ab0 */
	virtual void Attach();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void FinishDestroy();
	virtual void ShutdownAfterError();

	/** 0x5b1150: cancel every callback this component still owns, forget the radii, then StopAll on it */
	void Stop();

	/** UActorComponent::Scene is protected; UAkAudioDevice::Flush (0x5b3b30) filters on it. */
	FORCEINLINE class FSceneInterface* GetAkScene() const { return Scene; }

	/** The component pointer is the game object id (see above). */
	FORCEINLINE AkGameObjectID GetAkGameObjectID() const { return (AkGameObjectID)this; }
#endif // DISHONORED_WITH_WWISE
