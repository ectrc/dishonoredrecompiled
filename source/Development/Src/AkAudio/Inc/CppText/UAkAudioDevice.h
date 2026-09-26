// UAkAudioDevice cpptext: included inside the generated class body (AkAudioClasses.h).
// DISHONORED(port): akaudiodevice.cpp. 2013 rvas of every method, from resources/docs/symbols/match_2012_2013.csv.
#if DISHONORED_WITH_WWISE
public:
	/** 0x5aedf0 */
	static UAkAudioDevice* Get();
	/** 0x5b1740: EnsureInitialized(-1) and return TRUE regardless, like retail */
	virtual UBOOL Init();
	/** 0x5aec80: hand the world's audio system this device and the live component set */
	virtual void Update( UBOOL bGameTicking );
	/** 0x5b3fe0 / 0x5b4000 */
	virtual void FinishDestroy();
	virtual void ShutdownAfterError();

	/** 0x5aecb0 */
	void SetListener( INT ListenerIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front );

	/** 0x5b1780 / 0x5b17b0 / 0x5aed90 / 0x5aedb0 / 0x5b1750 / 0x5aed60 / 0x5b2b20 / 0x5b2b80 */
	AKRESULT LoadBank( const TCHAR* BankName, AkMemPoolId MemPoolId, AkBankID& OutBankID );
	AKRESULT LoadBank( const TCHAR* BankName, AkBankCallbackFunc Callback, void* Cookie, AkMemPoolId MemPoolId, AkBankID& OutBankID );
	AKRESULT UnloadBank( const TCHAR* BankName, AkMemPoolId* OutMemPoolId );
	AKRESULT UnloadBank( const TCHAR* BankName, AkBankCallbackFunc Callback, void* Cookie );
	AKRESULT PrepareBank( AK::SoundEngine::PreparationType PreparationType, const TCHAR* BankName, AK::SoundEngine::AkBankContent Content );
	AKRESULT PrepareEvent( AK::SoundEngine::PreparationType PreparationType, AkUniqueID EventID, AkBankCallbackFunc Callback, void* Cookie );
	AKRESULT ClearBanks();
	void LoadAllReferencedBanks();

	/** 0x5b2b60 / 0x5b20c0: the AKPK file package a bank's media lives in */
	AKRESULT LoadFilePackage( const TCHAR* PackageName, AkUInt32& OutPackageID );
	AKRESULT UnloadFilePackage( AkUInt32 PackageID );

	/** 0x5b2be0 / 0x5b2c40 / 0x5b2150 */
	AkPlayingID PostEvent( class UAkEvent* Event, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed );
	AkPlayingID PostEvent( const FString& EventName, const FLOAT& MaxRadius, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed );
	AkPlayingID PostEvent( AkUniqueID EventID, const FLOAT& MaxRadius, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed );

	/** 0x5b2250 / 0x5b2290 / 0x5b22e0 / 0x5aedd0 / 0x5b20d0 / 0x5b4020 */
	AKRESULT PostTrigger( const TCHAR* Trigger, AActor* Actor, FName BoneName );
	AKRESULT SetRTPCValue( const TCHAR* Param, FLOAT Value, AActor* Actor, FName BoneName );
	AKRESULT SetSwitch( const TCHAR* SwitchGroup, const TCHAR* Switch, AActor* Actor, FName BoneName );
	AKRESULT SetState( const TCHAR* StateGroup, const TCHAR* State );
	AKRESULT SeekOnEvent( const FString& EventName, AActor* Actor, FName BoneName, AkTimeMs Position );
	void StopAllSounds( UBOOL bImmediate );

	/** 0x5b3b00 / 0x5b17e0 / 0x5b3b30 */
	void RegisterComponent( class UAkComponent* Component );
	class UAkComponent* GetAkComponent( AActor* Actor, FName BoneName, UBOOL bStopWhenOwnerDestroyed );
	void Flush( class FSceneInterface* Scene );

	/** 0x5b4530 / 0x5b4820: the additional bank directories of the DLC file packages */
	void AddBaseDirectory( const FString& BaseDirectory, const TArray<FString>& SubDirectories );
	void RemoveAllAdditionalDirectories();

	/** DISHONORED(retail): a real static of the retail exe, ?m_bSoundEngineInitialized@UAkAudioDevice@@1_NA at 0x105b43c */
	static UBOOL m_bSoundEngineInitialized;

protected:
	/** 0x5b1430: the whole MemoryMgr -> StreamMgr -> SoundEngine -> MusicEngine -> plug-ins bring-up */
	UBOOL EnsureInitialized( INT SpeakerConfigOverride );
	/** 0x5b0b40 */
	void SetBankDirectory();
	/** 0x5b3de0 */
	void Teardown();
#endif // DISHONORED_WITH_WWISE
