/*=============================================================================
	AkRegistryMgr.cpp - the game object registry (see AkRegistryMgr.h).

	DISHONORED(port): agent IH1 (wave 26). Rvas on each definition.
=============================================================================*/
#include "AkRegistryMgr.h"
#include "AkHierarchy.h"

/** 2012 0x937070 (akpositionkeeper.h): the array is reallocated to the new count and copied. */
AKRESULT AkPositionKeeper::SetPosition( const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions )
{
	if( m_uNumPos != in_uNumPositions )
	{
		Term();
		if( in_uNumPositions )
		{
			m_aPos = (AkSoundPosition *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkSoundPosition ) * in_uNumPositions );
			if( !m_aPos )
			{
				return AK_InsufficientMemory;
			}
			m_uNumPos = in_uNumPositions;
		}
	}
	if( m_uNumPos )
	{
		memcpy( m_aPos, in_aPositions, sizeof( AkSoundPosition ) * m_uNumPos );
	}
	return AK_Success;
}

void AkPositionKeeper::Term()
{
	if( m_aPos )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_aPos );
		m_aPos = NULL;
	}
	m_uNumPos = 0;
}

/** 2012 0x937100. */
CAkRegisteredObj::CAkRegisteredObj( AkGameObjectID in_GameObjID )
	: m_pListModifiedNodes( NULL )
	, m_fDryLevelValue( 1.f )
	, m_fScalingFactor( 1.f )
	, m_GameObjID( in_GameObjID )
	, m_refCount( 1 )
{
}

/** 2012 0x9371e0. */
CAkRegisteredObj::~CAkRegisteredObj()
{
	FreeModifiedNodes();
	if( m_pListModifiedNodes )
	{
		m_pListModifiedNodes->Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pListModifiedNodes );
		m_pListModifiedNodes = NULL;
	}
	m_SwitchHist.Term();
	m_PosKeep.Term();
}

/** 2012 0x936e80. */
AKRESULT CAkRegisteredObj::Init()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_EnvironmentValues[ i ].EnvID = AK_INVALID_AUX_ID;
		m_EnvironmentValues[ i ].fControlValue = 0.f;
	}
	memset( m_fObstructionValue, 0, sizeof( m_fObstructionValue ) );
	memset( m_fOcclusionValue, 0, sizeof( m_fOcclusionValue ) );
	return AK_Success;
}

/** 2012 0x937150. */
void CAkRegisteredObj::SetPosition( const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions, AK::SoundEngine::MultiPositionType in_eMultiPositionType, AkUInt32 in_ulListenerIndex )
{
	m_PosKeep.SetPosition( in_aPositions, in_uNumPositions );
	m_PosKeep.m_eMultiPositionType = in_eMultiPositionType;
	m_PosKeep.m_iListenerIdx = (AkInt8)in_ulListenerIndex;
}

/** 2012 0x936ed0: at most 4 sends; zero ids and non-positive values are dropped; the rest are cleared. */
AKRESULT CAkRegisteredObj::SetGameObjectEnvironmentsValues( const AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues )
{
	if( in_uNumEnvValues > 4 )
	{
		return AK_Fail;
	}
	AkUInt32 uCount = 0;
	if( in_aEnvironmentValues )
	{
		for( AkUInt32 i = 0; i < in_uNumEnvValues; ++i )
		{
			if( in_aEnvironmentValues[ i ].EnvID && in_aEnvironmentValues[ i ].fControlValue > 0.f )
			{
				m_EnvironmentValues[ uCount++ ] = in_aEnvironmentValues[ i ];
			}
		}
	}
	for( ; uCount < 4; ++uCount )
	{
		m_EnvironmentValues[ uCount ].EnvID = AK_INVALID_AUX_ID;
		m_EnvironmentValues[ uCount ].fControlValue = 0.f;
	}
	return AK_Success;
}

