/*=============================================================================
	AkFxContext.cpp - AkFXMemAlloc (akfxmemalloc.cpp 0x920690..0x9206b0), AkDataReferenceArray and the effect plug-in
	contexts (akfxcontext.cpp 0x93a570..0x93ab20); 2013 matched by bytes (0x91ce80.., 0x9370f0..).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Retail behaviour throughout; no deviation.
	  * PostMonitorData answers AK_Fail and CanPostMonitorData false: retail's release build has no profiler link
	    (the two slots fold onto 0x964820 "mov eax,2; retn 8" and 0x9dee90 "xor al,al; retn").
	  * GetMaxBufferLength is 1024 for every context (the folded 0x93a9d0).
=============================================================================*/
#include "AkFxContext.h"
#include "AkPBI.h"
#include "AkBankMgr.h"
#include "AkHierarchy.h"
#include "AkAudioLibIndex.h"
#include "AkRuntimeEnvironmentMgr.h"
#include "AkVPLMixBusNode.h"
#include <AK/SoundEngine/Common/IAkStreamMgr.h>

AkFXMemAlloc AkFXMemAlloc::m_instanceUpper;
AkFXMemAlloc AkFXMemAlloc::m_instanceLower;

/** 2012 0x920690, 2013 0x91ce80: 16-byte aligned blocks of the instance's pool. */
void * AkFXMemAlloc::Malloc( size_t in_uSize )
{
	return AK::MemoryMgr::Malign( m_poolId, (AkUInt32)in_uSize, 16 );
}

/** 2012 0x9206b0, 2013 0x91cea0. */
void AkFXMemAlloc::Free( void * in_pMemAddress )
{
	AK::MemoryMgr::Free( m_poolId, in_pMemAddress );
}

/** 2012 0x93a6a0: every acquired media is released with its bank slot. */
AkDataReferenceArray::~AkDataReferenceArray()
{
	for( MapStruct<AkUInt32, AkDataReference> * p = m_pItems; p != m_pItemsEnd; ++p )
	{
		if( p->item.pData && p->item.uSourceID != (AkUniqueID)-1 )
		{
			g_pBankManager->ReleaseMedia( p->item.uSourceID );
			if( p->item.pUsageSlot )
			{
				p->item.pUsageSlot->Release( false );
			}
		}
	}
	Term();
}

/** 2012 0x93a7e0: the media of a loaded bank, referenced until the context dies. */
AkDataReference * AkDataReferenceArray::AcquireData( AkUInt32 in_uDataIdx, AkUniqueID in_uSourceID )
{
	CAkUsageSlot * pUsageSlot = NULL;
	const AkMediaInfo mediaInfo = g_pBankManager->GetMedia( in_uSourceID, pUsageSlot );
	if( !mediaInfo.pInMemoryData )
	{
		return NULL;
	}
	AkDataReference * pRef = Set( in_uDataIdx );
	if( pRef )
	{
		pRef->pData = mediaInfo.pInMemoryData;
		pRef->uSize = mediaInfo.uInMemoryDataSize;
		pRef->uSourceID = in_uSourceID;
		pRef->pUsageSlot = pUsageSlot;
		return pRef;
	}
	g_pBankManager->ReleaseMedia( in_uSourceID );
	if( pUsageSlot )
	{
		pUsageSlot->Release( false );
	}
	return NULL;
}

