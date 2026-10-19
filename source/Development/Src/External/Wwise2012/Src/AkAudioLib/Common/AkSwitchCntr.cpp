/*=============================================================================
	AkSwitchCntr.cpp - the playback half of the switch container (akswitchcntr.cpp, 2012 0x94f8e0..0x951480): Play
	with the game object's current switch (or state), the switch history per game object, the continuous instances a
	switch change restarts or stops (per AkSwitchNodeParams: first only, continue playback, play / stop on switch,
	fades) and game-sync preparation of the switch packages; 2013 matched by bytes (ih1_inventory.csv).

	DISHONORED(port): agent IH3, fork C (wave 26).
	Deviations (named): CAkRegisteredObj's inlined AddRef / Release (refcount at +84) are ++m_refCount and ReleaseGameObj;
	the CAkSwitchCntr's two extra bases (CAkSwitchAware, CAkPreparationAware) are reached by the
	compiler's this-adjustment instead of retail's hand-written -100 / -104 thunks (SetSwitch 0x951460, ModifyActiveState
	0x94fb50).
=============================================================================*/
#include "AkContainers.h"
#include "AkActivityChunk.h"
#include "AkPlayingMgr.h"

static void ReleaseGameObj( CAkRegisteredObj * in_pGameObj )
{
	if( --in_pGameObj->m_refCount <= 0 )
	{
		in_pGameObj->~CAkRegisteredObj();
		AK::MemoryMgr::Free( g_DefaultPoolId, in_pGameObj );
	}
}

/** 2012 0x94f8e0. */
bool CAkSwitchCntr::IsContinuousPlayback()
{
	return m_bIsContinuousValidation != 0;
}

/** 2012 0x94f950. */
void CAkSwitchCntr::ResumeContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID )
{
	for( SwitchContPlaybackList::Iterator it = m_listSwitchContPlayback.Begin(); it != m_listSwitchContPlayback.End(); ++it )
	{
		SwitchContPlaybackItem & rItem = *it;
		if( ( !in_pGameObj || in_pGameObj == rItem.GameObject ) && ( !in_PlayingID || rItem.UserParameters.m_PlayingID == in_PlayingID ) && rItem.ePlaybackState != PB_Playing )
		{
			rItem.ePlaybackState = PB_Playing;
		}
	}
}

/** 2012 0x94f990. */
void CAkSwitchCntr::PauseContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID )
{
	for( SwitchContPlaybackList::Iterator it = m_listSwitchContPlayback.Begin(); it != m_listSwitchContPlayback.End(); ++it )
	{
		SwitchContPlaybackItem & rItem = *it;
		if( ( !in_pGameObj || in_pGameObj == rItem.GameObject ) && ( !in_PlayingID || rItem.UserParameters.m_PlayingID == in_PlayingID ) && rItem.ePlaybackState != PB_Paused )
		{
			rItem.ePlaybackState = PB_Paused;
		}
	}
}

/** 2012 0x94f9d0. */
AKRESULT CAkSwitchCntr::PrepareNodeList( const AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 1> & in_rNodeList )
{
	AKRESULT eResult = AK_Success;
	for( AkUniqueID * it = in_rNodeList.m_pItems; it != in_rNodeList.m_pItemsEnd; ++it )
	{
		eResult = CAkParameterNodeBase::PrepareNodeData( *it );
		if( eResult != AK_Success )
		{
			for( AkUniqueID * itUn = in_rNodeList.m_pItems; itUn != it; ++itUn )
			{
				CAkParameterNodeBase::UnPrepareNodeData( *itUn );
			}
		}
	}
	return eResult;
}

/** 2012 0x94fa30. */
void CAkSwitchCntr::UnPrepareNodeList( const AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 1> & in_rNodeList )
{
	for( AkUniqueID * it = in_rNodeList.m_pItems; it != in_rNodeList.m_pItemsEnd; ++it )
	{
		CAkParameterNodeBase::UnPrepareNodeData( *it );
	}
}

