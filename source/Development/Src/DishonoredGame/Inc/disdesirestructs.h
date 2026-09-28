#pragma once
// DishonoredGame/inc/disdesirestructs.h
// DISHONORED(written): agent DF. The free functions of Arkane's desire layer plus the bring-up counters that make the
// layer measurable. Retail's unit is disdesirestructs.cpp (the PDB file of every FDis*Request body); the declarations
// that belong to the reflected structs themselves live in Inc/CppText/FDis*Request.h.
#ifndef _INC_DISDESIRESTRUCTS
#define _INC_DISDESIRESTRUCTS

/**
 * DISHONORED(layout): what an Ark component reports back to whoever asked it for something. 2012 PDB enums
 * EArkCpntFaceToEvent and EArkCpntLocoEvent; they belong beside FArkComponentFaceTo / FArkComponentLocomotion in Engine
 * and move there when those are ported (agentCG.md hand-over 3). IDisDesiresInterface::HandleFaceToEvent /
 * HandleLocoEvent (2013 rvas 0x8b7230 / 0x8b7330) switch on them and turn each into an AI stim, which is the entire
 * feedback path from the pawn's movement back into the behaviour stack.
 */
enum EArkCpntFaceToEvent
{
	CPNT_FACETO_EVENT_NONE = 0,
	CPNT_FACETO_EVENT_ORIENTATION_REACHED = 1,
	CPNT_FACETO_EVENT_ABORTED = 2,
};

enum EArkCpntLocoEvent
{
	CPNT_LOCO_EVENT_NONE = 0,
	CPNT_LOCO_EVENT_PATHFINDING_SUCCEED = 1,
	CPNT_LOCO_EVENT_PATHFINDING_FAILED = 2,
	CPNT_LOCO_EVENT_THRESHOLD_REACHED = 3,
	CPNT_LOCO_EVENT_DESTINATION_REACHED = 4,
	CPNT_LOCO_EVENT_ABORTED = 5,
	CPNT_LOCO_EVENT_HIT_OBSTACLE = 6,
};

namespace DisDesireStructs
{
	/** DISHONORED(port): 2013 rva 0x8a8100 (2012 0x8f90f0): an attention proxy resolves to a location only when it is
	    valid and not indeterminate - an NPC that merely suspects something is there cannot walk to it. */
	UBOOL ResolveProxyTarget( const struct FDisAttentionProxy& _rProxyTarget, FVector& _rLocationTarget );

	/**
	 * DISHONORED(bringup): the one place the desire layer touches the Ark component layer. FArkComponentFaceTo,
	 * FArkComponentLocomotion and FArkComponentLookat are not ported (FArkComponentLocomotion is 117 functions and a
	 * package of its own, agentCG.md hand-over 3), so ADishonoredNPCPawn::GetComponent{FaceTo,Locomotion,Lookat} all
	 * answer NULL. Retail reads `Component->m_bStarted` to notice a request the component has already finished; with no
	 * component no request id is ever issued, so m_RequestID stays INDEX_NONE and retail's guard is unreachable.
	 */
	UBOOL IsComponentRequestStarted( FPointer _Component );

	/**
	 * DISHONORED(bringup): called instead of the FArkComponent{FaceTo,Locomotion,Lookat} Start, Update and Stop entry
	 * points. It counts
	 * the order the AI wanted to give and, once per kind, says at the log which retail entry point is missing. This is
	 * what the brief means by "have it request and say what it requested": the desire is fully resolved and filtered,
	 * only the execution is absent. Counters are reported by the -disai census.
	 */
	enum EDisDesireKind
	{
		DDK_FaceTo = 0,
		DDK_Loco = 1,
		DDK_LookAt = 2,
		DDK_MAX = 3,
	};
	void NoteComponentGap( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint );

	/**
	 * DISHONORED(bringup): the stand-in for the FArkComponent{FaceTo,Locomotion,Lookat} Start and Update entry points.
	 * It counts the order, names the missing retail entry point once per kind, and - this is the part that matters -
	 * hands back a request id, because the status machine above it is written around a component that does. Without an
	 * id, FDisDesireRequest::GetRequestStatus answers NewRequestNeeded on every tick: measured at 1,523,652 loco
	 * "requests" from 26 standing NPCs in 119 seconds before this existed.
	 */
	INT AcceptRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint );

	/** The order is withdrawn; counted, and the id is released. */
	void ReleaseRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint );

	/*-------------------------------------------------------------------------
		agent DN: DisLocoSeam - the locomotion kind is no longer synthetic.

		FArkComponentLocomotion exists now (Inc/arkcomponentlocomotion.h), so the three loco calls below reach it instead of
		NoteComponentGap. The face-to and look-at kinds are unchanged and still counted only: FArkComponentFaceTo and
		FArkComponentLookat are not ported.

		These four are free functions rather than calls written inline in FDisLocoRequest because disdesirestructs.cpp is
		where retail's component boundary is, and keeping it in one place is what let agent DF measure what was being asked
		for. The counters stay: GDisDesireRequests[DDK_Loco] still counts orders given, and now every one of them is also a
		real request in the component's queue.
	-------------------------------------------------------------------------*/
	/** Retail's FArkComponent*::m_bStarted test: is this request still in the component's queue? */
	UBOOL IsLocoRequestStarted( FPointer _Component, INT _RequestID );
	/** StartLocoToActor / StartLocoToLocation; answers the component's request id, or INDEX_NONE. */
	INT StartLoco( FPointer _Component, class AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _LocationTarget,
		class UObject* _pAsker, BYTE _Priority, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold,
		FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent,
		UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist );
	/** UpdateLocoToActor / UpdateLocoToLocation on an existing id. */
	UBOOL UpdateLoco( FPointer _Component, INT _RequestID, class AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _LocationTarget,
		class UObject* _pAsker, BYTE _Priority, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold,
		FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent,
		UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist );
	/** StopLoco. */
	void StopLoco( FPointer _Component, INT _RequestID );

	/** The order was changed rather than replaced. */
	void UpdateRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint );

}

/**
 * DISHONORED(port): ToScriptInterface<T> (one instantiation per interface; 2012 rva 0x8fd0a0 is the IDisLookAtInterface
 * one). Retail asks the interface for its UObject and then re-derives the interface address from that object, so a
 * mismatched pair stores neither half and the garbage collector can null the object out on its own. InterfaceCast is the
 * tree's existing spelling of the second step.
 */
template< class InterfaceType >
void DisSetScriptInterface( TScriptInterface< InterfaceType >& _rOut, class UObject* _pObject )
{
	InterfaceType* Interface = _pObject ? InterfaceCast< InterfaceType >( _pObject ) : NULL;
	_rOut.SetObject( Interface ? _pObject : NULL );
	_rOut.SetInterface( Interface );
}

/** The interface half, or NULL. FScriptInterface::GetInterface already answers NULL once the object has been collected. */
template< class InterfaceType >
InterfaceType* DisGetScriptInterface( const TScriptInterface< InterfaceType >& _rInterface )
{
	return (InterfaceType*)_rInterface.GetInterface();
}

/** Per-kind counts of desires that reached the component boundary, and of the orders that were filtered out as
    redundant before they got there. Read by disaicensus.cpp. */
extern INT GDisDesireRequests[DisDesireStructs::DDK_MAX];
extern INT GDisDesireUpdates[DisDesireStructs::DDK_MAX];
extern INT GDisDesireStops[DisDesireStructs::DDK_MAX];
extern INT GDisDesireBodyIntentions;
extern INT GDisDesireSetCalls;

#endif
