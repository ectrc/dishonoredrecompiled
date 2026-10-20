/*=============================================================================
	AkRSIterator.cpp - CAkRSSub, RSStackItem, AkRSIterator (akrsiterator.cpp, 2012 0x96d0a0..0x96e290; the RSStackItem
	array's GrowArray 0x96d190 / AddLast 0x96d500 are this tree's AkArray template).

	DISHONORED(port): agent IH4 (wave 26).
	Deviations (named):
	  * RSStackItem::Clear (destroy the local selection state of a continuous group), PopLast (Clear + pop) and Select
	    (the RSType switch between SelectSequentially and SelectRandomly) are the bodies retail inlines at every use;
	    they are functions here.
	  * Init keeps retail's behaviour of not moving to the parent group when a group's selection ends inside its loop
	    (it pops the stack item and selects again from the same node), exactly as 0x96def0 does.
	  * SelectRandomly uses the CRT rand() as retail.
=============================================================================*/
#include "AkRSIterator.h"
#include <stdlib.h>

// ------------------------------------------------------------------------------------------------------------
// CAkRSSub
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x96d560, 2013 0x969d40: the global selection state and the children go. */
CAkRSSub::~CAkRSSub()
{
	Clear();
	for( CAkRSNode ** it = m_listChildren.m_pItems; it != m_listChildren.m_pItemsEnd; ++it )
	{
		if( *it )
		{
			( *it )->~CAkRSNode();
			AK::MemoryMgr::Free( g_DefaultPoolId, *it );
		}
	}
	m_listChildren.Term();
}

/** 2012 0x96d0a0. */
void CAkRSSub::Clear()
{
	if( m_pGlobalCntrBaseInfo )
	{
		m_pGlobalCntrBaseInfo->Destroy();
		m_pGlobalCntrBaseInfo = NULL;
	}
}

/** 2012 0x96d0c0, 2013 0x9698a0: marks this group and its ancestors as leading to a segment. */
void CAkRSSub::WasSegmentLeafFound()
{
	if( !m_bHasSegmentLeaves && m_pParent )
	{
		static_cast<CAkRSSub *>( m_pParent )->WasSegmentLeafFound();
	}
	m_bHasSegmentLeaves = true;
}

/** 2012 0x96d260, 2013 0x969a40. */
AkUInt32 CAkRSSub::CalculateTotalWeight()
{
	AkUInt32 uTotal = 0;
	for( CAkRSNode ** it = m_listChildren.m_pItems; it != m_listChildren.m_pItemsEnd; ++it )
	{
		uTotal += ( *it )->m_Weight;
	}
	return uTotal;
}

/** 2012 0x96d280, 2013 0x969a60: the step (shared by every context) selection state, created on first use. */
CAkContainerBaseInfo * CAkRSSub::GetGlobalRSInfo()
{
	if( !m_pGlobalCntrBaseInfo && m_listChildren.Length() )
	{
		if( m_eRSType == RSType_StepSequence )
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequenceInfo ) );
			m_pGlobalCntrBaseInfo = pMem ? AkPlacementNew( pMem ) CAkSequenceInfo : NULL;
		}
		else if( m_eRSType == RSType_StepRandom )
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRandomInfo ) );
			CAkRandomInfo * pRandomInfo = pMem ? AkPlacementNew( pMem ) CAkRandomInfo( (AkUInt16)m_listChildren.Length() ) : NULL;
			m_pGlobalCntrBaseInfo = pRandomInfo;
			if( pRandomInfo && pRandomInfo->Init( m_wAvoidRepeatCount ) != AK_Success )
			{
				m_pGlobalCntrBaseInfo->Destroy();
				m_pGlobalCntrBaseInfo = NULL;
			}
			if( m_pGlobalCntrBaseInfo && m_bIsUsingWeight )
			{
				CAkRandomInfo * pInfo = static_cast<CAkRandomInfo *>( m_pGlobalCntrBaseInfo );
				pInfo->m_ulRemainingWeight = CalculateTotalWeight();
				pInfo->m_ulTotalWeight = pInfo->m_ulRemainingWeight;
			}
		}
	}
	return m_pGlobalCntrBaseInfo;
}

