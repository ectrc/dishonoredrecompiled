// DISHONORED(port): agent IH4 (PHASE23 IH4). The state-change parameters of the music state machine (2012 PDB
// FDisMusicState_*_Param). The combat and chase parameters carry one float each that their OnPending writes into the pending
// state's first own member (@88: m_fCombatIntensityValue / m_fChaseTimer); retail folds the two vtables and the two OnPending
// bodies (2012 0x8b8c80 / 2013 0x848600). FDisMusicState_Base_Param::OnPending is 2012 0x8b8b00 / 2013 0x8484e0.
#pragma once

#include "DishonoredGameNative.h"

struct FDisMusicState_Base_Param : public FDisNativeStateParam
{
	FDisMusicState_Base_Param( UClass* StateClass ) : FDisNativeStateParam( StateClass ) {}
	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMusicState_Base_Param); }
};

struct FDisMusicState_Exploration_Param : public FDisMusicState_Base_Param
{
	FDisMusicState_Exploration_Param();
	virtual INT SizeOf() const { return sizeof(FDisMusicState_Exploration_Param); }
};

struct FDisMusicState_Suspense_Param : public FDisMusicState_Base_Param
{
	FDisMusicState_Suspense_Param();
	virtual INT SizeOf() const { return sizeof(FDisMusicState_Suspense_Param); }
};

struct FDisMusicState_RatAttack_Param : public FDisMusicState_Base_Param
{
	FDisMusicState_RatAttack_Param();
	virtual INT SizeOf() const { return sizeof(FDisMusicState_RatAttack_Param); }
};

struct FDisMusicState_Combat_Param : public FDisMusicState_Base_Param
{
	FDisMusicState_Combat_Param( FLOAT InitialCombatIntensityValue );
	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMusicState_Combat_Param); }
	FLOAT m_fInitialCombatIntensityValue;
};

struct FDisMusicState_Chase_Param : public FDisMusicState_Base_Param
{
	FDisMusicState_Chase_Param( FLOAT ChaseTime );
	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMusicState_Chase_Param); }
	FLOAT m_fChaseTime;
};

namespace Music
{
	/** 2012 0x8b8950 / 2013 0x8483c0 (disglobalmusicmanager.cpp). */
	UBOOL IsEnemyNearPlayer( FLOAT _fRange );
}

/** -akdiag / -ih4diag: every music state entered and the event it posts (agent IH4). */
UBOOL DisIH4DiagEnabled();
#define IH4DIAG( Fmt, ... ) do { if( DisIH4DiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): ih4diag t=%.2f ") TEXT(Fmt), GWorld ? GWorld->GetTimeSeconds() : 0.f, __VA_ARGS__ ); } } while( 0 )

/** The event a music state posts on the player pawn (retail AActor::PostAkEvent, 2013 0x2cd730), with the -akdiag line. */
void DisMusicPostStateEvent( const TCHAR* _StateName, class UAkEvent* _pEvent );
