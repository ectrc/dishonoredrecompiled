// FDisRollingAverage cpptext: included inside the generated struct body (DishonoredGameGlobalEnumsClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). AddValue 2012 0x887e10 / 2013 0x83d230 (74 B, same bytes): a ten-sample window.
	void AddValue( FLOAT Value )
	{
		const INT Slot = m_HistoryCount % 10;
		m_HistoryCount++;
		if( m_HistoryCount > 10 )
		{
			m_fCurrentTotal -= m_History[Slot];
		}
		m_History[Slot] = Value;
		m_fCurrentTotal += Value;
	}
