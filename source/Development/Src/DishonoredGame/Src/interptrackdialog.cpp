// DishonoredGame/src/interptrackdialog.cpp
// PDB functions attributed to this file (13):
//   0x8d9310  public: static void __cdecl UInterpTrackDialog::InitializePrivateStaticClassUInterpTrackDialog(void)
//   0x8d9330  public: static void __cdecl UInterpTrackDialogKeyProperties::InitializePrivateStaticClassUInterpTrackDialogKeyProperties(void)
//   0x8d9350  public: virtual void __thiscall UActorFactoryRigidBody::PostLoad(void)
//   0x8da7a0  public: virtual unsigned int __thiscall UInterpTrackDialog::IsPlayingASkippableItem(class UInterpTrackInst *)const
//   0x8dc5d0  public: virtual unsigned int __thiscall UInterpTrackDialog::FindNextSkipAnchor(float, float &)
//   0x8dc670  public: virtual unsigned int __thiscall UInterpTrackDialog::FindLastSkipAnchor(float &)
//   0x8e0100  public: virtual unsigned int __thiscall UInterpTrackInstDialog::NeedsSynchronizing(float, float, float &, unsigned int)
//   0x8e4830  public: static class UClass * __cdecl UInterpTrackDialogKeyProperties::GetPrivateStaticClassUInterpTrackDialogKeyProperties(wchar_t const *)
//   0x8e6e70  public: static class UClass * __cdecl UInterpTrackDialogKeyProperties::StaticClassNoInline(void)
//   0x8e9960  public: virtual class UObject * __thiscall UInterpTrackDialog::GetKeyProperties(int)
//   0x8ec050  public: virtual void __thiscall UInterpTrackInstDialog::UpdateTrackInst(float, unsigned int)
//   0x8edde0  public: static class UClass * __cdecl UInterpTrackDialog::GetPrivateStaticClassUInterpTrackDialog(wchar_t const *)
//   0x8ef0b0  public: static class UClass * __cdecl UInterpTrackDialog::StaticClassNoInline(void)
//
// DISHONORED(port): agent IE (PHASE23 IE). 2013 rvas (2012 in parentheses), every one matched by bytes in retail2013:
//   GetKeyProperties 0x86ebd0 (0x8e9960)          IsPlayingASkippableItem 0x8699a0 (0x8da7a0)
//   FindNextSkipAnchor 0x86ed00 (0x8dc5d0)        FindLastSkipAnchor 0x86eda0 (0x8dc670)
//   NeedsSynchronizing 0x8735c0 (0x8e0100)        UpdateTrackInst 0x873750 (0x8ec050)
// UpdateTrackInst plays the last key whose start the matinee crossed since the previous update (PlayBlurb_Shared through the
// 2013 thunk 0x89fb70 with look targets skipped and the line sought to the position inside the key), then ticks it once a
// frame (TickNode_Shared through 0x89fbd0) with the matinee's DeltaTime; a jump interrupts the playing line
// (DisConvoEndReason_MatineeSkipBlurb, 13). NeedsSynchronizing holds the matinee 0.0001 s before the end of a
// WaitForEndOfBlurb key until that key's blurb has completed. GetKeyProperties' editor transaction flags are left out
// (retail passes 0 flags to StaticConstructObject in 2013). Deviation: an instance whose InitTrackInst resolved no keys (the matinee
// had no soiree node) while a dialog instance is running is left alone; retail indexes m_DialogKeysBlurbs by key and would assert.

#include "DishonoredGame.h"
#include "iediag.h"

void DishonoredLinkIE_InterpTrackDialog()
{
}

static INT GDisIEDiag = -1;

UBOOL DisIEDiagEnabled()
{
	if( GDisIEDiag < 0 )
	{
		GDisIEDiag = ParseParam( appCmdLine(), TEXT("iediag") ) ? 1 : 0;
	}
	return GDisIEDiag != 0;
}

UObject* UInterpTrackDialog::GetKeyProperties( INT KeyIndex )
{
	if( DialogKeys(KeyIndex).Properties == NULL )
	{
		DialogKeys(KeyIndex).Properties = ConstructObject<UInterpTrackDialogKeyProperties>( UInterpTrackDialogKeyProperties::StaticClass(), this );
	}
	return DialogKeys(KeyIndex).Properties;
}