/** 2012 0x937020. */
AKRESULT CAkRegisteredObj::SetObjectObstructionAndOcclusion( AkUInt32 in_uListener, AkReal32 in_fObstructionValue, AkReal32 in_fOcclusionValue )
{
	if( in_uListener >= 8 )
	{
		return AK_Fail;
	}
	m_fObstructionValue[ in_uListener ] = (AkUInt8)(AkInt32)( in_fObstructionValue * 255.f );
	m_fOcclusionValue[ in_uListener ] = (AkUInt8)(AkInt32)( in_fOcclusionValue * 255.f );
	return AK_Success;
}

void CAkRegistryMgr::Destroy( CAkRegisteredObj * in_pObj )
{
	in_pObj->~CAkRegisteredObj();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pObj );
}

/** 2012 0x9308d0: game object 0 exists from the start, at position zero. */
AKRESULT CAkRegistryMgr::Init()
{
	m_mapRegisteredObj.Init( g_DefaultPoolId );
	const AKRESULT eResult = m_listModifiedNodes.Init( 8, (AkUInt32)-1 );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	CAkRegisteredObj * pObj = RegisterObject( 0, NULL );
	if( pObj )
	{
		AkSoundPosition pos;
		memset( &pos, 0, sizeof( pos ) );
		pObj->SetPosition( &pos, 1, AK::SoundEngine::MultiPositionType_SingleSource, 0 );
	}
	return AK_Success;
}

/** 2012 0x930c70. */
void CAkRegistryMgr::Term()
{
	if( m_mapRegisteredObj.m_MemPoolId != AK_INVALID_POOL_ID )
	{
		UnregisterObject( 0 );
	}
	UnregisterAll();
	m_mapRegisteredObj.Term();
	m_listModifiedNodes.Term();
}

/** 2012 0x930b00: the nodes a global SetValue / Mute touched, for the reset-all actions. */
void CAkRegistryMgr::SetNodeIDAsModified( CAkParameterNodeBase * in_pNode )
{
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = m_listModifiedNodes.m_pFirst; p; p = p->pNextListItem )
	{
		if( p->Item == in_pNode->key )
		{
			return;
		}
	}
	m_listModifiedNodes.AddLast( in_pNode->key );
}

/** 2012 0x937260. */
bool CAkRegisteredObj::CreateModifiedNodeList()
{
	if( !m_pListModifiedNodes )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> ) );
		if( pMem )
		{
			m_pListModifiedNodes = AkPlacementNew( pMem ) CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>();
			m_pListModifiedNodes->InitUnbounded();
		}
	}
	return m_pListModifiedNodes != NULL;
}

/** 2012 0x9372b0: the nodes a SetValue / Mute touched for this game object. */
AKRESULT CAkRegisteredObj::SetNodeAsModified( CAkParameterNodeBase * in_pNode )
{
	if( !CreateModifiedNodeList() )
	{
		return AK_InsufficientMemory;
	}
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = m_pListModifiedNodes->m_pFirst; p; p = p->pNextListItem )
	{
		if( p->Item == in_pNode->key )
		{
			return AK_Success;
		}
	}
	return m_pListModifiedNodes->AddLast( in_pNode->key ) ? AK_Success : AK_Fail;
}

/** 2012 0x937190: every node this object modified drops its per-object values, then the object's RTPC values go. */
void CAkRegisteredObj::FreeModifiedNodes()
{
	if( m_pListModifiedNodes )
	{
		for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = m_pListModifiedNodes->m_pFirst; p; p = p->pNextListItem )
		{
			CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
			if( pNode )
			{
				pNode->Unregister( this );
				pNode->Release();
			}
		}
	}
	g_pRTPCMgr->UnregisterGameObject( this );
}