// ------------------------------------------------------------------------------------------------------------
// RSStackItem
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x96d360, 2013 0x969b40: a continuous group gets its own selection state; a step group uses the global one (infinite loop). */
AKRESULT RSStackItem::Init( CAkRSSub * in_pSub )
{
	pRSNode = in_pSub;
	m_Loop.bIsEnabled = 1;
	m_Loop.bIsInfinite = in_pSub->m_Loop == 0;
	m_Loop.lLoopCount = in_pSub->m_Loop;
	switch( in_pSub->m_eRSType )
	{
	case RSType_ContinuousSequence:
		if( in_pSub->m_listChildren.Length() )
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequenceInfo ) );
			pLocalRSInfo = pMem ? AkPlacementNew( pMem ) CAkSequenceInfo : NULL;
		}
		return pLocalRSInfo ? AK_Success : AK_Fail;
	case RSType_StepSequence:
	case RSType_StepRandom:
		m_Loop.bIsInfinite = 1;
		return AK_Success;
	case RSType_ContinuousRandom:
		{
			if( in_pSub->m_listChildren.Length() )
			{
				void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRandomInfo ) );
				pLocalRSInfo = pMem ? AkPlacementNew( pMem ) CAkRandomInfo( (AkUInt16)in_pSub->m_listChildren.Length() ) : NULL;
			}
			CAkRandomInfo * pRandomInfo = static_cast<CAkRandomInfo *>( pLocalRSInfo );
			if( !pRandomInfo )
			{
				return AK_Fail;
			}
			if( pRandomInfo->Init( in_pSub->m_wAvoidRepeatCount ) != AK_Success )
			{
				pLocalRSInfo->Destroy();
				pLocalRSInfo = NULL;
				return AK_Fail;
			}
			if( in_pSub->m_bIsUsingWeight )
			{
				pRandomInfo->m_ulRemainingWeight = in_pSub->CalculateTotalWeight();
				pRandomInfo->m_ulTotalWeight = pRandomInfo->m_ulRemainingWeight;
			}
			return AK_Success;
		}
	default:
		return AK_Fail;
	}
}

/** Inlined in retail wherever a stack item is dropped: a continuous group's own state is destroyed. */
void RSStackItem::Clear()
{
	if( ( pRSNode->m_eRSType == RSType_ContinuousSequence || pRSNode->m_eRSType == RSType_ContinuousRandom ) && pLocalRSInfo )
	{
		pLocalRSInfo->Destroy();
	}
}

// ------------------------------------------------------------------------------------------------------------
// AkRSIterator
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x96d5e0, 2013 0x969dc0. */
AkRSIterator::AkRSIterator( CAkMusicRanSeqCntr * in_pRSCntr )
	: m_pRSCntr( in_pRSCntr )
	, m_actualSegment( 0 )
	, m_LastSegmentPlayingID( 0 )
	, m_bIsSegmentValid( false )
	, m_uSegmentLoopCount( 0 )
{
	m_bDoSaveOriginalGlobalRSInfo = 1;
}

/** 2012 0x96d0e0, 2013 0x9698c0: a shuffled group skips played and blocked positions; else only blocked ones (with avoid-repeat). */
bool AkRSIterator::CanPlayPosition( CAkRSSub * in_pSub, CAkRandomInfo * in_pRandomInfo, AkUInt16 in_wPosition )
{
	if( in_pSub->m_bIsShuffle )
	{
		return !in_pRandomInfo->IsFlagSetPlayed( in_wPosition ) && !in_pRandomInfo->IsFlagBlocked( in_wPosition );
	}
	return !in_pSub->m_wAvoidRepeatCount || !in_pRandomInfo->IsFlagBlocked( in_wPosition );
}

/** 2012 0x96d150, 2013 0x969930. */
AKRESULT AkRSIterator::SetCurrentSegmentToNode( CAkRSNode * in_pNode )
{
	if( !in_pNode )
	{
		return AK_Fail;
	}
	m_actualSegment = static_cast<CAkRSSegment *>( in_pNode )->m_SegmentID;
	m_bIsSegmentValid = m_actualSegment != 0;
	m_LastSegmentPlayingID = in_pNode->m_ID;
	m_uSegmentLoopCount = in_pNode->m_Loop;
	return AK_Success;
}

/** Inlined in retail: the top stack item's continuous state is destroyed and the item popped. */
void AkRSIterator::PopLast()
{
	( m_stack.m_pItemsEnd - 1 )->Clear();
	--m_stack.m_pItemsEnd;
}