UBOOL UInterpTrackDialog::IsPlayingASkippableItem( UInterpTrackInst* _pInst ) const
{
	return ((UInterpTrackInstDialog*)_pInst)->CurrentBlurbIndex >= 0;
}

UBOOL UInterpTrackDialog::FindNextSkipAnchor( FLOAT CurrentPosition, FLOAT& OutAnchor )
{
	for( INT i = 0; i < DialogKeys.Num(); i++ )
	{
		if( DialogKeys(i).StartTime > CurrentPosition + 0.0001f )
		{
			OutAnchor = DialogKeys(i).StartTime - 0.0001f;
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL UInterpTrackDialog::FindLastSkipAnchor( FLOAT& OutAnchor )
{
	if( DialogKeys.Num() <= 0 )
	{
		return FALSE;
	}
	const FDialogControlTrackKey& LastKey = DialogKeys.Last();
	OutAnchor = LastKey.StartTime + LastKey.KeyLength;
	return TRUE;
}

static TMap<const UInterpTrackInstDialog*, FVector2D> GIEDiagSyncWait;

UBOOL UInterpTrackInstDialog::NeedsSynchronizing( FLOAT OldPosition, FLOAT NewPosition, FLOAT& OutClampedPosition, UBOOL bPreview )
{
	UInterpTrackDialog* TrDialog = (UInterpTrackDialog*)Track;
	if( m_DialogKeysBlurbs.Num() <= 0 || m_DialogKeysBlurbs.Num() != TrDialog->DialogKeys.Num() )
	{
		return FALSE;
	}
	INT KeyIndex = TrDialog->DialogKeys.Num() - 1;
	for( ; KeyIndex >= 0; KeyIndex-- )
	{
		const FDialogControlTrackKey& Key = TrDialog->DialogKeys(KeyIndex);
		if( m_DialogKeysBlurbs(KeyIndex).m_pMainBlurb && Key.StartTime + Key.KeyLength >= OldPosition && NewPosition >= Key.StartTime )
		{
			break;
		}
	}
	if( KeyIndex < 0 )
	{
		return FALSE;
	}
	const FDialogControlTrackKey& Key = TrDialog->DialogKeys(KeyIndex);
	if( !Key.Properties->WaitForEndOfBlurb || LastCompletedBlurbIndex == KeyIndex )
	{
		return FALSE;
	}
	const FLOAT KeyEnd = Key.StartTime + Key.KeyLength;
	if( KeyEnd < OldPosition || NewPosition < KeyEnd )
	{
		return FALSE;
	}
	OutClampedPosition = KeyEnd - 0.0001f;
	if( DisIEDiagEnabled() && !GIEDiagSyncWait.Find( this ) )
	{
		GIEDiagSyncWait.Set( this, FVector2D( GWorld->GetTimeSeconds(), KeyIndex ) );
		IEDIAG( "dialog sync wait starts: key %d of %s holds the matinee at %.3f (key %.3f + %.3f), blurb %s", KeyIndex, *TrDialog->GetPathName(),
			OutClampedPosition, Key.StartTime, Key.KeyLength, *m_DialogKeysBlurbs(KeyIndex).m_pMainBlurb->GetName() );
	}
	return TRUE;
}

void UInterpTrackInstDialog::UpdateTrackInst( FLOAT NewPosition, UBOOL bJump )
{
	USeqAct_Interp* Interp = GetMatinee();
	UInterpTrackDialog* TrDialog = (UInterpTrackDialog*)Track;
	FDisDialogRunningInstance* RunningInst = (FDisDialogRunningInstance*)Interp->m_pDialogTree_RunningInst;
	if( RunningInst && m_DialogKeysBlurbs.Num() == TrDialog->DialogKeys.Num() )
	{
		if( bJump )
		{
			if( CurrentBlurbIndex >= 0 && m_pPlayingBlurb )
			{
				IEDIAG( "dialog jump to %.3f on %s: key %d interrupted", NewPosition, *TrDialog->GetPathName(), CurrentBlurbIndex );
				m_pPlayingBlurb->InterruptTickingNode( RunningInst, DisConvoEndReason_MatineeSkipBlurb );
			}
			CurrentBlurbIndex = -1;
			LastCompletedBlurbIndex = -1;
			m_LastPosition = NewPosition - 0.0001f;
		}
		for( INT KeyIndex = TrDialog->DialogKeys.Num() - 1; KeyIndex >= 0; KeyIndex-- )
		{
			const FDialogControlTrackKey& Key = TrDialog->DialogKeys(KeyIndex);
			UDisConv_Blurb* MainBlurb = m_DialogKeysBlurbs(KeyIndex).m_pMainBlurb;
			if( !MainBlurb || NewPosition < Key.StartTime || Key.StartTime < m_LastPosition )
			{
				continue;
			}
			if( CurrentBlurbIndex != KeyIndex )
			{
				CurrentBlurbIndex = KeyIndex;
				UBOOL bNotFinished = FALSE;
				INT Picked = -1;
				if( Key.Properties->m_AltBlurbsGUID.Num() )
				{
					UDisConv_Blurb* PreviousBlurb = m_pPlayingBlurb;
					TArray<UDisConv_Blurb*> BlurbsToPickFrom;
					if( PreviousBlurb != MainBlurb )
					{
						BlurbsToPickFrom.AddItem( MainBlurb );
					}
					for( INT AltIndex = 0; AltIndex < Key.Properties->m_AltBlurbsGUID.Num(); AltIndex++ )
					{
						UDisConv_Blurb* AltBlurb = GetAltBlurb( KeyIndex, AltIndex );
						if( PreviousBlurb != AltBlurb )
						{
							BlurbsToPickFrom.AddItem( AltBlurb );
						}
					}
					Picked = rand() % BlurbsToPickFrom.Num();
					m_pPlayingBlurb = BlurbsToPickFrom(Picked);
				}
				else
				{
					m_pPlayingBlurb = MainBlurb;
				}
				const FLOAT SeekTime = NewPosition - Key.StartTime;
				m_pPlayingBlurb->PlayBlurb_Shared( bNotFinished, RunningInst, TRUE, SeekTime );
				IEDIAG( "dialog key %d fires on %s at matinee %.3f (key start %.3f, seek %.3f): blurb %s (pick %d of %d alts) wait %d, playing %d",
					KeyIndex, *TrDialog->GetPathName(), NewPosition, Key.StartTime, SeekTime, m_pPlayingBlurb ? *m_pPlayingBlurb->GetName() : TEXT("<none>"),
					Picked, Key.Properties->m_AltBlurbsGUID.Num(), Key.Properties->WaitForEndOfBlurb ? 1 : 0, bNotFinished ? 1 : 0 );
				if( !bNotFinished )
				{
					LastCompletedBlurbIndex = KeyIndex;
					CurrentBlurbIndex = -1;
				}
			}
			break;
		}
		if( CurrentBlurbIndex >= 0 )
		{
			const UDisConv_Blurb* KeyBlurb = m_DialogKeysBlurbs(CurrentBlurbIndex).m_pMainBlurb;
			IDisConvSpeakerInterface* Speaker = RunningInst->GetInGameConv( KeyBlurb )->GetSpeaker( KeyBlurb->m_iSpeaker );
			if( !Speaker || !m_pPlayingBlurb || m_pPlayingBlurb->TickNode_Shared( Interp->DeltaTime, RunningInst, TRUE ) >= 0 )
			{
				IEDIAG( "dialog key %d done on %s at matinee %.3f (%s)", CurrentBlurbIndex, *TrDialog->GetPathName(), NewPosition,
					!Speaker ? TEXT("no speaker") : !m_pPlayingBlurb ? TEXT("no blurb") : TEXT("blurb finished") );
				LastCompletedBlurbIndex = CurrentBlurbIndex;
				CurrentBlurbIndex = -1;
			}
		}
	}
	FVector2D* SyncWait = DisIEDiagEnabled() ? GIEDiagSyncWait.Find( this ) : NULL;
	if( SyncWait && ( LastCompletedBlurbIndex == appTrunc( SyncWait->Y ) || bJump ) )
	{
		IEDIAG( "dialog sync wait ends: key %d of %s released after %.3f s%s", appTrunc( SyncWait->Y ), *TrDialog->GetPathName(),
			GWorld->GetTimeSeconds() - SyncWait->X, bJump ? TEXT(" (jump)") : TEXT("") );
		GIEDiagSyncWait.Remove( this );
	}
	m_LastPosition = NewPosition;
}
