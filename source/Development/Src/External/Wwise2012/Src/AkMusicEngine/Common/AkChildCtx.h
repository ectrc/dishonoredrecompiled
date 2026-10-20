/*=============================================================================
	AkChildCtx.h - CAkChildCtx (akchildctx.cpp): anything a music context owns and drives - a child music context or
	a music PBI. The parent keeps its children in an intrusive light list (pNextLightItem).

	DISHONORED(port): agent IH4 (wave 26). 12 bytes (vptr, pNextLightItem, m_pParentCtx), 2012 PDB; the seven virtual
	slots in retail's order.
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>

class CAkMusicCtx;

class CAkChildCtx
{
public:
	explicit CAkChildCtx( CAkMusicCtx * in_pParentCtx );

	virtual void OnLastFrame( AkUInt32 in_uNumSamples ) = 0;
	virtual void OnPaused() = 0;
	virtual void OnResumed() = 0;
	virtual void SetPBIFade( void * in_pOwner, AkReal32 in_fFadeRatio ) = 0;
	virtual void VirtualAddRef() = 0;
	virtual void VirtualRelease() = 0;

	CAkChildCtx *	pNextLightItem;
	CAkMusicCtx *	m_pParentCtx;

	virtual ~CAkChildCtx();

	void Connect();
};

static_assert( sizeof( CAkChildCtx ) == 12, "CAkChildCtx: 2012 PDB 12" );
