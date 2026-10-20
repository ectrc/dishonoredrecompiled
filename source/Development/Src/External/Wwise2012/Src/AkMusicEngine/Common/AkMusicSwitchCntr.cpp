/*=============================================================================
	AkMusicSwitchCntr.cpp - CAkMusicSwitchCntr (akmusicswitchcntr.cpp, 2012 0x968f30..0x969600; ExecuteAction 0x969730
	is filed under akmusicranseqcntr.cpp in the PDB).

	DISHONORED(port): agent IH4 (wave 26).
	Deviation (named): PrepareData's failure rollback unprepares the failing association's node for every earlier
	match (it passes i->item where j->item is meant) - retail 0x9691e0 does exactly this and it is kept.
=============================================================================*/
#include "AkMusicSwitchCntr.h"
#include "AkMusicSwitchCtx.h"

/** 2012 0x969070, 2013 0x9658c0: created under g_csMain. */
CAkMusicSwitchCntr * CAkMusicSwitchCntr::Create( AkUniqueID in_ulID )
{
	AkAutoLock<CAkLock> gate( g_csMain );
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSwitchCntr ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkMusicSwitchCntr * pCntr = AkPlacementNew( pMem ) CAkMusicSwitchCntr( in_ulID );
	pCntr->AddToIndex();
	return pCntr;
}

/** 2012 0x969020, 2013 0x965870. */
CAkMusicSwitchCntr::CAkMusicSwitchCntr( AkUniqueID in_ulID )
	: CAkMusicTransAware( in_ulID )
	, m_ulGroupID( 0 )
	, m_ulDefaultSwitch( 0 )
	, m_eGroupType( AkGroupType_Switch )
	, m_bIsContinuePlayback( true )
{
}

/** 2012 0x9695a0, 2013 0x965d70. */
CAkMusicSwitchCntr::~CAkMusicSwitchCntr()
{
	m_arSwitchNode.Term();
}

/** 2012 0x968f30, 2013 0x965780: a switch context, initialised on the group; a failed init is cancelled and NULL returned. */
CAkMatrixAwareCtx * CAkMusicSwitchCntr::CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSwitchCtx ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkMusicSwitchCtx * pCtx = AkPlacementNew( pMem ) CAkMusicSwitchCtx( this, in_pParentCtx );
	pCtx->AddRef();
	if( pCtx->Init( in_GameObject, in_rUserparams, m_ulGroupID, m_eGroupType ) == AK_Success )
	{
		pCtx->Release();
		return pCtx;
	}
	pCtx->_Cancel();
	pCtx->Release();
	return NULL;
}

/** 2012 0x968fb0, 2013 0x965800: a top-level switch context, its children's init completed, played with its silent look-ahead. */
AKRESULT CAkMusicSwitchCntr::Play( AkPBIParams & in_rPBIParams )
{
	CAkMatrixAwareCtx * pCtx = CreateContext( NULL, in_rPBIParams.pGameObj, in_rPBIParams.userParams );
	if( !pCtx )
	{
		return AK_Fail;
	}
	pCtx->EndInit();
	AkMusicFade fadeParams;
	fadeParams.transitionTime = in_rPBIParams.pTransitionParameters->TransitionTime;
	fadeParams.eFadeCurve = in_rPBIParams.pTransitionParameters->eFadeCurve;
	fadeParams.iFadeOffset = pCtx->GetSilentDuration();
	pCtx->_Play( fadeParams );
	return AK_Success;
}

/** 2012 0x969730, 2013 0x965f00: a music switch container cannot seek. */
AKRESULT CAkMusicSwitchCntr::ExecuteAction( ActionParams & in_rAction )
{
	if( in_rAction.eType == ActionParamType_Seek )
	{
		return AK_Fail;
	}
	return CAkMusicNode::ExecuteAction( in_rAction );
}

/** 2012 0x9690d0, 2013 0x965920: unparented segments and music containers, once each. */
AKRESULT CAkMusicSwitchCntr::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	const AkNodeCategory eCategory = in_pAudioNode->NodeCategory();
	if( Children() == 0xFFFF )
	{
		return AK_MaxReached;
	}
	if( eCategory != AkNodeCategory_MusicSegment && eCategory != AkNodeCategory_MusicRanSeqCntr && eCategory != AkNodeCategory_MusicSwitchCntr )
	{
		return AK_NotCompatible;
	}
	if( in_pAudioNode->m_pParentNode )
	{
		return AK_ChildAlreadyHasAParent;
	}
	const AkUniqueID key = in_pAudioNode->key;
	if( m_mapChildId.Exists( key ) )
	{
		return AK_AlreadyConnected;
	}
	return this->key == key ? AK_CannotAddItseflAsAChild : AK_Success;
}

/** 2012 0x969180, 2013 0x9659d0: a game sync of the group becomes (un)supported: its node is (un)prepared while this is prepared. */
AKRESULT CAkMusicSwitchCntr::ModifyActiveState( AkUInt32 in_stateID, bool in_bSupported )
{
	if( m_uPreparationCount )
	{
		AkUniqueID * pNodeID = m_arSwitchNode.Exists( in_stateID );
		if( pNodeID )
		{
			if( in_bSupported )
			{
				return CAkParameterNodeBase::PrepareNodeData( *pNodeID );
			}
			CAkParameterNodeBase::UnPrepareNodeData( *pNodeID );
		}
	}
	return AK_Success;
}

