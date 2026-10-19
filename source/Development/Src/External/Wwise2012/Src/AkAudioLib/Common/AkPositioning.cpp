/*=============================================================================
	AkPositioning.cpp - the positioning half of the node hierarchy and its queries: Gen3DParams::GetAttenuation,
	CAkGen3DParams::SetPathPlayList / SetTransition (akgen3dparams.cpp), CAkParameterNode::GetPathState
	(akparameternode.cpp), the positioning-RTPC notification of sounds and active parents (aksoundbase.cpp,
	akactiveparent.h), CAkPlayingMgr::NotifySpeakerVolumeMatrix, CAkURenderer::GetMaxRadius,
	AK::SoundEngine::Query::GetMaxRadius, and CAkPBI's positioning half (akpbi.cpp: Get3DPositioningParams,
	PositioningChangeNotification, Scale3DUserDefRTPCValue, Subscribe / UnsubscribeAttenuationRTPC).

	DISHONORED(port): agent IH3 (wave 26); 2012 rvas on the definitions, 2013 matched by bytes (ih1_inventory.csv).
	Deviation (named): the AkArray<GameObjDst> overloads of Query::GetMaxRadius / CAkURenderer::GetMaxRadius (2012
	0x91b150 / 0x91ffe0) are not ported - our public headers do not expose the AkArray-based Query overloads and the
	glue never calls them.
=============================================================================*/
#include "AkPath.h"
#include "AkPBI.h"
#include "AkPlayingMgr.h"
#include "AkURenderer.h"
#include "AkActivityChunk.h"
#include "AkListener.h"

#include <AK/SoundEngine/Common/AkQueryParameters.h>

/** 2012 0x935150: the attenuation is looked up (and referenced) the first time it is needed. */
CAkAttenuation * Gen3DParams::GetAttenuation()
{
	if( !m_pAttenuation )
	{
		m_pAttenuation = g_pIndex->m_idxAttenuations.GetPtrAndAddRef( m_uAttenuationID );
	}
	return m_pAttenuation;
}

/** 2012 0x954d20. */
AKRESULT CAkGen3DParams::SetPathPlayList( CAkPath * in_pPath, AkPathState * in_pState )
{
	return g_pPathManager->SetPathsList( in_pPath, m_Params.m_pArrayPlaylist, m_Params.m_ulNumPlaylistItem, (AkPathMode)m_Params.m_ePathMode, m_Params.m_bIsLooping != 0, in_pState );
}

/** 2012 0x954e50. */
void CAkGen3DParams::SetTransition( AkTimeMs in_TransitionTime )
{
	m_Params.m_TransitionTime = in_TransitionTime;
	UpdateTransitionTimeInVertex();
}

/** 2012 0x937cf0: the path state of the nearest node (this or an ancestor) that has 3D parameters. */
AkPathState * CAkParameterNode::GetPathState()
{
	for( CAkParameterNode * pNode = this; pNode; pNode = static_cast<CAkParameterNode *>( pNode->m_pParentNode ) )
	{
		if( pNode->m_p3DParameters )
		{
			return &pNode->m_p3DParameters->m_PathState;
		}
	}
	return NULL;
}

/** 2012 0x946a70: every PBI of the sound for the game object (all of them for a global change, minus the excepted
    game objects). */
void CAkSoundBase::PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		bool bNotify;
		if( in_pExceptArray && !in_pGameObj )
		{
			bNotify = !static_cast<GameObjExceptArray *>( in_pExceptArray )->Exists( pPBI->m_pGameObj );
		}
		else
		{
			bNotify = !in_pGameObj || pPBI->m_pGameObj == in_pGameObj;
		}
		if( bNotify )
		{
			pPBI->PositioningChangeNotification( in_RTPCValue, in_ParameterID );
		}
	}
}

/** CAkActiveParent<CAkParameterNode> 2012 0x94e140 / CAkActiveParent<CAkParameterNodeBase> 0x93cc90: forwarded to every
    playing child that does not override positioning with its own 3D parameters. */
template <class T>
void CAkParentNode<T>::PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
	{
		CAkParameterNodeBase * pChild = *it;
		if( !pChild->Has3DParams() && pChild->m_pActivityChunk && pChild->m_pActivityChunk->m_PlayCount )
		{
			pChild->PositioningChangeNotification( in_RTPCValue, in_ParameterID, in_pGameObj, in_pExceptArray );
		}
	}
}

