// DishonoredGame/src/disseqcond_hasobjective.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb2f0  public: static void __cdecl UDisSeqCond_HasObjective::InitializePrivateStaticClassUDisSeqCond_HasObjective(void)
//   0x7d1b80  public: virtual void __thiscall UDisSeqCond_HasObjective::Activated(void)
//   0x7e9f90  public: static class UClass * __cdecl UDisSeqCond_HasObjective::GetPrivateStaticClassUDisSeqCond_HasObjective(wchar_t const *)
//   0x7edc00  public: static class UClass * __cdecl UDisSeqCond_HasObjective::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x78ec20 (2012 0x7d1b80; 2013 added the m_pObjective test). Output 0 "True", 1 "False".
void UDisSeqCond_HasObjective::Activated()
{
	USequenceOp::Activated();
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	if( m_pObjective && PlayerController && PlayerController->m_pObjectivesComponent && PlayerController->m_pObjectivesComponent->HasObjective( m_pObjective ) )
	{
		OutputLinks(0).bHasImpulse = TRUE;
	}
	else
	{
		OutputLinks(1).bHasImpulse = TRUE;
	}
}

void DishonoredLinkDisSeqCondHasObjective()
{
}
