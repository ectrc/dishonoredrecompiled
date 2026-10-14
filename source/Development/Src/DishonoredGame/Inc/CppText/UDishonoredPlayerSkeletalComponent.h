// UDishonoredPlayerSkeletalComponent cpptext: included inside the generated class body (dishonoredgameclasses.h).
// agent GF (PHASE19 GF): the player's first-person mesh component - the arm offset and the arms' own field of view the camera
// pass feeds (ADishonoredPlayerPawn::OnCameraUpdate_Derived), rendered through FDishonoredPlayerSkeletalSceneProxy.
// 2012 rvas (2013 = VA - 0x400000, named in both): SetOffset 0x71dc90 / 0x6cdec0, SetFOV 0x71d880 / 0x6cdab0, DisableFOV
// 0x71dab0 / 0x6cdce0, UpdateBounds 0x7203d0 / 0x6c8880, CreateSceneProxy 0x71aeb0 / 0x6df220, UpdateChildComponents 0x707580 /
// 0x6c8360, UpdateChildForTickEnd_Derived 0x720360 / 0x6ce100. Bodies in dishonoredplayerskeletalcomponent.cpp.
public:
	void SetOffset( const FVector& Offset );
	void SetFOV( FLOAT NewFOV );
	void DisableFOV();
	virtual void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
protected:
	virtual void UpdateChildComponents();
	virtual void UpdateChildForTickEnd_Derived();
public:
