// UDisConv_Soiree cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The node that starts a scripted scene: its m_SoireeGUID is the
// USeqAct_Interp::m_MatineeGUID of the matinee it plays. 2013 rvas (??_7UDisConv_Soiree@@6B@ at 0x119ed30):
// 74 TickNode 0x8a7ac0, 76 InterruptTickingNode 0x8a9640*, 77 IsTickingNodeInterruptible 0x8a96c0*,
// 79 GetClassVars_Derived 0x8ad130, 80 PlayNode_Derived 0x8a7a60, 87 TickNodePreview 0x8acea0*, 88 TickNodeMatinee
// 0x8a95c0*, 89 PlayNodePreview 0x8acff0*, 90 PlayNodeMatinee 0x8b61f0*; FindMatinee 0x8b5de0, FindMatineeLinked
// 0x8b5ed0's sibling (* = by vtable slot, unnamed in the 2013 database). Bodies in disconv_soiree.cpp.
public:
	virtual INT TickNode( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual void InterruptTickingNode( struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const;
	virtual UBOOL IsTickingNodeInterruptible( struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const;
	virtual void GetClassVars_Derived( struct FDisConvClassVars& _rVarsOut ) const;
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT TickNodePreview( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT TickNodeMatinee( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT PlayNodePreview( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT PlayNodeMatinee( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	class USeqAct_Interp* FindMatinee() const;
	void FindMatineeLinked( TArray<class USeqAct_Interp*>& _lMatinee ) const;
protected:
	class UDisConv_Soiree_InGameData* GetSoireeData( struct FDisDialogRunningInstance* _pRunningInstance ) const;
