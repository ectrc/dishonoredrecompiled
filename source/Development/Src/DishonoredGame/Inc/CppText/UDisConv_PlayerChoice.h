// UDisConv_PlayerChoice cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x8a1850 (2012 0x8f1590) reads the per-language choice blocks after the tagged properties.
// agent JF (PHASE24 JF): the node itself, 2012 / 2013 rvas: PlayNode_Derived 0x8dd8e0 / 0x89c570, TickNode 0x8dd950 / 0x89c610,
// InitNode 0x8ea740 / 0x89ac90, MatchChoicesToOutputLinks 0x8e8070 / 0x8999f0, GetClassVars_Derived 0x8d9860 / 0x88aff0,
// GetOptionalChoiceForIndex 0x8daa00 / 0x88b810, IsValidChoice 0x8daac0 / 0x88b8d0, GetOptionalChoice 0x8ddf90 / 0x88cbd0 and
// 0x8ddfe0 / 0x88cb80, GetChoice 0x8de030 / 0x88cc20, GetNumChoices 0x8f9dd0 / 0xdc350, GetNumOptionalChoices 0x67b8e0 / 0x642de0
// (2013 vtable +344..+364). Bodies in disconv_playerchoice.cpp.
public:
	virtual void Serialize( FArchive& Ar );
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual INT TickNode( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual struct FDisPlayerChoice_Optional& GetOptionalChoice( INT _iChoice );
	virtual const struct FDisPlayerChoice_Optional& GetOptionalChoice( INT _iChoice ) const;
	virtual INT GetNumOptionalChoices() const { return m_Choices_Optional.Num(); }
	virtual const struct FDisPlayerChoice_Static& GetChoice( INT _iChoice ) const;
	virtual INT GetNumChoices() const { return m_Choices_Static.Num(); }
	virtual UBOOL IsValidChoice( INT _iChoice ) const;
	const struct FDisPlayerChoice_Optional* GetOptionalChoiceForIndex( INT _iIndex, INT& _rOutputIndex ) const;
protected:
	virtual void GetClassVars_Derived( struct FDisConvClassVars& _rVarsOut ) const;
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	void MatchChoicesToOutputLinks( UBOOL _bForEditor );
public:
