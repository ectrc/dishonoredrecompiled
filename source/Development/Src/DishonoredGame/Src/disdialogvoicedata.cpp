// DishonoredGame/src/disdialogvoicedata.cpp
// DISHONORED(port): agent FR (PHASE17 FR). InitForGame 2013 rva 0x88b2e0 (2012 0x8f87a0, 5 B), BindToOneShot 0x8a4b00
// (0x914cb0, 48 B). BindToOneShot's StartEditing (editor-only, 2012 0x914890) is left out.
// Agent ID (PHASE23 ID): PostLoad 0x892450* (2012 0x8fd550; 2013 hashes m_Text where 2012 hashed m_MicrophoneRecordedAudio),
// RebuildLookupMaps 0x8a1440 (0x90e1a0), FindTextBlurb 0x8a1550 (0x90e2a0). PostLoad's text hash
// (DisStriHashWithScanFunc + DisStrScan_IgnoreWS_SkipPunct, 2012 0x824750 / 0x8247c0, unported) feeds only m_TextHash, an
// editor field, and MarkPackageDirty; it is named, not computed. The m_Text it was taken from is emptied as in retail.
// Not ported (the FaceFX / editor surface, agentFR.md inventory): FindDataPackage 0x8fd7a0, GetImportGroup 0x8fff00, FindAkBank
// 0x900bd0, GetAkBankName 0x8ffe00, GetFaceFXAnimSetName 0x8fffc0, GetFxAnimGroup 0x8f87b0, HasAkEvents 0x8fafc0,
// VerifyAndUpdateDataPackage 0x90f250, GetVoiceDataGUID 0x8f9820, GetDisplayName 0x8fdbc0, PostDuplicate 0x914c90,
// BeginDestroy 0x8a4ac0.
#include "DishonoredGame.h"

void UDisDialogVoiceData::InitForGame()
{
	m_bMapsBuilt = FALSE;
}

void UDisDialogVoiceData::BindToOneShot( ADisDialogOneShot* _pOneShot )
{
	m_bOneShot = TRUE;
	m_pBoundToOneShot = _pOneShot;
	m_pDialogTree = _pOneShot ? _pOneShot->m_pDialogTree : NULL;
}

void UDisDialogVoiceData::PostLoad()
{
	Super::PostLoad();
	for( INT ListIndex = 0; ListIndex < m_VoiceData.Num(); ListIndex++ )
	{
		FDisVoiceTextBlurbList& List = m_VoiceData(ListIndex);
		for( INT BlurbIndex = 0; BlurbIndex < List.m_Blurbs.Num(); BlurbIndex++ )
		{
			List.m_Blurbs(BlurbIndex).m_Text.Empty();
		}
	}
	if( m_pAkBank )
	{
		m_pAkBank->Load();
	}
}

void UDisDialogVoiceData::RebuildLookupMaps()
{
	for( INT ListIndex = 0; ListIndex < m_VoiceData.Num(); ListIndex++ )
	{
		FDisVoiceTextBlurbList& List = m_VoiceData(ListIndex);
		List.m_ID_Lookup.Empty( List.m_Blurbs.Num() );
		for( INT BlurbIndex = 0; BlurbIndex < List.m_Blurbs.Num(); BlurbIndex++ )
		{
			List.m_ID_Lookup.Set( List.m_Blurbs(BlurbIndex).m_GUID, BlurbIndex );
		}
	}
	m_bMapsBuilt = TRUE;
}

FDisVoiceTextBlurb* UDisDialogVoiceData::FindTextBlurb( const UDisDialogTree* _pTree, const FGuid& _rBlurbGUID )
{
	if( !m_bMapsBuilt )
	{
		RebuildLookupMaps();
	}
	for( INT ListIndex = 0; ListIndex < m_VoiceData.Num(); ListIndex++ )
	{
		FDisVoiceTextBlurbList& List = m_VoiceData(ListIndex);
		if( List.m_pTree != _pTree )
		{
			continue;
		}
		const INT* BlurbIndex = List.m_ID_Lookup.Find( _rBlurbGUID );
		if( BlurbIndex )
		{
			return &List.m_Blurbs( *BlurbIndex );
		}
	}
	return NULL;
}
