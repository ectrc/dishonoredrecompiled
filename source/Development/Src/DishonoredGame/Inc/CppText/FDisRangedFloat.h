// FDisRangedFloat cpptext: included inside the generated struct body (DishonoredGameGlobalStructsClasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). 2012 rva 0x890940 (59 B): min + rand() * (max - min) / 32768.
public:
	FLOAT GetRandomFloatValue() const { return m_fMinValue + ( m_fMaxValue - m_fMinValue ) * ( (FLOAT)rand() * ( 1.f / 32768.f ) ); }