/** 2012 0x9691e0, 2013 0x965a30: with game-sync preparation, only the children of the prepared switches; else every child. */
AKRESULT CAkMusicSwitchCntr::PrepareData()
{
	if( !g_settings.bEnableGameSyncPreparation )
	{
		return CAkMusicNode::PrepareData();
	}
	AKRESULT eResult = AK_Success;
	if( m_uPreparationCount )
	{
		++m_uPreparationCount;
		return eResult;
	}
	eResult = PrepareMusicalDependencies();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	CAkPreparedContent * pPreparedContent = GetPreparedContent( m_ulGroupID, m_eGroupType );
	if( !pPreparedContent )
	{
		eResult = AK_InsufficientMemory;
		UnPrepareMusicalDependencies();
		return eResult;
	}
	for( MapStruct<AkUInt32, AkUniqueID> * i = m_arSwitchNode.m_pItems; i != m_arSwitchNode.m_pItemsEnd; ++i )
	{
		if( pPreparedContent->m_PreparableContentList.Exists( i->key ) )
		{
			eResult = CAkParameterNodeBase::PrepareNodeData( i->item );
		}
		if( eResult != AK_Success )
		{
			for( MapStruct<AkUInt32, AkUniqueID> * j = m_arSwitchNode.m_pItems; j != i; ++j )
			{
				if( pPreparedContent->m_PreparableContentList.Exists( j->key ) )
				{
					CAkParameterNodeBase::UnPrepareNodeData( i->item );
				}
			}
		}
	}
	if( eResult == AK_Success )
	{
		++m_uPreparationCount;
		eResult = SubscribePrepare( m_ulGroupID, m_eGroupType );
		if( eResult != AK_Success )
		{
			UnPrepareData();
			UnPrepareMusicalDependencies();
		}
		return eResult;
	}
	UnPrepareMusicalDependencies();
	return eResult;
}

/** 2012 0x969370, 2013 0x965bc0. */
void CAkMusicSwitchCntr::UnPrepareData()
{
	if( !g_settings.bEnableGameSyncPreparation )
	{
		CAkMusicNode::UnPrepareData();
		return;
	}
	if( !m_uPreparationCount )
	{
		return;
	}
	if( m_uPreparationCount-- != 1 )
	{
		return;
	}
	CAkPreparedContent * pPreparedContent = GetPreparedContent( m_ulGroupID, m_eGroupType );
	if( pPreparedContent )
	{
		for( MapStruct<AkUInt32, AkUniqueID> * i = m_arSwitchNode.m_pItems; i != m_arSwitchNode.m_pItemsEnd; ++i )
		{
			if( pPreparedContent->m_PreparableContentList.Exists( i->key ) )
			{
				CAkParameterNodeBase::UnPrepareNodeData( i->item );
			}
		}
	}
	UnsubscribePrepare( m_ulGroupID, m_eGroupType );
	UnPrepareMusicalDependencies();
}

/** 2012 0x9694b0, 2013 0x965c80. */
AKRESULT CAkMusicSwitchCntr::SetSwitchAssocs( AkUInt32 in_uNumAssocs, AkMusicSwitchAssoc * in_pAssocs )
{
	m_arSwitchNode.m_pItemsEnd = m_arSwitchNode.m_pItems;
	for( AkUInt32 i = 0; i < in_uNumAssocs; ++i )
	{
		AkUniqueID * pNodeID = m_arSwitchNode.Set( in_pAssocs[ i ].switchID );
		if( !pNodeID )
		{
			return AK_Fail;
		}
		*pNodeID = in_pAssocs[ i ].nodeID;
	}
	return AK_Success;
}

/** 2012 0x969510, 2013 0x965ce0: the switch's node, else the default switch's; AK_InvalidSwitchType (node 0) if neither. */
AKRESULT CAkMusicSwitchCntr::GetSwitchNode( AkUInt32 in_switchID, AkUniqueID & out_nodeID )
{
	AkUniqueID * pNodeID = m_arSwitchNode.Exists( in_switchID );
	if( !pNodeID )
	{
		pNodeID = m_arSwitchNode.Exists( m_ulDefaultSwitch );
		if( !pNodeID )
		{
			out_nodeID = 0;
			return AK_InvalidSwitchType;
		}
	}
	out_nodeID = *pNodeID;
	return AK_Success;
}

/** 2012 0x969600, 2013 0x965dd0: the music transition node part, the group type / id, the default switch, continue-to-play, the map. */
AKRESULT CAkMusicSwitchCntr::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	AKRESULT eResult = SetMusicTransNodeParams( in_pData, in_ulDataSize, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 eGroupType = *(AkUInt32 *)in_pData;
	in_pData += 4;
	const AkUInt32 ulGroupID = *(AkUInt32 *)in_pData;
	in_pData += 4;
	const AkUInt32 ulDefaultSwitch = *(AkUInt32 *)in_pData;
	in_pData += 4;
	m_bIsContinuePlayback = *in_pData++ != 0;
	m_eGroupType = (AkGroupType)eGroupType;
	m_ulGroupID = ulGroupID;
	m_ulDefaultSwitch = ulDefaultSwitch;
	const AkUInt32 uNumAssocs = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( uNumAssocs )
	{
		AkMusicSwitchAssoc * pAssocs = (AkMusicSwitchAssoc *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicSwitchAssoc ) * uNumAssocs );
		if( !pAssocs )
		{
			return AK_InsufficientMemory;
		}
		for( AkUInt32 i = 0; i < uNumAssocs; ++i )
		{
			pAssocs[ i ].switchID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pAssocs[ i ].nodeID = *(AkUInt32 *)in_pData;
			in_pData += 4;
		}
		SetSwitchAssocs( uNumAssocs, pAssocs );
		AK::MemoryMgr::Free( g_DefaultPoolId, pAssocs );
	}
	g_pHircParseEnd = in_pData;
	return AK_Success;
}
