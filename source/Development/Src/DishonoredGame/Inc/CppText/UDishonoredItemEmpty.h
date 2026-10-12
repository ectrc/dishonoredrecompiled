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
