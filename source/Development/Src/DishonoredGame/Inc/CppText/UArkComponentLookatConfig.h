// UArkComponentLookatConfig cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// Engine.ArkComponentLookatConfig is the per-NPC-type look-at tuning the NPC tweaks name (UDisTweaks_NPCPawn::
// m_pComponentLookatConfig): the blink, the small eye movements, the look-at modes (one 3x3 head grid and one torso
// grid of additive pose names with their ranges) and the procedural look-at patterns. Its two natives run the pattern
// state machine for FArkComponentLookat::ProceduralLookAt; bodies in Src/arkanimnodelookat.cpp (agent FT), retail's
// Engine/Src/arkcomponentlookatconfig.cpp.
public:
	// DISHONORED(port): 2012 rva 0x57cb30 (unnamed in the 2013 database; its callers are 0x552970 and 0x53c8a0).
	void InitProceduralLookAt( INT _PatternIdx, TArray<struct FArkCpntLookatProcSeqWork>& _WorkData ) const;
	// DISHONORED(port): 2013 rva 0x53c8a0 (2012 0x57cd10), 1,678 bytes.
	void UpdateProceduralLookAt( FLOAT _fTimeStep, INT _PatternIdx, const FVector2D& _CurAim, TArray<struct FArkCpntLookatProcSeqWork>& _WorkData, INT& _SequenceIdx, INT& _RuleIdx ) const;
