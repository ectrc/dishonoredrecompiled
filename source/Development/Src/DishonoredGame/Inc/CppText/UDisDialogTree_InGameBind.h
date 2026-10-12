// UDisDialogTree_InGameBind cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x8923d0 / 0x89d5b0. Body in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// DISHONORED(port): agent FR (PHASE17 FR). One dialog tree bound to one speaker: its two running instances, the
// per-conversation in-game data (the fallback chain) and the hook evaluation that picks a conversation. 2013 rvas:
// InitAndBind 0x8a0ba0, EvaluateDialogHook 0x8a4690, FireDialogHook 0x8a5150, FireDialogHook_Helper 0x8a49e0*,
// PlayFromNode 0x8a3c10*, TickDialogTree 0x8a66d0, IsSpeakerAvailableForDialog 0x89aec0, ChangeAttentionForSpeaker
// 0x89a550, OnOtherActorTerminated 0x8a4620 (* = found through the caller, unnamed in the 2013 database). Bodies in
// disdialogtree_ingamebind.cpp.
public:
	void InitAndBind( const class UDisDialogTree* _pTree, class IDisConvSpeakerInterface* _pMe );
	UBOOL EvaluateDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator, INT& _rHookSubIndex, UBOOL _bSkipAvailabilityTest );
	UBOOL FireDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator, INT _iHookSubIndex, UBOOL& _bHookFiredNoContent, eDisDialogCheckOnly* _pCheckOnly_ResultOut );
	void TickDialogTree( FLOAT _fDeltaSeconds );
	eDisDialogAvailability IsSpeakerAvailableForDialog( const class IDisConvSpeakerInterface* _pSpeaker, eDisConvPriority _Priority, eDisDialogHook _FromHook, INT _StartDialogFlags ) const;
	void OnOtherActorTerminated( const AActor& _rOther );
	UBOOL PlayFromNode( const class UDisConv_Node* _pNode, struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum& _rEndReason, INT _iLastKnownOutputIndex );
private:
	UBOOL FireDialogHook_Helper( eDisDialogHook _Hook, INT _iHookSubIndex, struct FDisDialogRunningInstance* _pRunningInstance, UBOOL& _bHookFiredNoContent, DisConvoEndReasonEnum& _rEndReason, UBOOL _bFirstTreeTried );
	void ChangeAttentionForSpeaker( class IDisConvSpeakerInterface* _pSpeakerWithBrain, class IDisConvSpeakerInterface* _pTargetSpeaker, EDisAttentionLevel _ToThreshold );
