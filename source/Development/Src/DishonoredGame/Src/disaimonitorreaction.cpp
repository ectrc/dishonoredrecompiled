// DishonoredGame/src/disaimonitorreaction.cpp
// DISHONORED(port): agent EN (PHASE13 EN). Ark component type 211, DisCpntType_AIMonitorReaction: the class, its
// creator registration and the eleven bytes plus four its Serialize reads. Retail's unit is this one (12 functions).
//   0x728af0  private: void __thiscall FDisAIMonitorReaction::DeleteLogicForAllResponseID(void)
//   0x728b30  protected: virtual unsigned long __thiscall FDisAIMonitorReaction::GetAllocatedSize(void)const
//   0x728b70  public: virtual void __thiscall FDisAIMonitorReaction::PreAsyncWorkTick(float)
//   0x72bc90  public: __thiscall FDisAIMonitorReaction::FDisAIMonitorReaction(void)
//   0x72bdb0  public: virtual void __thiscall FDisAIMonitorReaction::Starting(void)
//   0x72bde0  public: virtual void __thiscall FDisAIMonitorReaction::Stopping(void)
//   0x72be30  public: virtual void __thiscall FDisAIMonitorReaction::ManageReferences(class FArkComponentBase::FGCHelper &)
//   0x72fa40  private: void __thiscall FDisAIMonitorReaction::CreateLogicForResponseID(unsigned char)
//   0x735040  public: void __thiscall FDisAIMonitorReaction::Init(void)
//   0x73b130  public: unsigned int __thiscall FDisAIMonitorReaction::DoFilterStim(struct FAIStimStruct const &)
//   0x73b1a0  public: virtual void __thiscall FDisAIMonitorReaction::Serialize(class FArchive &)
//   0x73eab0  private: void __thiscall FDisAIMonitorReaction::CoS_OnTransgression(struct FResponseLogic_Base *, unsigned int, class FVector)
//             (unnamed in the 2013 list as sub_B3EAB0; matched from 2012 0x79c150 by match_2012_2013.csv, ratio
//              0.684 "callee". 2013's own 0x79c150 is UDisSeqAct_BodyShadowKill::Activated, a different function -
//              this line said 0x79c150 until build/agentEN/banner_check.txt resolved it by name.)

#include "DishonoredGame.h"
#include "disaimonitorreaction.h"

// DISHONORED(port): retail's own registrant - the PDB names it FDisAIMonitorReaction::s_TypeRegister and attributes its
// dynamic initializer to this unit (0xba9430 in the 2012 build). The creator behind it is
// FArkComponentCreatorRegister::CreatorFn<FDisAIMonitorReaction>, 2013 rva 0x5eb950.
ARKCOMPONENT_IMPLEMENT_TYPE( FDisAIMonitorReaction )

// DISHONORED(port): 2013 rva 0x72bc90. The base's three members, then the second vptr, then every DWORD of the
// component's own span zeroed: m_pOwningBrain and the ten response-logic pointers.
FDisAIMonitorReaction::FDisAIMonitorReaction()
	: m_pOwningBrain( NULL )
{
	appMemzero( m_pResponseLogics, sizeof( m_pResponseLogics ) );
}

// DISHONORED(port): 2013 rva 0x72be30. One reference, and it is written back only when the collector moved it.
void FDisAIMonitorReaction::ManageReferences( FGCHelper* _pHelper )
{
	UObject* pBrain = (UObject*)m_pOwningBrain;
	UObject* pManaged = _pHelper->manageReference( pBrain );
	if( pManaged != pBrain )
	{
		m_pOwningBrain = (UDishonoredAIBrain*)pManaged;
	}
}

