/*=============================================================================
	AkMusicTransAware.cpp - CAkMusicTransAware (akmusictransaware.cpp, 2012 0x96c1d0..0x96c7d0).

	DISHONORED(port): agent IH4 (wave 26).
	Deviation (named): the panic rule is a function-local static built on every call as retail's (its atexit
	destructor 0xbaedb0 is the compiler's); the source sync bits it sets (0x27) are SyncTypeExitMarker + bPlayPostExit.
=============================================================================*/
#include "AkMusicTransAware.h"

/** 2012 0x96c1d0, 2013 0x9689f0. */
CAkMusicTransAware::CAkMusicTransAware( AkUniqueID in_ulID )
	: CAkMusicNode( in_ulID )
{
}

/** 2012 0x96c530, 2013 0x968d50 (the rules' transition objects are freed by AkArray<AkMusicTransitionRule,..,0>::RemoveAll 0x96c4f0). */
CAkMusicTransAware::~CAkMusicTransAware()
{
	m_arTrRules.Term();
}

/** 2012 0x96c200, 2013 0x968a20: the most recently authored rule matching (src or any, dest or any); the first rule otherwise. */
const AkMusicTransitionRule & CAkMusicTransAware::GetTransitionRule( AkUniqueID in_srcID, AkUniqueID in_destID )
{
	for( AkInt32 i = (AkInt32)m_arTrRules.Length() - 1; i >= 0; --i )
	{
		const AkMusicTransitionRule & rule = m_arTrRules[ i ];
		if( ( rule.srcID == (AkUniqueID)-1 || rule.srcID == in_srcID ) && ( rule.destID == (AkUniqueID)-1 || rule.destID == in_destID ) )
		{
			return rule;
		}
	}
	return *m_arTrRules.m_pItems;
}

/**
 * 2012 0x96c260: as above, but a rule source / destination can also name an ancestor of the source / destination
 * segment below the owner; a destination rule naming a music random/sequence container is "sequence specific".
 */
const AkMusicTransitionRule & CAkMusicTransAware::GetTransitionRule( CAkParameterNodeBase * in_pOwnerNode, AkUniqueID in_srcID, CAkParameterNodeBase * in_pSrcParentNode, AkUniqueID in_destID, CAkParameterNodeBase * in_pDestParentNode, bool & out_bIsDestSequenceSpecific )
{
	for( AkInt32 i = (AkInt32)m_arTrRules.Length() - 1; i >= 0; --i )
	{
		const AkMusicTransitionRule & rule = m_arTrRules[ i ];
		bool bSrcMatch = rule.srcID == (AkUniqueID)-1 || rule.srcID == in_srcID;
		if( !bSrcMatch )
		{
			for( CAkParameterNodeBase * pNode = in_pSrcParentNode; pNode && pNode != in_pOwnerNode; pNode = pNode->m_pParentNode )
			{
				if( pNode->key == rule.srcID )
				{
					bSrcMatch = true;
					break;
				}
			}
		}
		if( !bSrcMatch )
		{
			continue;
		}
		if( rule.destID == (AkUniqueID)-1 || rule.destID == in_destID )
		{
			out_bIsDestSequenceSpecific = false;
			return rule;
		}
		for( CAkParameterNodeBase * pNode = in_pDestParentNode; pNode && pNode != in_pOwnerNode; pNode = pNode->m_pParentNode )
		{
			if( pNode->key == rule.destID )
			{
				out_bIsDestSequenceSpecific = pNode->NodeCategory() == AkNodeCategory_MusicRanSeqCntr;
				return rule;
			}
		}
	}
	return *m_arTrRules.m_pItems;
}