/** 2012 0x94fb50: a switch of the group became (un)prepared while this container is prepared. */
AKRESULT CAkSwitchCntr::ModifyActiveState( AkUInt32 in_stateID, bool in_bSupported )
{
	if( !m_uPreparationCount )
	{
		return AK_Success;
	}
	CAkSwitchPackage * pPackage = m_SwitchList.Exists( in_stateID );
	if( !pPackage )
	{
		return AK_Success;
	}
	if( in_bSupported )
	{
		return PrepareNodeList( pPackage->m_list );
	}
	UnPrepareNodeList( pPackage->m_list );
	return AK_Success;
}

static bool PreparedContentHas( CAkPreparedContent * in_pContent, AkUInt32 in_ulSwitch )
{
	return in_pContent->m_PreparableContentList.Exists( in_ulSwitch ) != NULL;
}

/** 2012 0x94fba0: with game-sync preparation only the prepared switches' packages are loaded (Dishonored keeps
    bEnableGameSyncPreparation false, so every child is prepared, as CAkActiveParent does). */
AKRESULT CAkSwitchCntr::PrepareData()
{
	if( !g_settings.bEnableGameSyncPreparation )
	{
		return CAkParentNode<CAkParameterNode>::PrepareData();
	}
	AKRESULT eResult = AK_Success;
	if( m_uPreparationCount )
	{
		++m_uPreparationCount;
		return eResult;
	}
	CAkPreparedContent * pPreparedContent = GetPreparedContent( m_ulGroupID, m_eGroupType );
	if( !pPreparedContent )
	{
		return AK_InsufficientMemory;
	}
	for( SwitchList::Iterator it = m_SwitchList.Begin(); it != m_SwitchList.End(); ++it )
	{
		if( PreparedContentHas( pPreparedContent, ( *it ).key ) )
		{
			eResult = PrepareNodeList( ( *it ).item.m_list );
		}
		if( eResult != AK_Success )
		{
			for( SwitchList::Iterator itUn = m_SwitchList.Begin(); itUn != it; ++itUn )
			{
				if( PreparedContentHas( pPreparedContent, ( *itUn ).key ) )
				{
					UnPrepareNodeList( ( *itUn ).item.m_list );
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
		}
	}
	return eResult;
}

/** 2012 0x94fd00. */
void CAkSwitchCntr::UnPrepareData()
{
	if( !g_settings.bEnableGameSyncPreparation )
	{
		for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
		{
			( *it )->UnPrepareData();
		}
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
		for( SwitchList::Iterator it = m_SwitchList.Begin(); it != m_SwitchList.End(); ++it )
		{
			if( PreparedContentHas( pPreparedContent, ( *it ).key ) )
			{
				UnPrepareNodeList( ( *it ).item.m_list );
			}
		}
	}
	UnsubscribePrepare( m_ulGroupID, m_eGroupType );
}

/** 2012 0x94fdc0. */
void CAkSwitchCntr::GetAllParams( AkUniqueID in_NodeID, AkSwitchNodeParams & out_rParams )
{
	AkSwitchNodeParams * pParams = m_listParameters.Exists( in_NodeID );
	if( pParams )
	{
		out_rParams = *pParams;
	}
	else
	{
		out_rParams.eOnSwitchMode = 0;
		out_rParams.bIsFirstOnly = 0;
		out_rParams.bContinuePlayback = 0;
		out_rParams.FadeInTime = 0;
		out_rParams.FadeOutTime = 0;
	}
}

/** 2012 0x94ff70: OnPreRelease's half - every remembered continuous instance gives back its counts. */
void CAkSwitchCntr::CleanSwitchContPlayback()
{
	if( !m_listSwitchContPlayback.m_ulMaxNumListItems )
	{
		return;
	}
	for( SwitchContPlaybackList::Iterator it = m_listSwitchContPlayback.Begin(); it != m_listSwitchContPlayback.End(); ++it )
	{
		g_pPlayingMgr->RemoveItemActiveCount( ( *it ).UserParameters.m_PlayingID );
		DecrementActivityCount( 3 );
		ReleaseGameObj( ( *it ).GameObject );
	}
}

/** 2012 0x950070. */
void CAkSwitchCntr::StopContSwitchInst( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID )
{
	bool bRemoved = false;
	SwitchContPlaybackList::ListItem * pPrev = NULL;
	SwitchContPlaybackList::ListItem * pItem = m_listSwitchContPlayback.m_pFirst;
	while( pItem )
	{
		SwitchContPlaybackList::ListItem * pNext = pItem->pNextListItem;
		if( ( !in_pGameObj || in_pGameObj == pItem->Item.GameObject ) && ( !in_PlayingID || pItem->Item.UserParameters.m_PlayingID == in_PlayingID ) )
		{
			CAkRegisteredObj * pGameObj = pItem->Item.GameObject;
			const AkPlayingID playingID = pItem->Item.UserParameters.m_PlayingID;
			m_listSwitchContPlayback.RemoveItem( pItem, pPrev );
			g_pPlayingMgr->RemoveItemActiveCount( playingID );
			DecrementActivityCount( 3 );
			ReleaseGameObj( pGameObj );
			bRemoved = true;
		}
		else
		{
			pPrev = pItem;
		}
		pItem = pNext;
	}
	if( bRemoved )
	{
		g_pRegistryMgr->ClearSwitchHist( key, in_pGameObj );
	}
}

/** 2012 0x950460. */
void CAkSwitchCntr::OnPreRelease()
{
	CleanSwitchContPlayback();
	FlushStateTransitions();
}

/** 2012 0x950470: the package of the game object's switch (else the default switch) plays; a first-only child plays
    only on the first play after a switch change; a continuous switch container remembers the instance so a later
    switch change can restart it. */
AKRESULT CAkSwitchCntr::Play( AkPBIParams & in_rPBIParams )
{
	const bool bIsContinuousPlay = in_rPBIParams.eType != AkPBIParams::PBI;
	AKRESULT eResult = AK_Fail;
	AkSwitchStateID ulSwitch = GetSwitchToUse( in_rPBIParams.pGameObj, m_ulGroupID, m_eGroupType );
	if( m_bIsContinuousValidation )
	{
		SwitchContPlaybackItem item;
		item.UserParameters = in_rPBIParams.userParams;
		item.GameObject = in_rPBIParams.pGameObj;
		item.PlayHist = in_rPBIParams.playHistory;
		item.ePlaybackState = in_rPBIParams.ePlaybackState;
		if( !IncrementActivityCount( 3 ) )
		{
			DecrementActivityCount( 3 );
			return AK_Fail;
		}
		if( !m_listSwitchContPlayback.AddLast( item ) )
		{
			DecrementActivityCount( 3 );
			return AK_Fail;
		}
		++in_rPBIParams.pGameObj->m_refCount;
		g_pPlayingMgr->AddItemActiveCount( in_rPBIParams.userParams.m_PlayingID );
	}
	SafeContinuationList safeContList( in_rPBIParams, this );
	AkUInt32 ulNumSoundsStarted = 0;
	CAkSwitchPackage * pSwPack = m_SwitchList.Exists( ulSwitch );
	if( !pSwPack )
	{
		ulSwitch = m_ulDefaultSwitch;
		pSwPack = m_SwitchList.Exists( ulSwitch );
	}
	if( !pSwPack )
	{
		AKDIAG( L"akdiag: switch 0x%08x group 0x%08x: no package for switch 0x%08x nor the default", key, m_ulGroupID, ulSwitch );
		if( !bIsContinuousPlay )
		{
			return AK_Success;
		}
	}
	else
	{
		AkSwitchHistItem SwitchHistItem = g_pRegistryMgr->GetSwitchHistItem( in_rPBIParams.pGameObj, key );
		if( SwitchHistItem.LastSwitch == ulSwitch )
		{
			++SwitchHistItem.NumPlayBack;
		}
		else
		{
			SwitchHistItem.LastSwitch = ulSwitch;
			SwitchHistItem.NumPlayBack = 1;
		}
		const bool bIsFirstPlay = SwitchHistItem.NumPlayBack == 1;
		g_pRegistryMgr->SetSwitchHistItem( in_rPBIParams.pGameObj, key, SwitchHistItem );
		AKDIAG( L"akdiag: switch 0x%08x %s 0x%08x = 0x%08x: %u child(ren), playback %u%s", key, m_eGroupType == AkGroupType_State ? L"state group" : L"switch group", m_ulGroupID, ulSwitch, pSwPack->m_list.Length(), SwitchHistItem.NumPlayBack, m_bIsContinuousValidation ? L", continuous" : L"" );
		if( !bIsContinuousPlay && pSwPack->m_list.IsEmpty() )
		{
			return AK_Success;
		}
		for( AkUniqueID * it = pSwPack->m_list.m_pItems; it != pSwPack->m_list.m_pItemsEnd; ++it )
		{
			AkPBIParams params( in_rPBIParams );
			AkSwitchNodeParams l_Params;
			GetAllParams( *it, l_Params );
			if( ( !bIsFirstPlay && l_Params.bIsFirstOnly ) || ( params.sequenceID && ulNumSoundsStarted ) )
			{
				continue;
			}
			CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( *it );
			if( !pNode )
			{
				continue;
			}
			if( !bIsContinuousPlay )
			{
				eResult = pNode->Play( params );
			}
			else
			{
				AkContParamsAndPath continuousParams( params.pContinuousParams );
				if( pSwPack->m_list.Length() == 1 )
				{
					ContGetList( params.pContinuousParams->spContList, continuousParams.Get().spContList );
					params.pContinuousParams = &continuousParams.Get();
					eResult = pNode->Play( params );
				}
				else
				{
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
			if( eResult == AK_Success )
			{
				++ulNumSoundsStarted;
			}
			pNode->Release();
		}
		if( !bIsContinuousPlay )
		{
			return eResult;
		}
		if( ulNumSoundsStarted )
		{
			return safeContList.Get() ? ContUnrefList( safeContList.Get() ) : eResult;
		}
	}
	if( m_bIsContinuousValidation )
	{
		return safeContList.Get() ? ContUnrefList( safeContList.Get() ) : eResult;
	}
	return PlayAndContinueAlternateMultiPlay( in_rPBIParams );
}

/** 2012 0x950860. */
AKRESULT CAkSwitchCntr::ExecuteAction( ActionParams & in_rAction )
{
	switch( in_rAction.eType )
	{
	case ActionParamType_Stop:
	case ActionParamType_Break:
		StopContSwitchInst( in_rAction.pGameObj, in_rAction.playingID );
		break;
	case ActionParamType_Pause:
		PauseContSwitchInst( in_rAction.pGameObj, in_rAction.playingID );
		break;
	case ActionParamType_Resume:
		ResumeContSwitchInst( in_rAction.pGameObj, in_rAction.playingID );
		break;
	default:
		break;
	}
	return CAkParentNode<CAkParameterNode>::ExecuteAction( in_rAction );
}

/** 2012 0x9508e0. */
AKRESULT CAkSwitchCntr::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	switch( in_rAction.eType )
	{
	case ActionParamType_Stop:
		StopContSwitchInst( in_rAction.pGameObj, AK_INVALID_PLAYING_ID );
		break;
	case ActionParamType_Pause:
		PauseContSwitchInst( in_rAction.pGameObj, AK_INVALID_PLAYING_ID );
		break;
	case ActionParamType_Resume:
		ResumeContSwitchInst( in_rAction.pGameObj, AK_INVALID_PLAYING_ID );
		break;
	default:
		break;
	}
	return CAkParentNode<CAkParameterNode>::ExecuteActionExcept( in_rAction );
}

/** 2012 0x950990. */
void CAkSwitchCntr::RemoveChild( CAkParameterNodeBase * in_pChild )
{
	const AkUniqueID childID = in_pChild->key;
	bool bReleaseSelf = false;
	if( in_pChild->m_pParentNode == this )
	{
		in_pChild->Parent( NULL );
		m_mapChildId.Unset( childID );
		bReleaseSelf = true;
	}
	m_listParameters.Unset( childID );
	if( bReleaseSelf )
	{
		Release();
	}
}

/** 2012 0x950a10: a continuous instance's new switch child starts with its fade-in. */
AKRESULT CAkSwitchCntr::PlayOnSwitch( AkUniqueID in_ID, SwitchContPlaybackItem & in_rContItem )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ID );
	if( !pNode )
	{
		return AK_Fail;
	}
	AkSwitchNodeParams * pParams = m_listParameters.Exists( in_ID );
	TransParams l_TParams;
	l_TParams.TransitionTime = pParams ? pParams->FadeInTime : 0;
	l_TParams.eFadeCurve = AkCurveInterpolation_Linear;
	AkPBIParams pbiParams;
	pbiParams.eType = AkPBIParams::PBI;
	pbiParams.pInstigator = pNode;
	pbiParams.pGameObj = in_rContItem.GameObject;
	pbiParams.pTransitionParameters = &l_TParams;
	pbiParams.userParams = in_rContItem.UserParameters;
	pbiParams.playHistory = in_rContItem.PlayHist;
	pbiParams.ePlaybackState = in_rContItem.ePlaybackState;
	pbiParams.uFrameOffset = 0;
	pbiParams.pContinuousParams = NULL;
	pbiParams.sequenceID = 0;
	pbiParams.bTargetFeedback = false;
	pbiParams.bIsFirst = true;
	const AKRESULT eResult = pNode->Play( pbiParams );
	pNode->Release();
	return eResult;
}

/** 2012 0x950b50: an old switch child stops with its fade-out (play-to-end mode lets it finish). */
AKRESULT CAkSwitchCntr::StopOnSwitch( AkUniqueID in_ID, AkSwitchNodeParams & in_rSwitchNodeParams, CAkRegisteredObj * in_GameObj )
{
	AKRESULT eResult = AK_Success;
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ID );
	if( pNode )
	{
		g_pAudioMgr->StopPendingAction( in_ID, in_GameObj, AK_INVALID_PLAYING_ID );
		if( in_rSwitchNodeParams.eOnSwitchMode == 1 )
		{
			ActionParams l_Params;
			l_Params.eType = ActionParamType_Stop;
			l_Params.pGameObj = in_GameObj;
			l_Params.playingID = AK_INVALID_PLAYING_ID;
			l_Params.transParams.TransitionTime = in_rSwitchNodeParams.FadeOutTime;
			l_Params.transParams.eFadeCurve = AkCurveInterpolation_Linear;
			l_Params.bIsFromBus = false;
			l_Params.bIsMasterCall = false;
			l_Params.bIsMasterResume = false;
			l_Params.targetNodeID = AK_INVALID_UNIQUE_ID;
			eResult = pNode->ExecuteAction( l_Params );
		}
		else
		{
			pNode->PlayToEnd( in_GameObj, key, AK_INVALID_PLAYING_ID );
		}
		pNode->Release();
	}
	return eResult;
}

