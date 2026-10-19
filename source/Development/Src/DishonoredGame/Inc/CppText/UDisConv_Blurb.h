// UDisConv_Blurb cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x8988a0 (2012 0x8e7d10) reads the per-language blurb block after the tagged properties.
// DISHONORED(port): agent ID (PHASE23 ID). One spoken line of a conversation, 2013 vtable ??_7UDisConv_Blurb@@6B@ slots 6 PostLoad,
// 73 InitNode, 74 TickNode, 76 InterruptTickingNode, 79 GetClassVars_Derived, 80 PlayNode_Derived, 81 ExtractText_Derived.
// Bodies in disconv_blurb.cpp with their 2013 and 2012 rvas.
public:
	virtual void Serialize( FArchive& Ar );
	virtual void PostLoad();
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual INT TickNode( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual void InterruptTickingNode( struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const;
	virtual void GetClassVars_Derived( struct FDisConvClassVars& _rVarsOut ) const;
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual const TCHAR* ExtractText_Derived() const;
	INT PlayBlurb_Shared( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance, UBOOL _bSkipLookTargets, FLOAT _fSeekTime ) const;
	INT TickNode_Shared( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance, UBOOL _bSkipLookTargets ) const;
protected:
	UBOOL StartBlurb( struct FDisDialogRunningInstance* _pRunningInstance, struct FDisBlurbPerSpeakerInstance& _rSpeakerData, class IDisConvSpeakerInterface* _pSpeaker, UBOOL _bSkipLookTargets ) const;
	class IDisConvSpeakerInterface* GetBlurbLookTarget( class UDisConversation_InGameData* _pOwner_InGame, struct FDisDialogRunningInstance* _pRunningInstance, BYTE& _rLookType, UBOOL& _rbUseAttentionProxy ) const;
