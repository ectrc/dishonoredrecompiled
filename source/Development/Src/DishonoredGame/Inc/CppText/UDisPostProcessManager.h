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

// ---- agent FA (PHASE12 FA): the interface's own post-process channel ----
public:
	// DISHONORED(port): 2013 rva 0x7e7ee0 (2012 0x849700). The parameters, the weight they reach and the two
	// fade times, copied whole; the state machine below is what walks the weight between 0 and it.
	void SetUIPPParams( const FArkUberPpParameters& _rParameters, FLOAT _fWeight, FLOAT _fFadeInTime,
						FLOAT _fFadeOutTime );
	// DISHONORED(port): 2013 rva 0x7efca0 (2012 0x8511a0). Called once per frame from
	// ADishonoredPlayerController::ModifyPostProcessSettings with the frame's config; blends m_UIPPParams into it
	// at whatever weight the fade has reached. This is the whole of "retail blurs the scene behind a modal".
	void ApplyUIPostProcessSettings( struct FArkPpConfig& _rConfig, FLOAT _fDeltaTime );

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x7eb2c0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x7e7e30 (2012 0x849660). Body in dispostprocessmanager.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
