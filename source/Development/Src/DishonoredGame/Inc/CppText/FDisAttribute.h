// FDisAttribute cpptext: included inside the generated struct body (DishonoredGameAttributesClasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). 2012 rva 0x8d93a0 (120 B): the base value of the attribute for a difficulty
// (EDifficulty_Easy..VeryHard pick the four fields; anything else answers 0).
public:
	FLOAT GetBaseValue( BYTE _Difficulty ) const
	{
		switch( _Difficulty )
		{
		case EDifficulty_Easy: return m_fBaseValue1_Easy;
		case EDifficulty_Normal: return m_fBaseValue2_Normal;
		case EDifficulty_Hard: return m_fBaseValue3_Hard;
		case EDifficulty_VeryHard: return m_fBaseValue4_VeryHard;
		default: return 0.f;
		}
	}
