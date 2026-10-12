// UDishonoredCamera_AnimDriven cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The influence a soiree blends the player camera to: the view is the
// player's camera bone, plus a player-controlled offset clamped to the limits UDisInterpTrackAllowPlayerLook sets.
// Retail 2012 rvas (dishonoredcamera_animdriven.cpp): OnActivate_Derived 0x721d40, OnDeactivate_Derived 0x721d90,
// GetAnimDrivenTransform 0x726e40 (2013 0x6c1440), SetPlayerControlledLimit 0x729960 (2013 0x6cb300),
// CalcBlendedPlayerDelta 0x729aa0 (2013 0x6cb440, a free function), UpdatePlayerControlLimit 0x729bb0 (2013 0x6cb550),
// GatherInfluence_Derived 0x72cf70 (2013 0x6d08a0). Bodies in Src/dishonoredcamera_animdriven.cpp.
public:
	void GetAnimDrivenTransform( const class ADishonoredPlayerPawn* Player, FVector& OutLocation, FRotator& OutRotation ) const;
	void SetPlayerControlledLimit( FLOAT MaxDegreesYaw_Neg, FLOAT MaxDegreesYaw_Pos, FLOAT MaxDegreesPitch_Neg, FLOAT MaxDegreesPitch_Pos, INT Flags );
	void UpdatePlayerControlLimit( APawn* Pawn, FLOAT DeltaTime );
protected:
	virtual void OnActivate_Derived( APawn* Pawn );
	virtual void OnDeactivate_Derived( APawn* Pawn );
	virtual UBOOL GatherInfluence_Derived( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
public:
