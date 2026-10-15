// DishonoredGame/src/dishonorednpcpawn_rotation.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x7c9570  public: void __thiscall ADishonoredNPCPawn::Init_Rotation(void)
//   0x7c95a0  public: void __thiscall ADishonoredNPCPawn::CancelAllNPCRotations(void)
//   0x7c95e0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::SetDesiredRotation(class FRotator, unsigned int, unsigned int, float, unsigned int)
//   0x7c95f0  private: virtual void __thiscall ADishonoredNPCPawn::physicsRotation(float, class FVector)
//   0x7cbee0  public: void __thiscall FDisNPCRotationIntent::SetTargetRotation(class ADishonoredNPCPawn * const, class FVector const &, float, float, enum EDisFaceToPriority, void * const, class FName)
//   0x7cbf90  public: void __thiscall ADishonoredNPCPawn::SetNPCRotationFocus(enum EDisFaceToPriority, class FVector const &, float, float, void * const, class FName)
//   0x7cc020  public: void __thiscall ADishonoredNPCPawn::LockNPCRotation(enum EDisFaceToPriority, void * const, class FName)
//   0x7cc0f0  private: void __thiscall ADishonoredNPCPawn::OnUnlockNPCRotation(enum EDisFaceToPriority)
//   0x7ce4a0  public: void __thiscall ADishonoredNPCPawn::SetNPCRotation(enum EDisFaceToPriority, class FRotator const &, float, float, void * const, class FName)
//   0x7ce540  public: void __thiscall ADishonoredNPCPawn::UnlockNPCRotation(enum EDisFaceToPriority)
//   0x7d4d40  public: void __thiscall ADishonoredNPCPawn::Tick_Rotation(float)

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x76e960 (2012 0x7c95e0): retail's ADishonoredNPCPawn::SetDesiredRotation is an 8-byte
// `mov eax,1 / ret 14h`. The class overrides the script-facing entry point and does NOT route it into the
// rotation-intent system: an NPC's facing comes from m_NPCRotationIntent through the rotation tick, so a script or
// Kismet request to set the desired rotation directly is accepted and then ignored. The 2013 body is the same eight
// bytes as the 2012 one, so this is retail behaviour and not a stripped build.
void ADishonoredNPCPawn::execSetDesiredRotation( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FRotator, _TargetDesiredRotation);
	P_GET_UBOOL(_bInLockDesiredRotation);
	P_GET_UBOOL(_bInUnlockWhenReached);
	P_GET_FLOAT(_InterpolationTime);
	P_GET_UBOOL(_bResetRotationRate);
	P_FINISH;
	*(UBOOL*)Result = SetDesiredRotation( _TargetDesiredRotation, _bInLockDesiredRotation, _bInUnlockWhenReached, _InterpolationTime, _bResetRotationRate );
}

UBOOL ADishonoredNPCPawn::SetDesiredRotation( FRotator _TargetDesiredRotation, UBOOL _bInLockDesiredRotation, UBOOL _bInUnlockWhenReached, FLOAT _InterpolationTime, UBOOL _bResetRotationRate )
{
	return TRUE;
}

// ---- agent HC (PHASE22 HC): the rotation intents ----
// DISHONORED(bringup): FArkComponentFaceTo (Engine/Src/arkcomponentfaceto.cpp) is a skeleton and no NPC owns one, so
// ADishonoredNPCPawn::GetComponentFaceTo (2012 0x7ab560) would answer NULL everywhere: the intents are kept exactly as retail
// keeps them, but no face-to request is started, updated or stopped (m_RequestID stays -1). What turns a retail NPC towards
// its intent is that component's PreAsyncWorkTick; here nothing does, and physicsRotation no longer turns him towards his
// controller either (APawn::physicsRotation), which is retail's.

// DISHONORED(port): 2012 0x7c95f0 (71 B): the NPC does not rotate in physics; it publishes Tick_Rotation's result as its
// desired rotation and rotation rate for the systems that read them
void ADishonoredNPCPawn::physicsRotation( FLOAT deltaTime, FVector OldVelocity )
{
	DesiredRotation = m_FinalDesiredRotation;
	RotationRate = m_FinalRotationRateFixed;
}

// DISHONORED(port): 2012 0x7d4d40 (441 B). A deactivate-when-reached intent at the current priority unlocks once the yaw
// to its focus is inside AllowedYawError; an inactive current intent hands over to the next lower active one; the active
// intent (or, with none, the current rotation at rate 0) becomes the final desired rotation, the rate in degrees turned into
// rotator units on all three axes - a rate of 0 or less keeps the current rotation.
void ADishonoredNPCPawn::Tick_Rotation( FLOAT _fDeltaTime )
{
	FDisNPCRotationIntent* Intent = &m_NPCRotationIntent[m_CurrentNPCRotationPriority];
	if( Intent->m_bDeactivateWhenReached )
	{
		const INT TargetYaw = ( Intent->m_RotationFocus - Location ).Rotation().Yaw & 0xFFFF;
		const INT Yaw = Rotation.Yaw & 0xFFFF;
		const INT Delta = Abs( TargetYaw - Yaw );
		if( Delta < AllowedYawError || Delta > 0xFFFF - AllowedYawError )
		{
			UnlockNPCRotation( (EDisFaceToPriority)m_CurrentNPCRotationPriority );
		}
	}
	if( !Intent->m_bIsActive )
	{
		if( m_CurrentNPCRotationPriority == 0 )
		{
			Intent = NULL;
		}
		else
		{
			OnUnlockNPCRotation( (EDisFaceToPriority)m_CurrentNPCRotationPriority );
			Intent = &m_NPCRotationIntent[m_CurrentNPCRotationPriority];
		}
	}
	if( Intent == NULL )
	{
		m_FinalDesiredRotation = Rotation;
		m_FinalRotationRateFixed = FRotator( 0, 0, 0 );
		return;
	}
	const INT Rate = appTrunc( Intent->m_fRotationRateDegrees * 182.04445f );
	m_FinalDesiredRotation = ( Rate > 0 ) ? ( Intent->m_RotationFocus - Location ).Rotation() : Rotation;
	m_FinalRotationRateFixed = FRotator( Rate, Rate, Rate );

	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("gadiag") );
	if( bDiag && GWorld && GWorld->GetWorldInfo() && m_CurrentNPCRotationPriority != 0 )
	{
		static TMap<ADishonoredNPCPawn*, FLOAT> s_NextReport;
		const FLOAT Now = GWorld->GetWorldInfo()->TimeSeconds;
		FLOAT* Next = s_NextReport.Find( this );
		if( Next == NULL || Now >= *Next )
		{
			s_NextReport.Set( this, Now + 1.f );
			debugf( TEXT("gadiag rot %s yaw %i/%i prio %i rate %i active %i reached %i"), *GetName(), Rotation.Yaw & 0xFFFF,
				m_FinalDesiredRotation.Yaw & 0xFFFF, (INT)m_CurrentNPCRotationPriority, Rate, (INT)Intent->m_bIsActive, (INT)Intent->m_bDeactivateWhenReached );
		}
	}
}

// DISHONORED(port): 2012 0x7c9570 (36 B): no intent active, every request id free
void ADishonoredNPCPawn::Init_Rotation()
{
	m_CurrentNPCRotationPriority = 0;
	for( INT Priority = 0; Priority < ARRAY_COUNT( m_NPCRotationIntent ); Priority++ )
	{
		m_NPCRotationIntent[Priority].m_bIsActive = FALSE;
		m_NPCRotationIntent[Priority].m_RequestID = INDEX_NONE;
	}
}

// DISHONORED(port): 2012 0x7c95a0 (64 B): every intent's face-to request stopped (see the banner) and the intent dropped
void ADishonoredNPCPawn::CancelAllNPCRotations()
{
	m_CurrentNPCRotationPriority = 0;
	for( INT Priority = 0; Priority < ARRAY_COUNT( m_NPCRotationIntent ); Priority++ )
	{
		m_NPCRotationIntent[Priority].m_bIsActive = FALSE;
		m_NPCRotationIntent[Priority].m_RequestID = INDEX_NONE;
	}
}

// DISHONORED(port): 2012 0x7cbee0 (165 B): the intent becomes active on the point, and its face-to request is started (or updated)
// with the rate and the acceleration
void FDisNPCRotationIntent::SetTargetRotation( ADishonoredNPCPawn* const _pNPCOwningPawn, const FVector& _rPawnLocation, FLOAT _fArkFaceToYawRateDegrees,
	FLOAT _fArkFaceToYawAccelerationDegrees, EDisFaceToPriority _RotationPriority, void* const _pAsker, FName _AskerName )
{
	m_bIsActive = TRUE;
	m_RotationFocus = _rPawnLocation;
	m_fRotationRateDegrees = _fArkFaceToYawRateDegrees;
}

// DISHONORED(port): 2012 0x7ce4a0 (156 B): a rotation is a focus 888888 uu away along it
void ADishonoredNPCPawn::SetNPCRotation( EDisFaceToPriority _RotationPriority, const FRotator& _rRotation, FLOAT _fYawRateDegrees, FLOAT _fYawAccelerationTime, void* const _pAsker, FName _AskerName )
{
	const FVector Target = Location + _rRotation.Vector() * 888888.f;
	SetNPCRotationFocus( _RotationPriority, Target, _fYawRateDegrees, _fYawAccelerationTime, _pAsker, _AskerName );
}

// DISHONORED(port): 2012 0x7cbf90 (140 B): the acceleration is rate / time (-200 for a time under 0.01 s); a focus request is
// not deactivated when reached; it raises the current priority
void ADishonoredNPCPawn::SetNPCRotationFocus( EDisFaceToPriority _RotationPriority, const FVector& _rTarget, FLOAT _fYawRateDegrees, FLOAT _fYawAccelerationTime, void* const _pAsker, FName _AskerName )
{
	FLOAT fYawAccelerationDegrees = -200.f;
	if( Abs( _fYawAccelerationTime ) >= 0.01f )
	{
		fYawAccelerationDegrees = _fYawRateDegrees / _fYawAccelerationTime;
	}
	m_NPCRotationIntent[_RotationPriority].m_bDeactivateWhenReached = FALSE;
	m_NPCRotationIntent[_RotationPriority].SetTargetRotation( this, _rTarget, _fYawRateDegrees, fYawAccelerationDegrees, _RotationPriority, _pAsker, _AskerName );
	if( _RotationPriority > m_CurrentNPCRotationPriority )
	{
		m_CurrentNPCRotationPriority = _RotationPriority;
	}
}

// DISHONORED(port): 2012 0x7cc020 (193 B): hold the current facing - a focus 88 uu ahead at rate -400 (keep), acceleration -200
void ADishonoredNPCPawn::LockNPCRotation( EDisFaceToPriority _RotationPriority, void* const _pAsker, FName _AskerName )
{
	const FVector FaceToLocation = Location + Rotation.Vector() * 88.f;
	m_NPCRotationIntent[_RotationPriority].SetTargetRotation( this, FaceToLocation, -400.f, -200.f, _RotationPriority, _pAsker, _AskerName );
	if( _RotationPriority > m_CurrentNPCRotationPriority )
	{
		m_CurrentNPCRotationPriority = _RotationPriority;
	}
}

// DISHONORED(port): 2012 0x7ce540 (89 B) / 2013 0xb74a00: the intent dropped at rate 0, its face-to request stopped (none here,
// see the banner)
void ADishonoredNPCPawn::UnlockNPCRotation( EDisFaceToPriority _RotationPriority )
{
	m_NPCRotationIntent[_RotationPriority].m_bIsActive = FALSE;
	m_NPCRotationIntent[_RotationPriority].m_fRotationRateDegrees = 0.f;
	m_NPCRotationIntent[_RotationPriority].m_RequestID = INDEX_NONE;
	OnUnlockNPCRotation( _RotationPriority );
}

// DISHONORED(port): 2012 0x7cc0f0 (86 B): when the current priority is the one unlocked, it falls to the next lower active intent
// (0 when none)
void ADishonoredNPCPawn::OnUnlockNPCRotation( EDisFaceToPriority _UnlockedPriority )
{
	if( _UnlockedPriority != m_CurrentNPCRotationPriority )
	{
		return;
	}
	m_CurrentNPCRotationPriority = ( _UnlockedPriority > 1 ) ? (BYTE)( _UnlockedPriority - 1 ) : 0;
	while( m_CurrentNPCRotationPriority && !m_NPCRotationIntent[m_CurrentNPCRotationPriority].m_bIsActive )
	{
		m_CurrentNPCRotationPriority--;
	}
}