/*
	DISHONORED(port): 2013 rva 0x73b1a0, 109 bytes, the whole body:

	  0073b1ad  call FArkComponentBase::Serialize             ; m_pOwner (a 2-byte dictionary index), m_bStarted, m_bPendingStop
	  0073b1b6  lea  edi,[esi+10h]                            ; (IArkComponentPreAsyncWorkJustBeforeProceduralAnim*)this,
	                                                          ;   with retail's null-preserving upcast
	  0073b1bd  mov  esi,[GWorld+2C8h]                        ; UWorld::m_pComponentManager (UnWorld.h @712)
	  0073b1cd  call 0x811820                                 ; FArkComponentManager::GetPolicy<...>() - IDA labels this
	                                                          ;   FStaticLightingVertexMapping::GetVertexMapping, an ICF fold
	  0073b1d9  lea  ecx,[eax+4] / call TArray::ContainsItem   ; is this component on the policy's list?
	  0073b1e9  call FArchive::ByteOrderSerialize( ,4 )        ; that answer, four bytes
	  0073b1f2  jz   loc_B3B204                               ; if it came back FALSE, done
	  0073b1f6  call 0x811820 / call [vtable+0]               ; FArkComponentPolicy::Register (slot 0), i.e. re-register
	  0073b204  retn 4

	Fourteen bytes on the wire: the base's ten (2 + 4 + 4) and this one INT. None of the component's own members is
	serialised - not m_pOwningBrain and not the ten response logics; retail rebuilds both from the tweaks in Init.
*/
void FDisAIMonitorReaction::Serialize( FArchive& _rArchive )
{
	FArkComponentBase::Serialize( _rArchive );

	// DISHONORED(bringup): FArkComponentManager is not ported (Engine/Src/arkcomponentmanager.cpp is a skeleton and
	// UWorld::m_pComponentManager is never assigned), so there is no policy list to ask and none to register with.
	// The four bytes are read either way, which is what keeps the object stream in step; a restored component that
	// retail would have put back on the JustBeforeProceduralAnim list is simply not on one here, and nothing in this
	// tree ticks that list yet.
	UBOOL bWasRegisteredWithPolicy = FALSE;
	_rArchive << bWasRegisteredWithPolicy;
}

// DISHONORED(bringup): 2013 rva 0x728b70 ticks all ten response logics -
// `m_pResponseLogics[i]->Tick( m_pOwningBrain, _fTimeStep )` through the logic's own vtable slot 0. FResponseLogic_Base
// is a forward declaration here (disaimonitorreaction_responses.cpp is a comment-only skeleton), so the ten slots are
// always NULL and retail's loop would do nothing; the loop itself is left out rather than written against an incomplete
// type. The interface declares it pure, so the body has to exist.
void FDisAIMonitorReaction::PreAsyncWorkTick( FLOAT _fTimeStep )
{
}

/*
	DISHONORED(bringup): the five retail bodies of this class that are NOT ported, and why. None of them reads a stream
	byte, so the object stream does not depend on any of them.

	  0x735040  Init: walks response ids 0..9, asks UDisTweaks_AIBrain_Reaction::FetchTweakAIResponseAt whether the row
	            is enabled and calls CreateLogicForResponseID for each that is. Needs the response-logic classes.
	  0x72fa40  CreateLogicForResponseID: a switch over the ten ids, each appMalloc-ing one FResponseLogic_* and storing
	            it in its slot. Needs the fourteen response-logic classes.
	  0x728af0  DeleteLogicForAllResponseID: operator delete over the ten slots, then NULLs them. Needs the type to be
	            complete. Every slot is NULL in this tree, so Stopping's call would be a no-op.
	  0x728b30  GetAllocatedSize: the base's answer plus each logic's own (its vtable slot 3). The base returns 0 and
	            every slot is NULL, so FArkComponentBase::GetAllocatedSize gives the same number here.
	  0x72bdb0 / 0x72bde0  Starting / Stopping: register with and unregister from the component manager's
	            JustBeforeProceduralAnim policy (slots 0 and 1 of FArkComponentPolicy), and Stopping then calls
	            DeleteLogicForAllResponseID and NULLs m_pOwningBrain. There is no component manager here.
	  0x73b130  DoFilterStim / 0x79c150 CoS_OnTransgression: the monitor's actual work, both on the response logics.
*/
