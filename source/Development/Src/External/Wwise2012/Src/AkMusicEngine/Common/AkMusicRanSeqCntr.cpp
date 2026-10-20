/*=============================================================================
	AkMusicRanSeqCntr.cpp - CAkMusicRanSeqCntr (akmusicranseqcntr.cpp, 2012 0x969750..0x969ce0).

	DISHONORED(port): agent IH4 (wave 26).
	Deviation (named): ExecuteAction is the body retail folds with CAkMusicSwitchCntr::ExecuteAction (0x969730, the
	PDB names the switch container's); it is defined for this class too.
=============================================================================*/
#include "AkMusicRanSeqCntr.h"
#include "AkSequenceCtx.h"

/** 2012 0x9699e0, 2013 0x9661b0. */
CAkMusicRanSeqCntr * CAkMusicRanSeqCntr::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicRanSeqCntr ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkMusicRanSeqCntr * pCntr = AkPlacementNew( pMem ) CAkMusicRanSeqCntr( in_ulID );
	pCntr->AddToIndex();
	return pCntr;
}

/** 2012 0x969850, 2013 0x966020. */
CAkMusicRanSeqCntr::CAkMusicRanSeqCntr( AkUniqueID in_ulID )
	: CAkMusicTransAware( in_ulID )
	, m_playListRoot( NULL )
{
}

/** 2012 0x969ca0 (the deleting destructor's body). */
CAkMusicRanSeqCntr::~CAkMusicRanSeqCntr()
{
	FlushPlaylist();
}

/** 2012 0x969750, 2013 0x965f20: a sequence context; a failed init is cancelled and NULL returned. */
CAkSequenceCtx * CAkMusicRanSeqCntr::CreateSequenceCtx( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequenceCtx ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSequenceCtx * pCtx = AkPlacementNew( pMem ) CAkSequenceCtx( this, in_pParentCtx );
	pCtx->AddRef();
	if( pCtx->Init( in_GameObject, in_rUserparams ) == AK_Success )
	{
		pCtx->Release();
		return pCtx;
	}
	pCtx->_Cancel();
	pCtx->Release();
	return NULL;
}

/** 2012 0x9697c0, 2013 0x965f90. */
AKRESULT CAkMusicRanSeqCntr::Play( AkPBIParams & in_rPBIParams )
{
	CAkSequenceCtx * pCtx = CreateSequenceCtx( NULL, in_rPBIParams.pGameObj, in_rPBIParams.userParams );
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

/** 2012 0x969730, 2013 0x965f00 (folded with CAkMusicSwitchCntr::ExecuteAction). */
AKRESULT CAkMusicRanSeqCntr::ExecuteAction( ActionParams & in_rAction )
{
	if( in_rAction.eType == ActionParamType_Seek )
	{
		return AK_Fail;
	}
	return CAkMusicNode::ExecuteAction( in_rAction );
}

/** 2012 0x9698d0, 2013 0x9660a0. */
CAkMatrixAwareCtx * CAkMusicRanSeqCntr::CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	return CreateSequenceCtx( in_pParentCtx, in_GameObject, in_rUserparams );
}

/** 2012 0x9698e0, 2013 0x9660b0. */
void CAkMusicRanSeqCntr::FlushPlaylist()
{
	for( CAkRSNode ** it = m_playListRoot.m_listChildren.m_pItems; it != m_playListRoot.m_listChildren.m_pItemsEnd; ++it )
	{
		if( *it )
		{
			( *it )->~CAkRSNode();
			AK::MemoryMgr::Free( g_DefaultPoolId, *it );
		}
	}
	m_playListRoot.m_listChildren.m_pItemsEnd = m_playListRoot.m_listChildren.m_pItems;
	m_playListRoot.Clear();
}

/** 2012 0x969940, 2013 0x966110: unparented segments only, once each. */
AKRESULT CAkMusicRanSeqCntr::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	const AkNodeCategory eCategory = in_pAudioNode->NodeCategory();
	if( Children() == 0xFFFF )
	{
		return AK_MaxReached;
	}
	if( eCategory != AkNodeCategory_MusicSegment )
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

/**
 * 2012 0x969a20: depth-first, the bank's flat item list: a segment id makes a CAkRSSegment leaf, no segment id a
 * CAkRSSub group whose m_NumChildren items follow. A non-default weight marks the parent as weighted.
 */
