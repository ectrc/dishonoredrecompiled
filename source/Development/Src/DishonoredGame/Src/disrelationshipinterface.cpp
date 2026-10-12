// DishonoredGame/src/disrelationshipinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15), 2012 rvas:
//   0x764c10  public: static void __cdecl UDisRelationshipInterface::InitializePrivateStaticClassUDisRelationshipInterface(void)
//   0x764c30  private: virtual enum ERelationship __thiscall IDisRelationshipInterface::DetermineRelationship_Derived(class IDisRelationshipInterface const &, enum ERelationship)const
//   0x7670c0  public: enum ERelationship __thiscall IDisRelationshipInterface::DetermineRelationship(class IDisRelationshipInterface const &)const
//   0x7670f0  private: enum ERelationship __thiscall IDisRelationshipInterface::DetermineRelationship_Internal(class IDisRelationshipInterface const &)const
//   0x767150  public: void __thiscall IDisRelationshipInterface::OverrideRelationToFaction(class UDisTweaks_Faction const &, enum ERelationship, unsigned int)
//   0x7671a0  public: void __thiscall IDisRelationshipInterface::OverrideRelationToIndividual(class IDisRelationshipInterface const &, enum ERelationship, unsigned int)
//   0x7671f0  public: void __thiscall IDisRelationshipInterface::ClearRelationOverrides(void)
//   0x767220  public: void __thiscall IDisRelationshipInterface::ClearRelationToFactionOverride(class UDisTweaks_Faction const &)
//   0x767260  protected: void __thiscall IDisRelationshipInterface::OnActorTerminated_RelationshipInterface(void)
//   0x76f060  public: static class UClass * __cdecl UDisRelationshipInterface::GetPrivateStaticClassUDisRelationshipInterface(wchar_t const *)
//   0x76f0f0  public: void __thiscall IDisRelationshipInterface::ClearRelationToIndividualOverride(class IDisRelationshipInterface const &)
//   0x772f10  public: static class UClass * __cdecl UDisRelationshipInterface::StaticClassNoInline(void)
//   0x7768b0  public: static class IDisRelationshipInterface * __cdecl IDisRelationshipInterface::FromUObjectOrController(class UObject *)
//   0x776910  protected: void __thiscall IDisRelationshipInterface::OnSetDispositionHelper(class UDisSeqAct_SetDisposition *)
//   0x776c70  protected: void __thiscall IDisRelationshipInterface::OnOtherActorTerminated_RelationshipInterface(class AActor const &)

