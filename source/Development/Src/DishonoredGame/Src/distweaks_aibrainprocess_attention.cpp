// DishonoredGame/src/distweaks_aibrainprocess_attention.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4): the one below plus the three class-registration helpers of the macro.
//
// ---- agent GI ports (PHASE20 GI) ----

#include "DishonoredGame.h"

void DishonoredLinkUDisTweaks_AIBrainProcess_Attention()
{
}

// DISHONORED(port): 2012 rva 0x7991c0 / 2013 0x755580 (181 B): a drain never matches; with no conditions any other reason does;
// otherwise the reason type must be listed, and a Heard reason also by its noise context (the reason's first int).
UBOOL FDisAttentionReactionContext::EvaluateAttnChangeReason( const FDisAttentionChangeReason& _rAttnChangeReason ) const
{
	if( _rAttnChangeReason.m_ReasonType == DACRT_Drain )
	{
		return FALSE;
	}
	if( m_ChangeConditions.Num() <= 0 )
	{
		return TRUE;
	}
	for( INT i = 0; i < m_ChangeConditions.Num(); i++ )
	{
		const FDisAttnChangeCondition& Condition = m_ChangeConditions(i);
		if( Condition.m_ChangeReason != _rAttnChangeReason.m_ReasonType )
		{
			continue;
		}
		if( _rAttnChangeReason.m_ReasonType != DACRT_Heard )
		{
			return TRUE;
		}
		if( Condition.m_OptionalNoiseContexts.ContainsItem( (BYTE)_rAttnChangeReason.m_ReasonInfo.m_iParam1 ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}
