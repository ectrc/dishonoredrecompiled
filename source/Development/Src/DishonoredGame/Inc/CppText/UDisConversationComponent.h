// UDisConversationComponent cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent EC (PHASE11 EC), 2013 rva 0x896600 (2012 0x8dab60) - the component's own script
// properties, then which of the bound dialog tree's two running instances was live. Reached from
// ADishonoredNPCPawn::GameLoad_Dialog; body in dissavegame.cpp.
public:
	void SerializeForGameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	/** DISHONORED(written): retail's GameLoad_Dialog reads UActorComponent::bAttached directly; it is
	    protected in this tree and this is the only place outside the component hierarchy that needs it. */
	UBOOL IsAttached() const { return bAttached; }

// DISHONORED(port): agent FR (PHASE17 FR). The conversation component's direct path - the hook that a Kismet
// DisSeqAct_DialogInputs fires on a speaker, the queue it goes through, and the one-shot dialog trees the component
// instances when its pawn is spawned. Retail vtable (2013 ??_7UDisConversationComponent@@6B@ at 0x1199b90): 75 Attach
// 0x893700, 77 Detach 0x88d260, 78 BeginPlay 0x88d2c0, 79 Tick 0x8a6860, 88 OnOtherActorTerminated 0x8a4f50. Bodies in
// disconversationcomponent.cpp with their 2013 rvas.
public:
	virtual void Attach();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void BeginPlay();
	virtual void Tick( FLOAT DeltaTime );
	void PostSpawned( TArray<class UDisDialogVoiceData*>& _rAdditionalGenerics, const TArray<struct FDisSpeakerOneShotInfo>& _rOneShots );
	void AddOneShotAfterSpawned( const struct FDisSpeakerOneShotInfo& _rOneShot );
	void FireDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator, UBOOL _bImmediate );
	UBOOL FireDialogHookNow( eDisDialogHook _Hook, INT _iHookSubIndex, class IDisConvSpeakerInterface* _pInitiator, eDisDialogCheckOnly* _pCheckOnly_ResultOut );
	eDisDialogCheckOnly CheckDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator ) const;
	UBOOL SupportsDialogHook( eDisDialogHook _Hook ) const;
	void StartDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bIsBlocking, INT _FiringHookIndex );
	void EndDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason );
	eDisConvPriority InConversation( UBOOL& _rIsBlocking_Out, UBOOL* _pIsNonInterruptable, const INT* _pMatchRunningInstID ) const;
	INT GetRunningInstID() const { return m_iRunningInstID; }
	struct FDisDialogRunningInstance* GetRunningInst() const { return (struct FDisDialogRunningInstance*)m_pDialogRunningInst; }
	void BreakOutOfConversation( DisConvoEndReasonEnum _ConvoEndReason, eDisDialogHookFireGroup _FiringGroup );
	void StopDialogBlurb();
	void OnOtherActorTerminated( const AActor& _rOtherActor );
	eDisQueuedFiringGroupType ClassifyGlobalFireGroup() const;
	UBOOL InFallbackChain( const class UDisDialogTree* _pTree ) const;
	UBOOL ShouldLowerAlertness() const;
protected:
	class UDisDialogTree_InGameBind* InstanceAndBindDialogTree( class UDisDialogVoiceData* _pDialogVoiceData );
	void BuildDialogHookSupportedCache();
	UBOOL SupportsDialogHook_Slow( eDisDialogHook _Hook ) const;
