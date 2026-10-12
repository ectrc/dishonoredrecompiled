// UDishonoredCamera_PlayerControl cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The ordinary first-person influence: the pawn's camera position and view
// rotation, with the arm-follow weights at 1. Retail 2012 rvas (dishonoredcamera_playercontrol.cpp):
// PostBeginPlay_Derived 0x721e20, OnActivate_Derived 0x721e30, OnTeleport 0x721e40, GoToTargetOnNextControllerReset
// 0x721e50 (2013 0x6c1660), CalcItemSpecificOffset 0x729f80 (2013 0x6cb8d0), GatherInfluence_Derived 0x72a0f0 (2013
// 0x6cba40). Bodies in Src/dishonoredcamera_playercontrol.cpp.
public:
	virtual void OnTeleport( class USeqAct_Teleport* Action ) { m_bResetControllerRot = TRUE; }
	void GoToTargetOnNextControllerReset( const FRotator& ResetTarget );
protected:
	virtual void PostBeginPlay_Derived() { m_bFirstRotation = TRUE; }
	virtual void OnActivate_Derived( APawn* Pawn );
	virtual UBOOL GatherInfluence_Derived( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
	FLOAT CalcItemSpecificOffset( const APawn* Pawn, FLOAT CurPitch, BYTE Usage ) const;
public:
