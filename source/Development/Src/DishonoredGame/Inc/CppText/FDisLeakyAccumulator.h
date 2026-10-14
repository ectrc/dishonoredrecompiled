// FDisLeakyAccumulator cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). The attention meter: a value that leaks towards its minimum at m_fLeakRate per
// second and is clamped to [min, max]. 2012 rvas: ctor 0x745df0, SetLeakRate 0x745e20, SetMaxValue 0x745e40, GetMaxValue
// 0x745e60, SetMinValue 0x745e70, Clear 0x745e90, SetCurrentValue 0x745ea0, Tick 0x747660 (2013 0x703ba0), Accumulate 0x7476b0
// (0x703bf0), GetCurrentValue 0x7476e0 (0x703c20). Retail's default constructor puts the maximum at FLT_MAX; the generated
// constructor above is empty, so Init() is what a fresh accumulator needs (UDisAttentionInfo_Complex::InitAttnInfo_Common).
public:
	void Init() { m_fCurrentValue = 0.f; m_fLeakRate = 0.f; m_fMinValue = 0.f; m_fMaxValue = 3.4028235e38f; }
	void SetLeakRate( FLOAT _fLeakRate ) { m_fLeakRate = _fLeakRate; }
	void SetMaxValue( FLOAT _fMaxValue ) { m_fMaxValue = _fMaxValue; }
	FLOAT GetMaxValue() const { return m_fMaxValue; }
	void SetMinValue( FLOAT _fMinValue ) { m_fMinValue = _fMinValue; }
	void Clear() { m_fCurrentValue = 0.f; }
	void SetCurrentValue( FLOAT _fNewValue ) { m_fCurrentValue = _fNewValue; }
	FLOAT GetCurrentValue() const { return m_fCurrentValue; }
	void Tick( FLOAT _fDeltaSeconds )
	{
		m_fCurrentValue -= m_fLeakRate * _fDeltaSeconds;
		if( m_fCurrentValue < m_fMinValue ) m_fCurrentValue = m_fMinValue;
		else if( m_fCurrentValue > m_fMaxValue ) m_fCurrentValue = m_fMaxValue;
	}
	void Accumulate( FLOAT _fHowMuch )
	{
		m_fCurrentValue += _fHowMuch;
		if( m_fCurrentValue < m_fMinValue ) m_fCurrentValue = m_fMinValue;
	}
