// UInterpGroupInstPlayer cpptext: included inside the generated class body.

// ---- agent FK (PHASE15 FK): the player group's instance ----
public:
	/** DISHONORED(port): agent FK, 2013 rva 0x5027f0 (2012 0x541960). Body in Src/interpgroupplayer.cpp. */
	virtual void InitGroupInst( class UInterpGroup* InGroup, class AActor* InGroupActor );

	/** DISHONORED(port): agent FK, 2013 rva 0x4fe910 (2012 0x53dd50). */
	virtual void TermGroupInst( UBOOL bDeleteTrackInst );

	/**
	 * DISHONORED(port): agent FK, 2013 rva 0x4fc500 (2012 0x53b6c0). Where the player has to stand when
	 * the matinee starts: the group actor of the group named by m_StageMarkGroup, dropped to the floor
	 * with a TRACE_World line check unless m_bDisablePlayerCollision is set.
	 */
	UBOOL GetStageMarkPosition( FVector& OutPos, FRotator* OutRot );

	/** DISHONORED(port): agent FK, 2013 rva 0x4fd880 (2012 0x53c5c0). The blend from where the player was to the mark. */
	void BlendPlayerToMark( FLOAT TimeSliderPos );
