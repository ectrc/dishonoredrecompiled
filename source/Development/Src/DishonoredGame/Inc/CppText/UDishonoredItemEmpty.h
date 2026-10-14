// UDishonoredItemEmpty cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// agent FV (PHASE18 FV): 2013 rva 0x83d0c0 (2012 0x887cf0): empty hands look EmptyIdle / EmptyWalk / EmptyRun by speed.
public:
	virtual BYTE GetDefaultNPCLookAtMode( BYTE _TransitSpeed ) const
	{
		if( _TransitSpeed == ETransitSpeed_Idle )
		{
			return eDisNPCLookAtMode_EmptyIdle;
		}
		return _TransitSpeed == ETransitSpeed_Walk ? eDisNPCLookAtMode_EmptyWalk : eDisNPCLookAtMode_EmptyRun;
	}
// agent GJ (PHASE20 GJ): the tweaks slots of IDisTweaksInterface on the empty hand - GetTweaks_Derived 2012 0x7cc530 (7 B, m_pEmptyHandsTweaks),
// SetTweaks_Derived 0x88b2c0 (16 B). Bodies in dishonoreditemempty.cpp.
public:
	virtual class UDisTweaksBase* GetTweaks_Derived();
protected:
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
public:
