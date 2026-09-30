#pragma once
// DishonoredGame/inc/disaimonitorreaction.h
// DISHONORED(port): agent EN (PHASE13 EN). The reaction monitor, Ark component type 211
// (DisCpntType_AIMonitorReaction) - the first component of every AI brain's container in Dishonored0.sav and the
// component the object stream stopped on at byte 43,133.
//   0x72bce0  public: virtual unsigned int __thiscall FDisAIMonitorReaction::IsOfType(int)const
//   0x72bd00  public: virtual int __thiscall FDisAIMonitorReaction::GetType(void)const
//   0x72bd10  public: static class FName __cdecl FDisAIMonitorReaction::GetInternalName(void)
//   0x72bd80  public: virtual class FName __thiscall FDisAIMonitorReaction::GetName(void)const

#include "arkcomponentlocomotion.h"	// IArkComponentPreAsyncWorkJustBeforeProceduralAnim

/**
 * DISHONORED(port): the per-response behaviour objects the monitor owns, one per response id. Retail's classes are
 * FResponseLogic_Base and its fourteen derivatives (disaimonitorreaction_responses.cpp, a comment-only skeleton in this
 * tree), so this is a forward declaration and the ten slots stay NULL - see disaimonitorreaction.cpp.
 */
class FResponseLogic_Base;

/**
 * DISHONORED(layout): retail sizeof 64 (2013 GetMemoryFootprint 0x728b70 is GetAllocatedSize() + 0x40, and the
 * constructor 0x72bc90 zeroes every DWORD from +0x14 to +0x3C). FArkComponentBase is +0x00..+0x0F, the
 * IArkComponentPreAsyncWorkJustBeforeProceduralAnim vptr is +0x10, m_pOwningBrain +0x14 and the ten response-logic
 * pointers +0x18..+0x3C.
 */
class FDisAIMonitorReaction : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( DisCpntType_AIMonitorReaction, "DisCpntType_AIMonitorReaction" )

public:
	/**
	 * DISHONORED(layout): ten response ids, and the number is retail's three times over: FDisAIMonitorReaction::Init
	 * (2013 0x735040) loops `while( id < 0xA )`, DeleteLogicForAllResponseID (0x728af0) and GetAllocatedSize
	 * (0x728b30) both loop ten pointers from +0x18, and CreateLogicForResponseID (0x72fa40) is a switch over the
	 * ten cases 0..9 storing into this+6 .. this+15.
	 */
	enum { RESPONSE_ID_COUNT = 10 };

	FDisAIMonitorReaction();

	// ---- FArkComponentBase ----
	virtual void ManageReferences( FGCHelper* _pHelper );			// 2013 0x72be30
	virtual void Serialize( FArchive& _rArchive );					// 2013 0x73b1a0
	/** 2013 rva 0x728b20, which IDA names FDisAIMonitorPawnReachability::GetMemoryFootprint because the linker folds
	    the two: both classes are 64 bytes and both bodies are `GetAllocatedSize() + 0x40`, i.e. slot 9 plus sizeof. */
	virtual DWORD GetMemoryFootprint() const { return GetAllocatedSize() + sizeof( FDisAIMonitorReaction ); }

	// ---- IArkComponentPreAsyncWorkJustBeforeProceduralAnim ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );				// 2013 0x728b70, not ported - see the .cpp
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	/** @0x14. Retail's Init (0x735040) takes it from the owning controller's m_pAIBrain (ADishonoredNPCController @896). */
	class UDishonoredAIBrain*	m_pOwningBrain;
	/** @0x18..@0x3C, indexed by response id. */
	FResponseLogic_Base*		m_pResponseLogics[ RESPONSE_ID_COUNT ];
};
