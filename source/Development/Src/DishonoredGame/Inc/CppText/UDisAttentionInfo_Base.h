// UDisAttentionInfo_Base cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x88af60 - retail folds GameLoad onto it. The
	// most numerous of the eleven: 837 of them in one real save. Body in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent GI (PHASE20 GI): the attention info proper ----
// DISHONORED(port): 2012 PDB UDisAttentionInfo_Complex_vtbl (UObject side) slots 72..96 (build/agentGI/vt2012_attention.txt):
// HasPsychicAttention, InitAttnInfo_Derived, TickAttnInfo_Derived, IsAttentionDecreasing, OnAttnTargetStateChange,
// SetAsTopAttnTarget_Derived, IncreaseAttention_Derived, GainLOS/LoseLOS/BeTouched/EnterHideout/ExitHideout/
// GainPsychicAttention/LosePsychicAttention_Derived, Set/GetMinAttentionLevel_Derived, OnOtherActorTerminated_AttnInfo_Derived,
// OnDifficultyChange_AttnInfo_Derived, HasAnyAttention_Derived, GetNormalizedAttnValue_Derived, GetAttnLevel_Derived,
// GetLastChangeReason_Derived, ShouldKeepAlarmRinging_Derived, GetTimeLeftInInvestigate_Derived, HasDirectAttention. The base
// bodies are the folded empties / zeroes of the PDB. Non-virtual bodies in Src/disattentioninfo_base.cpp.
public:
	// DISHONORED(port): the tweak-interface pair, 2012 vtable UDisAttentionInfo_Base{for IDisTweaksInterface} slots 4 and 5
	// (the same folds as UDisAISubProcess: UDisGrenadeComponent::GetTweaks_Derived 0x78da80 / ADisRatSpawner::SetTweaks_Derived
	// 0x78ff90, a load / store at interface+4 = object+60 = m_pAttentionTweaks). Without them the interface base answers NULL and
	// every complex info ticks against UDisTweaks_PawnAttention's all-zero class default (max attention 0: measured in GI2).
	// Retail's GetAttentionInfo resets a reused info with UObject::InitializeProperties( template ) (2012 0x93c20), which recreates
	// the object in place through StaticConstructObject. In this tree that in-place replacement of a live subobject left the exit
	// purge freeing twice (GI2 / GI2R / GI2D in agentGI.md section 6), so the reset is the property reset alone: ExitProperties +
	// InitProperties, the body Epic's InitializeProperties carries under #if 0 in UnObj.cpp.
	void ResetFromTemplate( class UObject* _pTemplate );
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
	void InitAttnInfo( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget, BYTE _OwnerRelationshipToTarget );
	void HandleStateChanges( class IDisAttentionTargetInterface* _pTarget, const class UDisAIBrainProcessAttention* _pAttnProcess, const class UDishonoredAIBrain* _pAttentiveBrain,
		UBOOL _bTweaksChanged, TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Messages );
	BYTE GetCurrentCorpseResponse( const class IDisAttentionTargetInterface* _pTarget, const class UDisAIBrainProcessAttention* _pAttnProcess );

	virtual UBOOL HasPsychicAttention() const { return FALSE; }
	virtual void InitAttnInfo_Derived( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget ) {}
	virtual void TickAttnInfo_Derived( FLOAT _fDeltaSeconds, class IDisAttentionTargetInterface* _pTarget, const class UDisAIBrainProcessAttention* _pAttnProcess, const class UDishonoredAIBrain* _pAttentiveBrain,
		UBOOL _bClearAttention, TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Changes,
		TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Messages ) {}
	virtual UBOOL IsAttentionDecreasing() const { return FALSE; }
	virtual void OnAttnTargetStateChange( const class UDisAIBrainProcessAttention* _pAttnProcess, const class IDisAttentionTargetInterface* _pTarget ) {}
	virtual void SetAsTopAttnTarget_Derived( class UDishonoredAIBrain* _pBrain, const struct FDisAttentionProxy& _rTopAttnTargetProxy ) {}
	virtual void IncreaseAttention_Derived( const struct FDisAttentionIncrease& _rIncrease, const struct FDisAttentionChangeReason& _rReason, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo, UBOOL& _rbOut_WitnessedDeath ) {}
	virtual void GainLOS_Derived() {}
	virtual void LoseLOS_Derived() {}
	virtual void BeTouched_Derived() {}
	virtual void EnterHideout_Derived() {}
	virtual void ExitHideout_Derived() {}
	virtual void GainPsychicAttention_Derived() {}
	virtual void LosePsychicAttention_Derived() {}
	virtual void SetMinAttentionLevel_Derived( BYTE _Type, BYTE _MinLevel ) {}
	virtual BYTE GetMinAttentionLevel_Derived( BYTE _Type ) const { return DAL_Unaware; }
	virtual void OnOtherActorTerminated_AttnInfo_Derived( const class AActor& _rActor ) {}
	virtual void OnDifficultyChange_AttnInfo_Derived() {}
	virtual UBOOL HasAnyAttention_Derived() const { return FALSE; }
	virtual FLOAT GetNormalizedAttnValue_Derived() const { return 0.f; }
	virtual BYTE GetAttnLevel_Derived() const { return DAL_Unaware; }
	virtual struct FDisAttentionChangeReason GetLastChangeReason_Derived() const;
	virtual UBOOL ShouldKeepAlarmRinging_Derived() const { return FALSE; }
	virtual FLOAT GetTimeLeftInInvestigate_Derived() const { return 0.f; }
	virtual UBOOL HasDirectAttention() const { return FALSE; }
