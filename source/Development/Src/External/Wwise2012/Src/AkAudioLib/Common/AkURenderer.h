/*=============================================================================
	AkURenderer.h - CAkURenderer (akurenderer.cpp): the upper renderer - every live PBI, the context notifications the
	lower engine posts back (play started, finished, play failed), the voice limits and kicks.

	DISHONORED(port): agent IH2 (wave 26). All members static, as in retail.
=============================================================================*/
#pragma once

#include "AkActivityChunk.h"

class CAkURenderer
{
public:
	struct ContextNotif
	{
		CAkPBI *			pPBI;
		AkCtxState			state;
		AkCtxDestroyReason	DestroyReason;
		AkReal32			fEstimatedLength;
	};

	static AKRESULT Init();
	static void Term();

	static AKRESULT Play( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams );
	static void Play( CAkPBI * in_pContext, TransParams & in_rTparameters, AkPlaybackState in_ePlaybackState );
	static AKRESULT Stop( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, AkPlayingID in_PlayingID );
	static AKRESULT Pause( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, AkPlayingID in_PlayingID );
	static AKRESULT Resume( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, bool in_bIsMasterResume, AkPlayingID in_PlayingID );
	static void StopAllPBIs( const CAkUsageSlot * in_pUsageSlot );

	static void EnqueueContext( CAkPBI * in_pContext );
	static void EnqueueContextNotif( CAkPBI * in_pPBI, AkCtxState in_eState, AkCtxDestroyReason in_eDestroyReason, AkReal32 in_fEstimatedTime );
	static void PerformContextNotif();

	static AKRESULT Kick( AkReal32 in_fPriority, CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, bool in_bKickNewest, bool in_bUseVirtualBehavior, CAkParameterNodeBase *& out_pKicked, KickFrom in_eReason );
	static bool ValidateMemoryLimit( AkReal32 in_fPriority );
	static AKRESULT ValidateMaximumNumberVoiceLimit( AkReal32 in_fPriority );
	static void DecrementVirtualCount( bool in_bAllowKick );
	static void ProcessLimiters();
	static AkReal32 GetMaxRadius( AkGameObjectID in_GameObjId );
	static AkReal32 GetMinDistance( const AkPositionKeeper & in_rPosKeep );
	static void AddBusLimiter( CAkLimiter * in_pLimiter ) { m_BusLimiters.AddFirst( in_pLimiter ); }
	static void AddAMLimiter( CAkLimiter * in_pLimiter ) { m_AMLimiters.AddFirst( in_pLimiter ); }
	static void RemoveBusLimiter( CAkLimiter * in_pLimiter ) { m_BusLimiters.Remove( in_pLimiter ); }
	static void RemoveAMLimiter( CAkLimiter * in_pLimiter ) { m_AMLimiters.Remove( in_pLimiter ); }
	static bool GetVirtualBehaviorAction( AkBelowThresholdBehavior in_belowThresholdBehavior ) { return in_belowThresholdBehavior != AkBelowThresholdBehavior_KillVoice; }

	static AkListBare<CAkPBI>										m_listCtxs;
	static CAkList2<ContextNotif, const ContextNotif &, 1, ArrayPoolDefault>	m_CtxNotifQueue;
	static AkListBareLight<CAkLimiter>								m_BusLimiters;
	static AkListBareLight<CAkLimiter>								m_AMLimiters;
	static CAkLimiter												m_GlobalLimiter;
	static AkUInt32													m_uNumVirtualizedSounds;

private:
	static PriorityInfoCurrent _CalcInitialPriority( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, AkReal32 & out_fMaxRadius );
	static AKRESULT ProcessCommand( ActionParamType in_eCommand, CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID, TransParams & in_rTparameters, bool in_bIsMasterResume );
	static void DestroyAllPBIs();
};
