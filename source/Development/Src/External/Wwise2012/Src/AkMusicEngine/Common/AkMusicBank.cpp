/*=============================================================================
	AkMusicBank.cpp - AkMusicBank::LoadBankItem (akmusicbank.cpp, 2012 0x964d90): the bank manager's external handler
	for the music HIRC types (10 segment, 11 track, 12 music switch, 13 music random/sequence), installed by
	AK::MusicEngine::Init. Any other type is declined with AK_PartialSuccess so CAkBankMgr::ProcessHircChunk skips it.

	DISHONORED(port): agent IH4 fork M (wave 26). The readers are retail's template instances
	CAkBankMgr::StdBankRead<CAkMusicSegment / CAkMusicSwitchCntr / CAkMusicRanSeqCntr, CAkParameterNodeBase>
	0x964b80 / 0x964c30 / 0x964ce0 and CAkBankMgr::ReadSourceParent<CAkMusicTrack> 0x964a70, written as members of the
	bank manager's friend AkMusicBankReader because CAkBankMgr's own StdBankRead lives in AkBankMgr.cpp.
	Deviation (named): a freshly created node's parse end is checked with CAkBankMgr::CheckParseEnd (the -akdiag
	misparse count), as for the sound engine's own readers.
=============================================================================*/
#include "AkMusicRenderer.h"
#include "AkMusicSegment.h"
#include "AkMusicTrack.h"
#include "AkMusicSwitchCntr.h"
#include "AkMusicRanSeqCntr.h"
#include <AkAudioLibIndex.h>

struct AkMusicBankReader
{
	template <class T>
	static AKRESULT StdBankRead( CAkBankMgr & in_rBankMgr, const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
	{
		AKRESULT eResult = AK_Success;
		AkUInt8 * pData = (AkUInt8 *)in_rBankMgr.m_BankReader.GetData( in_rSection.dwSectionSize );
		if( !pData )
		{
			return AK_Fail;
		}
		const AkUniqueID ulID = *(AkUInt32 *)pData;
		T * pObject = static_cast<T *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ulID ) );
		if( !pObject )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			pObject = T::Create( ulID );
			if( pObject )
			{
				g_pHircParseEnd = NULL;
				eResult = pObject->SetInitialValues( pData, in_rSection.dwSectionSize );
				if( eResult != AK_Success )
				{
					pObject->Release();
				}
				else
				{
					in_rBankMgr.CheckParseEnd( in_rSection, pData );
				}
			}
			else
			{
				eResult = AK_Fail;
			}
		}
		if( eResult == AK_Success )
		{
			in_rBankMgr.AddLoadedItem( in_pUsageSlot, pObject );
		}
		in_rBankMgr.m_BankReader.ReleaseData();
		return eResult;
	}

	/** 2012 0x964a70: a track that already holds a bank source keeps it; one without is re-read for its sources only. */
	static AKRESULT ReadSourceParentTrack( CAkBankMgr & in_rBankMgr, const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
	{
		AKRESULT eResult = AK_Success;
		AkUInt8 * pData = (AkUInt8 *)in_rBankMgr.m_BankReader.GetData( in_rSection.dwSectionSize );
		if( !pData )
		{
			return AK_Fail;
		}
		const AkUniqueID ulID = *(AkUInt32 *)pData;
		CAkMusicTrack * pTrack = static_cast<CAkMusicTrack *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ulID ) );
		if( pTrack )
		{
			if( pTrack->m_arSrcInfo.Length() == 0 || !pTrack->HasBankSource() )
			{
				{
					AkAutoLock<CAkLock> gate( g_csMain );
					eResult = pTrack->SetInitialValues( pData, in_rSection.dwSectionSize, in_pUsageSlot, true );
				}
				if( eResult != AK_Success )
				{
					pTrack->Release();
				}
			}
		}
		else
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			pTrack = CAkMusicTrack::Create( ulID );
			if( pTrack )
			{
				g_pHircParseEnd = NULL;
				eResult = pTrack->SetInitialValues( pData, in_rSection.dwSectionSize, in_pUsageSlot, false );
				if( eResult != AK_Success )
				{
					pTrack->Release();
				}
				else
				{
					in_rBankMgr.CheckParseEnd( in_rSection, pData );
				}
			}
			else
			{
				eResult = AK_Fail;
			}
		}
		if( eResult == AK_Success )
		{
			in_rBankMgr.AddLoadedItem( in_pUsageSlot, pTrack );
		}
		in_rBankMgr.m_BankReader.ReleaseData();
		return eResult;
	}
};

namespace AkMusicBank
{
	/** 2012 0x964d90, 2013 0x9615d0. */
	AKRESULT LoadBankItem( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot, AkUInt32 in_dwBankID )
	{
		switch( in_rSection.eHircType )
		{
		case AkBank::HIRCType_Segment:
			return AkMusicBankReader::StdBankRead<CAkMusicSegment>( *g_pBankManager, in_rSection, in_pUsageSlot );
		case AkBank::HIRCType_Track:
			return AkMusicBankReader::ReadSourceParentTrack( *g_pBankManager, in_rSection, in_pUsageSlot );
		case AkBank::HIRCType_MusicSwitch:
			return AkMusicBankReader::StdBankRead<CAkMusicSwitchCntr>( *g_pBankManager, in_rSection, in_pUsageSlot );
		case AkBank::HIRCType_MusicRanSeq:
			return AkMusicBankReader::StdBankRead<CAkMusicRanSeqCntr>( *g_pBankManager, in_rSection, in_pUsageSlot );
		default:
			return AK_PartialSuccess;
		}
	}
}
