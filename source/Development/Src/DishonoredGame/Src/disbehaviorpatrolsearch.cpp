// DishonoredGame/src/disbehaviorpatrolsearch.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13), 2012 rvas of the five ported here:
//   0x72bf00  private: unsigned int __thiscall UDisBehaviorPatrolSearch::EvaluatePatrolSearchRequest(struct FAIStimStruct_PatrolSearchRequest const &)const
//   0x72bf40  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorPatrolSearch::GetSetupFromStimDelegate(enum EAIStimID)
//   0x730e30  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrolSearch::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x742e60  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrolSearch::GetFilterStimDelegate(enum EAIStimID)
//   0x72e300  protected: virtual void __thiscall UDisBehaviorPatrolSearch::InitBehavior(class UDishonoredAIBrain * const)

// ---- agent EP ports (PHASE14 EP): the five lines that keep this subclass off its base class's stim ----
//
// See Inc/CppText/UDisBehaviorPatrolSearch.h for why, and for what is deliberately left out.

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"

// DISHONORED(port): the two masks. Retail's are ICF-folded onto other classes' - a mask of exactly one stim id is the
// same 0x100 bytes whichever class built it - so the addresses are not recoverable; the CONTENT is fixed by the three
// delegate getters, each of which tests id 72 and nothing else.
const BYTE* UDisBehaviorPatrolSearch::BuildEvaluateStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_PatrolSearchRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

const BYTE* UDisBehaviorPatrolSearch::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_PatrolSearchRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2013 rva 0x6ef220 (2012 0x730e30)
FDisStimPredicateDelegate UDisBehaviorPatrolSearch::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolSearchRequest )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorPatrolSearch, FAIStimStruct_PatrolSearchRequest, EvaluatePatrolSearchRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6fd5e0 (2012 0x742e60)
FDisStimPredicateDelegate UDisBehaviorPatrolSearch::GetFilterStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolSearchRequest )
	{
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorPatrolSearch, FAIStimStruct_PatrolSearchRequest, FilterPatrolSearchRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6e9ae0 (2012 0x72bf40)
FDisStimSetupDelegate UDisBehaviorPatrolSearch::GetSetupFromStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolSearchRequest )
	{
		return DIS_BIND_STIM_SETUP( UDisBehaviorPatrolSearch, FAIStimStruct_PatrolSearchRequest, SetupFromPatrolSearchRequest );
	}
	return FDisStimSetupDelegate();
}

// DISHONORED(port): 2013 rva 0x6e99b0 (2012 0x72bf00): the same test as the base class's - is there any route at all -
// which is why retail's two bodies are one function after ICF.
UBOOL UDisBehaviorPatrolSearch::EvaluatePatrolSearchRequest( const FAIStimStruct_PatrolSearchRequest& _rStim ) const
{
	UDishonoredMapInfo* MapInfo = GWorld && GWorld->GetWorldInfo() ? Cast<UDishonoredMapInfo>( GWorld->GetWorldInfo()->GetMapInfo() ) : NULL;
	UDisPatrolManager* PatrolManager = MapInfo ? MapInfo->m_pPatrolManager : NULL;
	return PatrolManager ? PatrolManager->HasPatrolRoutes() : FALSE;
}

// DISHONORED(port): folded onto UDisBehaviorPatrol::FilterPatrolRequest (2013 rva 0x6f3db0) and identical to it: both
// stim structs carry m_pStartingActor at the same offset and both write the same inherited member.
UBOOL UDisBehaviorPatrolSearch::FilterPatrolSearchRequest( const FAIStimStruct_PatrolSearchRequest& _rStim )
{
	m_pStartingActor = _rStim.m_pStartingActor;
	ResetPatrol();
	return TRUE;
}

// DISHONORED(port): folded onto UDisBehaviorPatrol::SetupFromPatrolRequest (2013 rva 0x6e3fa0).
void UDisBehaviorPatrolSearch::SetupFromPatrolSearchRequest( const FAIStimStruct_PatrolSearchRequest& _rStim )
{
	m_pStartingActor = _rStim.m_pStartingActor;
}
