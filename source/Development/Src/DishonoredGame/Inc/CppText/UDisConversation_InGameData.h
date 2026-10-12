// UDisConversation_InGameData cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). A conversation's speaker bindings and the acquisition that starts it.
// 2013 rvas InitForConvNode_Derived 0x8961e0*, AcquireSpeakers 0x8a3700*, AcquireSpeakersCheckOnly 0x89e3b0,
// BindAllSpeakers 0x89e2b0, BindSpeaker 0x89c6c0, GetSpeaker 0x88d180, PlayerIsNearEnough 0x896410, CheckForTickReroute
// 0x8a5610, CalcGlobalHookDominanceWeight 0x899f80, MarkNodeLabelSeen / RerouteConversationOutput / IsSpeakerEngaged
// 2012 0x8f5220 / 0x8de660 / 0x8de5f0 (* = by vtable slot or caller, unnamed in the 2013 database). Bodies in
// disconversation_ingamedata.cpp.
public:
	UBOOL AcquireSpeakers( struct FDisDialogRunningInstance* _pRunningInstance );
	UBOOL AcquireSpeakersCheckOnly( struct FDisDialogRunningInstance* _pRunningInstance );
	class IDisConvSpeakerInterface* GetSpeaker( INT _iIndex ) const;
	UBOOL IsSpeakerEngaged( INT _iIndex ) const;
	void MarkNodeLabelSeen( const class UDisConv_Node* _pNode );
	UBOOL CheckForTickReroute( struct FDisDialogRunningInstance* _pRunningInstance );
	UBOOL RerouteConversationOutput( struct FDisDialogRunningInstance* _pRunningInstance, INT _OutputIndex );
	FLOAT CalcGlobalHookDominanceWeight() const;
	UBOOL BindAllSpeakers( struct FDisDialogRunningInstance* _pRunningInstance, UBOOL _bMustBeAvailable );
protected:
	virtual void InitForConvNode_Derived( const class UDisConv_Node* _pNode );
	UBOOL BindSpeaker( struct FDisConvSpeaker_InGame& _rSpeaker, struct FDisDialogRunningInstance* _pRunningInstance, UBOOL _bMustBeAvailable );
	UBOOL PlayerIsNearEnough( struct FDisDialogRunningInstance* _pRunningInstance ) const;
