/*=============================================================================
	AkMultiPlayNode.cpp - continuation lists (akcontinuationlist.cpp, 2012 0x94d2e0..0x94d490), the multi-play node that
	switch and layer containers share (akmultiplaynode.cpp, 0x955220..0x955760), the layer container's Play
	(aklayercntr.cpp, 0x951560..0x9517e0) and CAkParameterNode::PlayAndContinueAlternate (akparameternode.cpp,
	0x938860); 2013 matched by bytes (ih1_inventory.csv).

	DISHONORED(port): agent IH3, fork C (wave 26).
	Deviations (named):
	  * CAkSmartPtr<CAkContinuationList> is a raw pointer with the AddRef / Release retail's inlined smart pointer does
	    (ContParams and CAkActionPlayAndContinue keep IH2's raw-pointer layout, same 4 bytes).
	  * AddMultiplayItem: the 2012 binary calls an identical-code-folded function (symbol
	    FResponseLogic_PlayerThreats::GetDisAIResponse) when the caller's continuous params have no list; read here as
	    attaching a new CAkContinuationList, the only body that fits the branch.
	  * AkTimeConv::MillisecondsToSamples (an SDK header inline) is the file-static MillisecondsToSamples.
=============================================================================*/
#include "AkContainers.h"
#include "AkContinuousPBI.h"
#include "AkActionPlayAndContinue.h"
#include "AkPath.h"

static AkInt32 MillisecondsToSamples( AkReal32 in_fMs )
{
	return (AkInt32)( in_fMs * (AkReal32)AkAudioLibSettings::g_pipelineCoreFrequency / 1000.f );
}

// ------------------------------------------------------------------------------------------------------------
// CAkContinuationList / CAkContinueListItem
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94d2e0. */
void CAkContinuationList::AddRef()
{
	++m_iRefCount;
}

/** 2012 0x94d2f0. */
CAkContinuationList * CAkContinuationList::Create()
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkContinuationList ) );
	return pMem ? AkPlacementNew( pMem ) CAkContinuationList() : NULL;
}

/** 2012 0x94d420: a multi-play item's alternate list loses this reference. */
void CAkContinuationList::Term()
{
	for( CAkContinueListItem * p = m_listItems.m_pItems; p != m_listItems.m_pItemsEnd; ++p )
	{
		if( p->m_pMultiPlayNode && p->m_pAlternateContList )
		{
			p->m_pMultiPlayNode->ContUnrefList( p->m_pAlternateContList );
		}
	}
	m_listItems.Term();
}

