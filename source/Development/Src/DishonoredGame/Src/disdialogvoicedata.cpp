// DishonoredGame/src/disdialogvoicedata.cpp
// DISHONORED(port): agent FR (PHASE17 FR). InitForGame 2013 rva 0x88b2e0 (2012 0x8f87a0, 5 B), BindToOneShot 0x8a4b00
// (0x914cb0, 48 B). BindToOneShot's StartEditing (editor-only, 2012 0x914890) is left out.
// Not ported (the voice / FaceFX / Wwise surface, agentFR.md inventory): PostLoad 2012 0x8fd550, FindTextBlurb 2013 0x8a1550,
// RebuildLookupMaps 0x90e1a0, FindDataPackage 0x8fd7a0, GetImportGroup 0x8fff00, FindAkBank 0x900bd0, GetAkBankName 0x8ffe00,
// GetFaceFXAnimSetName 0x8fffc0, GetFxAnimGroup 0x8f87b0, HasAkEvents 0x8fafc0, VerifyAndUpdateDataPackage 0x90f250,
// GetVoiceDataGUID 0x8f9820, GetDisplayName 0x8fdbc0, PostDuplicate 0x914c90.
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
