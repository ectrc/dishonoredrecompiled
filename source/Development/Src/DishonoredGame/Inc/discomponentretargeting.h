#pragma once
// DishonoredGame/inc/discomponentretargeting.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x8af840  public: static class FName __cdecl FDisComponentRetargeting::GetInternalName(void)
//   0x8af920  public: virtual unsigned int __thiscall FDisComponentRetargeting::IsOfType(int)const
//   0x8af940  public: virtual int __thiscall FDisComponentRetargeting::GetType(void)const
//   0x8af950  public: virtual class FName __thiscall FDisComponentRetargeting::GetName(void)const

/*-----------------------------------------------------------------------------
	DISHONORED(port): agent HC (PHASE22 HC). The component that glues a pawn's anchor bone onto another actor's bone or
	socket: a matinee Attachment key on an NPC (ADishonoredNPCPawn::MAT_Attach, 2012 0x7ab5e0 / 2013 0xb4b080) starts it
	with the key's bone/socket and the pawn's 'anchor_jnt'; every PreAsyncWork tick it rewrites the pawn's location and
	rotation from the master's bone (UpdateRetargeting) while the pawn sits in PHYS_None with its mesh offset locked and its
	rotation intents locked at DisFaceToPriority_ReTargeting. Layout: 2012 PDB, 88 bytes, both builds (the 2013 bodies
	address the same offsets). Created for every ADishonoredPawn by PreBeginPlay_NativeComponents (2012 0x793f50).
-----------------------------------------------------------------------------*/

#include "arkcomponentbase.h"

class ADishonoredPawn;
class USkeletalMeshSocket;
class FArkGameEvent;

class FDisComponentRetargeting : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( DisCpntType_Retargeting, "DisCpntType_Retargeting" )

public:
	FDisComponentRetargeting();												// 2012 0x8af8b0, 2013 0xc66c90

	virtual DWORD GetMemoryFootprint() const { return GetAllocatedSize() + sizeof( FDisComponentRetargeting ); }	// 2012 0x8af970
	virtual void Stopping();												// 2012 0x8d2330, 2013 0xc88370

	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );						// 2012 0x8d23c0, 2013 0xc88400
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	void StartRetargeting( AActor* const _pMaster, FName _MasterBoneName, FName _SlaveBoneName, UBOOL _bAllowRotation,
		UBOOL _bKeepActorCloser, FLOAT _fRetargetTime, UBOOL _bManageVolumeCollisions );	// 2012 0x8d2400, 2013 0xc88440
	void UpdateRetargeting( FLOAT _fTimeStep );								// 2012 0x8b37e0, 2013 0xc6ace0
	void StopRetargeting();													// 2012 0x8d0eb0, 2013 0xc87430

protected:
	void SetMasterActor( AActor* const _pActor );							// 2012 0x8d1100, 2013 0xc87690
	void OnStopRetargeting( const FArkGameEvent& _rEvent );					// 2012 0x8d10f0, 2013 0xc87680

public:
	INT						m_iMasterBoneIndex;			// @20
	INT						m_iSlaveBoneIndex;			// @24
	USkeletalMeshSocket*	m_pMasterSkocket;			// @28
	AActor*					m_pMaster;					// @32
	ADishonoredPawn*		m_pMasterPawn;				// @36
	ADishonoredPawn*		m_pSlave;					// @40
	USkeletalMeshComponent*	m_pMasterSkeletalComp;		// @44
	USkeletalMeshComponent*	m_pSlaveSkeletalComp;		// @48
	INT						m_OldRMM;					// @52
	INT						m_OldRMRM;					// @56
	UBOOL					m_bIsRetargeting;			// @60
	UBOOL					m_bAllowRotation;			// @64
	UBOOL					m_bKeepActorCloser;			// @68
	UBOOL					m_bManageVolumeCollisions;	// @72
	UBOOL					m_bIsInTick;				// @76
	UBOOL					m_bShouldbeStopped;			// @80
	FLOAT					m_fTimeLeft;				// @84
};
