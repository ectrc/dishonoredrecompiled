#pragma once
// DishonoredGame/inc/disaiknowledgecomponent.h
// DISHONORED(port): agent EN (PHASE13 EN). Ark component type 210 (DisCpntType_AIKnowledge) - the brain's blackboard
// holder, and the second component of every AI brain's container in Dishonored0.sav.
//   0x7038f0  public: virtual unsigned int __thiscall FDisAIKnowledgeComponent::IsOfType(int)const
//   0x703910  public: virtual int __thiscall FDisAIKnowledgeComponent::GetType(void)const
//   0x703920  public: static class FName __cdecl FDisAIKnowledgeComponent::GetInternalName(void)
//   0x703990  public: virtual class FName __thiscall FDisAIKnowledgeComponent::GetName(void)const
// Not ported here: GetMatchingKnowledge<N,Record> / UpdateKnowledge<N,Record>, the two templates every knowledge
// lookup goes through (2013 0x623c70, 0x736d80, 0x6f8600). They need the FAIKnowledgeRecord_* classes, which
// disaiblackboardrecords_descriptions.cpp does not have.

#include "arkcomponentlocomotion.h"	// IArkComponentPreAsyncWorkJustBeforeProceduralAnim

/**
 * DISHONORED(layout): retail sizeof 28 (2013 GetMemoryFootprint 0x700b80 is GetAllocatedSize() + 0x1C, and the
 * constructor 0x7038c0 writes the second vptr at +0x10 and zeroes +0x14). FArkComponentBase is +0x00..+0x0F, the
 * IArkComponentPreAsyncWorkJustBeforeProceduralAnim vptr +0x10, m_pBlackboard +0x14, m_pOwningBrain +0x18.
 */
class FDisAIKnowledgeComponent : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( DisCpntType_AIKnowledge, "DisCpntType_AIKnowledge" )

public:
	FDisAIKnowledgeComponent();

	// ---- FArkComponentBase ----
	virtual void ManageReferences( FGCHelper* _pHelper );			// 2013 0x7039b0
	virtual void Serialize( FArchive& _rArchive );					// 2013 0x7039f0
	virtual DWORD GetMemoryFootprint() const { return GetAllocatedSize() + sizeof( FDisAIKnowledgeComponent ); }	// 2013 0x700b80

	// ---- IArkComponentPreAsyncWorkJustBeforeProceduralAnim ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	class UDisAIBlackboard* GetBlackboard() const { return m_pBlackboard; }

	/** @0x14. Retail's Starting (0x7169e0) constructs it with the owning actor as Outer and roots it. */
	class UDisAIBlackboard*		m_pBlackboard;
	/** @0x18. Retail's Starting takes it from the owning controller's m_pAIBrain (ADishonoredNPCController @896). */
	class UDishonoredAIBrain*	m_pOwningBrain;
};
