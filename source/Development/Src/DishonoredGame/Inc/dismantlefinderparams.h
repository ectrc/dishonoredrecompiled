/*=============================================================================
	dismantlefinderparams.h: agent IG (PHASE23 IG) - the FDisNativeStateParam carriers of the mantle edge finder's FSM
	(UDishonoredMantleEdgeFinderComponent::m_pMantleFinderFSM). Each state is entered with the owner snapshot
	(FDisMantleFinderOwnerInfo) and what the previous step found. 2012 / 2013 rvas:
	  FDisMantleFinderState_Base_Param::OnPending                       0x7279f0 / 0x6c6b50 (also the FindClear and ForwardSearches
	                                                                    params' slot: /OPT:ICF)
	  FDisMantleFinderState_CheckCanStand_Param ctor / OnPending        0x731cf0 / 0x6d1cc0, 0x727c50 / 0x6c6dd0
	  FDisMantleFinderState_CheckDestinationValid_Param ctor / OnPending 0x731d80 / 0x6d1d50, 0x727cd0 / 0x6c6e70
	  FDisMantleFinderState_FindClearDistanceAboveOwner_Param ctor     0x731e30 / 0x6d1e00
	  FDisMantleFinderState_ForwardSearches_Param (built inline by FindClearDistanceAboveOwner::TickState; SizeOf 0x782540 / 0x7391a0)
	Bodies in Src/dismantlefinderstate_*.cpp.
=============================================================================*/
#ifndef _INC_DISMANTLEFINDERPARAMS
#define _INC_DISMANTLEFINDERPARAMS

struct FDisMantleFinderState_Base_Param : public FDisNativeStateParam
{
	FDisMantleFinderOwnerInfo m_OwnerInfo;

	FDisMantleFinderState_Base_Param( UClass* StateClass, const FDisMantleFinderOwnerInfo& OwnerInfo ) : FDisNativeStateParam( StateClass ), m_OwnerInfo( OwnerInfo ) {}
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMantleFinderState_Base_Param); }
};

struct FDisMantleFinderState_FindClearDistanceAboveOwner_Param : public FDisMantleFinderState_Base_Param
{
	FDisMantleFinderState_FindClearDistanceAboveOwner_Param();
	virtual INT SizeOf() const { return sizeof(FDisMantleFinderState_FindClearDistanceAboveOwner_Param); }
};

struct FDisMantleFinderState_ForwardSearches_Param : public FDisMantleFinderState_Base_Param
{
	FDisMantleFinderState_ForwardSearches_Param( const FDisMantleFinderOwnerInfo& OwnerInfo );
	virtual INT SizeOf() const { return sizeof(FDisMantleFinderState_ForwardSearches_Param); }
};

struct FDisMantleFinderState_CheckDestinationValid_Param : public FDisMantleFinderState_Base_Param
{
	FVector m_SearchStart;
	UBOOL m_bImpactAnimRequired;
	FVector m_DestLocation;
	FVector m_SearchPos;
	FLOAT m_fCurrentSearchHeight;
	FVector m_ForwardTraceDelta;

	FDisMantleFinderState_CheckDestinationValid_Param( const FDisMantleFinderOwnerInfo& OwnerInfo, const FVector& SearchStart, UBOOL bImpactAnimRequired,
		const FVector& DestLocation, const FVector& SearchPos, FLOAT CurrentSearchHeight, const FVector& ForwardTraceDelta );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMantleFinderState_CheckDestinationValid_Param); }
};

struct FDisMantleFinderState_CheckCanStand_Param : public FDisMantleFinderState_Base_Param
{
	UBOOL m_bImpactAnimRequired;
	FVector m_SearchStart;
	FVector m_ForwardTraceDelta;
	FLOAT m_fCurrentHeightOffset;

	FDisMantleFinderState_CheckCanStand_Param( const FDisMantleFinderOwnerInfo& OwnerInfo, UBOOL bImpactAnimRequired, const FVector& SearchStart,
		const FVector& ForwardTraceDelta, FLOAT CurrentHeightOffset );
	virtual void OnPending( class UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisMantleFinderState_CheckCanStand_Param); }
};

#endif
