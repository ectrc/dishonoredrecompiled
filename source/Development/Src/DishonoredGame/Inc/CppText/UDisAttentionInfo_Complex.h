// UDisAttentionInfo_Complex cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad, 2013 rva 0x74ff80 (2012 0x793830), retail vtable slot 70
// (vftable rva 0xd40618). GameSave (0x74ff00, slot 69) is not ported, for agent ED's reason: the writing half
// of the DisSaveLoad object layer does not exist here.
//
// The body is in Src/dissavegame.cpp rather than in retail's own Src/disattentioninfo_complex.cpp, because that
// unit is still an import_reference.py skeleton on DishonoredGame_EXCLUDE.
public:
	// -gidiag: the destruction order at exit (the GI2 exit crash hunt).
	virtual void FinishDestroy();
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent GI (PHASE20 GI): the complex (time-based) attention info, the one the player gets ----
// Bodies in Src/disattentioninfo_complex.cpp; 2012 / 2013 rvas at each body. DisAttentionInfo_Complex::SetupTickParamsFromTarget
// (2012 0x7a2120 / 2013 0x768e10) is a free function of that unit.
public:
	virtual UBOOL HasPsychicAttention() const;
	virtual void InitAttnInfo_Derived( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget );
	virtual void TickAttnInfo_Derived( FLOAT _fDeltaSeconds, class IDisAttentionTargetInterface* _pTarget, const class UDisAIBrainProcessAttention* _pAttnProcess, const class UDishonoredAIBrain* _pAttentiveBrain,
		UBOOL _bClearAttention, TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Changes,
		TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Messages );
	virtual UBOOL IsAttentionDecreasing() const;
	virtual void OnAttnTargetStateChange( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget );
	virtual void SetAsTopAttnTarget_Derived( class UDishonoredAIBrain* _pBrain, const struct FDisAttentionProxy& _rTopAttnTargetProxy );
	virtual void IncreaseAttention_Derived( const struct FDisAttentionIncrease& _rIncrease, const struct FDisAttentionChangeReason& _rReason, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo, UBOOL& _rbOut_WitnessedDeath );
	virtual void GainLOS_Derived();
	virtual void LoseLOS_Derived();
	virtual void BeTouched_Derived();
	virtual void EnterHideout_Derived();
	virtual void ExitHideout_Derived();
	virtual void GainPsychicAttention_Derived();
	virtual void LosePsychicAttention_Derived();
	virtual void SetMinAttentionLevel_Derived( BYTE _Type, BYTE _MinLevel );
	virtual BYTE GetMinAttentionLevel_Derived( BYTE _Type ) const;
	virtual void OnOtherActorTerminated_AttnInfo_Derived( const class AActor& _rActor );
	virtual void OnDifficultyChange_AttnInfo_Derived();
	virtual UBOOL HasAnyAttention_Derived() const;
	virtual FLOAT GetNormalizedAttnValue_Derived() const;
	virtual BYTE GetAttnLevel_Derived() const;
	virtual struct FDisAttentionChangeReason GetLastChangeReason_Derived() const;
	virtual UBOOL ShouldKeepAlarmRinging_Derived() const;
	virtual FLOAT GetTimeLeftInInvestigate_Derived() const;
	virtual UBOOL HasDirectAttention() const;

private:
	void InitAttnInfo_Common( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget );
	void SetAccumulatorParams( const struct FDisTickAttentionParams& _rTickParams, const class UDishonoredAIBrain* const _pOwningBrain );
	void UpdateTimeBasedAttention_Sight_UpdateSightedFlag( const struct FDisTickAttentionParams& _rTickParams );
	void UpdateTimeBasedAttention_Sight_MakePendingChange( const struct FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds );
	void UpdateTimeBasedAttention_Touch( const struct FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds );
	void UpdateTimeBasedAttention_Psychic( const struct FDisTickAttentionParams& _rTickParams );
	UBOOL ApplyAttentionIncreases( const struct FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds, TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rOut_AttentionChangeInfos,
		struct FDisAttentionChangeReason& _rOut_NonZeroChangeReason, TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_AttentionChangeMessages );
	void ApplyAttentionDrain( const struct FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds, UBOOL _bClearAttention, struct FDisAppliedAttentionChangeInfo& _rOut_DrainChangeInfo,
		TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_AttentionChangeMessages );
	void UpdateAttentionInfo( const struct FDisTickAttentionParams& _rTickParams, struct FDisAppliedAttentionChangeInfo& _rChangeInfo,
		TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_AttentionChangeMessages );
	void UpdateAttentionProxy( const struct FDisTickAttentionParams& _rTickParams, const TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rAttentionChangeInfos, FLOAT _fDeltaSeconds );
	void UpdateAttentionProxy_Player( const struct FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds );
	BYTE GetAttentionLevelFromAttentionValue( const struct FDisTickAttentionParams& _rTickParams, UBOOL _bAttentionIncreasing );
	BYTE GetMinAttentionLevel( UBOOL _bAlarmsKeepRingingUntilNPCsLoseAttention, BYTE _AlarmsKeepRingingAttentionLevel ) const;
	UBOOL ShouldClampAttentionToInvestigate( const struct FDisTickAttentionParams& _rTickParams, const struct FDisAttentionChangeReason& _rAttentionChangeReason, BYTE _OldAttentionLevel, BYTE _NewAttentionLevel, UBOOL& _rbLostTarget );
