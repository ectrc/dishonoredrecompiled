// ADishonoredPlayerCamera cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): 2013 rva 0x6ca780 (2012 0x70a1c0) - the CAMERA half of the ACTOR feed. Body in
// dishonoredplayercamera.cpp.
public:
	void ApplyCameraPostProcess( struct FArkPpConfig& Config );

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x6c01f0 (2012 0x6fbf40). Body in dishonoredplayercamera.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );

// ---- agent FZ (PHASE18 FZ): the influence pipeline that forces the view during a soiree ----
// DISHONORED(port): retail 2012 rvas (dishonoredplayercamera.cpp): PostBeginPlay 0x71bbc0 (2013 0x6d42b0), Tick 0x70e6e0
// (2013 0x6d0290), UpdateViewTarget 0x7135c0 (2013 0x6d80a0, 5883 B - the influence branch and the no-collision placement
// are ported; CamMoveSmooth 0x700120, HandleCollision 0x709e60, the rain impacts and the debug box are not), TickFOV 0x709a80,
// SetFOVTarget 0x709f40 (2013 0x6ca500), ClearFOVTarget 0x709fb0 (2013 0x6ca570), GetCamera_Influence 0x709cf0 (2013
// 0x6ca280), OnTeleport 0x709de0, the GetCamera_* accessors 0x6fbf80..0x6fc060. TickPostProcess 0x709ba0 is not ported.
// m_InfluenceGroups is the SDK dump's untyped 12-byte gap at 1128; script_classes_2013 types it as
// array<DishonoredCameraInfluenceGroup>, which is what the accessor reads it as. Bodies in dishonoredplayercamera.cpp.
public:
	virtual void PostBeginPlay();
	virtual UBOOL Tick( FLOAT DeltaTime, ELevelTick TickType );
	virtual void UpdateViewTarget( struct FTViewTarget& OutVT, FLOAT DeltaTime );
	void SetFOVTarget( BYTE FOVPriority, FLOAT FOV, FLOAT BlendSpeed, UBOOL bLockArms );
	void ClearFOVTarget( BYTE FOVPriority );
	void OnTeleport( class USeqAct_Teleport* Action );
	TArrayNoInit<class UDishonoredCameraInfluenceGroup*>& InfluenceGroups() { return *reinterpret_cast<TArrayNoInit<class UDishonoredCameraInfluenceGroup*>*>( m_InfluenceGroups ); }
	class UDishonoredCamera_PhysicalReact* GetCamera_PhysicalReact() const { return m_pPhysicalReact_Influence; }
	class UDishonoredCamera_BumpSmoother* GetCamera_BumpSmoother() const { return m_pBumpSmoother_Influence; }
	class UDishonoredCamera_PlayerControl* GetCamera_PlayerControl() const { return m_pPlayerControl_Influence; }
	class UDishonoredCamera_AnimDriven* GetCamera_AnimDriven() const { return m_pAnimDrive_Influence; }
	class UDishonoredCamera_DisableArmFollow* GetCamera_DisableArmFollow( BYTE Usage ) const { return Usage == EDisEquipUsage_Secondary ? m_pDisableArmFollow_Secondary_Influence : m_pDisableArmFollow_Primary_Influence; }
	FLOAT GetCurFOV() const { return m_fCurFOV; }
private:
	void TickFOV( FLOAT DeltaTime );
	class UDishonoredCameraInfluence* GetCamera_Influence( UClass* InfluenceType ) const;
public:

// ---- agent IG (PHASE23 IG): the mantle's camera calls. EnableCollision 2012 0x6fc260 / 2013 0x6c0520 (28 B, m_bCollisionEnabled),
// GetCamera_CrouchMantleOffset 2012 0x6fc050 (7 B, one of the GetCamera_* accessors) ----
public:
	void EnableCollision( UBOOL bEnable ) { m_bCollisionEnabled = bEnable; }
	class UDishonoredCamera_CrouchMantleOffset* GetCamera_CrouchMantleOffset() const { return m_pCrouchMantleOffset_Influence; }
public:
