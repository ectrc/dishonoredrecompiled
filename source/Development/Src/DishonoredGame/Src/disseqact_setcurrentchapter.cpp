// DishonoredGame/src/disseqact_setcurrentchapter.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb640  public: static void __cdecl UDisSeqAct_SetCurrentChapter::InitializePrivateStaticClassUDisSeqAct_SetCurrentChapter(void)
//   0x7cb660  public: virtual void __thiscall UDisSeqAct_SetCurrentChapter::Activated(void)
//   0x7eadc0  public: static class UClass * __cdecl UDisSeqAct_SetCurrentChapter::GetPrivateStaticClassUDisSeqAct_SetCurrentChapter(wchar_t const *)
//   0x7ee060  public: static class UClass * __cdecl UDisSeqAct_SetCurrentChapter::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): agent HG. The link anchor of a unit whose only function is a virtual (DishonoredGameRegistrants.cpp).
void DishonoredLinkUDisSeqAct_SetCurrentChapter()
{
}

// DISHONORED(port): agent HG. 2013 rva 0x7861e0 (2012 0x7cb660, byte-identical)
void UDisSeqAct_SetCurrentChapter::Activated()
{
	ADishonoredGameInfo* GameInfo = DisGetGameInfo();
	if( GameInfo )
	{
		GameInfo->SetCurrentChapter( m_ChapterTag );
	}
	USequenceAction::Activated();
}