/** 2012 0x96d490, 2013 0x969c70: pops the groups whose loops are done, from in_pNode up; returns the node to select from (or NULL). */
CAkRSNode * AkRSIterator::PopObsoleteStackedItems( CAkRSNode * in_pNode )
{
	CAkRSNode * pNode = in_pNode;
	while( pNode )
	{
		const RSType eType = static_cast<CAkRSSub *>( pNode )->m_eRSType;
		if( eType == RSType_ContinuousSequence || eType == RSType_ContinuousRandom )
		{
			break;
		}
		RSStackItem & rLast = *( m_stack.m_pItemsEnd - 1 );
		if( !rLast.m_Loop.lLoopCount )
		{
			break;
		}
		if( rLast.m_Loop.lLoopCount > 1 )
		{
			--rLast.m_Loop.lLoopCount;
			return pNode;
		}
		pNode = pNode->m_pParent;
		PopLast();
		if( !pNode )
		{
			return NULL;
		}
	}
	return pNode;
}

/** 2012 0x96d620: the global states touched during init are now owned by the playing context: drop the backups. */
void AkRSIterator::EndInit()
{
	for( MapStruct<CAkRSSub *, CAkContainerBaseInfo *> * it = m_arOriginalGlobalRSInfo.m_pItems; it != m_arOriginalGlobalRSInfo.m_pItemsEnd; ++it )
	{
		it->item->Destroy();
	}
	m_arOriginalGlobalRSInfo.m_pItemsEnd = m_arOriginalGlobalRSInfo.m_pItems;
	m_bDoSaveOriginalGlobalRSInfo = 0;
}

/** 2012 0x96d660, 2013 0x969e40. */
void AkRSIterator::FlushStack()
{
	for( RSStackItem * it = m_stack.m_pItems; it != m_stack.m_pItemsEnd; ++it )
	{
		it->Clear();
	}
	m_stack.m_pItemsEnd = m_stack.m_pItems;
}

/** 2012 0x96d6a0, 2013 0x969e80: the backed-up global states replace the ones init advanced. */
void AkRSIterator::RevertGlobalRSInfo()
{
	for( MapStruct<CAkRSSub *, CAkContainerBaseInfo *> * it = m_arOriginalGlobalRSInfo.m_pItems; it != m_arOriginalGlobalRSInfo.m_pItemsEnd; ++it )
	{
		it->key->m_pGlobalCntrBaseInfo->Destroy();
		it->key->m_pGlobalCntrBaseInfo = it->item;
	}
	m_arOriginalGlobalRSInfo.m_pItemsEnd = m_arOriginalGlobalRSInfo.m_pItems;
}

/** 2012 0x96d6e0, 2013 0x969ec0. */
void AkRSIterator::Term()
{
	FlushStack();
	m_stack.Term();
	for( MapStruct<CAkRSSub *, CAkContainerBaseInfo *> * it = m_arOriginalGlobalRSInfo.m_pItems; it != m_arOriginalGlobalRSInfo.m_pItemsEnd; ++it )
	{
		it->item->Destroy();
	}
	m_arOriginalGlobalRSInfo.Term();
}

