// DishonoredGame/src/disaisubprocessheadtrackbark.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x78d360  public: static void __cdecl UDisTweaks_AISubProcess_HeadTrackBark::InitializePrivateStaticClassUDisTweaks_AISubProcess_HeadTrackBark(void)
//   0x78d380  private: virtual void __thiscall UDisAISubProcessHeadTrackBark::OnHeadTrackProxySet(void)
//   0x797330  public: static class UClass * __cdecl UDisAISubProcessHeadTrackBark::GetPrivateStaticClassUDisAISubProcessHeadTrackBark(wchar_t const *)
//   0x79ab50  private: virtual void __thiscall UDisAISubProcessHeadTrackBark::TickSubProcess_Derived(float)
//   0x79e1c0  public: static void __cdecl UDisAISubProcessHeadTrackBark::InitializePrivateStaticClassUDisAISubProcessHeadTrackBark(void)
//   0x7a0490  public: static class UClass * __cdecl UDisAISubProcessHeadTrackBark::StaticClassNoInline(void)
//   0x7a3500  public: static class UClass * __cdecl UDisTweaks_AISubProcess_HeadTrackBark::GetPrivateStaticClassUDisTweaks_AISubProcess_HeadTrackBark(wchar_t const *)
//   0x7a3e20  public: static class UClass * __cdecl UDisTweaks_AISubProcess_HeadTrackBark::StaticClassNoInline(void)

// ---- agent GE ports (PHASE19 GE): the bark on a head track ----
#include "DishonoredGame.h"

// DISHONORED(bringup): every function of this unit is a virtual, so nothing references the object and MSVC would drop it out
// of the static library. Named from DishonoredGameRegistrants.cpp (the generator collects it).
void DishonoredLinkUDisAISubProcessHeadTrackBark()
{
}

// DISHONORED(port): 2012 rva 0x78d380 / 2013 0x728a50
void UDisAISubProcessHeadTrackBark::OnHeadTrackProxySet()
{
	m_bWantsToBark = TRUE;
}

// DISHONORED(port): 2012 rva 0x79ab50 (99 B) / 2013 0x73ac60 (104 B, vtable slot 98). 2013 fires through
// FDisAttentionProxy::GetProxySpeaker and additionally skips the bark when the target answers TRUE to both of its
// IDisAttentionTargetInterface slots 13 (IsAttnTargetDead, 2012 0x78e740) and 17 (an unnamed predicate folded on a
// return-FALSE body) - this tree's interface stand-in declares neither, so the bark is fired for any target with an actor
// reference, which is the 2012 body.
void UDisAISubProcessHeadTrackBark::TickSubProcess_Derived( const FLOAT _fDeltaTime )
{
	if( !m_bWantsToBark )
	{
		return;
	}
	const FDisAttentionProxy Proxy = GetHeadTrackProxy();
	if( Proxy.HasActorReference() && m_pOwningBehavior )
	{
		m_pOwningBehavior->FireDialogHook( DDH_ATTENTION_HEADTRACK, Proxy.GetProxySpeaker() );
	}
	m_bWantsToBark = FALSE;
}
