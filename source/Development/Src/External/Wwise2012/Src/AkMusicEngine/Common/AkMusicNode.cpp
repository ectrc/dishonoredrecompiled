/*=============================================================================
	AkMusicNode.cpp - CAkMusicNode (akmusicnode.cpp, 2012 0x967970..0x967f30; 2013 matched by bytes, rvas on each).

	DISHONORED(port): agent IH4 (wave 26). See AkMusicNode.h for the layout and the vtable deviation.
=============================================================================*/
#include "AkMusicNode.h"
#include "AkMusicRenderer.h"

/** 2012 0x967ea0, 2013 0x964750. */
CAkMusicNode::CAkMusicNode( AkUniqueID in_ulID )
	: CAkParentNode<CAkParameterNode>( in_ulID )
	, m_pStingers( NULL )
{
	m_grid.uBeatDuration = 0;
	m_grid.uBarDuration = 0;
	m_grid.uGridDuration = 0;
	m_grid.uGridOffset = 0;
	m_bOverrideParentGrid = 0;
}

/** 2012 0x967ee0, 2013 0x964790. */
CAkMusicNode::~CAkMusicNode()
{
	FlushStingers();
}

/** 2012 0x967970, 2013 0x964210: the grid in samples; the beat is a quarter note scaled by the time signature. */
void CAkMusicNode::MeterInfo( const AkMeterInfo * in_pMeterInfo )
{
	if( in_pMeterInfo )
	{
		m_bOverrideParentGrid = 1;
		const AkUInt32 uBeat = (AkUInt32)AkTimeConv::SecondsToSamples( 1.0 / in_pMeterInfo->fTempo * 60.0 * ( 4.0 / (AkReal64)in_pMeterInfo->uTimeSigBeatValue ) );
		m_grid.uBeatDuration = uBeat;
		m_grid.uBarDuration = uBeat * in_pMeterInfo->uTimeSigNumBeatsBar;
		m_grid.uGridDuration = (AkUInt32)AkTimeConv::MillisecondsToSamples( in_pMeterInfo->fGridPeriod );
		m_grid.uGridOffset = (AkUInt32)AkTimeConv::MillisecondsToSamples( in_pMeterInfo->fGridOffset );
	}
	else
	{
		m_bOverrideParentGrid = 0;
	}
}

/** 2012 0x9679f0, 2013 0x964290: the nearest node up the tree that overrides its parent's grid (or the root). */
const AkMusicGrid & CAkMusicNode::GetMusicGrid()
{
	CAkMusicNode * pNode = this;
	if( !pNode->m_bOverrideParentGrid )
	{
		do
		{
			CAkParameterNodeBase * pParent = pNode->m_pParentNode;
			if( !pParent )
			{
				break;
			}
			pNode = static_cast<CAkMusicNode *>( pParent );
		}
		while( !pNode->m_bOverrideParentGrid );
	}
	return pNode->m_grid;
}

/** 2012 0x967a60, 2013 0x964310: every stinger's segment is prepared; the ones done are undone on failure. */
AKRESULT CAkMusicNode::PrepareMusicalDependencies()
{
	AKRESULT eResult = AK_Success;
	if( m_pStingers )
	{
		CAkStinger * pIt = m_pStingers->m_StingerArray.m_pItems;
		for( ; pIt != m_pStingers->m_StingerArray.m_pItemsEnd; ++pIt )
		{
			eResult = CAkParameterNodeBase::PrepareNodeData( pIt->m_SegmentID );
			if( eResult != AK_Success )
			{
				break;
			}
		}
		if( eResult != AK_Success )
		{
			for( CAkStinger * pUndo = m_pStingers->m_StingerArray.m_pItems; pUndo != pIt; ++pUndo )
			{
				CAkParameterNodeBase::UnPrepareNodeData( pUndo->m_SegmentID );
			}
		}
	}
	return eResult;
}

/** 2012 0x967ac0, 2013 0x964370. */
void CAkMusicNode::UnPrepareMusicalDependencies()
{
	if( m_pStingers )
	{
		for( CAkStinger * pIt = m_pStingers->m_StingerArray.m_pItems; pIt != m_pStingers->m_StingerArray.m_pItemsEnd; ++pIt )
		{
			CAkParameterNodeBase::UnPrepareNodeData( pIt->m_SegmentID );
		}
	}
}

/** 2012 0x967af0, 2013 0x9643a0. */
AKRESULT CAkMusicNode::PrepareData()
{
	AKRESULT eResult = PrepareMusicalDependencies();
	if( eResult == AK_Success )
	{
		eResult = CAkParentNode<CAkParameterNode>::PrepareData();
		if( eResult != AK_Success )
		{
			UnPrepareMusicalDependencies();
		}
	}
	return eResult;
}

/** 2012 0x967b30, 2013 0x9643e0. */
void CAkMusicNode::UnPrepareData()
{
	for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
	{
		( *it )->UnPrepareData();
	}
	UnPrepareMusicalDependencies();
}

