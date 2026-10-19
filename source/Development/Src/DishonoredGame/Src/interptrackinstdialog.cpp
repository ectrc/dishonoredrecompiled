// DishonoredGame/src/interptrackinstdialog.cpp
// PDB functions attributed to this file (7):
//   0x8d9360  public: static void __cdecl UInterpTrackInstDialog::InitializePrivateStaticClassUInterpTrackInstDialog(void)
//   0x8dc6e0  public: class UDisConv_Blurb * __thiscall UInterpTrackInstDialog::GetBlurb(int)const
//   0x8dc730  public: class UDisConv_Blurb * __thiscall UInterpTrackInstDialog::GetAltBlurb(int, int)const
//   0x8e0290  public: virtual void __thiscall UInterpTrackInstDialog::TermTrackInst(class UInterpTrack *)
//   0x8e9a90  public: virtual void __thiscall UInterpTrackInstDialog::InitTrackInst(class UInterpTrack *)
//   0x8ec3b0  public: static class UClass * __cdecl UInterpTrackInstDialog::GetPrivateStaticClassUInterpTrackInstDialog(wchar_t const *)
//   0x8ede70  public: static class UClass * __cdecl UInterpTrackInstDialog::StaticClassNoInline(void)
//
// DISHONORED(port): agent IE (PHASE23 IE). 2013 rvas (2012 in parentheses), every one matched by bytes in retail2013:
//   InitTrackInst 0x882330 (0x8e9a90, 1143 B; 2013 reads the blurb GUID at +136, 2012 at +128 - the blurb grew by 8)
//   TermTrackInst 0x873ab0 (0x8e0290)    GetBlurb 0x86ee10 (0x8dc6e0)    GetAltBlurb 0x86ee60 (0x8dc730)
// The keys' blurbs come from UDisConv_Soiree::FindBlurbs on the soiree node that started the matinee (StartFromDialog), keyed
// by the group's name: a key whose GUID matches none of them keeps a NULL main blurb, an unmatched alternative a NULL slot.

#include "DishonoredGame.h"
#include "iediag.h"

void DishonoredLinkIE_InterpTrackInstDialog()
{
}

void UInterpTrackInstDialog::InitTrackInst( UInterpTrack* _Track )
{
	Track = _Track;
	CurrentBlurbIndex = -1;
	LastCompletedBlurbIndex = -1;
	m_LastPosition = -1.0f;
	m_pGroupPawn = Cast<ADishonoredNPCPawn>( GetGroupActor() );
	UInterpTrackDialog* TrDialog = (UInterpTrackDialog*)Track;
	const UDisConv_Soiree* Soiree = ConstCast<UDisConv_Soiree>( GetMatinee()->GetConversationNodePointer() );
	if( !Soiree )
	{
		IEDIAG( "dialog track %s: the matinee has no soiree node, %d key(s) left without blurbs", *TrDialog->GetPathName(), TrDialog->DialogKeys.Num() );
		return;
	}
	TArray<UDisConv_Blurb*> Blurbs;
	Soiree->FindBlurbs( TrDialog->GetOwningGroup()->GroupName, Blurbs, NULL );
	TArray<FGuid> BlurbGUID;
	BlurbGUID.AddZeroed( Blurbs.Num() );
	for( INT i = 0; i < Blurbs.Num(); i++ )
	{
		BlurbGUID(i) = Blurbs(i)->m_iBlurbGUID;
	}
	for( INT KeyIndex = 0; KeyIndex < TrDialog->DialogKeys.Num(); KeyIndex++ )
	{
		const UInterpTrackDialogKeyProperties* KeyProp = TrDialog->DialogKeys(KeyIndex).Properties;
		FDialogControlTrackInstKey KeyBlurb( EC_EventParm );
		INT GuidIndex = INDEX_NONE;
		if( BlurbGUID.FindItem( KeyProp->m_BlurbGUID, GuidIndex ) )
		{
			KeyBlurb.m_pMainBlurb = Blurbs(GuidIndex);
		}
		KeyBlurb.m_AltBlurbs.Empty( KeyProp->m_AltBlurbsGUID.Num() );
		for( INT AltIndex = 0; AltIndex < KeyProp->m_AltBlurbsGUID.Num(); AltIndex++ )
		{
			GuidIndex = INDEX_NONE;
			KeyBlurb.m_AltBlurbs.AddItem( BlurbGUID.FindItem( KeyProp->m_AltBlurbsGUID(AltIndex), GuidIndex ) ? Blurbs(GuidIndex) : NULL );
		}
		IEDIAG( "dialog key %d of %s (group %s): start %.3f length %.3f blurb %s alts %d wait %d", KeyIndex, *TrDialog->GetPathName(),
			*TrDialog->GetOwningGroup()->GroupName.ToString(), TrDialog->DialogKeys(KeyIndex).StartTime, TrDialog->DialogKeys(KeyIndex).KeyLength,
			KeyBlurb.m_pMainBlurb ? *KeyBlurb.m_pMainBlurb->GetName() : TEXT("<none>"), KeyBlurb.m_AltBlurbs.Num(), KeyProp->WaitForEndOfBlurb ? 1 : 0 );
		m_DialogKeysBlurbs.AddItem( KeyBlurb );
	}
	IEDIAG( "dialog track %s: soiree %s found %d blurb(s) for group %s", *TrDialog->GetPathName(), *Soiree->GetName(), Blurbs.Num(),
		*TrDialog->GetOwningGroup()->GroupName.ToString() );
}

void UInterpTrackInstDialog::TermTrackInst( UInterpTrack* _Track )
{
	if( CurrentBlurbIndex < 0 )
	{
		return;
	}
	UInterpTrackDialog* TrDialog = (UInterpTrackDialog*)Track;
	UDisConv_Blurb* Blurb = GetBlurb( CurrentBlurbIndex );
	if( TrDialog->DialogKeys(CurrentBlurbIndex).Properties && Blurb )
	{
		FDisDialogRunningInstance* RunningInst = (FDisDialogRunningInstance*)GetMatinee()->m_pDialogTree_RunningInst;
		if( RunningInst )
		{
			IEDIAG( "dialog term %s: key %d interrupted (behaviour change)", *TrDialog->GetPathName(), CurrentBlurbIndex );
			Blurb->InterruptTickingNode( RunningInst, DisConvoEndReason_BehaviorChange );
		}
	}
}

UDisConv_Blurb* UInterpTrackInstDialog::GetBlurb( INT iKeyIndex ) const
{
	return m_DialogKeysBlurbs(iKeyIndex).m_pMainBlurb;
}

UDisConv_Blurb* UInterpTrackInstDialog::GetAltBlurb( INT iKeyIndex, INT iBlurbIndex ) const
{
	return m_DialogKeysBlurbs(iKeyIndex).m_AltBlurbs(iBlurbIndex);
}
