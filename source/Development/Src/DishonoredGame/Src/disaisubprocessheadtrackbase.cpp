// DishonoredGame/src/disaisubprocessheadtrackbase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x78d390  private: virtual unsigned char const * __thiscall UDisAISubProcessHeadTrackBase::BuildFilterStimMask(void)const
//   0x78f0f0  private: void __thiscall UDisAISubProcessHeadTrackBase::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x7973c0  public: static class UClass * __cdecl UDisAISubProcessHeadTrackBase::GetPrivateStaticClassUDisAISubProcessHeadTrackBase(wchar_t const *)
//   0x797450  protected: struct FDisAttentionProxy __thiscall UDisAISubProcessHeadTrackBase::GetHeadTrackProxy(void)const
//   0x79abc0  public: static void __cdecl UDisAISubProcessHeadTrackBase::InitializePrivateStaticClassUDisAISubProcessHeadTrackBase(void)
//   0x79abe0  private: void __thiscall UDisAISubProcessHeadTrackBase::SetHeadTrackProxy(struct FDisAttentionProxy const &)
//   0x79bd60  public: static class UClass * __cdecl UDisAISubProcessHeadTrackBase::StaticClassNoInline(void)
//   0x79bd90  protected: virtual void __thiscall UDisAISubProcessHeadTrackBase::BeginSubProcess_Derived(void)
//   0x79bdc0  protected: virtual void __thiscall UDisAISubProcessHeadTrackBase::EndSubProcess_Derived(unsigned int)
//   0x79bdf0  private: unsigned int __thiscall UDisAISubProcessHeadTrackBase::FilterHeadTrackBegin(struct FAIStimStruct_HeadTrackBegin const &)
//   0x79be10  private: unsigned int __thiscall UDisAISubProcessHeadTrackBase::FilterNoticeBegin(struct FAIStimStruct_NoticeBegin const &)
//   0x79be40  private: virtual void __thiscall UDisAISubProcessHeadTrackBase::BeginDestroy(void)
//   0x7a04c0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAISubProcessHeadTrackBase::GetFilterStimDelegate_SubProcess(enum EAIStimID)

// ---- agent GE ports (PHASE19 GE): the head-track sub-process base ----
//
// Resolved by hand in both databases (build/agentGE/idb, decompiles in build/agentGE/decomp2012 and decomp2013). The 2013
// vtable (0xd3a65c for Bark, 0xd3a130 for Pause) carries one slot more than 2012's before the sub-process tail, which is
// how the unnamed 2013 bodies were pinned: BeginSubProcess_Derived 0x73e5d0 (30 B), EndSubProcess_Derived 0x73e5f0 (32 B),
// BuildFilterStimMask 0x728a60 (43 B, bits 44/45/63 = HeadTrackBegin/HeadTrackEnd/NoticeBegin, the 2013 ids this tree
// declares), GetFilterStimDelegate_SubProcess 0x744d00 (named), SetHeadTrackProxy 0x73acd0 (168 B, the one callee of
// FilterHeadTrackBegin 0x73e610), GetHeadTrackProxy 0x734fd0 (named). The three class-registration functions are the
// macro's. Nothing in this unit looks at the player: the proxy arrives from the brain's attention level (see the cpptext).
#include "DishonoredGame.h"
#include "disaisubstate.h"

// DISHONORED(bringup): every function of this unit is a virtual, so nothing references the object and MSVC would drop it out
// of the static library. Named from DishonoredGameRegistrants.cpp (the generator collects it).
void DishonoredLinkUDisAISubProcessHeadTrackBase()
{
}

// DISHONORED(port): 2012 rva 0x797450 / 2013 0x734fd0 - a copy of the proxy (the interface pair is copied only when the object is set)
FDisAttentionProxy UDisAISubProcessHeadTrackBase::GetHeadTrackProxy() const
{
	FDisAttentionProxy Proxy;
	Proxy.m_pBrain = m_HeadTrackProxy.m_pBrain;
	Proxy.m_pTarget = m_HeadTrackProxy.m_pTarget.GetObject() ? m_HeadTrackProxy.m_pTarget : TScriptInterface<IDisAttentionTargetInterface>();
	Proxy.m_pCachedTargetActor = m_HeadTrackProxy.m_pCachedTargetActor;
	return Proxy;
}

