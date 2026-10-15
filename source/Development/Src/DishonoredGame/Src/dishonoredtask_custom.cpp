// DishonoredGame/src/dishonoredtask_custom.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x72d800  public: class FString __thiscall FWindowsViewport::GetName(void)
//   0x72e7a0  void __cdecl EvaluateOneVar<int>(class FString &, int const &)
//   0x72e870  void __cdecl EvaluateOneVar<float>(class FString &, float const &)
//   0x72e940  void __cdecl EvaluateOneVar<wchar_t const *>(class FString &, wchar_t const * const &)
//   0x72f820  public: virtual void __thiscall UDishonoredTask_Custom::OnDisSeqAct_UpdateCustomTask_Impl(class UDisSeqAct_UpdateCustomTask *)
//   0x738700  public: static class UClass * __cdecl UDishonoredTask_Custom::GetPrivateStaticClassUDishonoredTask_Custom(wchar_t const *)
//   0x739150  public: static void __cdecl UDishonoredTask_Custom::InitializePrivateStaticClassUDishonoredTask_Custom(void)
//   0x73ab00  public: static class UClass * __cdecl UDishonoredTask_Custom::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
// DISHONORED(bringup): OnDisSeqAct_UpdateCustomTask_Impl (2013 0x6d5010, 455 bytes, with the EvaluateOneVar<int|float|const
// TCHAR*> formatters 2012 0x72e7a0..0x72e940) is not ported: no DisSeqAct_UpdateCustomTask is instantiated on the first
// mission's route (GB2.log census), so m_ActiveStatus stays as cooked.
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x6cc2c0 (vtable +300)
FString UDishonoredTask_Custom::GetStatusString_Impl() const
{
	return m_ActiveStatus;
}

void DishonoredLinkDishonoredTaskCustom()
{
}
