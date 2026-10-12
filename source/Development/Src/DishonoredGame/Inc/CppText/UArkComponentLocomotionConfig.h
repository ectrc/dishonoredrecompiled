// UArkComponentLocomotionConfig cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// Engine.ArkComponentLocomotionConfig is the per-NPC-type locomotion tuning the NPC tweaks name (UDisTweaks_NPCPawn::
// m_pComponentLocomotionConfig). The speed-mode table is m_Modifiers x m_SpeedNames entries of m_SpeedModes, each an index
// into m_SpeedModePropDictionary; a speed mode's m_MoveAnimProps are its 1, 2, 4 or 8 direction animations with their
// foot speed. The four natives below are what UArkAnimNodeLocomotion and FArkComponentLocomotion read the table through;
// bodies in Src/arkanimnodelocomotion.cpp (agent GC), retail's Engine/Src/arkcomponentlocomotionconfig.cpp.
public:
	// DISHONORED(port): 2013 rva 0x5331d0 (2012 0x574b20), 217 bytes. _Angle in Unreal units, -32768..32767, 0 forward; the
	// two direction animations around it in the 8-slot direction ring (index = slot * 8 / count) and their weights, snapped
	// to one animation past _fSnapOnOneAngleThreshold.
	void ComputeAnimDirAndWeightFromAngleAndDirCount( INT _Angle, INT _AnimNumber, struct FArkCpntLocoDirAndWeight* const _DirAndWeight, FLOAT _fSnapOnOneAngleThreshold ) const;
	// DISHONORED(port): 2013 rva 0x5343a0 (2012 0x575c30), 224 bytes. The two speed modes of _ModifierIdx around _fSpeed
	// (global speed-mode indices) and the weight of the slower one; a speed past the fastest mode gives one mode at 1.
	void GetSpeedModeFromForwardSpeed( FLOAT _fSpeed, INT _ModifierIdx, INT& _SpeedModeIdx0, INT& _SpeedModeIdx1, FLOAT& _fSpeedMode0Weight ) const;
	// DISHONORED(port): 2013 rva 0x5342a0 (2012 0x575b30), 212 bytes: the forward animation's foot speed of one speed mode,
	// and 0x534380 (2012 0x575c10), the (speed, modifier) overload.
	FLOAT GetForwardSpeedOfSpeedMode( INT _SpeedModeIdx ) const;
	FLOAT GetForwardSpeedOfSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const;
