// DishonoredGame/src/disattentionproxy.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21): the eighteen FDisAttentionProxy methods below, the two constructors and
// DisStructDevLoad.

// ---- agent CG ports (PHASE9 CG): FDisAttentionProxy ----
//
// A proxy is a (brain, attention target) pair and holds no attention state of its own: every query goes back through
// the brain, which is why the same target can be Busted for one guard and Unaware for the guard beside him. The cached
// actor pointer is the only thing the proxy keeps, and it exists so that IsEqualToActor and GetProxySpeaker do not have
// to walk the interface.
//
// DISHONORED(bringup) for the whole file, stated once: the eight queries that go through
// UDishonoredAIBrain::GetAttentionProxyInfo / GetAttentionLevel read their answer from UDisAIBrainProcessAttention
// (2013 rvas 0x78e0f0 GetAttentionLevel, 0x78e130 GetAttentionProxyInfo), which is not ported. A brain with no
// attention process answers DAL_Unaware and a zeroed FDisAttentionProxyInfo, which is retail's own path for a brain
// whose tweaks did not give it the attention process - so these bodies are correct, they just have nothing to report
// until that process lands. The ten that do not touch attention (the target, the speaker, the actor tests, the clear,
// the GC pass) are fully live.

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x74c8e0 (2012 0x79eb70): the brain and its target, with the target's actor cached.
FDisAttentionProxy::FDisAttentionProxy( UDishonoredAIBrain* _pBrain, IDisAttentionTargetInterface* _pTarget )
{
	m_pBrain = _pBrain;
	m_pTarget = TScriptInterface<IDisAttentionTargetInterface>();
	m_pCachedTargetActor = NULL;
	if( _pTarget )
	{
		m_pCachedTargetActor = _pTarget->GetAttnTargetActor();
		m_pTarget.SetObject( m_pCachedTargetActor );
		m_pTarget.SetInterface( _pTarget );
	}
}

// DISHONORED(port): 2013 rva 0x74cb60 (2012 0x790810): the interface half is only valid when the object half is, which
// is the TScriptInterface contract every reader here repeats.
IDisAttentionTargetInterface* FDisAttentionProxy::GetProxyAttnTarget() const
{
	return m_pTarget.GetObject() ? (IDisAttentionTargetInterface*)m_pTarget.GetInterface() : NULL;
}

// DISHONORED(port): 2013 rva 0x7504b0 (2012 0x793cf0): the speaker interface of the cached actor, for the dialog hooks.
IDisConvSpeakerInterface* FDisAttentionProxy::GetProxySpeaker() const
{
	if( !m_pTarget.GetObject() || !m_pTarget.GetInterface() || !m_pCachedTargetActor )
	{
		return NULL;
	}
	return (IDisConvSpeakerInterface*)m_pCachedTargetActor->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() );
}

// DISHONORED(port): 2013 rva 0x74c8f0 (2012 0x7905a0)
BYTE FDisAttentionProxy::GetProxyAttnLevel() const
{
	return m_pBrain ? m_pBrain->GetAttentionLevel( GetProxyAttnTarget() ) : DAL_Unaware;
}

