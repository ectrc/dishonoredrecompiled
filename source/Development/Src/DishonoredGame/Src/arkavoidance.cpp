// DishonoredGame/src/arkavoidance.cpp
// DISHONORED(port, agent JD): the crowd avoidance - FArkComponentAvoidance (retail Engine/Src/arkcomponentavoidance.cpp, 2012
// 0x575340..0x578ec0) and UArkAvoidable's out-of-line methods (retail Engine/Src/arkavoidable.cpp, 2012 0x526c80..0x529d00).
// Both are Engine units in retail; they are defined here because the tree declares UArkAvoidable only in DishonoredGame's shim
// header (Inc/CppText/UArkAvoidable.h carries the inline half). The component's class is in Engine/Inc/arkcomponentavoidance.h.

#include "DishonoredGame.h"
#include "arkcomponentpolicy.h"
#include "arkcomponentavoidance.h"
#include "jddiag.h"

ARKCOMPONENT_IMPLEMENT_TYPE( FArkComponentAvoidance )

UBOOL UArkAvoidable::m_bDisableAvoidable = FALSE;

// 2012 0xe33308..0xe3331c: the component's tuning, file-scope constants in retail
static const FLOAT fMinAvoidanceDist = 80.f;
static const FLOAT fAvoidanceDistRatio = 3.f;
static const FLOAT fMaxAvoidanceZDist = 300.f;
static const FLOAT fMaxAvoidanceCacheDist = 500.f;
static const FLOAT fForceMultiplierChangeSpeed = 10.f;
static const FLOAT fRadiusChangeSpeed = 100.f;

/*-----------------------------------------------------------------------------
	UArkAvoidable
-----------------------------------------------------------------------------*/

// 2012 0x526d30 (70 B): the owner is recorded whether or not the game has an avoidance component; registered only with one.
void UArkAvoidable::RegisterAvoidable( AActor* const _pActorOwner )
{
	FArkComponentAvoidance* pAvoidance = FArkComponentAvoidance::GetAvoidanceComponent( GWorld->GetWorldInfo()->Game );
	if( pAvoidance )
	{
		pAvoidance->RegisterAvoidable( this );
		m_bIsRegistered = TRUE;
	}
	m_pActorOwner = _pActorOwner;
}

// 2012 0x526d80 (72 B)
void UArkAvoidable::UnregisterAvoidable()
{
	m_pActorOwner = NULL;
	if( !m_bIsRegistered || !GWorld || !GWorld->GetWorldInfo() || !GWorld->GetWorldInfo()->Game )
	{
		return;
	}
	FArkComponentAvoidance* pAvoidance = FArkComponentAvoidance::GetAvoidanceComponent( GWorld->GetWorldInfo()->Game );
	if( pAvoidance )
	{
		pAvoidance->UnregisterAvoidable( this );
		m_bIsRegistered = FALSE;
	}
}

// 2012 0x526f50 (42 B)
void UArkAvoidable::ResetForceAndAffectorCount()
{
	m_Force = FVector( 0.f, 0.f, 0.f );
	m_AffectorCount = 0;
	m_pStrongerColAvoidable = NULL;
	m_fStrongerColStrength = 0.f;
}

// 2012 0x529d00 (57 B)
void UArkAvoidable::OnOtherAvoidableUnregistered( UArkAvoidable* _pOther )
{
	if( m_pStrongerColAvoidable == _pOther )
	{
		ResetForceAndAffectorCount();
	}
}

/*-----------------------------------------------------------------------------
	FArkComponentAvoidance
-----------------------------------------------------------------------------*/

FArkComponentAvoidance::FArkComponentAvoidance()
{
}

FArkComponentAvoidance::~FArkComponentAvoidance()
{
}

void FArkComponentAvoidance::Starting()
{
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Register( this );
	}
}

void FArkComponentAvoidance::Stopping()
{
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Unregister( this );
	}
}

FArkComponentAvoidance* FArkComponentAvoidance::GetAvoidanceComponent( const AGameInfo* const _pGameInfo )
{
	return _pGameInfo->m_ComponentContainer ? _pGameInfo->m_ComponentContainer->GetFirstComponent<FArkComponentAvoidance>( TRUE ) : NULL;
}

void FArkComponentAvoidance::RegisterAvoidable( UArkAvoidable* const _pAvoidable )
{
	m_Avoidables.AddUniqueItem( _pAvoidable );
}

