/*=============================================================================
	AkDynamicSequence.h - CAkDynamicSequence (akdynamicsequence.cpp, 0x88 bytes) - the PBI instigator of a dynamic
	sequence, holding the game's playlist - and CAkDynamicSequencePBI (akdynamicsequencepbi.cpp, 0x1C8 bytes), the
	continuous PBI that asks the sequence for its next item. Plus the public Playlist / PlaylistItem of the SDK's
	AkDynamicSequence.h (header inlines).

	DISHONORED(port): agent IH3, fork C (wave 26). Layouts are the 2012 PDB's (m_userParams at +64 with its playing id
	at +80, m_bClosed at +88 - ProcessMsgQueue's DynamicSequenceCmd Close writes those offsets).
	Deviation (named): Category() answers ObjCategory_Undefined (the port's CAkIndexable declares it pure; retail's
	dynamic sequence has no Category override in the PDB).
=============================================================================*/
#pragma once

#include "AkContinuousPBI.h"
#include <AK/SoundEngine/Common/AkDynamicSequence.h>

namespace AK
{
	namespace SoundEngine
	{
		namespace DynamicSequence
		{
			class PlaylistItem
			{
			public:
				PlaylistItem() : audioNodeID( AK_INVALID_UNIQUE_ID ), msDelay( 0 ), pCustomInfo( NULL ), pExternalSrcs( NULL ) {}
				PlaylistItem( const PlaylistItem & in_rCopy ) : audioNodeID( AK_INVALID_UNIQUE_ID ), msDelay( 0 ), pCustomInfo( NULL ), pExternalSrcs( NULL ) { *this = in_rCopy; }
				~PlaylistItem()
				{
					if( pExternalSrcs )
					{
						pExternalSrcs->Release();
					}
				}
				PlaylistItem & operator=( const PlaylistItem & in_rCopy );

				AkUniqueID				audioNodeID;
				AkTimeMs				msDelay;
				void *					pCustomInfo;
				AkExternalSourceArray *	pExternalSrcs;
			};

			class Playlist : public AkArray<PlaylistItem, const PlaylistItem &, ArrayPoolDefault, 4>
			{
			public:
				AKRESULT Enqueue( AkUniqueID in_audioNodeID, AkTimeMs in_msDelay = 0, void * in_pCustomInfo = NULL, AkUInt32 in_cExternals = 0, AkExternalSourceInfo * in_pExternalSources = NULL )
				{
					PlaylistItem * pItem = AddLast();
					if( !pItem )
					{
						return AK_Fail;
					}
					pItem->audioNodeID = in_audioNodeID;
					pItem->msDelay = in_msDelay;
					pItem->pCustomInfo = in_pCustomInfo;
					if( in_cExternals )
					{
						pItem->pExternalSrcs = AkExternalSourceArray::Create( in_cExternals, in_pExternalSources );
						if( !pItem->pExternalSrcs )
						{
							return AK_Fail;
						}
					}
					return AK_Success;
				}
			};
		}
	}
}

class CAkDynamicSequence : public CAkPBIAware
{
public:
	enum AkDynamicSequenceState
	{
		State_Stopped	= 0,
		State_Playing	= 1,
		State_Waiting	= 2
	};

	enum AkDynamicSequenceStopMode
	{
		AK_StopImmediate	= 0,
		AK_StopAfterCurrentItem	= 1
	};

	static CAkDynamicSequence * Create( AkPlayingID in_PlayingID, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType );
	virtual ~CAkDynamicSequence();
	virtual AkUInt32 AddRef();
	virtual AkUInt32 Release();
	virtual AkObjectCategory Category() { return ObjCategory_Undefined; }
	virtual CAkPBI * CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const;

	AKRESULT Init();
	void SetGameObject( CAkRegisteredObj * in_pGameObj );
	void UnlockPlaylist();
	AkUniqueID GetNextToPlay( AkTimeMs & out_delay, void *& out_rpCustomParam, UserParams & out_rUserParam );
	AKRESULT Play( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
	AKRESULT Pause( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
	AKRESULT Resume( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
	AKRESULT Stop( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
	AKRESULT Break();
	void ResumeWaiting();
	void AllExec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj );

	AK::SoundEngine::DynamicSequence::Playlist		m_playList;
	CAkLock											m_lockPlaylist;
	AkDynamicSequenceState							m_eState;
	CAkRegisteredObj *								m_pGameObj;
	UserParams										m_userParams;
	bool											m_bClosed;
	AK::SoundEngine::DynamicSequence::DynamicSequenceType	m_eDynamicSequenceType;
	AK::SoundEngine::DynamicSequence::PlaylistItem	m_playingItem;
	AK::SoundEngine::DynamicSequence::PlaylistItem	m_queuedItem;
	AkUInt32										m_ulPauseCount;
	AkUInt32										m_uSequenceID;

protected:
	CAkDynamicSequence( AkPlayingID in_playingID, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType );
	AkUniqueID _GetNextToPlay( AkTimeMs & out_delay, void *& out_rpCustomParam );
	AKRESULT _PlayNode( AkUniqueID in_nodeID, AkTimeMs in_delay, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
	AKRESULT _Stop( AkDynamicSequenceStopMode in_eStopMode, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve );
};

class CAkDynamicSequencePBI : public CAkContinuousPBI
{
public:
	CAkDynamicSequencePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, ContParams & in_rCparameters, UserParams & in_rUserparams, PlayHistory & in_rPlayHistory, bool in_bIsFirst, AkUniqueID in_SeqID, CAkPBIAware * in_pInstigator, const PriorityInfoCurrent & in_rPriority, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter );
	virtual void Term( bool in_bFailedToInit );
	virtual void PrepareNextToPlay( bool in_bIsPreliminary );

	bool					m_bRequestNextFromDynSeq;
	AK::SoundEngine::DynamicSequence::DynamicSequenceType	m_eDynamicSequenceType;
	AkUniqueID				m_startingNode;
	void *					m_pDynSecCustomInfo;

private:
	AKRESULT PlayNextElement( AkUniqueID in_nextElementID, AkTimeMs in_delay );
};