/** 2012 0x93a670. */
bool AkDataReferenceArray::IsUsingThisSlot( const CAkUsageSlot * in_pUsageSlot )
{
	for( MapStruct<AkUInt32, AkDataReference> * p = m_pItems; p != m_pItemsEnd; ++p )
	{
		if( p->item.pUsageSlot == in_pUsageSlot )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x93a570. */
AK::IAkStreamMgr * CAkEffectContextBase::GetStreamMgr() const
{
	return AK::IAkStreamMgr::Get();
}

/** 2012 0x93a5a0. */
AK::IAkProcessorFeatures * CAkEffectContextBase::GetProcessorFeatures()
{
	return AK::AkRuntimeEnvironmentMgr::Instance();
}

/** 2012 0x93a880. */
CAkInsertFXContext::CAkInsertFXContext( CAkPBI * in_pCtx, AkUInt32 in_uFXIndex )
	: CAkEffectContextBase( in_uFXIndex )
	, m_pContext( in_pCtx )
{
}

/** 2012 0x93a8b0. */
AkPlayingID CAkInsertFXContext::GetPlayingID() const
{
	return m_pContext ? m_pContext->GetPlayingID() : AK_INVALID_PLAYING_ID;
}

/** 2012 0x93a900: the media index of the sound's insert effect, acquired once. */
void CAkInsertFXContext::GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize )
{
	AkDataReference * pRef = m_dataArray.Exists( in_dataIndex );
	if( !pRef )
	{
		AkUniqueID dataID = (AkUniqueID)-1;
		m_pContext->GetFXDataID( m_uFXIndex, in_dataIndex, dataID );
		if( dataID != (AkUniqueID)-1 )
		{
			pRef = m_dataArray.AcquireData( in_dataIndex, dataID );
		}
	}
	if( pRef )
	{
		out_rDataSize = pRef->uSize;
		out_rpData = pRef->pData;
	}
	else
	{
		out_rpData = NULL;
		out_rDataSize = 0;
	}
}

/** 2012 0x93a990. */
CAkBusFXContext::CAkBusFXContext( CAkBusFX * in_pBusFX, AkUInt32 in_uFXIndex, const AK::CAkBusCtx & in_rBusContext, AkUniqueID in_EnvID )
	: CAkEffectContextBase( in_uFXIndex )
	, m_pBusFX( in_pBusFX )
	, m_BuxCtx( in_rBusContext )
	, m_EnvID( in_EnvID )
{
}

/** 2012 0x93a580: an environment (aux send) bus's effect. */
bool CAkBusFXContext::IsSendModeEffect() const
{
	return m_EnvID != AK_INVALID_UNIQUE_ID;
}

/** 2012 0x93aa50: an environment's media comes from its share set, a bus effect's from the bus. */
void CAkBusFXContext::GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize )
{
	AkDataReference * pRef = m_dataArray.Exists( in_dataIndex );
	if( !pRef )
	{
		AkUniqueID dataID = (AkUniqueID)-1;
		if( IsSendModeEffect() )
		{
			CAkFxShareSet * pFx = g_pIndex->m_idxFxShareSets.GetPtrAndAddRef( m_EnvID );
			if( pFx )
			{
				dataID = pFx->GetMediaID( in_dataIndex );
				pFx->Release();
			}
		}
		else
		{
			m_BuxCtx.GetFXDataID( m_uFXIndex, in_dataIndex, dataID );
		}
		if( dataID != (AkUniqueID)-1 )
		{
			pRef = m_dataArray.AcquireData( in_dataIndex, dataID );
		}
	}
	if( pRef )
	{
		out_rDataSize = pRef->uSize;
		out_rpData = pRef->pData;
	}
	else
	{
		out_rpData = NULL;
		out_rDataSize = 0;
	}
}

/** 2012 0x93a720. */
CAkSourceFXContext::CAkSourceFXContext( CAkPBI * in_pCtx )
	: m_pContext( in_pCtx )
{
}

/** 2012 0x93a590. */
AkUInt16 CAkSourceFXContext::GetNumLoops() const
{
	return (AkUInt16)m_pContext->GetLooping();
}

/** 2012 0x93a570 (shared with CAkEffectContextBase). */
AK::IAkStreamMgr * CAkSourceFXContext::GetStreamMgr() const
{
	return AK::IAkStreamMgr::Get();
}

/** 2012 0x93a740. */
AkPlayingID CAkSourceFXContext::GetPlayingID() const
{
	return m_pContext ? m_pContext->GetPlayingID() : AK_INVALID_PLAYING_ID;
}

/** 2012 0x93a5a0 (shared). */
AK::IAkProcessorFeatures * CAkSourceFXContext::GetProcessorFeatures()
{
	return AK::AkRuntimeEnvironmentMgr::Instance();
}

/** 2012 0x93ab20: a source plug-in's media is its FX custom's (the source id is the custom's id). */
void CAkSourceFXContext::GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize )
{
	AkDataReference * pRef = m_dataArray.Exists( in_dataIndex );
	if( !pRef )
	{
		CAkFxCustom * pFx = g_pIndex->m_idxFxCustom.GetPtrAndAddRef( m_pContext->GetSource()->m_sSrcTypeInfo.mediaInfo.sourceID );
		if( pFx )
		{
			const AkUniqueID dataID = pFx->GetMediaID( in_dataIndex );
			pFx->Release();
			if( dataID != (AkUniqueID)-1 )
			{
				pRef = m_dataArray.AcquireData( in_dataIndex, dataID );
			}
		}
	}
	if( pRef )
	{
		out_rDataSize = pRef->uSize;
		out_rpData = pRef->pData;
	}
	else
	{
		out_rpData = NULL;
		out_rDataSize = 0;
	}
}