// 2012 0x577470 (219 B): the leaving avoidable's force is cleared, every cached pair holding it is dropped, and the others
// forget it as their stronger collision.
void FArkComponentAvoidance::UnregisterAvoidable( UArkAvoidable* const _pAvoidable )
{
	_pAvoidable->ResetForceAndAffectorCount();
	m_Avoidables.RemoveSingleItem( _pAvoidable );
	for( INT PairIdx = m_ProxyCache.Num() - 1; PairIdx >= 0; --PairIdx )
	{
		const FArkCpntAvoidCacheProps& rPair = m_ProxyCache( PairIdx );
		if( rPair.m_pAvoidable1 == _pAvoidable || rPair.m_pAvoidable2 == _pAvoidable )
		{
			m_ProxyCache.Remove( PairIdx, 1 );
		}
	}
	for( INT AvoidableIdx = 0; AvoidableIdx < m_Avoidables.Num(); ++AvoidableIdx )
	{
		m_Avoidables( AvoidableIdx )->OnOtherAvoidableUnregistered( _pAvoidable );
	}
}

// 2012 0x578ec0 (1,406 B). Per avoidable: the current radius / force multiplier step towards their targets (100 uu/s, 10/s,
// snapping within one step; nothing under 0.1), then the force and affector count are cleared. Per pair of avoidables with
// different owners and a common group flag, less than 300 uu apart in Z: within 500 uu in 2D the pair is cached; closer than
// max(80, 3 x the radius sum) each is pushed away from the other along the 2D line between them - full strength inside the
// radius sum, fading to 0 at that distance; with different force multipliers each strength is scaled by 2 x the other's
// multiplier / the sum, the one on the lighter side capped at 1 - unless the other avoidable's property flag 2 says it does
// not push. Every overlap counts as an affector, and the strongest pusher is remembered.
void FArkComponentAvoidance::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	const INT AvoidableCount = m_Avoidables.Num();
	for( INT AvoidableIdx = 0; AvoidableIdx < AvoidableCount; ++AvoidableIdx )
	{
		UArkAvoidable* pAvoidable = m_Avoidables( AvoidableIdx );
		const FLOAT fRadiusDif = pAvoidable->GetTargetedRadius() - pAvoidable->GetRadius();
		if( Abs( fRadiusDif ) > 0.1f )
		{
			const FLOAT fStep = fRadiusChangeSpeed * _fTimeStep;
			if( fStep <= Abs( fRadiusDif ) )
			{
				pAvoidable->UpdateRadius( fRadiusDif <= 0.f ? pAvoidable->GetRadius() - fStep : pAvoidable->GetRadius() + fStep );
			}
			else
			{
				pAvoidable->UpdateRadius( pAvoidable->GetTargetedRadius() );
			}
		}
		const FLOAT fMultiplierDif = pAvoidable->GetTargetedForceMultiplier() - pAvoidable->GetForceMultiplier();
		if( Abs( fMultiplierDif ) > 0.1f )
		{
			const FLOAT fStep = fForceMultiplierChangeSpeed * _fTimeStep;
			if( fStep <= Abs( fMultiplierDif ) )
			{
				pAvoidable->UpdateForceMultiplier( fMultiplierDif <= 0.f ? pAvoidable->GetForceMultiplier() - fStep : pAvoidable->GetForceMultiplier() + fStep );
			}
			else
			{
				pAvoidable->UpdateForceMultiplier( pAvoidable->GetTargetedForceMultiplier() );
			}
		}
		pAvoidable->ResetForceAndAffectorCount();
	}

	m_ProxyCache.Reset();
	const FLOAT fMaxAvoidanceCacheSqDist = fMaxAvoidanceCacheDist * fMaxAvoidanceCacheDist;
	for( INT AvoidableIdx1 = 0; AvoidableIdx1 < AvoidableCount; ++AvoidableIdx1 )
	{
		UArkAvoidable* pAvoidable1 = m_Avoidables( AvoidableIdx1 );
		const FVector& Location1 = pAvoidable1->GetLocation();
		for( INT AvoidableIdx2 = AvoidableIdx1 + 1; AvoidableIdx2 < AvoidableCount; ++AvoidableIdx2 )
		{
			UArkAvoidable* pAvoidable2 = m_Avoidables( AvoidableIdx2 );
			if( pAvoidable2->GetOwner() == pAvoidable1->GetOwner() || ( pAvoidable2->GetGroupFlags() & pAvoidable1->GetGroupFlags() ) == 0 )
			{
				continue;
			}
			const FVector& Location2 = pAvoidable2->GetLocation();
			if( fMaxAvoidanceZDist <= Abs( Location2.Z - Location1.Z ) )
			{
				continue;
			}
			FVector Avoidable1ToAvoidable2( Location2.X - Location1.X, Location2.Y - Location1.Y, 0.f );
			const FLOAT fRadiusSum = pAvoidable2->GetRadius() + pAvoidable1->GetRadius();
			const FLOAT fMaxAvoidanceDist = Max( fMinAvoidanceDist, fAvoidanceDistRatio * fRadiusSum );
			const FLOAT fSquaredDist = Avoidable1ToAvoidable2.Y * Avoidable1ToAvoidable2.Y + Avoidable1ToAvoidable2.X * Avoidable1ToAvoidable2.X;
			if( fMaxAvoidanceCacheSqDist > fSquaredDist )
			{
				FArkCpntAvoidCacheProps& rPair = m_ProxyCache( m_ProxyCache.Add() );
				rPair.m_pAvoidable1 = pAvoidable1;
				rPair.m_pAvoidable2 = pAvoidable2;
			}
			if( fSquaredDist <= 0.f || fMaxAvoidanceDist * fMaxAvoidanceDist <= fSquaredDist )
			{
				continue;
			}
			const FLOAT fDist = appSqrt( fSquaredDist );
			Avoidable1ToAvoidable2 *= 1.f / fDist;
			FLOAT fForceStrength = 1.f;
			if( fDist > fRadiusSum )
			{
				fForceStrength = ( fMaxAvoidanceDist - fDist ) / ( fMaxAvoidanceDist - fRadiusSum );
			}
			const FLOAT fForceMultiplier1 = pAvoidable1->GetForceMultiplier();
			const FLOAT fForceMultiplier2 = pAvoidable2->GetForceMultiplier();
			FLOAT fForceStrength1 = fForceStrength;
			FLOAT fForceStrength2 = fForceStrength;
			if( Abs( fForceMultiplier1 - fForceMultiplier2 ) >= 0.01f )
			{
				const FLOAT fScale = ( 2.f / ( fForceMultiplier2 + fForceMultiplier1 ) ) * fForceStrength;
				if( fForceMultiplier1 <= fForceMultiplier2 )
				{
					fForceStrength1 = Min( fForceMultiplier2 * fScale, 1.f );
					fForceStrength2 = fForceMultiplier1 * fScale;
				}
				else
				{
					fForceStrength1 = fForceMultiplier2 * fScale;
					fForceStrength2 = Min( fForceMultiplier1 * fScale, 1.f );
				}
			}
			if( pAvoidable2->GetPropertyFlags() & 2 )
			{
				fForceStrength1 = 0.f;
			}
			else
			{
				pAvoidable1->SubForce( Avoidable1ToAvoidable2 * fForceStrength1 );
			}
			pAvoidable1->IncrementAffectorCount();
			if( fForceStrength1 >= pAvoidable1->GetStrongerColStrength() )
			{
				pAvoidable1->SetStrongerColAvoidable( pAvoidable2, fForceStrength1 );
			}
			if( pAvoidable1->GetPropertyFlags() & 2 )
			{
				fForceStrength2 = 0.f;
			}
			else
			{
				pAvoidable2->AddForce( Avoidable1ToAvoidable2 * fForceStrength2 );
			}
			pAvoidable2->IncrementAffectorCount();
			if( fForceStrength2 >= pAvoidable2->GetStrongerColStrength() )
			{
				pAvoidable2->SetStrongerColAvoidable( pAvoidable1, fForceStrength2 );
			}
			if( DisJDDiag( TEXT("avoid") ) && ( DisJDWatched( pAvoidable1->GetOwner() ) || DisJDWatched( pAvoidable2->GetOwner() ) ) )
			{
				debugf( TEXT("DISHONORED(bringup): jddiag avoid world %.3f %s/%s dist %.1f radii %.1f+%.1f max %.1f strength %.2f/%.2f mult %.2f/%.2f"),
					GWorld->GetTimeSeconds(), *pAvoidable1->GetOwner()->GetName(), *pAvoidable2->GetOwner()->GetName(), fDist,
					pAvoidable1->GetRadius(), pAvoidable2->GetRadius(), fMaxAvoidanceDist, fForceStrength1, fForceStrength2,
					fForceMultiplier1, fForceMultiplier2 );
			}
		}
	}
}