/** 2012 0x96d750, 2013 0x969f30: the path from in_pNode to the playlist item, depth first. */
AKRESULT AkRSIterator::FindAndSelect( CAkRSNode * in_pNode, AkUniqueID in_playlistElementID, AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> & io_jmpList, bool & io_bFound )
{
	CAkRSNode ** ppSlot = io_jmpList.AddLast();
	if( !ppSlot )
	{
		return AK_Fail;
	}
	*ppSlot = in_pNode;
	if( in_pNode->m_ID == in_playlistElementID )
	{
		io_bFound = true;
		return AK_Success;
	}
	if( !in_pNode->IsSegment() )
	{
		CAkRSSub * pSub = static_cast<CAkRSSub *>( in_pNode );
		for( CAkRSNode ** it = pSub->m_listChildren.m_pItems; !io_bFound && it != pSub->m_listChildren.m_pItemsEnd; ++it )
		{
			const AKRESULT eResult = FindAndSelect( *it, in_playlistElementID, io_jmpList, io_bFound );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	if( !io_bFound )
	{
		--io_jmpList.m_pItemsEnd;
	}
	return AK_Success;
}

/** 2012 0x96d810, 2013 0x969ff0. */
AKRESULT AkRSIterator::StackItem( CAkRSSub * in_pSub )
{
	RSStackItem item;
	if( item.Init( in_pSub ) == AK_Success )
	{
		RSStackItem * pItem = m_stack.AddLast();
		if( pItem )
		{
			*pItem = item;
			return AK_Success;
		}
	}
	item.Clear();
	return AK_Fail;
}

/** 2012 0x96d8a0, 2013 0x96a080: the first change of a global selection state during init is backed up (a clone). */
void AkRSIterator::SaveOriginalGlobalRSInfo( CAkRSSub * in_pSub, CAkContainerBaseInfo * in_pRSInfo )
{
	if( m_arOriginalGlobalRSInfo.Exists( in_pSub ) )
	{
		return;
	}
	CAkContainerBaseInfo * pClone = in_pRSInfo->Clone( (AkUInt16)in_pSub->m_listChildren.Length() );
	if( pClone && !m_arOriginalGlobalRSInfo.Set( in_pSub, pClone ) )
	{
		pClone->Destroy();
	}
}

/** 2012 0x96d900, 2013 0x96a0e0: the sequence state is set as if in_pForcedNode were just chosen. */
void AkRSIterator::ForceSelectSequentially( CAkRSNode * in_pForcedNode )
{
	CAkRSSub * pParent = static_cast<CAkRSSub *>( in_pForcedNode->m_pParent );
	AkUInt16 wPosition = 0;
	for( CAkRSNode ** it = pParent->m_listChildren.m_pItems; it != pParent->m_listChildren.m_pItemsEnd && *it != in_pForcedNode; ++it )
	{
		++wPosition;
	}
	RSStackItem & rLast = *( m_stack.m_pItemsEnd - 1 );
	CAkContainerBaseInfo * pRSInfo;
	if( rLast.pRSNode->m_eRSType == RSType_StepSequence || rLast.pRSNode->m_eRSType == RSType_StepRandom )
	{
		pRSInfo = rLast.pRSNode->GetGlobalRSInfo();
	}
	else
	{
		pRSInfo = rLast.pLocalRSInfo;
	}
	if( pRSInfo )
	{
		CAkRSSub * pSub = ( m_stack.m_pItemsEnd - 1 )->pRSNode;
		if( m_bDoSaveOriginalGlobalRSInfo && pRSInfo == pSub->m_pGlobalCntrBaseInfo )
		{
			SaveOriginalGlobalRSInfo( pSub, pRSInfo );
		}
		static_cast<CAkSequenceInfo *>( pRSInfo )->m_i16LastPositionChosen = (AkInt16)wPosition;
	}
}

/** 2012 0x96d970, 2013 0x96a150: the next position; at the end of the list a loop is consumed (out_bIsEnd when none is left). */
AkUInt16 AkRSIterator::SelectSequentially( RSStackItem & in_rStackItem, bool & out_bIsEnd )
{
	out_bIsEnd = false;
	CAkSequenceInfo * pSeqInfo;
	if( in_rStackItem.pRSNode->m_eRSType == RSType_StepSequence || in_rStackItem.pRSNode->m_eRSType == RSType_StepRandom )
	{
		pSeqInfo = static_cast<CAkSequenceInfo *>( in_rStackItem.pRSNode->GetGlobalRSInfo() );
	}
	else
	{
		pSeqInfo = static_cast<CAkSequenceInfo *>( in_rStackItem.pLocalRSInfo );
	}
	if( !pSeqInfo )
	{
		out_bIsEnd = true;
		return 0;
	}
	CAkRSSub * pSub = in_rStackItem.pRSNode;
	if( m_bDoSaveOriginalGlobalRSInfo && pSeqInfo == pSub->m_pGlobalCntrBaseInfo )
	{
		SaveOriginalGlobalRSInfo( pSub, pSeqInfo );
	}
	const AkUInt16 wLast = (AkUInt16)pSeqInfo->m_i16LastPositionChosen;
	if( (AkUInt32)wLast + 1 == pSub->m_listChildren.Length() )
	{
		pSeqInfo->m_i16LastPositionChosen = 0;
		if( !in_rStackItem.m_Loop.bIsEnabled || ( !in_rStackItem.m_Loop.bIsInfinite && !--in_rStackItem.m_Loop.lLoopCount ) )
		{
			out_bIsEnd = true;
			return 0;
		}
	}
	else
	{
		pSeqInfo->m_i16LastPositionChosen = (AkInt16)( wLast + 1 );
	}
	return (AkUInt16)pSeqInfo->m_i16LastPositionChosen;
}

/** 2012 0x96da30, 2013 0x96a210: the random state after choosing in_wPosition (played flags, avoid-repeat window, weights). */
void AkRSIterator::UpdateRandomItem( CAkRSSub * in_pSub, AkUInt16 in_wPosition, AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> * in_pRSList, CAkRandomInfo * in_pRanInfo )
{
	if( m_bDoSaveOriginalGlobalRSInfo && in_pRanInfo == in_pSub->m_pGlobalCntrBaseInfo )
	{
		SaveOriginalGlobalRSInfo( in_pSub, in_pRanInfo );
	}
	if( in_pSub->m_bIsShuffle )
	{
		const AkUInt16 wBlockCount = in_pSub->m_wAvoidRepeatCount ? in_pSub->m_wAvoidRepeatCount : 1;
		in_pRanInfo->m_ulRemainingWeight -= ( *in_pRSList )[ in_wPosition ]->m_Weight;
		--in_pRanInfo->m_wRemainingItemsToPlay;
		--in_pRanInfo->m_wCounter;
		in_pRanInfo->FlagSetPlayed( in_wPosition );
		if( in_pRanInfo->m_listAvoid.AddLast( in_wPosition ) )
		{
			in_pRanInfo->FlagAsBlocked( in_wPosition );
			AkUInt32 uMaxBlocked = in_pRSList->Length() - 1;
			if( wBlockCount < uMaxBlocked )
			{
				uMaxBlocked = wBlockCount;
			}
			if( in_pRanInfo->m_listAvoid.Length() > uMaxBlocked )
			{
				const AkUInt16 wToBeRemoved = in_pRanInfo->m_listAvoid[ 0 ];
				in_pRanInfo->m_listAvoid.Erase( 0 );
				in_pRanInfo->FlagAsUnBlocked( wToBeRemoved );
				if( !in_pRanInfo->IsFlagSetPlayed( wToBeRemoved ) )
				{
					in_pRanInfo->m_ulRemainingWeight += ( *in_pRSList )[ wToBeRemoved ]->m_Weight;
					++in_pRanInfo->m_wRemainingItemsToPlay;
				}
			}
		}
		else
		{
			in_pRanInfo->m_wCounter = 0;
		}
	}
	else
	{
		if( !in_pRanInfo->IsFlagSetPlayed( in_wPosition ) )
		{
			in_pRanInfo->FlagSetPlayed( in_wPosition );
			--in_pRanInfo->m_wCounter;
		}
		if( in_pSub->m_wAvoidRepeatCount )
		{
			--in_pRanInfo->m_wRemainingItemsToPlay;
			if( in_pRanInfo->m_listAvoid.AddLast( in_wPosition ) )
			{
				in_pRanInfo->FlagAsBlocked( in_wPosition );
				in_pRanInfo->m_ulRemainingWeight -= ( *in_pRSList )[ in_wPosition ]->m_Weight;
				AkUInt32 uMaxBlocked = in_pRSList->Length() - 1;
				if( in_pSub->m_wAvoidRepeatCount < uMaxBlocked )
				{
					uMaxBlocked = in_pSub->m_wAvoidRepeatCount;
				}
				if( in_pRanInfo->m_listAvoid.Length() > uMaxBlocked )
				{
					const AkUInt16 wToBeRemoved = in_pRanInfo->m_listAvoid[ 0 ];
					in_pRanInfo->FlagAsUnBlocked( wToBeRemoved );
					in_pRanInfo->m_ulRemainingWeight += ( *in_pRSList )[ wToBeRemoved ]->m_Weight;
					++in_pRanInfo->m_wRemainingItemsToPlay;
					in_pRanInfo->m_listAvoid.Erase( 0 );
				}
			}
			else
			{
				in_pRanInfo->m_wCounter = 0;
			}
		}
	}
}

/** 2012 0x96dc40, 2013 0x96a420: a weighted or uniform pick among the playable positions; a new round resets the played flags. */
AkUInt16 AkRSIterator::SelectRandomly( RSStackItem & in_rStackItem, bool & out_bIsEnd )
{
	out_bIsEnd = false;
	CAkRandomInfo * pRanInfo;
	if( in_rStackItem.pRSNode->m_eRSType == RSType_StepSequence || in_rStackItem.pRSNode->m_eRSType == RSType_StepRandom )
	{
		pRanInfo = static_cast<CAkRandomInfo *>( in_rStackItem.pRSNode->GetGlobalRSInfo() );
	}
	else
	{
		pRanInfo = static_cast<CAkRandomInfo *>( in_rStackItem.pLocalRSInfo );
	}
	if( !pRanInfo )
	{
		out_bIsEnd = true;
		return 0;
	}
	CAkRSSub * pSub = in_rStackItem.pRSNode;
	AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> & rList = pSub->m_listChildren;
	if( !pRanInfo->m_wCounter )
	{
		if( !in_rStackItem.m_Loop.bIsEnabled || ( !in_rStackItem.m_Loop.bIsInfinite && !--in_rStackItem.m_Loop.lLoopCount ) )
		{
			out_bIsEnd = true;
			return 0;
		}
		pRanInfo->m_wCounter = (AkUInt16)rList.Length();
		pRanInfo->ResetFlagsPlayed( rList.Length() );
		pRanInfo->m_ulRemainingWeight = pRanInfo->m_ulTotalWeight;
		for( AkUInt16 * it = pRanInfo->m_listAvoid.m_pItems; it != pRanInfo->m_listAvoid.m_pItemsEnd; ++it )
		{
			pRanInfo->m_ulRemainingWeight -= rList[ *it ]->m_Weight;
		}
		pRanInfo->m_wRemainingItemsToPlay -= (AkUInt16)pRanInfo->m_listAvoid.Length();
	}
	AkUInt16 wPosition = 0;
	if( pSub->m_bIsUsingWeight )
	{
		if( !pRanInfo->m_ulRemainingWeight )
		{
			pRanInfo->m_wCounter = 0;
			return 0;
		}
		const AkInt32 iRandomValue = pRanInfo->GetRandomValue();
		AkInt32 iSum = -1;
		if( iRandomValue > -1 )
		{
			do
			{
				if( CanPlayPosition( pSub, pRanInfo, wPosition ) )
				{
					iSum += rList[ wPosition ]->m_Weight;
				}
				++wPosition;
			}
			while( iSum < iRandomValue );
		}
	}
	else
	{
		if( !pRanInfo->m_wRemainingItemsToPlay )
		{
			pRanInfo->m_wCounter = 0;
			return 0;
		}
		const AkInt32 iRandomIndex = (AkUInt16)( rand() % pRanInfo->m_wRemainingItemsToPlay );
		AkInt32 iValidCount = -1;
		do
		{
			if( CanPlayPosition( pSub, pRanInfo, wPosition ) )
			{
				++iValidCount;
			}
			++wPosition;
		}
		while( iValidCount < iRandomIndex );
	}
	--wPosition;
	UpdateRandomItem( pSub, wPosition, &rList, pRanInfo );
	return wPosition;
}

/** 2012 0x96ddd0, 2013 0x96a5b0: the random state is reset as if a new round started with in_pForcedNode chosen. */
void AkRSIterator::ForceSelectRandomly( CAkRSNode * in_pForcedNode )
{
	CAkRSSub * pSub = static_cast<CAkRSSub *>( in_pForcedNode->m_pParent );
	AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> & rList = pSub->m_listChildren;
	AkUInt16 wPosition = 0;
	for( CAkRSNode ** it = rList.m_pItems; it != rList.m_pItemsEnd && *it != in_pForcedNode; ++it )
	{
		++wPosition;
	}
	RSStackItem & rLast = *( m_stack.m_pItemsEnd - 1 );
	CAkRandomInfo * pRanInfo;
	if( rLast.pRSNode->m_eRSType == RSType_StepSequence || rLast.pRSNode->m_eRSType == RSType_StepRandom )
	{
		pRanInfo = static_cast<CAkRandomInfo *>( rLast.pRSNode->GetGlobalRSInfo() );
	}
	else
	{
		pRanInfo = static_cast<CAkRandomInfo *>( rLast.pLocalRSInfo );
	}
	if( !pRanInfo )
	{
		return;
	}
	pRanInfo->FlagAsUnBlocked( wPosition );
	for( AkUInt32 i = 0; i < pRanInfo->m_listAvoid.Length(); ++i )
	{
		if( pRanInfo->m_listAvoid[ i ] == wPosition )
		{
			pRanInfo->m_listAvoid.Erase( i );
			break;
		}
	}
	pRanInfo->m_wCounter = (AkUInt16)rList.Length();
	pRanInfo->ResetFlagsPlayed( rList.Length() );
	pRanInfo->m_ulRemainingWeight = pRanInfo->m_ulTotalWeight;
	for( AkUInt16 * it = pRanInfo->m_listAvoid.m_pItems; it != pRanInfo->m_listAvoid.m_pItemsEnd; ++it )
	{
		pRanInfo->m_ulRemainingWeight -= rList[ *it ]->m_Weight;
	}
	pRanInfo->m_wRemainingItemsToPlay -= (AkUInt16)pRanInfo->m_listAvoid.Length();
	UpdateRandomItem( pSub, wPosition, &rList, pRanInfo );
}

/** Inlined in retail (Init / JumpNext / JumpNextInternal): the RSType switch of the top stack item. */
AkUInt16 AkRSIterator::Select( RSStackItem & in_rStackItem, bool & out_bIsEnd )
{
	switch( in_rStackItem.pRSNode->m_eRSType )
	{
	case RSType_ContinuousSequence:
	case RSType_StepSequence:
		return SelectSequentially( in_rStackItem, out_bIsEnd );
	case RSType_ContinuousRandom:
	case RSType_StepRandom:
		return SelectRandomly( in_rStackItem, out_bIsEnd );
	default:
		return 0;
	}
}

/** 2012 0x96def0, 2013 0x96a6d0: walks down from the root to the first segment. */
AKRESULT AkRSIterator::Init()
{
	m_bDoSaveOriginalGlobalRSInfo = 1;
	m_actualSegment = 0;
	m_bIsSegmentValid = true;
	if( !m_pRSCntr )
	{
		return AK_Fail;
	}
	CAkRSNode * pNode = &m_pRSCntr->m_playListRoot;
	while( !pNode->IsSegment() )
	{
		CAkRSSub * pSub = static_cast<CAkRSSub *>( pNode );
		if( pSub->m_listChildren.Length() && pSub->m_bHasSegmentLeaves )
		{
			if( StackItem( pSub ) != AK_Success )
			{
				Term();
				return AK_Fail;
			}
		}
		else
		{
			pNode = pNode->m_pParent;
			if( !pNode )
			{
				return AK_Fail;
			}
		}
		bool bIsEnd = false;
		AkUInt16 wIndex;
		for( ;; )
		{
			wIndex = Select( *( m_stack.m_pItemsEnd - 1 ), bIsEnd );
			if( !bIsEnd )
			{
				break;
			}
			CAkRSNode * pParent = pNode->m_pParent;
			PopLast();
			if( !pParent )
			{
				return AK_Fail;
			}
		}
		pNode = static_cast<CAkRSSub *>( pNode )->m_listChildren[ wIndex ];
		if( !pNode )
		{
			return AK_Fail;
		}
	}
	m_actualSegment = static_cast<CAkRSSegment *>( pNode )->m_SegmentID;
	m_bIsSegmentValid = m_actualSegment != 0;
	m_LastSegmentPlayingID = pNode->m_ID;
	m_uSegmentLoopCount = pNode->m_Loop;
	return AK_Success;
}

/** 2012 0x96e030, 2013 0x96a810: the current segment's own loop first, then the next segment of the playlist (or the end). */
void AkRSIterator::JumpNext()
{
	if( m_uSegmentLoopCount > 1 )
	{
		--m_uSegmentLoopCount;
		return;
	}
	if( !m_uSegmentLoopCount )
	{
		return;
	}
	if( !m_actualSegment )
	{
		m_bIsSegmentValid = false;
		return;
	}
	m_actualSegment = 0;
	m_bIsSegmentValid = true;
	if( m_stack.m_pItemsEnd == m_stack.m_pItems )
	{
		return;
	}
	bool bIsEnd = true;
	CAkRSNode * pNode = PopObsoleteStackedItems( ( m_stack.m_pItemsEnd - 1 )->pRSNode );
	while( pNode )
	{
		const AkUInt16 wIndex = Select( *( m_stack.m_pItemsEnd - 1 ), bIsEnd );
		if( bIsEnd )
		{
			CAkRSNode * pParent = pNode->m_pParent;
			PopLast();
			pNode = PopObsoleteStackedItems( pParent );
			continue;
		}
		pNode = static_cast<CAkRSSub *>( pNode )->m_listChildren[ wIndex ];
		if( pNode->IsSegment() )
		{
			break;
		}
		CAkRSSub * pSub = static_cast<CAkRSSub *>( pNode );
		if( pSub->m_listChildren.Length() && pSub->m_bHasSegmentLeaves )
		{
			if( StackItem( pSub ) != AK_Success )
			{
				Term();
				return;
			}
		}
		else
		{
			pNode = pNode->m_pParent;
		}
		bIsEnd = true;
	}
	SetCurrentSegmentToNode( pNode );
}

/** 2012 0x96e170, 2013 0x96a950: from the top of the stack to the next segment (used after JumpTo stacked a path). */
AKRESULT AkRSIterator::JumpNextInternal()
{
	m_actualSegment = 0;
	m_bIsSegmentValid = true;
	if( m_stack.m_pItemsEnd == m_stack.m_pItems )
	{
		return AK_Fail;
	}
	CAkRSNode * pNode = ( m_stack.m_pItemsEnd - 1 )->pRSNode;
	for( ;; )
	{
		bool bIsEnd = true;
		AkUInt16 wIndex;
		for( ;; )
		{
			if( !pNode )
			{
				return AK_Fail;
			}
			wIndex = Select( *( m_stack.m_pItemsEnd - 1 ), bIsEnd );
			if( !bIsEnd )
			{
				break;
			}
			CAkRSNode * pParent = pNode->m_pParent;
			PopLast();
			pNode = PopObsoleteStackedItems( pParent );
		}
		pNode = static_cast<CAkRSSub *>( pNode )->m_listChildren[ wIndex ];
		if( pNode->IsSegment() )
		{
			break;
		}
		if( StackItem( static_cast<CAkRSSub *>( pNode ) ) != AK_Success )
		{
			Term();
			return AK_Fail;
		}
	}
	m_actualSegment = static_cast<CAkRSSegment *>( pNode )->m_SegmentID;
	m_bIsSegmentValid = m_actualSegment != 0;
	m_LastSegmentPlayingID = pNode->m_ID;
	m_uSegmentLoopCount = pNode->m_Loop;
	return AK_Success;
}

/** 2012 0x96e290, 2013 0x96aa70: the global states are reverted, the path to the item is stacked and its selection forced. */
AKRESULT AkRSIterator::JumpTo( AkUniqueID in_playlistElementID )
{
	RevertGlobalRSInfo();
	FlushStack();
	CAkRSNode * pRoot = &m_pRSCntr->m_playListRoot;
	m_actualSegment = 0;
	m_bIsSegmentValid = true;
	AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> jumpList;
	bool bFound = false;
	const AKRESULT eFindResult = FindAndSelect( pRoot, in_playlistElementID, jumpList, bFound );
	AKRESULT eResult = AK_Fail;
	if( bFound && eFindResult == AK_Success )
	{
		CAkRSNode * pNode = NULL;
		CAkRSNode ** it = jumpList.m_pItems;
		while( it != jumpList.m_pItemsEnd )
		{
			pNode = *it;
			if( pNode->IsSegment() )
			{
				break;
			}
			if( StackItem( static_cast<CAkRSSub *>( pNode ) ) != AK_Success )
			{
				Term();
				jumpList.Term();
				return AK_Fail;
			}
			if( ++it == jumpList.m_pItemsEnd )
			{
				eResult = JumpNextInternal();
				jumpList.Term();
				return eResult;
			}
			switch( static_cast<CAkRSSub *>( ( *it )->m_pParent )->m_eRSType )
			{
			case RSType_ContinuousSequence:
			case RSType_StepSequence:
				ForceSelectSequentially( *it );
				break;
			case RSType_ContinuousRandom:
			case RSType_StepRandom:
				ForceSelectRandomly( *it );
				break;
			default:
				break;
			}
		}
		eResult = SetCurrentSegmentToNode( pNode );
	}
	jumpList.Term();
	return eResult;
}
