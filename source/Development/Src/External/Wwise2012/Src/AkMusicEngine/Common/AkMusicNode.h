/*=============================================================================
	AkMusicNode.h - CAkMusicNode (akmusicnode.cpp): the base of every interactive-music node (segment, music
	switch container, music random/sequence container): the musical grid (tempo / time signature / grid), the
	stingers, the music-node part of the bank format, and the stop / pause / resume path through the music renderer.

	DISHONORED(port): agent IH4 (wave 26). Layout: CAkActiveParent<CAkParameterNode> (this tree's
	CAkParentNode<CAkParameterNode>, 88 bytes) + m_grid (+88) + m_bOverrideParentGrid (+104 bit 0) + m_pStingers (+108)
	= 112 bytes, as the 2012 PDB (CAkMusicNode ctor 0x967ea0 clears bit 0 of +104).
	Deviation (named): the vtable is this tree's CAkParameterNodeBase vtable plus CreateContext /
	PrepareMusicalDependencies / UnPrepareMusicalDependencies (retail slots 75..77); Children() is the inline count of
	the child map (retail slot 73, CAkParentNode<CAkParameterNode>::Children 0x94e310).
=============================================================================*/
#pragma once

#include <AkHierarchy.h>
#include "AkMusicStructs.h"

class CAkMatrixAwareCtx;
class CAkRegisteredObj;
class UserParams;

class CAkMusicNode : public CAkParentNode<CAkParameterNode>
{
public:
	/** 12 bytes: the stingers of a node, merged down the tree by GetStingers. */
	class CAkStingers
	{
	public:
		AkArray<CAkStinger, const CAkStinger &, ArrayPoolDefault, 2> m_StingerArray;
	};

	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual AKRESULT PrepareData();
	virtual void UnPrepareData();

	virtual CAkMatrixAwareCtx * CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams ) = 0;

	AkUInt16 Children() const { return (AkUInt16)m_mapChildId.Length(); }

	void MeterInfo( const AkMeterInfo * in_pMeterInfo );
	const AkMusicGrid & GetMusicGrid();
	AKRESULT SetStingers( CAkStinger * in_pStingers, AkUInt32 in_NumStingers );
	void GetStingers( CAkStingers * io_pStingers );

	AkMusicGrid		m_grid;
	AkUInt8			m_bOverrideParentGrid : 1;
	CAkStingers *	m_pStingers;

protected:
	explicit CAkMusicNode( AkUniqueID in_ulID );
	virtual ~CAkMusicNode();

	virtual AKRESULT PrepareMusicalDependencies();
	virtual void UnPrepareMusicalDependencies();

	AKRESULT SetMusicNodeParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly );
	void FlushStingers();
};

static_assert( sizeof( CAkParentNode<CAkParameterNode> ) == 88, "CAkActiveParent<CAkParameterNode>: 2012 PDB 88 bytes (music m_grid at +88)" );
static_assert( sizeof( CAkMusicNode ) == 112, "CAkMusicNode: 2012 PDB 112 bytes" );