/** 2012 0x94d490. */
void CAkContinuationList::Release()
{
	if( m_iRefCount-- == 1 )
	{
		Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
}

/** 2012 0x94d320. */
CAkContinueListItem::CAkContinueListItem()
	: m_pContainer( NULL )
	, m_pContainerInfo( NULL )
	, m_pMultiPlayNode( NULL )
	, m_pAlternateContList( NULL )
{
	m_LoopingInfo.lLoopCount = 0;
	m_LoopingInfo.bIsEnabled = 0;
	m_LoopingInfo.bIsInfinite = 0;
}

CAkContinueListItem::CAkContinueListItem( const CAkContinueListItem & in_listItem )
	: m_pContainer( NULL )
	, m_pContainerInfo( NULL )
	, m_pMultiPlayNode( NULL )
	, m_pAlternateContList( NULL )
{
	m_LoopingInfo.lLoopCount = 0;
	m_LoopingInfo.bIsEnabled = 0;
	m_LoopingInfo.bIsInfinite = 0;
	*this = in_listItem;
}

/** 2012 0x94d340. */
CAkContinueListItem::~CAkContinueListItem()
{
	if( m_pContainerInfo )
	{
		m_pContainerInfo->Destroy();
		m_pContainerInfo = NULL;
	}
	if( m_pMultiPlayNode )
	{
		m_pMultiPlayNode->Release();
		m_pMultiPlayNode = NULL;
	}
	if( m_pContainer )
	{
		m_pContainer->Release();
		m_pContainer = NULL;
	}
}

/** 2012 0x94d390: the container info is cloned, not shared. */
const CAkContinueListItem & CAkContinueListItem::operator=( const CAkContinueListItem & in_listItem )
{
	if( &in_listItem != this )
	{
		CAkRanSeqCntr * pContainer = in_listItem.m_pContainer;
		if( pContainer )
		{
			pContainer->AddRef();
		}
		if( m_pContainer )
		{
			m_pContainer->Release();
		}
		m_pContainer = pContainer;
		m_pContainerInfo = in_listItem.m_pContainerInfo ? in_listItem.m_pContainerInfo->Clone( (AkUInt16)m_pContainer->GetPlaylistLength() ) : NULL;
		m_LoopingInfo = in_listItem.m_LoopingInfo;
		CAkMultiPlayNode * pMultiPlayNode = in_listItem.m_pMultiPlayNode;
		if( pMultiPlayNode )
		{
			pMultiPlayNode->AddRef();
		}
		if( m_pMultiPlayNode )
		{
			m_pMultiPlayNode->Release();
		}
		m_pMultiPlayNode = pMultiPlayNode;
		m_pAlternateContList = in_listItem.m_pAlternateContList;
	}
	return *this;
}

/** Pops the last item (the inlined AkArray::RemoveLast of the continuation walks). */
void CAkContinuationList::RemoveLast()
{
	( m_listItems.m_pItemsEnd - 1 )->~CAkContinueListItem();
	--m_listItems.m_pItemsEnd;
}

// ------------------------------------------------------------------------------------------------------------
// AkContParamsAndPath / SafeContinuationList
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9554c0: a copy of the caller's continuous params with its own (empty) list; the path gets one more user. */
AkContParamsAndPath::AkContParamsAndPath( ContParams * in_pFrom )
{
	m_continuousParams.pPlayStopTransition = in_pFrom->pPlayStopTransition;
	m_continuousParams.pPauseResumeTransition = in_pFrom->pPauseResumeTransition;
	m_continuousParams.pPathInfo = in_pFrom->pPathInfo;
	m_continuousParams.bIsPlayStopTransitionFading = in_pFrom->bIsPlayStopTransitionFading;
	m_continuousParams.bIsPauseResumeTransitionFading = in_pFrom->bIsPauseResumeTransitionFading;
	m_continuousParams.spContList = NULL;
	m_continuousParams.ulPauseCount = in_pFrom->ulPauseCount;
	if( g_pPathManager && m_continuousParams.pPathInfo->pPBPath )
	{
		g_pPathManager->AddPotentialUser( m_continuousParams.pPathInfo->pPBPath );
	}
}

/** 2012 0x9552f0. */
AkContParamsAndPath::~AkContParamsAndPath()
{
	if( g_pPathManager && m_continuousParams.pPathInfo->pPBPath )
	{
		g_pPathManager->RemovePotentialUser( m_continuousParams.pPathInfo->pPBPath );
	}
	if( m_continuousParams.spContList )
	{
		m_continuousParams.spContList->Release();
		m_continuousParams.spContList = NULL;
	}
}

/** 2012 0x955760: keeps the caller's list alive (and referenced by the node) while each child starts its own. */
SafeContinuationList::SafeContinuationList( AkPBIParams & in_rPBIParams, CAkMultiPlayNode * in_pMultiPlayNode )
	: m_spBackupContinuationList( NULL )
{
	if( in_rPBIParams.eType != AkPBIParams::PBI )
	{
		CAkContinuationList * pList = in_rPBIParams.pContinuousParams->spContList;
		if( pList )
		{
			pList->AddRef();
			m_spBackupContinuationList = pList;
			in_pMultiPlayNode->ContRefList( m_spBackupContinuationList );
		}
	}
}

SafeContinuationList::~SafeContinuationList()
{
	if( m_spBackupContinuationList )
	{
		m_spBackupContinuationList->Release();
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkMultiPlayNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x955330. */
CAkMultiPlayNode::CAkMultiPlayNode( AkUniqueID in_ulID )
	: CAkContainerBase( in_ulID )
{
}

/** 2012 0x955220. */
CAkMultiPlayNode::~CAkMultiPlayNode()
{
}

/** 2012 0x955390. */
AKRESULT CAkMultiPlayNode::Init()
{
	AddToIndex();
	return AK_Success;
}

/** 2012 0x955510. */
void CAkMultiPlayNode::Term()
{
	m_listContParameters.Term();
}

bool CAkMultiPlayNode::IsContinuousPlayback()
{
	return false;
}

/** 2012 0x9553a0: the last child to finish a continuous multi-play hands the caller's list back (when the node does not
    keep playing continuously itself). */
void CAkMultiPlayNode::ContGetList( CAkContinuationList * in_pList, CAkContinuationList *& io_spList )
{
	ContParam * pParam = m_listContParameters.Exists( in_pList );
	if( !pParam )
	{
		return;
	}
	if( pParam->uRefCount > 1 )
	{
		--pParam->uRefCount;
		return;
	}
	if( !IsContinuousPlayback() )
	{
		CAkContinuationList * pList = pParam->pContList;
		if( pList )
		{
			pList->AddRef();
		}
		if( io_spList )
		{
			io_spList->Release();
		}
		io_spList = pList;
	}
	pParam->pContList->Release();
	m_listContParameters.Erase( (AkUInt32)( pParam - m_listContParameters.m_pItems ) );
}

/** 2012 0x955440. */
AKRESULT CAkMultiPlayNode::ContUnrefList( CAkContinuationList * in_pList )
{
	ContParam * pParam = m_listContParameters.Exists( in_pList );
	if( !pParam )
	{
		return AK_Success;
	}
	if( pParam->uRefCount > 1 )
	{
		--pParam->uRefCount;
		return AK_Success;
	}
	pParam->pContList->Release();
	m_listContParameters.Erase( (AkUInt32)( pParam - m_listContParameters.m_pItems ) );
	return AK_Success;
}

/** 2012 0x955550. */
AKRESULT CAkMultiPlayNode::ContRefList( CAkContinuationList * in_pList )
{
	ContParam * pParam = m_listContParameters.Exists( in_pList );
	if( !pParam )
	{
		pParam = m_listContParameters.AddNoSetKey( in_pList );
		if( !pParam )
		{
			return AK_Fail;
		}
		pParam->pContList = in_pList;
		pParam->uRefCount = 0;
	}
	if( !pParam->uRefCount )
	{
		in_pList->AddRef();
	}
	++pParam->uRefCount;
	return AK_Success;
}

/** 2012 0x9555c0: the child's own list starts with this node as a multi-play item pointing back at the caller's list. */
AKRESULT CAkMultiPlayNode::AddMultiplayItem( AkContParamsAndPath & in_rContParams, AkPBIParams & in_rParams, SafeContinuationList & in_rSafeContList )
{
	CAkContinueListItem * pItem = in_rContParams.Get().spContList->m_listItems.AddLast();
	if( pItem )
	{
		if( !in_rParams.pContinuousParams->spContList )
		{
			in_rParams.pContinuousParams->spContList = CAkContinuationList::Create();
		}
		if( in_rParams.pContinuousParams->spContList )
		{
			pItem->m_pAlternateContList = in_rSafeContList.Get();
			AddRef();
			if( pItem->m_pMultiPlayNode )
			{
				pItem->m_pMultiPlayNode->Release();
			}
			pItem->m_pMultiPlayNode = this;
			CAkContinuationList * pCallerList = in_rParams.pContinuousParams->spContList;
			if( pCallerList->m_listItems.IsEmpty() )
			{
				pItem->m_LoopingInfo.bIsEnabled = 0;
				pItem->m_LoopingInfo.bIsInfinite = 0;
				pItem->m_LoopingInfo.lLoopCount = 1;
			}
			else
			{
				pItem->m_LoopingInfo = pCallerList->m_listItems.m_pItems->m_LoopingInfo;
			}
			if( pItem->m_pAlternateContList )
			{
				ContRefList( pItem->m_pAlternateContList );
			}
			return AK_Success;
		}
	}
	if( in_rContParams.Get().spContList )
	{
		in_rContParams.Get().spContList->Release();
		in_rContParams.Get().spContList = NULL;
	}
	return AK_InsufficientMemory;
}

/** 2012 0x955690: no child started - continue the caller's pass from its list. */
AKRESULT CAkMultiPlayNode::PlayAndContinueAlternateMultiPlay( AkPBIParams & in_rPBIParams )
{
	AkContParamsAndPath continuousParams( in_rPBIParams.pContinuousParams );
	AKRESULT eResult = AK_Success;
	ContGetList( in_rPBIParams.pContinuousParams->spContList, continuousParams.Get().spContList );
	if( continuousParams.Get().spContList )
	{
		in_rPBIParams.pContinuousParams = &continuousParams.Get();
		eResult = PlayAndContinueAlternate( in_rPBIParams );
		if( eResult == AK_PartialSuccess )
		{
			eResult = AK_Success;
		}
	}
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNode::PlayAndContinueAlternate
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x938860: nothing played here - the next item of the innermost continuous container still on the list is
    scheduled as a PlayAndContinue action (after the container's delay, or the 10 240-sample minimum). */
AKRESULT CAkParameterNode::PlayAndContinueAlternate( AkPBIParams & in_rPBIParams )
{
	PlayHistory & rHistory = in_rPBIParams.playHistory;
	AKRESULT eResult = AK_Fail;
	while( rHistory.HistArray.uiArraySize )
	{
		const AkUInt32 uLast = rHistory.HistArray.uiArraySize - 1;
		if( uLast < 32 && rHistory.arrayIsContinuous.IsSet( uLast ) )
		{
			break;
		}
		rHistory.HistArray.uiArraySize = uLast;
	}
	CAkContinuationList *& rspContList = in_rPBIParams.pContinuousParams->spContList;
	if( rspContList->m_listItems.IsEmpty() )
	{
		eResult = AK_Fail;
	}
	else
	{
		while( !in_rPBIParams.pContinuousParams->spContList->m_listItems.IsEmpty() )
		{
			CAkContinuationList * pList = in_rPBIParams.pContinuousParams->spContList;
			CAkContinueListItem & rItem = *( pList->m_listItems.m_pItemsEnd - 1 );
			if( rItem.m_pMultiPlayNode )
			{
				rItem.m_pMultiPlayNode->ContGetList( rItem.m_pAlternateContList, in_rPBIParams.pContinuousParams->spContList );
				pList->RemoveLast();
				if( !in_rPBIParams.pContinuousParams->spContList )
				{
					eResult = AK_PartialSuccess;
					break;
				}
				continue;
			}
			AkUInt16 wPositionSelected = 0;
			AkUniqueID uSelectedNodeID;
			CAkParameterNodeBase * pNextNode = rItem.m_pContainer->GetNextToPlayContinuous( in_rPBIParams.pGameObj, wPositionSelected, uSelectedNodeID, rItem.m_pContainerInfo, rItem.m_LoopingInfo );
			if( !pNextNode )
			{
				while( rHistory.HistArray.uiArraySize-- != 1 )
				{
					const AkUInt32 uSize = rHistory.HistArray.uiArraySize;
					if( uSize - 1 < 32 && rHistory.arrayIsContinuous.IsSet( uSize - 1 ) )
					{
						break;
					}
				}
				pList->RemoveLast();
				continue;
			}
			rHistory.HistArray.aCntrHist[ rHistory.HistArray.uiArraySize - 1 ] = wPositionSelected;
			const AkUniqueID nextKey = pNextNode->key;
			pNextNode->Release();
			const AkTransitionMode eTransitionMode = rItem.m_pContainer->TransitionMode();
			const AkReal32 fTransitionTime = rItem.m_pContainer->TransitionTime( in_rPBIParams.pGameObj );
			if( nextKey )
			{
				CAkActionPlayAndContinue * pAction = CAkActionPlayAndContinue::Create( (AkActionType)AkActionType_PlayAndContinue, 0, in_rPBIParams.pContinuousParams->spContList );
				if( pAction )
				{
					pAction->m_ulPauseCount = in_rPBIParams.pContinuousParams->ulPauseCount;
					pAction->SetHistory( rHistory );
					pAction->SetElementID( nextKey );
					pAction->SetInstigator( in_rPBIParams.pInstigator );
					void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPendingAction ) );
					AkPendingAction * pPendingAction = pMem ? AkPlacementNew( pMem ) AkPendingAction( in_rPBIParams.pGameObj ) : NULL;
					if( pPendingAction )
					{
						pAction->SetSAInfo( in_rPBIParams.sequenceID );
						pAction->SetIsFirstPlay( in_rPBIParams.bIsFirst );
						pAction->SetInitialPlaybackState( in_rPBIParams.ePlaybackState );
						ContParams * pCont = in_rPBIParams.pContinuousParams;
						const AkInt32 iDelay = eTransitionMode == Transition_Delay ? MillisecondsToSamples( fTransitionTime ) : 10240;
						if( pAction->SetPlayStopTransition( pCont->pPlayStopTransition, pCont->bIsPlayStopTransitionFading, pPendingAction ) == AK_Success
							&& pAction->SetPauseResumeTransition( pCont->pPauseResumeTransition, pCont->bIsPauseResumeTransitionFading, pPendingAction ) == AK_Success )
						{
							pAction->SetPathInfo( pCont->pPathInfo );
							eResult = pAction->SetAkProp( AkPropID_DelayTime, iDelay, 0, 0 );
						}
						else
						{
							eResult = AK_Fail;
						}
						if( eResult == AK_Success )
						{
							pPendingAction->pAction = pAction;
							pPendingAction->UserParam = in_rPBIParams.userParams;
							AKDIAG( L"akdiag: continuous alternate: next 0x%08x of ranseq 0x%08x scheduled (transition mode %u, delay %d samples)", nextKey, rItem.m_pContainer->key, (AkUInt32)eTransitionMode, iDelay );
							g_pAudioMgr->EnqueueOrExecuteAction( pPendingAction );
						}
						else
						{
							pPendingAction->~AkPendingAction();
							AK::MemoryMgr::Free( g_DefaultPoolId, pPendingAction );
						}
					}
					pAction->Release();
				}
			}
			break;
		}
	}
	ContParams * pCont = in_rPBIParams.pContinuousParams;
	if( pCont->spContList && eResult != AK_Success && eResult != AK_PartialSuccess )
	{
		CAkContinuationList * pList = pCont->spContList;
		pCont->spContList = NULL;
		pList->Release();
	}
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkLayerCntr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x951720: the layers drop their association first. */
void CAkLayerCntr::RemoveChild( CAkParameterNodeBase * in_pChild )
{
	for( CAkLayer ** it = m_layers.m_pItems; it != m_layers.m_pItemsEnd; ++it )
	{
		( *it )->UnsetChildAssoc( in_pChild->key );
	}
	if( in_pChild->m_pParentNode == this )
	{
		in_pChild->Parent( NULL );
		m_mapChildId.Unset( in_pChild->key );
		Release();
	}
}

/** 2012 0x9517e0: every child plays; in a continuous pass each child after the first gets its own list headed by this
    node, and the caller's pass continues when the last of them ends. */
AKRESULT CAkLayerCntr::Play( AkPBIParams & in_rPBIParams )
{
	AKRESULT eResult = AK_Success;
	const bool bIsContinuousPlay = in_rPBIParams.eType != AkPBIParams::PBI;
	SafeContinuationList safeContList( in_rPBIParams, this );
	const AkUInt32 uNumChildren = m_mapChildId.Length();
	AkUInt32 ulNumSoundsStarted = 0;
	for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
	{
		CAkParameterNodeBase * pNode = *it;
		AkPBIParams params( in_rPBIParams );
		if( !bIsContinuousPlay )
		{
			eResult = pNode->Play( params );
		}
		else
		{
			AkContParamsAndPath continuousParams( params.pContinuousParams );
			if( uNumChildren == 1 )
			{
				ContGetList( params.pContinuousParams->spContList, continuousParams.Get().spContList );
				params.pContinuousParams = &continuousParams.Get();
				eResult = pNode->Play( params );
			}
			else
			{
				params.sequenceID = 0;
				continuousParams.Get().spContList = CAkContinuationList::Create();
				if( !continuousParams.Get().spContList )
				{
					eResult = AK_InsufficientMemory;
				}
				else
				{
					eResult = AddMultiplayItem( continuousParams, params, safeContList );
					if( eResult == AK_Success )
					{
						params.pContinuousParams = &continuousParams.Get();
						eResult = pNode->Play( params );
					}
				}
			}
		}
		AKDIAG( L"akdiag: layer 0x%08x child 0x%08x %s -> %d", key, pNode->key, bIsContinuousPlay ? L"continuous" : L"step", (AkInt32)eResult );
		if( eResult != AK_Success )
		{
			break;
		}
		++ulNumSoundsStarted;
	}
	if( bIsContinuousPlay )
	{
		if( !ulNumSoundsStarted )
		{
			eResult = PlayAndContinueAlternateMultiPlay( in_rPBIParams );
		}
		else if( safeContList.Get() )
		{
			eResult = ContUnrefList( safeContList.Get() );
		}
	}
	return eResult;
}
