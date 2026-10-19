// ADishonoredAudioPortal cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent IH3 (fork G). 2012 rvas (2013 in the bodies): GetVolumes 0x7f3de0, GetNearestPoint 0x7f5080 and
// 0x7f8280, CanConsiderPointInOtherCell 0x7f5200. Bodies in dishonoredaudioportal.cpp. GetVolumes is a virtual of the retail
// AGenericPortal, which this tree's Engine shim does not declare, so its slot starts here.
public:
	virtual void GetVolumes( AInOutVolume** _pVolA, AInOutVolume** _pVolB );
	void GetNearestPoint( const FVector& _rPoint, FVector& _rNearestPoint ) const;
	void GetNearestPoint( const FVector& _rPointInCellA, const FVector& _rPointInCellB, FVector& _rNearestPoint ) const;
	UBOOL CanConsiderPointInOtherCell( FVector _Point, ADishonoredAudioVolume* _pCell );