template void CAkParentNode<CAkParameterNode>::PositioningChangeNotification( AkReal32, AkRTPC_ParameterID, CAkRegisteredObj *, void * );
template void CAkParentNode<CAkParameterNodeBase>::PositioningChangeNotification( AkReal32, AkRTPC_ParameterID, CAkRegisteredObj *, void * );

/** 2012 0x928150. */
void CAkPlayingMgr::NotifySpeakerVolumeMatrix( AkPlayingID in_PlayingID, AkSpeakerVolumeMatrixCallbackInfo * in_pInfo )
{
	m_csMapLock.Lock();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( !pItem || !( pItem->uiRegisteredNotif & AK_SpeakerVolumeMatrix ) )
	{
		m_csMapLock.Unlock();
		return;
	}
	in_pInfo->pCookie = pItem->pCookie;
	in_pInfo->gameObjID = pItem->GameObj;
	in_pInfo->playingID = in_PlayingID;
	in_pInfo->eventID = pItem->eventID;
	const AkCallbackFunc pfnCallback = pItem->pfnCallback;
	ResetEvent( m_CallbackEvent.m_Event );
	m_csMapLock.Unlock();
	pfnCallback( AK_SpeakerVolumeMatrix, in_pInfo );
	SetEvent( m_CallbackEvent.m_Event );
}

/** 2012 0x91faa0: the largest attenuation radius among the game object's playing voices; -1 when it plays none. */
AkReal32 CAkURenderer::GetMaxRadius( AkGameObjectID in_GameObjId )
{
	AkReal32 fMaxDistance = -1.f;
	CAkRegisteredObj * pGameObj = g_pRegistryMgr->GetObjAndAddref( in_GameObjId );
	if( !pGameObj )
	{
		return fMaxDistance;
	}
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		if( pPBI->m_pGameObj == pGameObj && fMaxDistance <= pPBI->m_fMaxDistance )
		{
			fMaxDistance = pPBI->m_fMaxDistance;
		}
	}
	if( --pGameObj->m_refCount <= 0 )
	{
		pGameObj->~CAkRegisteredObj();
		AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
	}
	return fMaxDistance;
}

