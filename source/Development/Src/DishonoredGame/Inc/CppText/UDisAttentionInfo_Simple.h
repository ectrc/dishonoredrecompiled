// UDisAttentionInfo_Simple cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). The pooled, flag-based attention info every NPC keeps for every target that is
// not the player: Unaware or Busted, nothing in between. Bodies in Src/disattentioninfo_simple.cpp; 2012 / 2013 rvas there.
public:
	// -gidiag: the destruction order at exit (the GI2 exit crash hunt).
	virtual void FinishDestroy();
	virtual UBOOL HasPsychicAttention() const;
	virtual void InitAttnInfo_Derived( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget );
	virtual void TickAttnInfo_Derived( FLOAT _fDeltaSeconds, class IDisAttentionTargetInterface* _pTarget, const class UDisAIBrainProcessAttention* _pAttnProcess, const class UDishonoredAIBrain* _pAttentiveBrain,
		UBOOL _bClearAttention, TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Changes,
		TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Messages );
	virtual void OnAttnTargetStateChange( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget );
	virtual void IncreaseAttention_Derived( const struct FDisAttentionIncrease& _rIncrease, const struct FDisAttentionChangeReason& _rReason, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo, UBOOL& _rbOut_WitnessedDeath );
	virtual void GainLOS_Derived();
	virtual void LoseLOS_Derived();
	virtual void BeTouched_Derived();
	virtual void GainPsychicAttention_Derived();
	virtual void LosePsychicAttention_Derived();
	virtual void SetMinAttentionLevel_Derived( BYTE _Type, BYTE _MinLevel );
	virtual BYTE GetMinAttentionLevel_Derived( BYTE _Type ) const;
	virtual UBOOL HasAnyAttention_Derived() const;
	virtual FLOAT GetNormalizedAttnValue_Derived() const;
	virtual BYTE GetAttnLevel_Derived() const;
	virtual struct FDisAttentionChangeReason GetLastChangeReason_Derived() const;
	virtual UBOOL HasDirectAttention() const;

private:
	void UpdateSightedFlag( class IDisAttentionTargetInterface* _pAttnTarget, const class UDishonoredAIBrain* _pAttentiveBrain );