/** 2012 0x930420: an already registered id returns the existing object. */
CAkRegisteredObj * CAkRegistryMgr::RegisterObject( AkGameObjectID in_GameObjectID, void * )
{
	CAkRegisteredObj ** ppObj = m_mapRegisteredObj.Exists( in_GameObjectID );
	if( ppObj )
	{
		return *ppObj;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRegisteredObj ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkRegisteredObj * pObj = AkPlacementNew( pMem ) CAkRegisteredObj( in_GameObjectID );
	if( pObj->Init() == AK_Success )
	{
		CAkRegisteredObj ** ppSlot = m_mapRegisteredObj.Set( in_GameObjectID );
		if( ppSlot )
		{
			*ppSlot = pObj;
			return pObj;
		}
	}
	Destroy( pObj );
	return NULL;
}

/** 2012 0x9304d0. */
void CAkRegistryMgr::UnregisterObject( AkGameObjectID in_GameObjectID )
{
	AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::IteratorEx it = m_mapRegisteredObj.FindEx( in_GameObjectID );
	if( !it.pItem )
	{
		return;
	}
	CAkRegisteredObj * pObj = it.pItem->Assoc.item;
	m_mapRegisteredObj.Erase( it );
	if( --pObj->m_refCount <= 0 )
	{
		Destroy( pObj );
	}
}

/** 2012 0x930770: everything but game object 0. */
void CAkRegistryMgr::UnregisterAll()
{
	if( m_mapRegisteredObj.m_MemPoolId == AK_INVALID_POOL_ID )
	{
		return;
	}
	for( AkUInt32 i = 0; i < 193; ++i )
	{
		AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Item * pItem = m_mapRegisteredObj.m_table[ i ];
		while( pItem )
		{
			AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Item * pNext = pItem->pNextItem;
			if( pItem->Assoc.key )
			{
				CAkRegisteredObj * pObj = pItem->Assoc.item;
				m_mapRegisteredObj.Unset( pItem->Assoc.key );
				if( --pObj->m_refCount <= 0 )
				{
					Destroy( pObj );
				}
			}
			pItem = pNext;
		}
	}
}

/** 2012 0x9303d0. */
CAkRegisteredObj * CAkRegistryMgr::GetObjAndAddref( AkGameObjectID in_GameObjectID )
{
	CAkRegisteredObj ** ppObj = m_mapRegisteredObj.Exists( in_GameObjectID );
	if( !ppObj )
	{
		return NULL;
	}
	++( *ppObj )->m_refCount;
	return *ppObj;
}

CAkRegisteredObj * CAkRegistryMgr::GetObj( AkGameObjectID in_GameObjectID )
{
	CAkRegisteredObj ** ppObj = m_mapRegisteredObj.Exists( in_GameObjectID );
	return ppObj ? *ppObj : NULL;
}

/** 2012 0x930310: active means something besides the registry holds a reference (a playing sound). */
bool CAkRegistryMgr::IsGameObjectActive( AkGameObjectID in_GameObjectID )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	return pObj && pObj->m_refCount > 1;
}

/** 2012 0x930530. */
AKRESULT CAkRegistryMgr::SetPosition( AkGameObjectID in_GameObjectID, const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions, AK::SoundEngine::MultiPositionType in_eMultiPositionType, AkUInt32 in_ulListenerIndex )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	if( !pObj )
	{
		return AK_Fail;
	}
	pObj->SetPosition( in_aPositions, in_uNumPositions, in_eMultiPositionType, in_ulListenerIndex );
	return AK_Success;
}

/** 2012 0x9305a0: game object 0's listeners cannot be changed. */
AKRESULT CAkRegistryMgr::SetActiveListeners( AkGameObjectID in_GameObjectID, AkUInt32 in_uListenerMask )
{
	if( !in_GameObjectID )
	{
		return AK_Fail;
	}
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	if( !pObj )
	{
		return AK_Fail;
	}
	pObj->SetActiveListeners( in_uListenerMask );
	return AK_Success;
}

