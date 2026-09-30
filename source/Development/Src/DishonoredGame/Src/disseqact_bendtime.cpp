// DishonoredGame/src/disseqact_bendtime.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12):
//   0x7cbc50  public: static void __cdecl UDisSeqAct_BendTime::InitializePrivateStaticClassUDisSeqAct_BendTime(void)
//   0x7cbc70  public: static void __cdecl UDisSeqAct_PostProcess::InitializePrivateStaticClassUDisSeqAct_PostProcess(void)
//   0x7cbc90  public: static void __cdecl UDisSeqAct_UberPostProcess::InitializePrivateStaticClassUDisSeqAct_UberPostProcess(void)
//   0x7d3ef0  public: virtual void __thiscall UDisSeqAct_BendTime::Activated(void)
//   0x7d4030  public: virtual void __thiscall UDisSeqAct_PostProcess::Activated(void)
//   0x7d4140  public: virtual void __thiscall UDisSeqAct_UberPostProcess::Activated(void)
//   0x7ecae0  public: static class UClass * __cdecl UDisSeqAct_BendTime::GetPrivateStaticClassUDisSeqAct_BendTime(wchar_t const *)
//   0x7ecb70  public: static class UClass * __cdecl UDisSeqAct_PostProcess::GetPrivateStaticClassUDisSeqAct_PostProcess(wchar_t const *)
//   0x7ecc00  public: static class UClass * __cdecl UDisSeqAct_UberPostProcess::GetPrivateStaticClassUDisSeqAct_UberPostProcess(wchar_t const *)
//   0x7ee930  public: static class UClass * __cdecl UDisSeqAct_BendTime::StaticClassNoInline(void)
//   0x7ee960  public: static class UClass * __cdecl UDisSeqAct_PostProcess::StaticClassNoInline(void)
//   0x7ee990  public: static class UClass * __cdecl UDisSeqAct_UberPostProcess::StaticClassNoInline(void)

// ---- agent FE (PHASE13 package FE) ----
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "arkpp.h"

/**
 * DISHONORED(port): 2013 rva 0x791570 (2012 0x7d4140), 263 bytes. Two input links: the first pushes this
 * action's own uber parameters into the post-process manager's Kismet channel and starts Epp_UberKismet,
 * the second stops it. Both arms sit behind the same gate retail uses - a game info, and the "Blinded"
 * post-process material node resolving - so an action in a level whose post-process graph has not been
 * built does nothing rather than half of something. USequenceAction::Activated runs on every path.
 *
 * The retail tail call is identical-code-folded with UDishonoredTask_Base::OnAdded_Impl, an empty body;
 * USequenceAction::Activated is empty in this tree too, so the two agree.
 */
void UDisSeqAct_UberPostProcess::Activated()
{
	if( DisGetGameInfo() != NULL && DisGetArkPpNodeMaterial( FName(TEXT("Blinded")), TRUE ) != NULL )
	{
		UDisPostProcessManager* PpManager = DisGetPpManager();
		if( PpManager != NULL )
		{
			if( InputLinks.Num() > 0 && InputLinks(0).bHasImpulse )
			{
				PpManager->SetKismetPPParams( m_Parameters, m_fWeight, m_fFadeInTime, m_fFadeOutTime );
				PpManager->StartEffect( Epp_UberKismet, TRUE );
				debugf( TEXT("DISHONORED(bringup): Kismet uber post-process ON from %s: weight %.3f fade %.3f/%.3f"),
					*GetPathName(), m_fWeight, m_fFadeInTime, m_fFadeOutTime );
			}
			else if( InputLinks.Num() > 1 && InputLinks(1).bHasImpulse )
			{
				PpManager->StopEffect( Epp_UberKismet );
				debugf( TEXT("DISHONORED(bringup): Kismet uber post-process OFF from %s"), *GetPathName() );
			}
		}
	}
	USequenceAction::Activated();
}