// DISHONORED(port): 2013 rva 0x74cb70 (2012 0x790820)
INT FDisAttentionProxy::GetProxyAttnTag() const
{
	if( !m_pBrain )
	{
		return 0;
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	return Info.m_AttentionTag;
}

// DISHONORED(port): 2013 rva 0x74c920 (2012 0x7905d0)
FVector FDisAttentionProxy::GetProxyLocation() const
{
	if( !m_pBrain )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	return Info.m_Loc;
}

// DISHONORED(port): 2013 rva 0x74c9a0 (2012 0x790650): the focal point lowered by the target's focal height. When the
// brain has no height for the target (retail's sentinel is FLT_MAX) the target's own
// IDisAttentionTargetInterface::GetAttnTargetFocalHeight is asked instead.
FVector FDisAttentionProxy::GetProxyFeetLocation() const
{
	if( !m_pBrain )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	FVector Result = Info.m_Loc;
	if( Info.m_fHeight != 3.4028235e38f )
	{
		Result.Z -= Info.m_fHeight;
		return Result;
	}
	IDisAttentionTargetInterface* Target = GetProxyAttnTarget();
	if( Target )
	{
		Result.Z -= Target->GetAttnTargetFocalHeight();
	}
	return Result;
}

// DISHONORED(port): 2013 rva 0x74ca80 (2012 0x790730)
FLOAT FDisAttentionProxy::GetProxyHeight() const
{
	if( !m_pBrain )
	{
		return 0.f;
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	return Info.m_fHeight;
}

// DISHONORED(port): 2013 rva 0x74cc80 (2012 0x790930): where to aim or walk. A proxy that was verified once and is not
// verified now reports the last place it WAS verified rather than the brain's current guess, which is what makes a
// guard search the doorway it last saw you in rather than your real position.
FVector FDisAttentionProxy::GetBestTargetLocation() const
{
	FDisAttentionProxyInfo Info( EC_EventParm );
	if( m_pBrain )
	{
		m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	}
	return ( Info.m_Status == DAPS_PreviouslyVerified ) ? Info.m_MostRecentVerifiedLoc : Info.m_Loc;
}

// DISHONORED(port): 2013 rva 0x74cb00 (2012 0x7907b0)
FDisAttentionChangeReason FDisAttentionProxy::GetProxyChangeReason() const
{
	FDisAttentionChangeReason Result( EC_EventParm );
	if( m_pBrain )
	{
		FDisAttentionProxyInfo Info( EC_EventParm );
		m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
		Result = Info.m_ProxyUpdateReason;
	}
	return Result;
}

// DISHONORED(port): 2013 rva 0x74cbc0 (2012 0x790870)
UBOOL FDisAttentionProxy::IsBusted() const
{
	return GetProxyAttnLevel() == DAL_Busted;
}

// DISHONORED(port): 2013 rva 0x74cc00 (2012 0x7908b0) / 0x74cc40 (0x7908f0): the two ends of the status scale.
UBOOL FDisAttentionProxy::IsIndeterminate() const
{
	if( !m_pBrain )
	{
		return FALSE;
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	return Info.m_Status == DAPS_Indeterminate;
}

UBOOL FDisAttentionProxy::IsVerifiedAsTarget() const
{
	if( !m_pBrain )
	{
		return FALSE;
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	m_pBrain->GetAttentionProxyInfo( GetProxyAttnTarget(), Info );
	return Info.m_Status == DAPS_Verified;
}

// DISHONORED(port): 2013 rva 0x7504f0 (2012 0x793d30): a proxy is valid when it has a brain and a live target. In the
// game (as against the editor) a target actor that has both terminated and been marked for deletion invalidates it -
// retail tests AActor @300 bits 0x8 (m_bHasTerminated) and 0x10 (m_bLegalToTerminate) together.
UBOOL FDisAttentionProxy::IsValid() const
{
	if( !m_pBrain || !m_pTarget.GetObject() || !m_pTarget.GetInterface() )
	{
		return FALSE;
	}
	if( !GIsGame )
	{
		return TRUE;
	}
	AActor* Actor = ( (FDisAttentionProxy*)this )->GetProxyAttnTarget()->GetAttnTargetActor();
	if( !Actor )
	{
		return TRUE;
	}
	return !( Actor->m_bHasTerminated && Actor->m_bLegalToTerminate );
}

// DISHONORED(port): 2013 rva 0x74ccd0 (2012 0x790980) / 0x748880 (0x78de30)
UBOOL FDisAttentionProxy::HasActorReference() const
{
	return m_pTarget.GetObject() != NULL && m_pTarget.GetInterface() != NULL;
}

UBOOL FDisAttentionProxy::IsEqualToActor( const AActor& _rActor ) const
{
	return &_rActor == m_pCachedTargetActor;
}

// DISHONORED(port): 2013 rva 0x74ccf0 (2012 0x7909a0): the target itself decides (IDisAttentionTargetInterface vtable
// slot 76 in 2012, the "can be attacked" query).
UBOOL FDisAttentionProxy::CanProxyBeAttacked() const
{
	IDisAttentionTargetInterface* Target = ( (FDisAttentionProxy*)this )->GetProxyAttnTarget();
	return Target ? Target->CanAttnTargetBeAttacked() : FALSE;
}

// DISHONORED(port): 2013 rva 0x74cd20 (2012 0x7909d0)
void FDisAttentionProxy::ClearAttnProxy()
{
	m_pBrain = NULL;
	m_pTarget = TScriptInterface<IDisAttentionTargetInterface>();
	m_pCachedTargetActor = NULL;
}

// DISHONORED(port): 2013 rva 0x750470 (2012 0x793cb0): the brain and the target object are both visible to a collector,
// and a target whose object was collected away has its interface half cleared too - without that the proxy would keep a
// live interface pointer into a dead object.
void FDisAttentionProxy::SerializeForGC( FArchive& _rAr )
{
	_rAr << *(UObject**)&m_pBrain;
	UObject* TargetObject = m_pTarget.GetObject();
	_rAr << TargetObject;
	m_pTarget.SetObject( TargetObject );
	if( !TargetObject )
	{
		m_pTarget.SetInterface( NULL );
	}
}
