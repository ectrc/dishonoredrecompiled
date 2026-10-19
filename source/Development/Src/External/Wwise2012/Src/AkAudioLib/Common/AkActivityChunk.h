/*=============================================================================
	AkActivityChunk.h - AkActivityChunk, CAkLimiter, StructMaxInst, CounterParameters (activitychunk.h,
	akparameternodebase.h/.cpp): the per-node play counts and voice limiters the play path increments and the
	renderer kicks against.

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list because every play / stop runs through it.
	Layouts are the 2012 PDB's (AkActivityChunk 0x30 bytes). CAkLimiter's AkSortedKeyArray keeps its PBIs by
	priority descending, then PBIID ascending (kill newest: AkPriorityStruct_INC) or descending (kill oldest:
	AkPriorityStruct_DEC), so the voices to kick are always the tail (2012 0x9435b0 / 0x9436?? BinarySearch).
=============================================================================*/
#pragma once

#include "AkPBI.h"

class CAkLimiter
{
public:
	CAkLimiter( AkUInt16 in_u16LimiterMax, bool in_bDoesKillNewest, bool in_bAllowUseVirtualBehavior )
		: m_u16LimiterMax( in_u16LimiterMax ), m_bDoesKillNewest( in_bDoesKillNewest ), m_bAllowUseVirtualBehavior( in_bAllowUseVirtualBehavior ), pNextLightItem( NULL ) {}

	AKRESULT Add( CAkPBI * in_pPBI, AKVoiceLimiterType in_Type );
	void Remove( CAkPBI * in_pPBI, AKVoiceLimiterType in_Type );
	void Update( AkReal32 in_NewPriority, CAkPBI * in_pPBI );
	void UpdateFlags();
	void Term() { m_sortedPBIList.Term(); }

	AkArray<CAkPBI *, CAkPBI * const &, ArrayPoolDefault, 8>	m_sortedPBIList;
	AkUInt16		m_u16LimiterMax;
	bool			m_bDoesKillNewest;
	bool			m_bAllowUseVirtualBehavior;
	CAkLimiter *	pNextLightItem;

private:
	CAkPBI ** BinarySearch( const AkPriorityStruct & in_key, bool & out_bFound );
};

struct StructMaxInst
{
	StructMaxInst() : m_pLimiter( NULL ), u16Current( 0 ), u16CurrentVirtual( 0 ) {}
	StructMaxInst( AkUInt16 in_u16Current, AkUInt16 in_u16Max, bool in_bDoesKillNewest, bool in_bAllowUseVirtualBehavior );
	void DisableLimiter();

	CAkLimiter *	m_pLimiter;
	AkUInt16		u16Current;
	AkUInt16		u16CurrentVirtual;
};

struct AkActivityChunk
{
	AkActivityChunk( AkUInt16 in_u16MaxNumInstance, bool in_bIsGlobalLimit, bool in_bDoesKillNewest, bool in_bAllowUseVirtualBehavior );
	~AkActivityChunk();

	bool ChunkIsUseless() const;

	CAkKeyArray<CAkRegisteredObj *, StructMaxInst, 5>	m_ListPlayCountPerObj;
	AkListBareLight<CAkPBI>								m_listPBI;
	CAkLimiter											m_Limiter;
	AkUInt16											m_PlayCount;
	AkUInt16											m_uActivityCount;
	AkUInt16											m_iPlayCountValid;
	AkUInt16											m_iVirtualCountValid;
	AkUInt8												m_bIsGlobalLimit : 1;
};

struct CounterParameters
{
	CounterParameters() : fPriority( 0.f ), pGameObj( NULL ), pAMLimiter( NULL ), pBusLimiter( NULL ), uiFlagForwardToBus( 3 ), ui16NumKicked( 0 ), bMaxConsidered( false ), bAllowKick( true ) {}

	AkReal32			fPriority;
	CAkRegisteredObj *	pGameObj;
	CAkLimiter *		pAMLimiter;
	CAkLimiter *		pBusLimiter;
	AkUInt16			uiFlagForwardToBus;
	AkUInt16			ui16NumKicked;
	bool				bMaxConsidered;
	bool				bAllowKick;
};
