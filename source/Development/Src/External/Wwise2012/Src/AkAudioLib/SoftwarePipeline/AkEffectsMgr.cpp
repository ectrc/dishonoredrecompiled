/*=============================================================================
	AkEffectsMgr.cpp - CAkEffectsMgr (akeffectsmgr.cpp, 2012 0x925330..0x925570 and the list initializers
	0xbab150 / 0xbab160; 2013 matched by bytes): the codec half.

	DISHONORED(port): agent IH2 fork S (wave 26). A registered type's key is its AkPluginID:
	(plugin ID << 16) | (company ID << 4) | plugin type, the codec type being 1.
	Agent IH4 fork F1: CAkEffectsMgr::Alloc / AllocParams (0x9254b0 / 0x925510, 2013 0x921ca0 / 0x921d00), the
	plug-in instances and parameter objects from the registered factories.
	Deviations (named):
	  * The motion half (m_RegisteredFeedbackBus, RegisterFeedbackBus 0x9255c0, AllocFeedbackBus 0x925610) is not
	    declared - no feedback device exists - so Term frees the two lists it has, in retail's order.
=============================================================================*/
#include "AkVPLNode.h"

CAkKeyArray<AkPluginID, CAkEffectsMgr::EffectTypeRecord, 5> CAkEffectsMgr::m_RegisteredFXList;
CAkKeyArray<AkCodecID, CAkEffectsMgr::CodecTypeRecord, 5> CAkEffectsMgr::m_RegisteredCodecList;

/** 2012 0x925330. */
AKRESULT CAkEffectsMgr::Term()
{
	m_RegisteredFXList.Term();
	m_RegisteredCodecList.Term();
	return AK_Success;
}

/** 2012 0x9253d0: a type registers once; both factories are required. */
AKRESULT CAkEffectsMgr::RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AK::AkCreatePluginCallback in_pCreateFunc, AK::AkCreateParamCallback in_pCreateParamFunc )
{
	const AkPluginID uPluginID = ( in_ulPluginID << 16 ) + ( in_ulCompanyID << 4 ) + ( in_eType & 0xF );
	if( m_RegisteredFXList.Exists( uPluginID ) )
	{
		return AK_Fail;
	}
	if( !in_pCreateFunc || !in_pCreateParamFunc )
	{
		return AK_InvalidParameter;
	}
	EffectTypeRecord record;
	record.pCreateFunc = in_pCreateFunc;
	record.pCreateParamFunc = in_pCreateParamFunc;
	return m_RegisteredFXList.Set( uPluginID, record ) ? AK_Success : AK_Fail;
}

/** 2012 0x9254b0: AK_Fail for an unregistered plug-in or a failed creation. */
AKRESULT CAkEffectsMgr::Alloc( AK::IAkPluginMemAlloc * in_pAllocator, AkPluginID in_EffectTypeID, AK::IAkPlugin *& out_pEffect )
{
	out_pEffect = NULL;
	EffectTypeRecord * pRecord = m_RegisteredFXList.Exists( in_EffectTypeID );
	if( !pRecord )
	{
		return AK_Fail;
	}
	out_pEffect = pRecord->pCreateFunc( in_pAllocator );
	return out_pEffect ? AK_Success : AK_Fail;
}

/** 2012 0x925510: a plug-in registered without a parameter factory has no parameter object (AK_Success). */
AKRESULT CAkEffectsMgr::AllocParams( AK::IAkPluginMemAlloc * in_pAllocator, AkPluginID in_EffectTypeID, AK::IAkPluginParam *& out_pEffectParam )
{
	out_pEffectParam = NULL;
	EffectTypeRecord * pRecord = m_RegisteredFXList.Exists( in_EffectTypeID );
	if( !pRecord )
	{
		return AK_Fail;
	}
	if( !pRecord->pCreateParamFunc )
	{
		return AK_Success;
	}
	out_pEffectParam = pRecord->pCreateParamFunc( in_pAllocator );
	return out_pEffectParam ? AK_Success : AK_Fail;
}

/** 2012 0x925440. */
AKRESULT CAkEffectsMgr::RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc )
{
	const AkCodecID uCodecID = ( in_ulPluginID << 16 ) + ( in_ulCompanyID << 4 ) + AkPluginTypeCodec_Value;
	if( m_RegisteredCodecList.Exists( uCodecID ) )
	{
		return AK_Fail;
	}
	if( !in_pFileCreateFunc || !in_pBankCreateFunc )
	{
		return AK_InvalidParameter;
	}
	CodecTypeRecord record;
	record.pFileCreateFunc = in_pFileCreateFunc;
	record.pBankCreateFunc = in_pBankCreateFunc;
	return m_RegisteredCodecList.Set( uCodecID, record ) ? AK_Success : AK_Fail;
}

/** 2012 0x925570: the file factory for a streamed source (type 1), the bank factory otherwise. */
IAkSoftwareCodec * CAkEffectsMgr::AllocCodec( CAkPBI * in_pCtx, AkUInt32 in_uSrcType, AkCodecID in_CodecID )
{
	CodecTypeRecord * pRecord = m_RegisteredCodecList.Exists( in_CodecID );
	if( !pRecord )
	{
		return NULL;
	}
	if( in_uSrcType == 1 )
	{
		return pRecord->pFileCreateFunc( in_pCtx );
	}
	return pRecord->pBankCreateFunc( in_pCtx );
}