/** 2012 0x96c330, 2013 0x968b50: any to any, exit at the exit cue playing the post-exit, enter at the entry cue, no fades. */
const AkMusicTransitionRule & CAkMusicTransAware::GetPanicTransitionRule()
{
	static AkMusicTransitionRule panicRule;
	panicRule.srcID = (AkUniqueID)-1;
	panicRule.destID = (AkUniqueID)-1;
	panicRule.srcRule.eSyncType = SyncTypeExitMarker;
	panicRule.srcRule.bPlayPostExit = 1;
	panicRule.srcRule.fadeParams.transitionTime = 0;
	panicRule.srcRule.fadeParams.iFadeOffset = 0;
	panicRule.srcRule.uCueFilterHash = 0;
	panicRule.destRule.fadeParams.transitionTime = 0;
	panicRule.destRule.fadeParams.iFadeOffset = 0;
	panicRule.destRule.uCueFilterHash = 0;
	panicRule.destRule.uJumpToID = 0;
	panicRule.destRule.eEntryType = EntryTypeEntryMarker;
	panicRule.destRule.bPlayPreEntry = 0;
	panicRule.destRule.bDestMatchSourceCueName = 0;
	panicRule.pTransObj = NULL;
	return panicRule;
}

/** 2012 0x96c3d0, 2013 0x968bf0: the node's stingers, then every transition object's segment. */
AKRESULT CAkMusicTransAware::PrepareMusicalDependencies()
{
	AKRESULT eResult = CAkMusicNode::PrepareMusicalDependencies();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	AkUInt32 uRule = 0;
	for( ; uRule < m_arTrRules.Length(); ++uRule )
	{
		const AkMusicTransitionObject * pTransObj = m_arTrRules[ uRule ].pTransObj;
		if( pTransObj )
		{
			eResult = CAkParameterNodeBase::PrepareNodeData( pTransObj->segmentID );
			if( eResult != AK_Success )
			{
				break;
			}
		}
	}
	if( eResult != AK_Success )
	{
		for( AkUInt32 i = 0; i < uRule; ++i )
		{
			const AkMusicTransitionObject * pTransObj = m_arTrRules[ i ].pTransObj;
			if( pTransObj )
			{
				CAkParameterNodeBase::UnPrepareNodeData( pTransObj->segmentID );
			}
		}
		CAkMusicNode::UnPrepareMusicalDependencies();
	}
	return eResult;
}

/** 2012 0x96c480, 2013 0x968ca0. */
void CAkMusicTransAware::UnPrepareMusicalDependencies()
{
	for( AkUInt32 i = 0; i < m_arTrRules.Length(); ++i )
	{
		const AkMusicTransitionObject * pTransObj = m_arTrRules[ i ].pTransObj;
		if( pTransObj )
		{
			CAkParameterNodeBase::UnPrepareNodeData( pTransObj->segmentID );
		}
	}
	CAkMusicNode::UnPrepareMusicalDependencies();
}