AKRESULT CAkMusicRanSeqCntr::AddPlaylistChildren( CAkRSSub * in_pParent, AkMusicRanSeqPlaylistItem *& io_pArrayItems, AkUInt32 in_ulNumItems )
{
	for( AkUInt32 i = 0; i < in_ulNumItems; ++i )
	{
		AkMusicRanSeqPlaylistItem & item = *io_pArrayItems++;
		if( item.m_SegmentID )
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRSSegment ) );
			if( !pMem )
			{
				return AK_Fail;
			}
			CAkRSSegment * pSegment = AkPlacementNew( pMem ) CAkRSSegment( in_pParent );
			CAkRSNode ** ppSlot = in_pParent->m_listChildren.AddLast();
			if( !ppSlot )
			{
				pSegment->~CAkRSSegment();
				AK::MemoryMgr::Free( g_DefaultPoolId, pSegment );
				return AK_Fail;
			}
			*ppSlot = pSegment;
			pSegment->m_Loop = item.m_Loop;
			pSegment->m_Weight = item.m_Weight;
			pSegment->m_SegmentID = item.m_SegmentID;
			pSegment->m_ID = item.m_playlistItemID;
			in_pParent->WasSegmentLeafFound();
		}
		else
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRSSub ) );
			if( !pMem )
			{
				return AK_Fail;
			}
			CAkRSSub * pSub = AkPlacementNew( pMem ) CAkRSSub( in_pParent );
			CAkRSNode ** ppSlot = in_pParent->m_listChildren.AddLast();
			if( !ppSlot )
			{
				pSub->~CAkRSSub();
				AK::MemoryMgr::Free( g_DefaultPoolId, pSub );
				return AK_Fail;
			}
			*ppSlot = pSub;
			pSub->m_wAvoidRepeatCount = item.m_wAvoidRepeatCount;
			pSub->m_Loop = item.m_Loop;
			pSub->m_Weight = item.m_Weight;
			pSub->m_eRSType = item.m_eRSType;
			pSub->m_bIsUsingWeight = false;
			pSub->m_bIsShuffle = item.m_bIsShuffle;
			pSub->m_ID = item.m_playlistItemID;
			if( item.m_NumChildren && AddPlaylistChildren( pSub, io_pArrayItems, item.m_NumChildren ) != AK_Success )
			{
				return AK_Fail;
			}
		}
		if( item.m_Weight != 50000 )
		{
			in_pParent->m_bIsUsingWeight = true;
		}
	}
	return AK_Success;
}

/** 2012 0x969c20, 2013 0x9663f0: the first item describes the root group. */
AKRESULT CAkMusicRanSeqCntr::SetPlayList( AkMusicRanSeqPlaylistItem * in_pArrayItems )
{
	FlushPlaylist();
	AkMusicRanSeqPlaylistItem & root = *in_pArrayItems++;
	m_playListRoot.m_wAvoidRepeatCount = root.m_wAvoidRepeatCount;
	m_playListRoot.m_Loop = root.m_Loop;
	m_playListRoot.m_Weight = root.m_Weight;
	m_playListRoot.m_eRSType = root.m_eRSType;
	m_playListRoot.m_bIsUsingWeight = false;
	m_playListRoot.m_bIsShuffle = root.m_bIsShuffle;
	m_playListRoot.m_ID = root.m_playlistItemID;
	if( root.m_NumChildren )
	{
		return AddPlaylistChildren( &m_playListRoot, in_pArrayItems, root.m_NumChildren );
	}
	return AK_Success;
}

/**
 * 2012 0x969ce0: the music transition node part, then the playlist items (segment, item id, child count, type, loop,
 * weight, avoid-repeat, using-weight, shuffle). The transition-node result is returned (retail reads on regardless).
 */
AKRESULT CAkMusicRanSeqCntr::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	const AKRESULT eResult = SetMusicTransNodeParams( in_pData, in_ulDataSize, false );
	const AkUInt32 uNumPlaylistItems = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( uNumPlaylistItems )
	{
		AkMusicRanSeqPlaylistItem * pItems = (AkMusicRanSeqPlaylistItem *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicRanSeqPlaylistItem ) * uNumPlaylistItems );
		if( !pItems )
		{
			return AK_Fail;
		}
		for( AkUInt32 i = 0; i < uNumPlaylistItems; ++i )
		{
			pItems[ i ].m_SegmentID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pItems[ i ].m_playlistItemID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pItems[ i ].m_NumChildren = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pItems[ i ].m_eRSType = (RSType)*(AkUInt32 *)in_pData;
			in_pData += 4;
			pItems[ i ].m_Loop = *(AkInt16 *)in_pData;
			in_pData += 2;
			pItems[ i ].m_Weight = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pItems[ i ].m_wAvoidRepeatCount = *(AkUInt16 *)in_pData;
			in_pData += 2;
			pItems[ i ].m_bIsUsingWeight = *in_pData++ != 0;
			pItems[ i ].m_bIsShuffle = *in_pData++ != 0;
		}
		SetPlayList( pItems );
		AK::MemoryMgr::Free( g_DefaultPoolId, pItems );
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}