/** 2012 0x967b70, 2013 0x964420: stop / pause / resume the music contexts of this node, then the children. */
AKRESULT CAkMusicNode::ExecuteAction( ActionParams & in_rAction )
{
	AddRef();
	if( in_rAction.eType == ActionParamType_Stop )
	{
		CAkMusicRenderer::m_pMusicRenderer->Stop( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	}
	else if( in_rAction.eType == ActionParamType_Pause )
	{
		CAkMusicRenderer::m_pMusicRenderer->Pause( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	}
	else if( in_rAction.eType == ActionParamType_Resume )
	{
		CAkMusicRenderer::m_pMusicRenderer->Resume( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.bIsMasterResume, in_rAction.playingID );
	}
	const AKRESULT eResult = CAkParentNode<CAkParameterNode>::ExecuteAction( in_rAction );
	Release();
	return eResult;
}

/** 2012 0x967c00, 2013 0x9644b0. */
AKRESULT CAkMusicNode::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	AddRef();
	if( in_rAction.eType == ActionParamType_Stop )
	{
		CAkMusicRenderer::m_pMusicRenderer->Stop( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	}
	else if( in_rAction.eType == ActionParamType_Pause )
	{
		CAkMusicRenderer::m_pMusicRenderer->Pause( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	}
	else if( in_rAction.eType == ActionParamType_Resume )
	{
		CAkMusicRenderer::m_pMusicRenderer->Resume( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.bIsMasterResume, in_rAction.playingID );
	}
	const AKRESULT eResult = CAkParentNode<CAkParameterNode>::ExecuteActionExcept( in_rAction );
	Release();
	return eResult;
}

/** 2012 0x967d00, 2013 0x9645b0. */
void CAkMusicNode::FlushStingers()
{
	if( m_pStingers )
	{
		m_pStingers->m_StingerArray.Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pStingers );
		m_pStingers = NULL;
	}
}

/** 2012 0x967d60, 2013 0x964610: the stinger list is replaced (kept allocated if it exists). */
AKRESULT CAkMusicNode::SetStingers( CAkStinger * in_pStingers, AkUInt32 in_NumStingers )
{
	if( !in_NumStingers )
	{
		FlushStingers();
		return AK_Success;
	}
	if( m_pStingers )
	{
		m_pStingers->m_StingerArray.m_pItemsEnd = m_pStingers->m_StingerArray.m_pItems;
	}
	else
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkStingers ) );
		m_pStingers = pMem ? AkPlacementNew( pMem ) CAkStingers : NULL;
		if( !m_pStingers || m_pStingers->m_StingerArray.Reserve( in_NumStingers ) != AK_Success )
		{
			return AK_Fail;
		}
	}
	for( AkUInt32 i = 0; i < in_NumStingers; ++i )
	{
		if( !m_pStingers->m_StingerArray.AddLast( in_pStingers[ i ] ) )
		{
			return AK_Fail;
		}
	}
	return AK_Success;
}

/** 2012 0x967e00, 2013 0x9646b0: the parent's stingers first; a child's stinger replaces the same trigger's. */
void CAkMusicNode::GetStingers( CAkStingers * io_pStingers )
{
	CAkMusicNode * pParent = static_cast<CAkMusicNode *>( m_pParentNode );
	if( pParent )
	{
		pParent->GetStingers( io_pStingers );
	}
	if( !m_pStingers )
	{
		return;
	}
	for( const CAkStinger * pIt = m_pStingers->m_StingerArray.m_pItems; pIt != m_pStingers->m_StingerArray.m_pItemsEnd; ++pIt )
	{
		CAkStinger * pExisting = io_pStingers->m_StingerArray.m_pItems;
		while( pExisting != io_pStingers->m_StingerArray.m_pItemsEnd )
		{
			if( pExisting->m_TriggerID == pIt->m_TriggerID )
			{
				if( io_pStingers->m_StingerArray.Length() > 1 )
				{
					*pExisting = *( io_pStingers->m_StingerArray.m_pItemsEnd - 1 );
				}
				--io_pStingers->m_StingerArray.m_pItemsEnd;
			}
			else
			{
				++pExisting;
			}
		}
		io_pStingers->m_StingerArray.AddLast( *pIt );
	}
}

/**
 * 2012 0x967f30: the node id, the node base params, the children, the meter (period, offset, tempo,
 * time signature, override flag) and the stingers (trigger, segment, sync, cue filter, don't-repeat, look-ahead).
 */
AKRESULT CAkMusicNode::SetMusicNodeParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool /*in_bPartialLoadOnly*/ )
{
	io_rpData += 4;
	AKRESULT eResult = SetNodeBaseParams( io_rpData, io_rulDataSize, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = SetChildren( io_rpData, io_rulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	AkMeterInfo meterInfo;
	meterInfo.fGridPeriod = *(AkReal64 *)io_rpData;
	meterInfo.fGridOffset = *(AkReal64 *)( io_rpData + 8 );
	meterInfo.fTempo = *(AkReal32 *)( io_rpData + 16 );
	io_rpData += 20;
	meterInfo.uTimeSigNumBeatsBar = *io_rpData++;
	meterInfo.uTimeSigBeatValue = *io_rpData++;
	const AkUInt8 bMeterInfoFlag = *io_rpData++;
	if( bMeterInfoFlag )
	{
		MeterInfo( &meterInfo );
	}
	const AkUInt32 uNumStingers = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( !uNumStingers )
	{
		FlushStingers();
		return AK_Success;
	}
	CAkStinger * pStingers = (CAkStinger *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkStinger ) * uNumStingers );
	if( !pStingers )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < uNumStingers; ++i )
	{
		pStingers[ i ].m_TriggerID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		pStingers[ i ].m_SegmentID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		pStingers[ i ].m_SyncPlayAt = (AkSyncType)*(AkUInt32 *)io_rpData;
		io_rpData += 4;
		pStingers[ i ].m_uCueFilterHash = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		pStingers[ i ].m_DontRepeatTime = *(AkInt32 *)io_rpData;
		io_rpData += 4;
		pStingers[ i ].m_numSegmentLookAhead = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
	}
	eResult = SetStingers( pStingers, uNumStingers );
	AK::MemoryMgr::Free( g_DefaultPoolId, pStingers );
	return eResult;
}