/** 2012 0x96c5a0, 2013 0x968dc0: fade offsets convert ms to samples (frequency / 1000 truncated, as retail); 0 rules is AK_Fail. */
AKRESULT CAkMusicTransAware::SetRules( AkUInt32 in_NumRules, AkWwiseMusicTransitionRule * in_pRules )
{
	m_arTrRules.Term();
	if( !in_NumRules || m_arTrRules.Reserve( in_NumRules ) != AK_Success )
	{
		return AK_Fail;
	}
	const AkInt32 iSamplesPerMs = (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	for( AkUInt32 i = 0; i < in_NumRules; ++i )
	{
		const AkWwiseMusicTransitionRule & in = in_pRules[ i ];
		AkMusicTransitionRule * pRule = m_arTrRules.AddLast();
		pRule->srcID = in.srcID;
		pRule->destID = in.destID;
		pRule->srcRule.bPlayPostExit = in.bSrcPlayPostExit;
		pRule->srcRule.eSyncType = in.eSrcSyncType;
		pRule->srcRule.uCueFilterHash = in.uSrcCueFilterHash;
		pRule->srcRule.fadeParams.transitionTime = in.srcFade.transitionTime;
		pRule->srcRule.fadeParams.eFadeCurve = in.srcFade.eFadeCurve;
		pRule->srcRule.fadeParams.iFadeOffset = in.srcFade.iFadeOffset * iSamplesPerMs;
		pRule->destRule.fadeParams.transitionTime = in.destFade.transitionTime;
		pRule->destRule.fadeParams.eFadeCurve = in.destFade.eFadeCurve;
		pRule->destRule.fadeParams.iFadeOffset = in.destFade.iFadeOffset * iSamplesPerMs;
		pRule->destRule.uCueFilterHash = in.uDestCueFilterHash;
		pRule->destRule.uJumpToID = in.uDestJumpToID;
		pRule->destRule.eEntryType = in.eDestEntryType;
		pRule->destRule.bPlayPreEntry = in.bDestPlayPreEntry & 1;
		pRule->destRule.bDestMatchSourceCueName = in.bDestMatchSourceCueName & 1;
		if( in.bIsTransObjectEnabled )
		{
			pRule->pTransObj = (AkMusicTransitionObject *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicTransitionObject ) );
			if( pRule->pTransObj )
			{
				pRule->pTransObj->bPlayPostExit = in.bPlayPostExit & 1;
				pRule->pTransObj->bPlayPreEntry = in.bPlayPreEntry & 1;
				pRule->pTransObj->fadeInParams.transitionTime = in.transFadeIn.transitionTime;
				pRule->pTransObj->fadeInParams.eFadeCurve = in.transFadeIn.eFadeCurve;
				pRule->pTransObj->fadeInParams.iFadeOffset = in.transFadeIn.iFadeOffset * iSamplesPerMs;
				pRule->pTransObj->fadeOutParams.transitionTime = in.transFadeOut.transitionTime;
				pRule->pTransObj->fadeOutParams.eFadeCurve = in.transFadeOut.eFadeCurve;
				pRule->pTransObj->fadeOutParams.iFadeOffset = in.transFadeOut.iFadeOffset * iSamplesPerMs;
				pRule->pTransObj->segmentID = in.segmentID;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x96c7d0, 2013 0x968ff0: the music node part, then the rules (92 bytes each in the bank, field by field). */
AKRESULT CAkMusicTransAware::SetMusicTransNodeParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly )
{
	AKRESULT eResult = SetMusicNodeParams( io_rpData, io_rulDataSize, in_bPartialLoadOnly );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 uNumRules = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( !uNumRules )
	{
		return AK_Success;
	}
	AkWwiseMusicTransitionRule * pRules = (AkWwiseMusicTransitionRule *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkWwiseMusicTransitionRule ) * uNumRules );
	if( !pRules )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < uNumRules; ++i )
	{
		AkWwiseMusicTransitionRule & r = pRules[ i ];
		r.srcID = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.destID = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.srcFade.transitionTime = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.srcFade.eFadeCurve = (AkCurveInterpolation)*(AkUInt32 *)io_rpData; io_rpData += 4;
		r.srcFade.iFadeOffset = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.eSrcSyncType = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.uSrcCueFilterHash = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.bSrcPlayPostExit = *io_rpData++;
		r.destFade.transitionTime = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.destFade.eFadeCurve = (AkCurveInterpolation)*(AkUInt32 *)io_rpData; io_rpData += 4;
		r.destFade.iFadeOffset = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.uDestCueFilterHash = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.uDestJumpToID = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.eDestEntryType = *(AkUInt16 *)io_rpData; io_rpData += 2;
		r.bDestPlayPreEntry = *io_rpData++;
		r.bDestMatchSourceCueName = *io_rpData++;
		r.bIsTransObjectEnabled = *io_rpData++;
		r.segmentID = *(AkUInt32 *)io_rpData; io_rpData += 4;
		r.transFadeIn.transitionTime = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.transFadeIn.eFadeCurve = (AkCurveInterpolation)*(AkUInt32 *)io_rpData; io_rpData += 4;
		r.transFadeIn.iFadeOffset = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.transFadeOut.transitionTime = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.transFadeOut.eFadeCurve = (AkCurveInterpolation)*(AkUInt32 *)io_rpData; io_rpData += 4;
		r.transFadeOut.iFadeOffset = *(AkInt32 *)io_rpData; io_rpData += 4;
		r.bPlayPreEntry = *io_rpData++;
		r.bPlayPostExit = *io_rpData++;
	}
	SetRules( uNumRules, pRules );
	AK::MemoryMgr::Free( g_DefaultPoolId, pRules );
	return AK_Success;
}
