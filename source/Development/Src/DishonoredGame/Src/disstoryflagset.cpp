// DishonoredGame/src/disstoryflagset.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x849b50  public: static void __cdecl UDisStoryFlagSet::InitializePrivateStaticClassUDisStoryFlagSet(void)
//   0x84dad0  public: void __thiscall FDisStoryFlagInstance::BuildStoryFlagInstance(class FString const &, class FGuid const &)
//   0x84db10  public: unsigned int __thiscall FDisStoryFlagInstance::MatchesStoryFlagInstance(class FString const &, class FGuid const &)const
//   0x84db60  public: unsigned int __thiscall FDisStoryFlagInstance::CheckStoryFlagValue(class FString const &, class FGuid const &)const
//   0x851c50  public: struct FDisStoryFlag const * __thiscall UDisStoryFlagSet::FindStoryFlag_ByGUID(class FGuid const &)const
//   0x851ce0  protected: void __thiscall UDisStoryFlagSet::RebuildGUIDs(void)
//   0x8559e0  public: virtual void __thiscall UDisStoryFlagSet::PostDuplicate(void)
//   0x85a4f0  public: static class UClass * __cdecl UDisStoryFlagSet::GetPrivateStaticClassUDisStoryFlagSet(wchar_t const *)
//   0x85b730  public: static class UClass * __cdecl UDisStoryFlagSet::StaticClassNoInline(void)

// ---- agent EL (PHASE12 EL): the lookup and the three FDisStoryFlagInstance helpers ----
#include "DishonoredGame.h"

// DISHONORED(port): agent EL, 2013 rva 0x8049f0 (2012 0x851c50). The set declares flags; only the GUID is
// matched, and the returned pointer is into m_StoryFlags, so it is only valid while the set is.
const FDisStoryFlag* UDisStoryFlagSet::FindStoryFlag_ByGUID( const FGuid& _rGUID ) const
{
	for( INT FlagIndex = 0; FlagIndex < m_StoryFlags.Num(); FlagIndex++ )
	{
		const FDisStoryFlag* pFlag = &m_StoryFlags(FlagIndex);
		if( pFlag->m_GUID == _rGUID )
		{
			return pFlag;
		}
	}
	return NULL;
}

// DISHONORED(port): agent EL, 2013 rva 0x7febc0 (2012 0x84dad0). Retail 2013 keys the instance by the set's
// path as an FName; the 2012 PDB signature is a FString, which is the one layout change in this struct.
void FDisStoryFlagInstance::BuildStoryFlagInstance( const FName& _rStoryFlagSetPath, const FGuid& _rGUID )
{
	m_StoryFlagSet_Path = _rStoryFlagSetPath;
	m_GUID = _rGUID;
}

// DISHONORED(port): agent EL, 2013 rva 0x801ab0 (2012 0x84db10)
UBOOL FDisStoryFlagInstance::MatchesStoryFlagInstance( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const
{
	return m_GUID == _rGUID && m_StoryFlagSet_Path == _rStoryFlagSetPath;
}

// DISHONORED(port): agent EL, 2013 rva 0x801b00 (2012 0x84db60). Retail inlines the match rather than calling
// MatchesStoryFlagInstance; the two tests are byte-for-byte the same comparison plus m_bCurValue.
UBOOL FDisStoryFlagInstance::CheckStoryFlagValue( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const
{
	return MatchesStoryFlagInstance( _rStoryFlagSetPath, _rGUID ) && m_bCurValue;
}