namespace AK
{
	namespace SoundEngine
	{
		namespace Query
		{
			/** 2012 0x91b180. */
			AkReal32 GetMaxRadius( AkGameObjectID in_GameObjectID )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				return CAkURenderer::GetMaxRadius( in_GameObjectID );
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkPBI: the positioning half (akpbi.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x93ffe0. */
AKRESULT CAkPBI::SubscribeAttenuationRTPC( CAkAttenuation * in_pAttenuation )
{
	AKRESULT eResult = AK_Success;
	for( CAkAttenuation::RTPCSubs * pSub = in_pAttenuation->m_rtpcsubs.m_pItems; pSub != in_pAttenuation->m_rtpcsubs.m_pItemsEnd; ++pSub )
	{
		eResult = g_pRTPCMgr->SubscribeRTPC( this, pSub->RTPCID, (AkRTPC_ParameterID)pSub->ParamID, pSub->RTPCCurveID, pSub->ConversionTable.m_eScaling, pSub->ConversionTable.m_pArrayGraphPoints, pSub->ConversionTable.m_ulArraySize, m_pGameObj, CAkRTPCMgr::SubscriberType_PBI );
		if( eResult != AK_Success )
		{
			break;
		}
	}
	return eResult;
}

/** 2012 0x940040. */
void CAkPBI::UnsubscribeAttenuationRTPC( CAkAttenuation * in_pAttenuation )
{
	for( CAkAttenuation::RTPCSubs * pSub = in_pAttenuation->m_rtpcsubs.m_pItems; pSub != in_pAttenuation->m_rtpcsubs.m_pItemsEnd; ++pSub )
	{
		g_pRTPCMgr->UnSubscribeRTPC( this, (AkRTPC_ParameterID)pSub->ParamID );
	}
}

/** 2012 0x940fc0: an RTPC-driven pan of a 3D sound is a percentage of the dry attenuation curve's last distance. */
AkReal32 CAkPBI::Scale3DUserDefRTPCValue( AkReal32 in_fValue )
{
	if( !m_p3DSound )
	{
		return in_fValue;
	}
	CAkAttenuation * pAttenuation = m_p3DSound->m_Params.GetAttenuation();
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeDry = pAttenuation ? pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeDry ) : NULL;
	if( !pVolumeDry )
	{
		return in_fValue;
	}
	return pVolumeDry->m_pArrayGraphPoints[ pVolumeDry->m_ulArraySize - 1 ].From * in_fValue * 0.0099999998f;
}

/** 2012 0x941030: a positioning RTPC (or positioning property) changed for this voice. Ids 32..37 are the authoring
    tool's positioning parameters past RTPC_MaxNumRTPC: type, panner on/off, dynamic, looping, transition, path mode. */
void CAkPBI::PositioningChangeNotification( AkReal32 in_RTPCValue, AkRTPC_ParameterID in_ParameterID )
{
	switch( (AkUInt32)in_ParameterID )
	{
	case RTPC_Positioning_Divergence_Center_PCT:
		m_BasePosParams.m_fCenterPCT = in_RTPCValue;
		return;
	case RTPC_Position_PAN_RL:
		m_BasePosParams.m_fPAN_RL = Scale3DUserDefRTPCValue( in_RTPCValue );
		return;
	case RTPC_Position_PAN_FR:
		m_BasePosParams.m_fPAN_FR = Scale3DUserDefRTPCValue( in_RTPCValue );
		return;
	case RTPC_MaxNumRTPC + 1:
		m_BasePosParams.bIsPannerEnabled = in_RTPCValue > 0.f;
		return;
	}
	if( !m_p3DSound )
	{
		return;
	}
	Gen3DParams & rParams = m_p3DSound->m_Params;
	switch( (AkUInt32)in_ParameterID )
	{
	case RTPC_MaxNumRTPC:
		rParams.m_eType = (AkPositioningType)(AkInt32)in_RTPCValue;
		break;
	case RTPC_MaxNumRTPC + 2:
		rParams.m_bIsDynamic = in_RTPCValue != 0.f;
		break;
	case RTPC_MaxNumRTPC + 3:
		rParams.m_bIsLooping = in_RTPCValue != 0.f;
		break;
	case RTPC_MaxNumRTPC + 4:
		m_p3DSound->SetTransition( (AkTimeMs)in_RTPCValue );
		break;
	case RTPC_MaxNumRTPC + 5:
		rParams.m_ePathMode = (AkPathMode)(AkInt32)in_RTPCValue;
		break;
	}
}

/** 2012 0x9411b0: the positions the 3D volume computation uses. User-defined: the path's (or the RTPC pan's) single
    position relative to the listener. Game-defined: the game object's positions, re-read every frame when dynamic,
    else once; a game object attached to a listener plays 1 mm in front of it. The game-defined branch also offsets
    the voice priority by the distance to the nearest listener (the priority's distance factor). */
void CAkPBI::Get3DPositioningParams( AkSoundPositioningParams & io_Parameters )
{
	Gen3DParams & rParams = m_p3DSound->m_Params;
	io_Parameters.ePosType = rParams.m_eType;
	io_Parameters.fPAN_RL = m_BasePosParams.m_fPAN_RL;
	io_Parameters.fPAN_FR = m_BasePosParams.m_fPAN_FR;
	io_Parameters.fDivergenceCenter = m_BasePosParams.m_fCenterPCT * 0.0099999998f;
	if( rParams.m_eType == Ak3DUserDef )
	{
		CAkPath * pPath = m_PathInfo.pPBPath;
		if( pPath )
		{
			const bool bIsLooping = rParams.m_bIsLooping != 0;
			pPath->SetIsLooping( bIsLooping );
			if( bIsLooping && ( pPath->m_PathMode & AkPathContinuous ) && pPath->IsIdle() )
			{
				g_pPathManager->Start( pPath, m_pSound->GetPathState() );
			}
		}
		if( rParams.m_bIsPanningFromRTPC )
		{
			rParams.m_Position.X = m_BasePosParams.m_fPAN_RL;
			rParams.m_Position.Y = 0.f;
			rParams.m_Position.Z = m_BasePosParams.m_fPAN_FR;
		}
		m_LastComputedPosition.m_uListenerMask = 1;
		bool bHasPosition = true;
		if( m_LastComputedPosition.m_uNumPos != 1 )
		{
			m_LastComputedPosition.Term();
			m_LastComputedPosition.m_aPos = (AkSoundPosition *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkSoundPosition ) );
			bHasPosition = m_LastComputedPosition.m_aPos != NULL;
			if( bHasPosition )
			{
				m_LastComputedPosition.m_uNumPos = 1;
			}
		}
		if( bHasPosition )
		{
			AkSoundPosition & rPos = m_LastComputedPosition.m_aPos[ 0 ];
			rPos.Orientation.X = 0.f;
			rPos.Orientation.Y = 0.f;
			rPos.Orientation.Z = -1.f;
			rPos.Position = rParams.m_Position;
		}
		if( !rParams.m_bFollowOrientation )
		{
			m_LastComputedPosition.m_uListenerMask = m_pGameObj->m_PosKeep.m_uListenerMask;
		}
	}
	else if( rParams.m_eType == Ak3DGameDef && ( rParams.m_bIsDynamic || !m_b3DPositionDone ) )
	{
		const AkPositionKeeper & rPosKeep = m_pGameObj->m_PosKeep;
		m_LastComputedPosition.m_uListenerMask = rPosKeep.m_uListenerMask;
		if( rPosKeep.m_iListenerIdx != -1 )
		{
			const AkListenerPosition & rListener = CAkListener::m_listeners[ rPosKeep.m_iListenerIdx ].data.Pos;
			if( m_LastComputedPosition.m_uNumPos != 1 )
			{
				m_LastComputedPosition.Term();
				m_LastComputedPosition.m_aPos = (AkSoundPosition *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkSoundPosition ) );
				if( m_LastComputedPosition.m_aPos )
				{
					m_LastComputedPosition.m_uNumPos = 1;
				}
			}
			if( m_LastComputedPosition.m_aPos )
			{
				AkSoundPosition & rPos = m_LastComputedPosition.m_aPos[ 0 ];
				rPos.Position.X = rListener.OrientationFront.X * 0.001f + rListener.Position.X;
				rPos.Position.Y = rListener.OrientationFront.Y * 0.001f + rListener.Position.Y;
				rPos.Position.Z = rListener.OrientationFront.Z * 0.001f + rListener.Position.Z;
				rPos.Orientation.X = -rListener.OrientationFront.X;
				rPos.Orientation.Y = -rListener.OrientationFront.Y;
				rPos.Orientation.Z = -rListener.OrientationFront.Z;
			}
			m_b3DPositionDone = 1;
			io_Parameters.pPosRef = &m_LastComputedPosition;
			return;
		}
		m_LastComputedPosition.m_eMultiPositionType = rPosKeep.m_eMultiPositionType;
		m_LastComputedPosition.SetPosition( rPosKeep.m_aPos, rPosKeep.m_uNumPos );
		const AkReal32 fDistanceOffset = m_PriorityInfoCurrent.priorityInfo.distanceOffset;
		AkReal32 fPriority = m_PriorityInfoCurrent.priorityInfo.priority;
		if( fDistanceOffset != 0.f )
		{
			const AkReal32 fMinDistance = CAkURenderer::GetMinDistance( rPosKeep ) / m_pGameObj->m_fScalingFactor;
			CAkAttenuation * pAttenuation = rParams.GetAttenuation();
			CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeDry = pAttenuation ? pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeDry ) : NULL;
			AkReal32 fNewPriority = fPriority;
			if( pVolumeDry )
			{
				const AkReal32 fMaxDistance = pVolumeDry->m_pArrayGraphPoints[ pVolumeDry->m_ulArraySize - 1 ].From;
				if( fMaxDistance <= fMinDistance || fMaxDistance <= 0.f )
				{
					fNewPriority = fPriority + fDistanceOffset;
				}
				else
				{
					fNewPriority = fMinDistance / fMaxDistance * fDistanceOffset + fPriority;
				}
			}
			if( fNewPriority < 0.f )
			{
				fNewPriority = 0.f;
			}
			else if( fNewPriority > 100.f )
			{
				fNewPriority = 100.f;
			}
			fPriority = fNewPriority;
		}
		UpdatePriority( fPriority );
		m_b3DPositionDone = 1;
	}
	io_Parameters.pPosRef = &m_LastComputedPosition;
}
