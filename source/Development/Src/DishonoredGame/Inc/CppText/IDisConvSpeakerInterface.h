// IDisConvSpeakerInterface cpptext: included inside the generated interface body (DishonoredGameClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The speaker interface every conversation participant implements, in retail's
// vtable order (2012 ??_7ADishonoredNPCPawn@@6BIDisConvSpeakerInterface@@@ at 0xd39ae8, 30 slots; 2013 the same order):
//   1 GetUObjectInterfaceDisConvSpeakerInterface (pure)   2 PostSpawned_Dialog 0x8a2900   3 RemoveOneShot 0x8a66b0
//   4 IsAvailableForDialog 0x89d1f0   5 StartDialog 0x8a6680   6 EndDialog 0x88bc10's sibling   7 GetDialogSaveData 0x88bc10
//   8 FireDialogHook 0x8a7360   9 CheckDialogHook 0x8a7390   10 FireDialogHookForAudioLog 0x8a73c0   11 SupportsDialogHook
//   12 InConversation   13 BreakOutOfConversation 0x8a5130   14 StreamDialogBlurb 0x88bcc0   15 PlayDialogBlurb 0x8a0930
//   16 StopDialogBlurb 0x8a0980   17 FindTextBlurb 0x8a2920   18 SupportsDialogTree 0x88d770   19 GetConvLookTarget 0x88d7a0
//   20 SetLookTarget 0x8970e0   21 IsReadyToSpeak   22 IsDonePlayingBlurb 0x89a380   23 GetSpeakerLocInfo (pure)
//   24 GetHookParameters_Derived 0x88bc30   25 GetConversationComponent_Derived (pure)   26 GetDebugSpeechText_Derived 0x88bc50
//   27 GameSave_Dialog_Derived   28 GameLoad_Dialog_Derived   29 PostShutdown_Dialog_Derived (the three ICF-folded no-ops).
// The default bodies forward to the owner's UDisConversationComponent when it is attached (disconvspeakerinterface.cpp).
// The three retail pure virtuals are given NULL/empty defaults here because three implementing classes
// (ADisAudioLogPlayer, ADisDialogInanimateDummy, ADisSpeakerGroup_PA) are still comment-only units whose registrants must
// construct them; their overrides are inventoried in agentFR.md.
public:
	virtual UObject* GetUObjectInterfaceDisConvSpeakerInterface() { return NULL; }
	virtual void PostSpawned_Dialog( TArray<class UDisDialogVoiceData*>& _rAdditionalGenerics, const TArray<struct FDisSpeakerOneShotInfo>& _rOneShots );
	virtual void RemoveOneShot( const struct FDisSpeakerOneShotInfo& _rOneShot );
	virtual eDisDialogAvailability IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const;
	virtual void StartDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex );
	virtual void EndDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason );
	virtual struct FDisConvSaveData* GetDialogSaveData();
	virtual void FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator );
	virtual eDisDialogCheckOnly CheckDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator ) const;
	virtual void FireDialogHookForAudioLog( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator );
	virtual UBOOL SupportsDialogHook( eDisDialogHook _Hook ) const;
	virtual eDisConvPriority InConversation( UBOOL& _rIsBlocking_Out, const INT* _pMatchRunningInstID ) const;
	virtual void BreakOutOfConversation( DisConvoEndReasonEnum _ConvoEndReason, eDisDialogHookFireGroup _FiringGroup );
	virtual void StreamDialogBlurb( struct FDisVoiceTextBlurb* _pBlurb ) {}
	/** retail: (UDisDialogVoiceData*, FDisVoiceTextBlurb*, const TCHAR*, FLOAT, ...) - the tail of the signature is not
	    in the PDB's demangled name; nothing on the direct path calls it (the blurb/voice package). */
	virtual void PlayDialogBlurb( class UDisDialogVoiceData* _pVoiceData, struct FDisVoiceTextBlurb* _pBlurb, const TCHAR* _pText, FLOAT _fDuration ) {}
	virtual void StopDialogBlurb();
	virtual struct FDisVoiceTextBlurb* FindTextBlurb( const class UDisDialogTree* _pTree, const FGuid& _rBlurbGUID, class UDisDialogVoiceData** _ppVoiceDataOut ) { return NULL; }
	virtual UBOOL SupportsDialogTree( const class UDisDialogTree* _pTree ) const;
	virtual const AActor* GetConvLookTarget( BYTE& _rLookType ) const { return NULL; }
	virtual void SetLookTarget( AActor* _pTarget, BYTE _LookType, UBOOL _bUseAttentionProxy ) {}
	virtual UBOOL IsReadyToSpeak() const { return TRUE; }
	virtual UBOOL IsDonePlayingBlurb() const { return TRUE; }
	virtual void GetSpeakerLocInfo( TArray<struct FDisConvSpeakerLocInfo>& _rInfoOut ) const {}
	virtual struct FDisHookParameters* GetHookParameters_Derived();
	virtual class UDisConversationComponent* GetConversationComponent_Derived() { return NULL; }
	virtual FString* GetDebugSpeechText_Derived();
	virtual void GameSave_Dialog_Derived( FArchive& _rArchive, ESaveLoadLocation _Location ) {}
	virtual void GameLoad_Dialog_Derived( FArchive& _rArchive, ESaveLoadLocation _Location ) {}
	virtual void PostShutdown_Dialog_Derived() {}

	struct FDisHookParameters* GetHookParameters();
	class UDisConversationComponent* GetConversationComponent( UBOOL _bAllowedUnAttached = FALSE );
	const class UDisConversationComponent* GetConversationComponent( UBOOL _bAllowedUnAttached = FALSE ) const
	{
		return const_cast<IDisConvSpeakerInterface*>( this )->GetConversationComponent( _bAllowedUnAttached );
	}
