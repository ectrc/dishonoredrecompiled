// DishonoredGame/Inc/disattentioninfo.h
// DISHONORED(written): agent GI (PHASE20 GI). The native (unreflected) types of the attention brain process, read off the
// 2012 PDB (resources/docs/types/all_types.h): FDisTickAttentionParams (100 bytes), FAttentionTweakCategoryInfo (12),
// DisAttention::FDisAttentionChangeMessage (44), and the two enums the PDB names only through their users
// (UDisAttentionInfo_Base::EDisWitnessDeathResult, EDisTopAttnTargetPriorityType). Shared by disaibrainprocessattention.cpp
// and the three disattentioninfo_*.cpp units; the brain and the pawns see only the reflected classes.
#pragma once

#define DIS_MEMSTACK_ARRAY( T ) TArray< T, TMemStackAllocator< GMainThreadMemStack > >

struct FAttentionTweakCategoryInfo
{
	class UDisTweaks_PawnAttention* m_pComplexAttnTweaks;
	FLOAT* m_pComplexAttnStopThresholdArray;
	class UDisTweaks_SimpleAttention* m_pSimpleAttnTweaks;
};

// The one tick's view of a target, built by DisAttentionInfo_Complex::SetupTickParamsFromTarget (2012 PDB struct 41036).
struct FDisTickAttentionParams
{
	FVector m_OwnerLocation;
	INT m_LatestAttentionTagForPlayer;
	const struct FDisTickAttentionParams_PawnAttentionTweaks* m_pPawnAttentionTweaks;
	const struct FDisTickAttentionParams_AttentionTargetTweaks* m_pAttentionTargetTweaks;
	class AActor* m_pAttnTargetActor;
	UBOOL m_bAttnTargetAllowsAttentionGain;
	UBOOL m_bAttnTargetHasObservableComponent;
	FLOAT m_fAttnTargetVisibilityLevel;
	FLOAT m_fAttnTargetSpottedDistance;
	FVector m_AttnTargetLocation;
	FVector m_AttnTargetVelocity;
	FVector m_AttnTargetFocalPoint;
	FLOAT m_fAttnTargetHeight;
	UBOOL m_bShouldMagicDrainAttn;
	UBOOL m_bBlinkInProgress;
	UBOOL m_bCanLoseBustedTarget;
	BYTE m_SenseFlags;
};

// The brain's final sense mask bits (UDishonoredAIBrain::m_FinalSenseMask_Player / _NonPlayer): 1 blind, 2 deaf, 4 ... 8 numb.
// FDisTickAttentionParams::m_SenseFlags folds them as SetupTickParamsFromTarget does: 1 blind, 2 (mask & 3), 4 (mask & 5), 8 (mask & 9).
enum EDisTickAttentionSenseFlags
{
	DTASF_Blind				= 1,
	DTASF_Deaf				= 2,
	DTASF_BlindOrNumb		= 4,
	DTASF_BlindOrNumbTouch	= 8,
};

namespace DisAttention
{
	// What one attention info tells the process about a level change this tick (2012 PDB struct 23520, 44 bytes).
	struct FDisAttentionChangeMessage
	{
		UBOOL m_bActorDead;
		BYTE m_PreviousAttentionLevel;
		BYTE m_NextAttentionLevel;
		FDisAttentionChangeReason m_ChangeReason;
		FVector m_ProxyLoc;
		UBOOL m_bResetAttentionReceived;
	};

	// DISHONORED(port): 2012 rva 0x78fe30 / 2013 0x728e60 (41 B) and 0x78fe60 / 0x72c9e0 (59 B); bodies in disaibrainprocessattention.cpp.
	FDisAttentionChangeReason MakeAttentionChangeReason( BYTE _ReasonType, class AActor* const _pActor, INT _iParam1, INT _iParam2 );
	FLOAT FindAttentionHeight( const FVector& _rAttentionLoc, const FVector& _rAttentionExtent, const FVector& _rAttentionFocalPoint );

	// ?NewInformationAttentionTag@DisAttention@@3HB (2013 VA 0x113d234): the tag of an increase that carries no tag of its own.
	static const INT NewInformationAttentionTag = -1;
}

// UDisAttentionInfo_Base::EDisWitnessDeathResult, as GetCurrentCorpseResponse (2012 0x7a79e0) returns it and
// UDisAIBrainProcessAttention::TickAttention_ReactToCorpses (0x79caa0) switches on it.
enum EDisWitnessDeathResult
{
	DWDR_None					= 0,
	DWDR_WitnessedDeath			= 1,
	DWDR_FoundNewCorpse_Direct	= 2,
	DWDR_FoundNewCorpse_Indirect= 3,
	DWDR_ExistingCorpse			= 4,
	DWDR_ExistingIgnoredCorpse	= 5,
};

// UDisAttentionInfo_Base::m_WitnessDeathState, the state machine of GetCurrentCorpseResponse.
enum EDisWitnessDeathState
{
	DWDS_Alive					= 0,
	DWDS_DiedUnseen				= 1,
	DWDS_DiedWitnessed			= 2,
	DWDS_CorpseToFind			= 3,
	DWDS_CorpseSeenNotBusted	= 4,
	DWDS_CorpseFoundIndirect	= 5,
	DWDS_CorpseHandled			= 6,
};

// EDisTopAttnTargetPriorityType: the ranking HandleTopAttnTarget (2012 0x7a67a0 / 2013 0x7305a0) gives a slot from its
// relationship and life state. Only DTATPT_DontCare is named in the PDB; the others are named here after what the
// arithmetic of that body means (neutral: dead 1 / alive 3; ally: alive 2 / dead 4; enemy: dead 1 / alive 5).
enum EDisTopAttnTargetPriorityType
{
	DTATPT_DontCare				= 0,
	DTATPT_DeadNeutralOrEnemy	= 1,
	DTATPT_LiveAlly				= 2,
	DTATPT_LiveNeutral			= 3,
	DTATPT_DeadAlly				= 4,
	DTATPT_LiveEnemy			= 5,
};

// The DisLookAtPriority / UDisTweaks_AIBrain flags ChangeTopAttnTargetAttentionLevel reads; kept with the attention types.
enum EDisAttentionTweakCategory
{
	DATC_Neutral				= 0,
	DATC_Ally					= 1,
	DATC_Enemy_Unsuspecting		= 2,
	DATC_Enemy_Suspecting		= 3,
	DATC_MAX					= 4,
};

// The -gisight STAND-IN for the vision component (disaibrainprocessattention.cpp); off unless the switch is on.
UBOOL DisGISightStandInEnabled();
UBOOL DisGIDiagEnabled();
UBOOL DisILDiagEnabled();
FLOAT DisGISightStandInRange();
UBOOL DisGISightStandInSeesPlayer( const class UDishonoredAIBrain* _pBrain, FLOAT& _rOutNormalisedDistance );
