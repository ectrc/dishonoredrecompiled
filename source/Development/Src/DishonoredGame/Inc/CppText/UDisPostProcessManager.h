// UDisPostProcessManager cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): the three effect accessors every post-process feed goes through. m_RequiredEffects is what the
// gameplay asks for and m_EffectStates is what the manager's own Tick is running; StartEffect's second argument says
// the caller will drive the state itself, so the default (FALSE) puts the effect straight into state 2.
//   IsEffectRequired 2013 rva 0x7e7da0 (2012 0x8495d0)
//   StartEffect      2013 rva 0x7e7dc0 (2012 0x8495f0)
//   StopEffect       2013 rva 0x7e7df0 (2012 0x849620)
// Bodies are three lines each and stay here rather than in a new compile unit.
public:
	UBOOL IsEffectRequired( BYTE Effect ) const { return m_RequiredEffects[Effect] != 0; }
	void StartEffect( BYTE Effect, UBOOL bStateDrivenByCaller )
	{
		m_RequiredEffects[Effect] = 1;
		if( !bStateDrivenByCaller )
		{
			m_EffectStates[Effect] = 2;
		}
	}
	void StopEffect( BYTE Effect ) { m_RequiredEffects[Effect] = 0; }
