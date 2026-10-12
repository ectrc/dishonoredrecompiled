// DishonoredGame/src/distweaks_faction.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5), 2012 rvas:
//   0x8b0240  public: static void __cdecl UDisTweaks_Faction::InitializePrivateStaticClassUDisTweaks_Faction(void)
//   0x8c6b10  private: virtual void __thiscall UDisTweaks_Faction::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x8cce20  public: static class UClass * __cdecl UDisTweaks_Faction::GetPrivateStaticClassUDisTweaks_Faction(wchar_t const *)
//   0x8cf5b0  public: static class UClass * __cdecl UDisTweaks_Faction::StaticClassNoInline(void)
//   0x8d8600  public: virtual void __thiscall UDisTweaks_Faction::OnSetDisposition(class UDisSeqAct_SetDisposition *)

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x879000 (2012 0x8d8600, 692 B): DisSeqAct_SetDisposition with a faction as its target writes
// the global faction manager's table: Set overrides this faction's relation to each recipient (faction or individual),
// Clear removes those overrides, Clear All drops this faction's whole row and its caches; Reciprocate writes the other
// side as well - a recipient faction's row, or a recipient individual's own faction override through its interface.
// Every write goes through UDisGlobalFactionManager::s_pInstance, which retail never tests; see the head of
// disrelationshipinterface.cpp for why it is tested here.
void UDisTweaks_Faction::OnSetDisposition( UDisSeqAct_SetDisposition* _pAction )
{
	if( !_pAction )
	{
		return;
	}
	UDisGlobalFactionManager* Manager = UDisGlobalFactionManager::s_pInstance;
	const INT NumRecipients = _pAction->m_Recipients.Num();
	if( _pAction->m_SetDispositionKismetInput == 0 )
	{
		for( INT Idx = 0; Idx < NumRecipients; Idx++ )
		{
			UObject* Recipient = _pAction->m_Recipients(Idx);
			UDisTweaks_Faction* Faction = Cast<UDisTweaks_Faction>( Recipient );
			if( Faction )
			{
				if( Manager )
				{
					Manager->OverrideRelationToFaction( *this, *Faction, _pAction->m_Disposition );
					if( _pAction->m_bReciprocate )
					{
						Manager->OverrideRelationToFaction( *Faction, *this, _pAction->m_Disposition );
					}
				}
			}
			else
			{
				IDisRelationshipInterface* Individual = IDisRelationshipInterface::FromUObjectOrController( Recipient );
				if( Individual )
				{
					if( Manager )
					{
						Manager->OverrideRelationToIndividual( *this, *Individual, _pAction->m_Disposition );
					}
					if( _pAction->m_bReciprocate )
					{
						Individual->OverrideRelationToFaction( *this, _pAction->m_Disposition, TRUE );
					}
				}
			}
		}
	}
	else if( _pAction->m_SetDispositionKismetInput == 1 )
	{
		for( INT Idx = 0; Idx < NumRecipients; Idx++ )
		{
			UObject* Recipient = _pAction->m_Recipients(Idx);
			UDisTweaks_Faction* Faction = Cast<UDisTweaks_Faction>( Recipient );
			if( Faction )
			{
				if( Manager )
				{
					Manager->ClearRelationToFactionOverride( *this, *Faction );
					if( _pAction->m_bReciprocate )
					{
						Manager->ClearRelationToFactionOverride( *Faction, *this );
					}
				}
			}
			else
			{
				IDisRelationshipInterface* Individual = IDisRelationshipInterface::FromUObjectOrController( Recipient );
				if( Individual )
				{
					if( Manager )
					{
						Manager->ClearRelationToIndividualOverride( *this, *Individual );
					}
					if( _pAction->m_bReciprocate )
					{
						Individual->ClearRelationToFactionOverride( *this );
					}
				}
			}
		}
	}
	else if( _pAction->m_SetDispositionKismetInput == 2 )
	{
		if( Manager )
		{
			if( Manager->m_GlobalRelationships.Remove( this ) > 0 )
			{
				Manager->ClearFactionCache( *this );
			}
		}
		if( _pAction->m_bReciprocate )
		{
			for( INT Idx = 0; Idx < NumRecipients; Idx++ )
			{
				UObject* Recipient = _pAction->m_Recipients(Idx);
				UDisTweaks_Faction* Faction = Cast<UDisTweaks_Faction>( Recipient );
				if( Faction )
				{
					if( Manager )
					{
						Manager->ClearRelationToFactionOverride( *Faction, *this );
					}
				}
				else
				{
					IDisRelationshipInterface* Individual = IDisRelationshipInterface::FromUObjectOrController( Recipient );
					if( Individual )
					{
						Individual->ClearRelationToFactionOverride( *this );
					}
				}
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x5ec020 (2012 0x632440), the generated exec wrapper: one object, then the virtual.
void UDisTweaks_Faction::execOnSetDisposition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDisSeqAct_SetDisposition, _pAction);
	P_FINISH;
	OnSetDisposition( _pAction );
}
