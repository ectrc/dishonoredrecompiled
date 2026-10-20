/*=============================================================================
	AkMusicTransAware.h - CAkMusicTransAware (akmusictransaware.cpp): a music node with transition rules (the music
	switch and random/sequence containers): rule lookup by source / destination, the panic rule, and the
	transition segments prepared as musical dependencies.

	DISHONORED(port): agent IH4 (wave 26). Layout: CAkMusicNode (112) + m_arTrRules (+112) = 124 bytes (2012 PDB).
=============================================================================*/
#pragma once

#include "AkMusicNode.h"

class CAkMusicTransAware : public CAkMusicNode
{
public:
	const AkMusicTransitionRule & GetTransitionRule( AkUniqueID in_srcID, AkUniqueID in_destID );
	const AkMusicTransitionRule & GetTransitionRule( CAkParameterNodeBase * in_pOwnerNode, AkUniqueID in_srcID, CAkParameterNodeBase * in_pSrcParentNode, AkUniqueID in_destID, CAkParameterNodeBase * in_pDestParentNode, bool & out_bIsDestSequenceSpecific );
	static const AkMusicTransitionRule & GetPanicTransitionRule();

	AKRESULT SetRules( AkUInt32 in_NumRules, AkWwiseMusicTransitionRule * in_pRules );

	AkArray<AkMusicTransitionRule, const AkMusicTransitionRule &, ArrayPoolDefault, 0> m_arTrRules;

protected:
	explicit CAkMusicTransAware( AkUniqueID in_ulID );
	virtual ~CAkMusicTransAware();

	virtual AKRESULT PrepareMusicalDependencies();
	virtual void UnPrepareMusicalDependencies();

	AKRESULT SetMusicTransNodeParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly );

};

static_assert( sizeof( CAkMusicTransAware ) == 124, "CAkMusicTransAware: 2012 PDB 124 bytes" );
