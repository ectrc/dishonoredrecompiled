// DishonoredGame/src/disaiknowledgecomponent.cpp
// DISHONORED(port): agent EN (PHASE13 EN). Ark component type 210, DisCpntType_AIKnowledge: the class, its creator
// registration and the two object references its Serialize adds to the base's three values.
//   0x7038c0  public: __thiscall FDisAIKnowledgeComponent::FDisAIKnowledgeComponent(void)
//   0x7039b0  public: virtual void __thiscall FDisAIKnowledgeComponent::ManageReferences(class FArkComponentBase::FGCHelper &)
//   0x7039f0  public: virtual void __thiscall FDisAIKnowledgeComponent::Serialize(class FArchive &)
//   0x700b80  public: virtual unsigned long __thiscall FDisAIKnowledgeComponent::GetMemoryFootprint(void)const
//   0x714820  private: void __thiscall FDisAIKnowledgeComponent::OnAnyPawnShutdown(class FArkGameEvent const &)
//   0x7169e0  public: virtual void __thiscall FDisAIKnowledgeComponent::Starting(void)
//   0x716a80  public: virtual void __thiscall FDisAIKnowledgeComponent::Stopping(void)

#include "DishonoredGame.h"
#include "disaiknowledgecomponent.h"

// DISHONORED(port): retail's registrant - the PDB names it FDisAIKnowledgeComponent::s_TypeRegister and attributes its
// dynamic initializer to this unit (0xba9390 in the 2012 build). Creator: 2013 rva 0x5eb870.
ARKCOMPONENT_IMPLEMENT_TYPE( FDisAIKnowledgeComponent )

// DISHONORED(port): 2013 rva 0x7038c0. Retail zeroes m_pBlackboard and leaves m_pOwningBrain untouched, because
// Starting assigns it before anything reads it.
// DISHONORED(written): m_pOwningBrain is zeroed here as well. A save whose stream stops before this component's second
// reference would otherwise leave a wild pointer in a component the container's GC walk visits.
FDisAIKnowledgeComponent::FDisAIKnowledgeComponent()
	: m_pBlackboard( NULL )
	, m_pOwningBrain( NULL )
{
}

// DISHONORED(port): 2013 rva 0x7039b0. Both references, each written back only when the collector moved it.
void FDisAIKnowledgeComponent::ManageReferences( FGCHelper* _pHelper )
{
	UDisAIBlackboard* pBlackboard = _pHelper->manageReference( m_pBlackboard );
	if( pBlackboard != m_pBlackboard )
	{
		m_pBlackboard = pBlackboard;
	}
	UDishonoredAIBrain* pBrain = _pHelper->manageReference( m_pOwningBrain );
	if( pBrain != m_pOwningBrain )
	{
		m_pOwningBrain = pBrain;
	}
}

/*
	DISHONORED(port): 2013 rva 0x7039f0, 48 bytes, the whole body:

	  007039fb  call FArkComponentBase::Serialize        ; m_pOwner (2-byte dictionary index), m_bStarted, m_bPendingStop
	  00703a02  mov  edx,[eax+18h] / lea ecx,[edi+14h] / call edx    ; FArchive slot 6, operator<<(UObject*&): m_pBlackboard
	  00703a0f  mov  edx,[eax+18h] / add edi,18h        / call edx   ; the same slot:                          m_pOwningBrain

	Fourteen bytes in Dishonored0.sav's brain containers: the base's ten and two 2-byte dictionary indices. The first of
	those two is not only two bytes on the wire, though: DisSaveLoad::FLevelLoader::operator<<(UObject*&) (0x60c930)
	reads the WORD and then, unless bit 0x8000 is set and unless the object has been loaded already, runs the referenced
	object's whole GameLoad inline. m_pBlackboard is a UDisAIBlackboard the save has not loaded yet, so four more bytes
	of UDisAIBlackboard::GameLoad land between the two references - see resources/docs/agents/agentEN.md section 3.
*/
void FDisAIKnowledgeComponent::Serialize( FArchive& _rArchive )
{
	FArkComponentBase::Serialize( _rArchive );
	_rArchive << *(UObject**)&m_pBlackboard;
	_rArchive << *(UObject**)&m_pOwningBrain;
}

// DISHONORED(bringup): 2013 rva 0xd328f4 slot 0 is APawn::MAT_BlendOut, i.e. the linker folded this class's
// PreAsyncWorkTick onto an unrelated empty body - the knowledge component does no per-frame work. The interface
// declares the method pure, so the body has to exist; an empty one is retail's.
void FDisAIKnowledgeComponent::PreAsyncWorkTick( FLOAT _fTimeStep )
{
}

/*
	DISHONORED(bringup): the retail bodies of this class that are NOT ported, and why. Neither reads a stream byte.

	  0x7169e0  Starting: takes m_pOwningBrain from the owning controller's m_pAIBrain, constructs a UDisAIBlackboard
	            with the owner as Outer, calls InitBlackboard and AddToRoot on it, and subscribes OnAnyPawnShutdown to
	            game event 0x12. UDisAIBlackboard::InitBlackboard is itself a bringup no-op here (the blackboard record
	            classes are unported - dishonoredglobalaimanager.cpp says so), and constructing and rooting an object
	            during a restore, on a path no measurement in this package covers, is a risk with no reader behind it.
	  0x716a80  Stopping: the mirror - RemoveFromRoot, Terminate, mark the blackboard pending-kill, NULL it and
	            unsubscribe.
	  0x714820  OnAnyPawnShutdown: drops the pawn's records from the blackboard. Needs the record classes.
*/
