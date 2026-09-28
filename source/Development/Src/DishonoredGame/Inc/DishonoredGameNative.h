// DishonoredGameNative.h - hand-written native types of the retail DishonoredGame module that the generated class bodies
// (Inc/CppText/*.h) refer to. Included by the generated DishonoredGame.h before the class headers (generator hook, agent AJ).
// DISHONORED(written): 2012 PDB types FDisNativeStateParam (8 bytes, vtable OnPending/SizeOf + m_pStateClass),
// FSpawnActor_TweakObj (8 bytes: FSpawnActorInitFunctor vtable + m_pTweakObj), FSpawnNPCPawn_TweakObj (+ m_pSpawner).
#ifndef _INC_DISHONOREDGAMENATIVE
#define _INC_DISHONOREDGAMENATIVE

// DISHONORED(retail): UProperty::PropertyFlags bit 0x0000400000000000 marks the tweak properties the fallback chain copies
// (UDisTweaksBase::SkipFallback 2013 rva 0x87f3e0 and ApplyFallbackChain_Struct test it); the reference headers have no name for it
#define CPF_DisTweakFallback DECLARE_UINT64(0x0000400000000000)

// DISHONORED(written): agent CG. The AI cpptext bodies (UDishonoredAIBehavior, UDisAISubState, UDisAISubProcess,
// UDisAIBrainProcess) declare stim-delegate accessors, so the delegate template and the types its instantiations name
// have to be complete before the generated class headers. disdelegate.h is header-only and pulls in nothing.
#include "disdelegate.h"
struct FAIStimStruct;
struct FDisAttentionProxyInfo;
struct FDisAttentionChangeReason;
struct FDisBodyIntention;
struct FDisStimRef;
class IDisAttentionTargetInterface;
class IDisConvSpeakerInterface;
class IDisDesiresInterface;
class IDisRelationshipInterface;
class FDisAIKnowledgeComponent;
class FArkComponentLookat;
// DISHONORED(written): agent DF. The desire layer's request structs name these two in their signatures; both are Engine
// classes of the unported FArkComponentLocomotion package (agentCG.md hand-over 3).
class FArkComponentFaceTo;
class FArkComponentLocomotion;

class UDishonoredNativeState;
class UDisTweaksBase;
class ADishonoredSpawner;
struct FDisRelationshipOverrideInfo;
struct FAnimPlayerNotificationParams;
// DISHONORED(written): agent AV, the anim-state node set. UDisAnimStatePool's cpptext names these before
// DishonoredGameAnimClasses.h defines them.
class UDishonoredAnimTree;
class UDishonoredAnimNodeTreeRef_Dynamic;
struct FDynamicTreeTemplate;

// DISHONORED(written): the state-change request carried through UDishonoredNativeStateMachine::RequestStateChange /
// DemandStateChange (2013 rvas 0x674fa0 / 0x672190); InitFSM (0x67ba50) copies SizeOf() bytes of it into m_DefaultStateParam
struct FDisNativeStateParam
{
	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject ) {}
	virtual INT SizeOf() const { return sizeof(FDisNativeStateParam); }
	UClass* m_pStateClass;

	FDisNativeStateParam( UClass* StateClass = NULL ) : m_pStateClass( StateClass ) {}
};

#ifndef DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR
// DISHONORED(bringup): retail UWorld::SpawnActor takes a trailing FSpawnActorInitFunctor* and calls DoInit(Actor) before
// PostBeginPlay (agent AI's package). Until Engine declares it, the base lives here; define DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR
// in Engine when it does.
struct FSpawnActorInitFunctor
{
	virtual void DoInit( AActor* Actor ) = 0;
};
#endif

// DISHONORED(written): 2013 rva 0x885650 (2012 0x8ec500): applies the tweak object to the spawned actor's IDisTweaksInterface
struct FSpawnActor_TweakObj : public FSpawnActorInitFunctor
{
	UDisTweaksBase* m_pTweakObj;

	FSpawnActor_TweakObj( UDisTweaksBase* TweakObj ) : m_pTweakObj( TweakObj ) {}
	virtual void DoInit( AActor* Actor );
};

// DISHONORED(written): 2012 PDB FSpawnNPCPawn_TweakObj (12 bytes); DoInit 2012 rva 0x8ec550 also hands the spawner to the NPC pawn
struct FSpawnNPCPawn_TweakObj : public FSpawnActor_TweakObj
{
	ADishonoredSpawner* m_pSpawner;

	FSpawnNPCPawn_TweakObj( UDisTweaksBase* TweakObj, ADishonoredSpawner* Spawner ) : FSpawnActor_TweakObj( TweakObj ), m_pSpawner( Spawner ) {}
	virtual void DoInit( AActor* Actor );
};

// DISHONORED(written): FDisRelationshipOverrideInfo::Serialize 2013 rva 0x85e3d0 (2012 0x89f900, disglobalenums.cpp:29): the
// faction-override keys are handed to object reference collectors; the struct itself is generated, so the body is a free function
void DisSerializeRelationshipOverrideInfo( FDisRelationshipOverrideInfo& Info, FArchive& Ar );

#endif // _INC_DISHONOREDGAMENATIVE
