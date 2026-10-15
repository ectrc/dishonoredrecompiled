// FDisNPCRotationIntent cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(port): agent HC (PHASE22 HC). One rotation request of an NPC at one EDisFaceToPriority: the point to face, the
// yaw rate, and the FArkComponentFaceTo request carrying it (m_RequestID, -1 = none). Body in Src/dishonorednpcpawn_rotation.cpp.
public:
	// 2012 0x7cbee0 (165 B) / 2013 0xb714c0
	void SetTargetRotation( class ADishonoredNPCPawn* const _pNPCOwningPawn, const FVector& _rPawnLocation, FLOAT _fArkFaceToYawRateDegrees,
		FLOAT _fArkFaceToYawAccelerationDegrees, EDisFaceToPriority _RotationPriority, void* const _pAsker, FName _AskerName );
