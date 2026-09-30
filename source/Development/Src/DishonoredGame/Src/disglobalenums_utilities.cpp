// DishonoredGame/src/disglobalenums_utilities.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent EP ports (PHASE14 EP): the one helper the patrol route's random mode needs ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x840790 (2012 0x88b310): a value in [0,_Range) that is not the one returned last time.
// Retail's own expression, kept exactly: rand() * _Range * (1/32768), truncated - so it is appRand()'s 15-bit range and
// not appRandHelper, and a _Range of 1 keeps returning 0 because the no-repeat step only runs when _Range > 1.
INT FDisNonRepeatINTRandomHelper::GetNonRepeatingRandomValue( INT _Range )
{
	INT Value = 0;
	if( _Range > 0 )
	{
		Value = (INT)( (FLOAT)appRand() * (FLOAT)_Range * 0.000030517578f );
	}
	if( _Range > 1 )
	{
		if( Value == m_iLastReturnedValue )
		{
			Value = ( Value + 1 ) % _Range;
		}
		m_iLastReturnedValue = Value;
	}
	return Value;
}