/** 2012 0x950c10: children of the old package stop unless they continue playback into the new one. */
AKRESULT CAkSwitchCntr::StopPrevious( CAkSwitchPackage * in_pPreviousSwitchPack, CAkSwitchPackage * in_pNextSwitchPack, CAkRegisteredObj * in_GameObj )
{
	AKRESULT eResult = AK_Success;
	if( !in_pPreviousSwitchPack )
	{
		return eResult;
	}
	for( AkUniqueID * it = in_pPreviousSwitchPack->m_list.m_pItems; it != in_pPreviousSwitchPack->m_list.m_pItemsEnd; ++it )
	{
		AkSwitchNodeParams l_Params;
		GetAllParams( *it, l_Params );
		if( m_bIsContinuousValidation && l_Params.bContinuePlayback && in_pNextSwitchPack && in_pNextSwitchPack->m_list.Exists( *it ) )
		{
			continue;
		}
		eResult = StopOnSwitch( *it, l_Params, in_GameObj );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	return eResult;
}

/** 2012 0x950e30: one game object's switch changed - stop the old package, then every continuous instance of that
    object plays the new package's children (those already playing with continue-playback excepted). */
AKRESULT CAkSwitchCntr::PerformSwitchChangeContPerObject( AkSwitchStateID in_SwitchTo, CAkRegisteredObj * in_GameObj )
{
	AkSwitchHistItem SwitchHistItem = g_pRegistryMgr->GetSwitchHistItem( in_GameObj, key );
	if( in_SwitchTo == SwitchHistItem.LastSwitch )
	{
		return AK_Success;
	}
	CAkSwitchPackage * pPreviousSwitchPack = m_SwitchList.Exists( SwitchHistItem.LastSwitch );
	CAkSwitchPackage * pNextSwitchPack = m_SwitchList.Exists( in_SwitchTo );
	if( !pNextSwitchPack )
	{
		in_SwitchTo = m_ulDefaultSwitch;
		pNextSwitchPack = m_SwitchList.Exists( in_SwitchTo );
	}
	AKDIAG( L"akdiag: switch 0x%08x: game object %u switch 0x%08x -> 0x%08x on a playing continuous container", key, in_GameObj ? (AkUInt32)in_GameObj->m_GameObjID : 0, SwitchHistItem.LastSwitch, in_SwitchTo );
	AKRESULT eResult = StopPrevious( pPreviousSwitchPack, pNextSwitchPack, in_GameObj );
	g_pRegistryMgr->ClearSwitchHist( key, in_GameObj );
	SwitchHistItem.NumPlayBack = 0;
	SwitchHistItem.LastSwitch = 0;
	for( SwitchContPlaybackList::Iterator it = m_listSwitchContPlayback.Begin(); it != m_listSwitchContPlayback.End(); ++it )
	{
		SwitchContPlaybackItem & rItem = *it;
		if( rItem.GameObject != in_GameObj )
		{
			continue;
		}
		if( SwitchHistItem.LastSwitch == in_SwitchTo )
		{
			++SwitchHistItem.NumPlayBack;
		}
		else
		{
			SwitchHistItem.LastSwitch = in_SwitchTo;
			SwitchHistItem.NumPlayBack = 1;
		}
		if( !pNextSwitchPack )
		{
			continue;
		}
		for( AkUniqueID * itNode = pNextSwitchPack->m_list.m_pItems; itNode != pNextSwitchPack->m_list.m_pItemsEnd; ++itNode )
		{
			AkSwitchNodeParams l_Params;
			GetAllParams( *itNode, l_Params );
			if( l_Params.bContinuePlayback && pPreviousSwitchPack && pPreviousSwitchPack->m_list.Exists( *itNode ) )
			{
				continue;
			}
			eResult = PlayOnSwitch( *itNode, rItem );
		}
	}
	g_pRegistryMgr->SetSwitchHistItem( in_GameObj, key, SwitchHistItem );
	return eResult;
}

/** 2012 0x951280: a global switch change applies to every game object with a continuous instance. */
AKRESULT CAkSwitchCntr::PerformSwitchChange( AkSwitchStateID in_SwitchTo, CAkRegisteredObj * in_GameObj )
{
	if( in_GameObj )
	{
		PerformSwitchChangeContPerObject( in_SwitchTo, in_GameObj );
		return AK_Success;
	}
	const AkUInt32 uNumItems = m_listSwitchContPlayback.Length();
	if( !uNumItems )
	{
		g_pRegistryMgr->ClearSwitchHist( key, NULL );
		return AK_Success;
	}
	CAkRegisteredObj ** ppGameObj = (CAkRegisteredObj **)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRegisteredObj * ) * uNumItems );
	if( !ppGameObj )
	{
		return AK_Fail;
	}
	CAkRegisteredObj ** ppWrite = ppGameObj;
	for( SwitchContPlaybackList::Iterator it = m_listSwitchContPlayback.Begin(); it != m_listSwitchContPlayback.End(); ++it )
	{
		*ppWrite++ = ( *it ).GameObject;
	}
	for( AkUInt32 i = 0; i < uNumItems; ++i )
	{
		PerformSwitchChangeContPerObject( in_SwitchTo, ppGameObj[ i ] );
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, ppGameObj );
	return AK_Success;
}

/** 2012 0x951350. */
void CAkSwitchCntr::ClearSwitches()
{
	if( !m_SwitchList.m_ulMaxNumListItems )
	{
		return;
	}
	for( SwitchList::Iterator it = m_SwitchList.Begin(); it != m_SwitchList.End(); ++it )
	{
		( *it ).item.m_list.Term();
	}
	m_SwitchList.RemoveAll();
}

/** 2012 0x951460: CAkSwitchAware's slot; only a continuous switch container reacts to a switch change. */
void CAkSwitchCntr::SetSwitch( AkUInt32 in_Switch, CAkRegisteredObj * in_GameObj )
{
	if( m_bIsContinuousValidation )
	{
		PerformSwitchChange( in_Switch, in_GameObj );
	}
}
