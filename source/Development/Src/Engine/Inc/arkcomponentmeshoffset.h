#pragma once
// Engine/inc/arkcomponentmeshoffset.h
// DISHONORED(port): agent GA (PHASE18 package GA). Arkane's mesh-offset component: the thing that keeps a pawn's body
// mesh on the ground when its collision cylinder is not - over a step, on a slope, on a moving base - by writing the
// mesh's Translation.Z (and, when the config asks for it, a ground-following pitch) every frame. Retail creates one on
// every NPC whose tweaks carry a UDisTweaks_ComponentMeshOffset (ADishonoredNPCPawn::PreBeginPlay_NativeComponents,
// 2012 rva 0x7b0c10), ticks it from FArkComponentManager's PreAsyncWork policy (or the JustBeforeProceduralAnim policy
// while foot props are pending), and reaches it from the pawn (SetMeshTranslation / LockMeshTranslation / the MAT_
// mesh-translation trio), the locomotion component (UpdateMesh writes m_ExternalRotationOffset) and the foot-placement
// skel control (m_FeetProps).
//
// DISHONORED(layout): 2012 PDB FArkComponentMeshOffset, 136 bytes, in the PDB's order; asserted in the .cpp.

#ifndef _INC_ARKCOMPONENTMESHOFFSET
#define _INC_ARKCOMPONENTMESHOFFSET

#include "arkcomponentbase.h"

class APawn;

class FArkComponentMeshOffset : public FArkComponentBase, public IArkComponentPreAsyncWork, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( ArkCpntType_MeshOffset, "ArkCpntType_MeshOffset" )

public:
	/** DISHONORED(layout): 2012 PDB FArkComponentMeshOffset::FArkCpntMeshOffsetFootProps, 16 bytes: one foot the
	    foot-placement skel control has planted, and how far below the cylinder bottom it wants to be. */
	struct FArkCpntMeshOffsetFootProps
	{
		FVector	m_WorldLocation;
		FLOAT	m_fTargetedZDelta;
	};

	FArkComponentMeshOffset();												// 2012 0x533620, 2013 rva 0x4f9640 region
	virtual ~FArkComponentMeshOffset() {}

	// ---- FArkComponentBase ----
	virtual void Starting();												// 2012 0x535840
	virtual void Stopping();												// 2012 0x529d40, 2013 0x4f17d0
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentMeshOffset ); }	// 2012 0x527040

	// ---- IArkComponentPreAsyncWork / ...JustBeforeProceduralAnim ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );						// 2012 0x5358e0
	virtual const FArkComponentBase* GetComponentBase() const { return this; }
	void PreAsyncWorkJustBeforeProceduralAnimTick( FLOAT _fTimeStep );		// 2012 0x535990

	static FArkComponentMeshOffset* GetMeshOffsetComponent( const APawn* _pPawnOwner, UBOOL _bMustBeStarted );	// 2012 0x535810, 2013 0x4faa90
	void ForceMeshOffset( FVector _vOffset, UBOOL _bInstant );				// 2012 0x52a130, 2013 0x4f1bc0
	void ForceInstantSnapMesh();											// 2012 0x535a40
	void EnableByFlags( INT _bFlags );										// 2012 0x535a90
	void DisableByFlags( INT _bFlags ) { m_DisableFlags |= _bFlags; }		// 2012 0x527050
	UBOOL IsDisabled() const { return m_DisableFlags != 0; }
	FLOAT GetCurrentOffsetZ() const { return m_fCurrentOffsetZ; }
	INT GetDisableFlags() const { return m_DisableFlags; }

	void SetExternalRotationOffset( const FRotator& _Rotation ) { m_ExternalRotationOffset = _Rotation; }
	void ForcePitch( INT _Pitch );											// 2013 0x8f1c10 (unnamed, not in 2012)
	TArray<FArkCpntMeshOffsetFootProps>& FeetProps() { return m_FeetProps; }

protected:
	void GetGroundOrientation( FVector& _rOrientation );					// 2012 0x529db0, 2013 0x4f1840

private:
	void Tick( FLOAT _fTimeStep );											// 2012 0x52f730, 2013 0x4f5e20
	void UpdateBaseMove();

	APawn*									m_pPawn;						// @24
	FLOAT									m_fLastPawnZ;					// @28
	FLOAT									m_fLastBaseZ;					// @32
	AActor*									m_pLastBase;					// @36
	FLOAT									m_fCurrentOffsetZ;				// @40
	FLOAT									m_fCurrentSpeedZ;				// @44
	FLOAT									m_fBaseZMove;					// @48
	INT										m_CurrentPitch;					// @52
	INT										m_DisableFlags;					// @56
	FVector									m_LastLineCheckLocation;		// @60
	FLOAT									m_fLastLineCheckHitZ;			// @72
	FRotator								m_ExternalRotationOffset;		// @76
	FVector									m_ExternalTranslationOffset;	// @88
	TArray<FArkCpntMeshOffsetFootProps>		m_FeetProps;					// @100

public:
	// the config, copied in by the pawn out of its UDisTweaks_ComponentMeshOffset (retail writes the members directly)
	UBOOL									m_bPitchOnGround;				// @112
	FLOAT									m_fPitchSpeed;					// @116
	FLOAT									m_fMinHeightSpeed;				// @120
	FLOAT									m_fMinHeightSpeedWithFootProps;	// @124
	FLOAT									m_fOffsetDown;					// @128
	FLOAT									m_fGoDownMinZSpeed;				// @132
};

#endif