/*-----------------------------------------------------------------------------
	agent FV (PHASE18 FV): the relationship-override API.

	Every pawn, watch tower, river krust and movable limb is an IDisRelationshipInterface; what it thinks of another
	individual is its faction's opinion unless one of two overrides says otherwise: the individual's own
	FDisRelationshipOverrideInfo (per individual and per faction), or the global faction manager's per-faction table. The
	Kismet action DisSeqAct_SetDisposition writes the first through OnSetDispositionHelper (and, with Reciprocate, the
	recipient's side too); UDisTweaks_Faction::OnSetDisposition writes the second.

	Every write clears the faction manager's relationship caches for the pair and tells the individual its relations
	changed (OnRelationshipChange - the NPC raises its AI notification). The manager is UDisGlobalFactionManager::
	s_pInstance; retail's constructor sets it for the one non-archetype instance (2012 0x8d8a30) and every caller here
	dereferences it unguarded. This tree has the manager as the game info's sub-object but no port of its caches, so the
	pointer is set in ADishonoredGameInfo::InitGlobalManagers and every use is guarded: an empty cache has nothing to clear.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x70f6a0 (2012 0x7768b0, 92 B): the interface of any object that implements it, and the player
// pawn's for the player controller - which is how a designer can name the controller in Kismet.
IDisRelationshipInterface* IDisRelationshipInterface::FromUObjectOrController( UObject* _pObject )
{
	IDisRelationshipInterface* Interface = _pObject ? (IDisRelationshipInterface*)_pObject->GetInterfaceAddress( UDisRelationshipInterface::StaticClass() ) : NULL;
	if( Interface )
	{
		return Interface;
	}
	if( Cast<ADishonoredPlayerController>( _pObject ) && ADishonoredPlayerPawn::s_pInstance )
	{
		return (IDisRelationshipInterface*)ADishonoredPlayerPawn::s_pInstance;
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x7043a0 (2012 0x7670c0): the manager's answer, then the implementor's last word.
BYTE IDisRelationshipInterface::DetermineRelationship( const IDisRelationshipInterface& _rToIndividual ) const
{
	UDisGlobalFactionManager* Manager = UDisGlobalFactionManager::s_pInstance;
	const BYTE Relationship = Manager ? Manager->DetermineRelationship( *this, _rToIndividual ) : DetermineRelationship_Internal( _rToIndividual );
	return DetermineRelationship_Derived( _rToIndividual, Relationship );
}

// DISHONORED(port): 2013 rva 0x7043d0 (2012 0x7670f0, 93 B): no faction means Neutral; an individual or faction override on
// this side wins; otherwise the faction's view of the other individual, which is the manager's (trespass, global
// override, faction table).
BYTE IDisRelationshipInterface::DetermineRelationship_Internal( const IDisRelationshipInterface& _rToIndividual ) const
{
	BYTE Result = Relationship_Neutral;
	UDisTweaks_Faction* Faction = GetFactionTweak();
	if( !Faction )
	{
		return Result;
	}
	FDisRelationshipOverrideInfo* Overrides = const_cast<IDisRelationshipInterface*>( this )->GetRelationshipOverrideInfo();
	if( Overrides && Overrides->GetIndividualOverride( _rToIndividual, Result ) )
	{
		return Result;
	}
	UDisGlobalFactionManager* Manager = UDisGlobalFactionManager::s_pInstance;
	return Manager ? Manager->DetermineRelationship( *Faction, _rToIndividual ) : Relationship_Neutral;
}

// DISHONORED(port): 2013 rva 0x704430 (2012 0x767150)
void IDisRelationshipInterface::OverrideRelationToFaction( const UDisTweaks_Faction& _rToFaction, BYTE _Relationship, UBOOL _bRefreshRelations )
{
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides && Overrides->OverrideFactionRelationship( _rToFaction, _Relationship ) )
	{
		if( UDisGlobalFactionManager::s_pInstance )
		{
			UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this, _rToFaction );
		}
		if( _bRefreshRelations )
		{
			OnRelationshipChange();
		}
	}
}

// DISHONORED(port): 2013 rva 0x704480 (2012 0x7671a0)
void IDisRelationshipInterface::OverrideRelationToIndividual( const IDisRelationshipInterface& _rToIndividual, BYTE _Relationship, UBOOL _bRefreshRelations )
{
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides && Overrides->OverrideIndividualRelationship( _rToIndividual, _Relationship ) )
	{
		if( UDisGlobalFactionManager::s_pInstance )
		{
			UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this, _rToIndividual );
		}
		if( _bRefreshRelations )
		{
			OnRelationshipChange();
		}
	}
}

// DISHONORED(port): 2013 rva 0x7044d0 (2012 0x7671f0)
void IDisRelationshipInterface::ClearRelationOverrides()
{
	if( UDisGlobalFactionManager::s_pInstance )
	{
		UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this );
	}
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides )
	{
		Overrides->ClearAllOverrides();
	}
	OnRelationshipChange();
}

// DISHONORED(port): 2013 rva 0x704500 (2012 0x767220)
void IDisRelationshipInterface::ClearRelationToFactionOverride( const UDisTweaks_Faction& _rToFaction )
{
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides && Overrides->ClearFactionOverride( _rToFaction ) )
	{
		if( UDisGlobalFactionManager::s_pInstance )
		{
			UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this, _rToFaction );
		}
		OnRelationshipChange();
	}
}

// DISHONORED(port): 2013 rva 0x70c4b0 (2012 0x76f0f0)
void IDisRelationshipInterface::ClearRelationToIndividualOverride( const IDisRelationshipInterface& _rToIndividual )
{
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides && Overrides->ClearIndividualOverride( _rToIndividual ) )
	{
		if( UDisGlobalFactionManager::s_pInstance )
		{
			UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this, _rToIndividual );
		}
		OnRelationshipChange();
	}
}

// DISHONORED(port): 2013 rva 0x70f700 (2012 0x776910, 858 B): the whole of DisSeqAct_SetDisposition for an individual target.
// The action's three inputs (m_SetDispositionKismetInput 0 Set, 1 Clear, 2 Clear All) run over its recipients, each of
// which is a faction tweak or anything FromUObjectOrController resolves; with m_bReciprocate the recipient's side is
// written too - a faction's through the global manager, an individual's through its own override table.
void IDisRelationshipInterface::OnSetDispositionHelper( UDisSeqAct_SetDisposition* _pAction )
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
				FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
				if( Overrides && Overrides->OverrideFactionRelationship( *Faction, _pAction->m_Disposition ) )
				{
					if( Manager )
					{
						Manager->ClearIndividualCache( *this, *Faction );
					}
					OnRelationshipChange();
				}
				if( _pAction->m_bReciprocate && Manager )
				{
					Manager->OverrideRelationToIndividual( *Faction, *this, _pAction->m_Disposition );
				}
			}
			else
			{
				IDisRelationshipInterface* Individual = FromUObjectOrController( Recipient );
				if( Individual )
				{
					FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
					if( Overrides && Overrides->OverrideIndividualRelationship( *Individual, _pAction->m_Disposition ) )
					{
						if( Manager )
						{
							Manager->ClearIndividualCache( *this, *Individual );
						}
						OnRelationshipChange();
					}
					if( _pAction->m_bReciprocate )
					{
						FDisRelationshipOverrideInfo* TheirOverrides = Individual->GetRelationshipOverrideInfo();
						if( TheirOverrides && TheirOverrides->OverrideIndividualRelationship( *this, _pAction->m_Disposition ) )
						{
							if( Manager )
							{
								Manager->ClearIndividualCache( *Individual, *this );
							}
							Individual->OnRelationshipChange();
						}
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
				FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
				if( Overrides && Overrides->ClearFactionOverride( *Faction ) )
				{
					if( Manager )
					{
						Manager->ClearIndividualCache( *this, *Faction );
					}
					OnRelationshipChange();
				}
				if( _pAction->m_bReciprocate && Manager )
				{
					Manager->ClearRelationToIndividualOverride( *Faction, *this );
				}
			}
			else
			{
				IDisRelationshipInterface* Individual = FromUObjectOrController( Recipient );
				if( Individual )
				{
					FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
					if( Overrides && Overrides->ClearIndividualOverride( *Individual ) )
					{
						if( Manager )
						{
							Manager->ClearIndividualCache( *this, *Individual );
						}
						OnRelationshipChange();
					}
					if( _pAction->m_bReciprocate )
					{
						Individual->ClearRelationToIndividualOverride( *this );
					}
				}
			}
		}
	}
	else if( _pAction->m_SetDispositionKismetInput == 2 )
	{
		if( Manager )
		{
			Manager->ClearIndividualCache( *this );
		}
		FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
		if( Overrides )
		{
			Overrides->ClearAllOverrides();
		}
		OnRelationshipChange();
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
						Manager->ClearRelationToIndividualOverride( *Faction, *this );
					}
				}
				else
				{
					IDisRelationshipInterface* Individual = FromUObjectOrController( Recipient );
					if( Individual )
					{
						Individual->ClearRelationToIndividualOverride( *this );
					}
				}
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x704540 (2012 0x767260): retail's one guarded use of the manager.
void IDisRelationshipInterface::OnActorTerminated_RelationshipInterface()
{
	// DISHONORED(bringup): UDisGlobalFactionManager::OnIndividualDestroyed (2012 0x8d88d0) drops the individual from the
	// trespasser list and every cache; the caches this tree keeps are the two individual maps, cleared here by the same
	// call the override writers use.
	if( UDisGlobalFactionManager::s_pInstance )
	{
		UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this );
	}
}

// DISHONORED(port): 2013 rva 0x70fa60 (2012 0x776c70): another relationship holder went away; its override on this side goes.
void IDisRelationshipInterface::OnOtherActorTerminated_RelationshipInterface( const AActor& _rOtherActor )
{
	IDisRelationshipInterface* Other = (IDisRelationshipInterface*)const_cast<AActor&>( _rOtherActor ).GetInterfaceAddress( UDisRelationshipInterface::StaticClass() );
	if( !Other )
	{
		return;
	}
	FDisRelationshipOverrideInfo* Overrides = GetRelationshipOverrideInfo();
	if( Overrides && Overrides->ClearIndividualOverride( *Other ) && UDisGlobalFactionManager::s_pInstance )
	{
		UDisGlobalFactionManager::s_pInstance->ClearIndividualCache( *this, *Other );
	}
}