/** 2012 0x9305e0. */
AKRESULT CAkRegistryMgr::SetGameObjectEnvironmentsValues( AkGameObjectID in_GameObjectID, const AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	return pObj ? pObj->SetGameObjectEnvironmentsValues( in_aEnvironmentValues, in_uNumEnvValues ) : AK_Fail;
}

/** 2012 0x930640. */
AKRESULT CAkRegistryMgr::SetGameObjectDryLevelValue( AkGameObjectID in_GameObjectID, AkReal32 in_fControlValue )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	if( !pObj )
	{
		return AK_Fail;
	}
	pObj->m_fDryLevelValue = in_fControlValue;
	return AK_Success;
}

/** 2012 0x9306a0. */
AKRESULT CAkRegistryMgr::SetGameObjectScalingFactor( AkGameObjectID in_GameObjectID, AkReal32 in_fControlValue )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	if( !pObj )
	{
		return AK_Fail;
	}
	pObj->m_fScalingFactor = in_fControlValue;
	return AK_Success;
}

/** 2012 0x930700. */
AKRESULT CAkRegistryMgr::SetObjectObstructionAndOcclusion( AkGameObjectID in_GameObjectID, AkUInt32 in_uListener, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel )
{
	CAkRegisteredObj * pObj = GetObj( in_GameObjectID );
	return pObj ? pObj->SetObjectObstructionAndOcclusion( in_uListener, in_fObstructionLevel, in_fOcclusionLevel ) : AK_Fail;
}

/** 2012 0x9309a0 (IH3 fork R): the container's history on this game object, {0, 0} without one. */
AkSwitchHistItem CAkRegistryMgr::GetSwitchHistItem( CAkRegisteredObj * in_pGameObj, AkUniqueID in_SwitchContID )
{
	AkSwitchHistItem item;
	item.LastSwitch = 0;
	item.NumPlayBack = 0;
	if( in_pGameObj )
	{
		AkSwitchHistItem * pItem = in_pGameObj->m_SwitchHist.Exists( in_SwitchContID );
		if( pItem )
		{
			item = *pItem;
		}
	}
	return item;
}

/** 2012 0x9309f0 (IH3 fork R). */
AKRESULT CAkRegistryMgr::SetSwitchHistItem( CAkRegisteredObj * in_pGameObj, AkUniqueID in_SwitchContID, const AkSwitchHistItem & in_SwitchHistItem )
{
	if( !in_pGameObj )
	{
		return AK_Fail;
	}
	return in_pGameObj->m_SwitchHist.Set( in_SwitchContID, in_SwitchHistItem ) ? AK_Success : AK_Fail;
}

/** 2012 0x930a30 (IH3 fork R): one game object's history of the container, or (NULL) every game object's. */
AKRESULT CAkRegistryMgr::ClearSwitchHist( AkUniqueID in_SwitchContID, CAkRegisteredObj * in_pGameObj )
{
	if( in_pGameObj )
	{
		in_pGameObj->m_SwitchHist.Unset( in_SwitchContID );
		return AK_Success;
	}
	for( AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Iterator it = m_mapRegisteredObj.Begin(); it.pItem; ++it )
	{
		it.pItem->Assoc.item->m_SwitchHist.Unset( in_SwitchContID );
	}
	return AK_Success;
}

/** 2012 0x930b40 (IH3 fork R): the game objects something besides the registry holds (playing ones). */
AKRESULT CAkRegistryMgr::GetActiveGameObjects( AkArray<AkGameObjectID, AkGameObjectID, ArrayPoolDefault, 32> & io_GameObjectList )
{
	for( AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Iterator it = m_mapRegisteredObj.Begin(); it.pItem; ++it )
	{
		if( it.pItem->Assoc.item->m_refCount > 1 && !io_GameObjectList.AddLast( it.pItem->Assoc.key ) )
		{
			return AK_InsufficientMemory;
		}
	}
	return AK_Success;
}