// DISHONORED(port): 2012 rva 0x79abe0 / 2013 0x73acd0. 2013 tests HasActorReference() where 2012 tested the interface pair; the
// transition empty -> set subscribes to the other-actor-terminated event (3) and tells the subclass, set -> empty unsubscribes.
void UDisAISubProcessHeadTrackBase::SetHeadTrackProxy( const FDisAttentionProxy& _rProxy )
{
	const UBOOL bHadReference = m_HeadTrackProxy.HasActorReference();
	m_HeadTrackProxy = _rProxy;
	const UBOOL bHasReference = m_HeadTrackProxy.HasActorReference();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( !bHadReference && bHasReference )
	{
		FArkGameEventDispatcher::GetInstance()->RegisterToEvent( EventType, this, &UDisAISubProcessHeadTrackBase::OnOtherActorTerminatedEvent );
		OnHeadTrackProxySet();
	}
	else if( bHadReference && !bHasReference )
	{
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcessHeadTrackBase::OnOtherActorTerminatedEvent );
		OnHeadTrackProxyClear();
	}
}

// DISHONORED(port): 2012 rva 0x78f0f0 (2013 0x72b9d0, the method SetHeadTrackProxy registers)
void UDisAISubProcessHeadTrackBase::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_HeadTrackProxy.m_pCachedTargetActor )
	{
		SetHeadTrackProxy( FDisAttentionProxy( EC_EventParm ) );
	}
}

// DISHONORED(port): 2012 rva 0x79bd90 / 2013 0x73e5d0
void UDisAISubProcessHeadTrackBase::BeginSubProcess_Derived()
{
	SetHeadTrackProxy( FDisAttentionProxy( EC_EventParm ) );
}

// DISHONORED(port): 2012 rva 0x79bdc0 / 2013 0x73e5f0
void UDisAISubProcessHeadTrackBase::EndSubProcess_Derived( UBOOL _bIsBeingTerminated )
{
	SetHeadTrackProxy( FDisAttentionProxy( EC_EventParm ) );
}

// DISHONORED(port): 2012 rva 0x79be40 (2013: the UObject vtable slot, unnamed) - the subscription must not outlive the object
void UDisAISubProcessHeadTrackBase::BeginDestroy()
{
	SetHeadTrackProxy( FDisAttentionProxy( EC_EventParm ) );
	UDisAISubProcess::BeginDestroy();
}

// DISHONORED(port): 2012 rva 0x79bdf0 / 2013 0x73e610 - the stim's proxy becomes ours; the stim is not consumed
UBOOL UDisAISubProcessHeadTrackBase::FilterHeadTrackBegin( const FAIStimStruct_HeadTrackBegin& _rHeadTrackBeginStim )
{
	SetHeadTrackProxy( _rHeadTrackBeginStim.m_HeadTrackTargetProxy );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x79be10 (2013 through the delegate thunk 0x7415d0) - a notice or the end of the head track clears it
UBOOL UDisAISubProcessHeadTrackBase::FilterNoticeBegin( const FAIStimStruct_NoticeBegin& _rNoticeBeginStim )
{
	SetHeadTrackProxy( FDisAttentionProxy( EC_EventParm ) );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x78d390 / 2013 0x728a60 - the base mask (NULL on the base class) plus the three ids
const BYTE* UDisAISubProcessHeadTrackBase::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_HeadTrackBegin, EAIStimID_HeadTrackEnd, EAIStimID_NoticeBegin };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x7a04c0 / 2013 0x744d00. Retail binds FilterNoticeBegin to HeadTrackEnd as well (the body never
// reads the stim, so the FAIStimStruct_HeadTrackEnd it receives through the NoticeBegin reference is harmless) - kept as is.
FDisStimPredicateDelegate UDisAISubProcessHeadTrackBase::GetFilterStimDelegate_SubProcess( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_HeadTrackBegin:
		return DIS_BIND_STIM_PREDICATE( UDisAISubProcessHeadTrackBase, FAIStimStruct_HeadTrackBegin, FilterHeadTrackBegin );
	case EAIStimID_HeadTrackEnd:
	case EAIStimID_NoticeBegin:
		return DIS_BIND_STIM_PREDICATE( UDisAISubProcessHeadTrackBase, FAIStimStruct_NoticeBegin, FilterNoticeBegin );
	default:
		return FDisStimPredicateDelegate();
	}
}
