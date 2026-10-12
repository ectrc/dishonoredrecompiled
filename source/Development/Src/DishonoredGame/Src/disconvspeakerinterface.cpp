// DishonoredGame/src/disconvspeakerinterface.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The default bodies of IDisConvSpeakerInterface: every one forwards to the
// owner's UDisConversationComponent when it is attached. 2013 rvas (2012 in parentheses):
//   GetHookParameters            0x88b1a0 (0x8f8690, 7)     GetHookParameters_Derived 0x88bc30 (0x8f9500, 31)
//   GetConversationComponent     0x88bbf0 (0x8f93a0, 32)    GetDialogSaveData 0x88bc10 (0x8f94a0, 31)
//   GetDebugSpeechText_Derived   0x88bc50 (0x8f9520, 31)    PostSpawned_Dialog 0x8a2900 (0x8f94c0, 32)
//   RemoveOneShot                0x8a66b0 (0x8f94e0, 32)    IsAvailableForDialog 0x89d1f0 (0x8f93e0, 111)
//   StartDialog                  0x8a6680 (0x8f9450, 43)    EndDialog (0x8f9480, 32)
//   FireDialogHook               0x8a7360 (0x8f9540, 41)    CheckDialogHook 0x8a7390 (0x8f9570, 40)
//   FireDialogHookForAudioLog    0x8a73c0 (0x8f95a0, 41)    SupportsDialogHook (0x8f95d0, 39)
//   InConversation               (0x8f9600, 60)             BreakOutOfConversation 0x8a5130 (0x8f9640, 26)
//   StopDialogBlurb              0x8a0980 (0x8f96d0, 25)    SupportsDialogTree 0x88d770 (0x8f9720, 39)
// Not ported (the blurb / voice / look-at surface; agentFR.md inventory): StreamDialogBlurb 0x88bcc0, PlayDialogBlurb
// 0x8a0930, FindTextBlurb 0x8a2920, GetConvLookTarget 0x88d7a0, SetLookTarget 0x8970e0, IsReadyToSpeak, IsDonePlayingBlurb
// 0x89a380, GetPsychicConvoPartners 0x89a360, ExtractDialogHookText 0x8a5090, GameSave_Dialog 0x88d6f0, GameLoad_Dialog
// 0x897060, PostShutdown_Dialog 0x8a08e0.
#include "DishonoredGame.h"

UDisConversationComponent* IDisConvSpeakerInterface::GetConversationComponent( UBOOL _bAllowedUnAttached )
{
	UDisConversationComponent* Comp = GetConversationComponent_Derived();
	if( Comp && !_bAllowedUnAttached && !Comp->IsAttached() )
	{
		return NULL;
	}
	return Comp;
}

FDisHookParameters* IDisConvSpeakerInterface::GetHookParameters()
{
	return GetHookParameters_Derived();
}

FDisHookParameters* IDisConvSpeakerInterface::GetHookParameters_Derived()
{
	UDisConversationComponent* Comp = GetConversationComponent();
	return Comp ? &Comp->m_HookParams : NULL;
}

FDisConvSaveData* IDisConvSpeakerInterface::GetDialogSaveData()
{
	UDisConversationComponent* Comp = GetConversationComponent();
	return Comp ? &Comp->m_SaveData : NULL;
}

FString* IDisConvSpeakerInterface::GetDebugSpeechText_Derived()
{
	UDisConversationComponent* Comp = GetConversationComponent();
	return Comp ? &Comp->m_DebugSpeechText : NULL;
}

void IDisConvSpeakerInterface::PostSpawned_Dialog( TArray<UDisDialogVoiceData*>& _rAdditionalGenerics, const TArray<FDisSpeakerOneShotInfo>& _rOneShots )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->PostSpawned( _rAdditionalGenerics, _rOneShots );
	}
}

void IDisConvSpeakerInterface::RemoveOneShot( const FDisSpeakerOneShotInfo& _rOneShot )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		for( INT i = 0; i < Comp->m_OneShotInfo.Num(); i++ )
		{
			if( Comp->m_OneShotInfo(i).m_pOneShot_DialogData == _rOneShot.m_pOneShot )
			{
				FDisDialogRunningInstance* Inst = Comp->GetRunningInst();
				if( Inst && Comp->m_OneShotInfo(i).m_pOneShot_DialogData && Inst->GetDialogTree() == Comp->m_OneShotInfo(i).m_pOneShot_DialogData->m_pDialogTree )
				{
					Comp->BreakOutOfConversation( DisConvoEndReason_Destroyed, eDisDialogHookFireGroup_MAX );
				}
				Comp->m_OneShotInfo.Remove( i-- );
			}
		}
	}
}

eDisDialogAvailability IDisConvSpeakerInterface::IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	const UDisConversationComponent* Comp = GetConversationComponent();
	if( !Comp )
	{
		return DDA_IGNORED;
	}
	UBOOL bDummy = FALSE;
	UBOOL bIsNonInteruptable = FALSE;
	eDisConvPriority Current = Comp->InConversation( bDummy, &bIsNonInteruptable, NULL );
	if( bIsNonInteruptable )
	{
		return DDA_IN_NON_INTERRUPTABLE_CONVO;
	}
	if( Current < _Priority )
	{
		return DDA_AVAILABLE;
	}
	return ( _StartDialogFlags & 4 ) ? DDA_IN_HIGHER_PRIORITY_CONVO : DDA_AVAILABLE;
}

void IDisConvSpeakerInterface::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->StartDialog( _pDialogRunningInst, _bBlocking, _FiringHookIndex );
	}
}

void IDisConvSpeakerInterface::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->EndDialog( _pDialogRunningInst, _ConvoEndReason );
	}
}

void IDisConvSpeakerInterface::FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->FireDialogHook( _Hook, _pInitiator, FALSE );
	}
}

eDisDialogCheckOnly IDisConvSpeakerInterface::CheckDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator ) const
{
	const UDisConversationComponent* Comp = GetConversationComponent();
	return Comp ? Comp->CheckDialogHook( _Hook, _pInitiator ) : eDisDialogCheckOnly_NotFound;
}

// DISHONORED(bringup): retail's audio-log variant (0x8a73c0) is byte-for-byte the FireDialogHook shape above with the
// component's audio-log entry point, which belongs to the audio-log package; the plain hook is fired here.
void IDisConvSpeakerInterface::FireDialogHookForAudioLog( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator )
{
	FireDialogHook( _Hook, _pInitiator );
}

UBOOL IDisConvSpeakerInterface::SupportsDialogHook( eDisDialogHook _Hook ) const
{
	const UDisConversationComponent* Comp = GetConversationComponent();
	return Comp && Comp->SupportsDialogHook( _Hook );
}

eDisConvPriority IDisConvSpeakerInterface::InConversation( UBOOL& _rIsBlocking_Out, const INT* _pMatchRunningInstID ) const
{
	const UDisConversationComponent* Comp = GetConversationComponent();
	_rIsBlocking_Out = FALSE;
	return Comp ? Comp->InConversation( _rIsBlocking_Out, NULL, _pMatchRunningInstID ) : NOT_IN_A_CONV;
}

void IDisConvSpeakerInterface::BreakOutOfConversation( DisConvoEndReasonEnum _ConvoEndReason, eDisDialogHookFireGroup _FiringGroup )
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->BreakOutOfConversation( _ConvoEndReason, _FiringGroup );
	}
}

void IDisConvSpeakerInterface::StopDialogBlurb()
{
	UDisConversationComponent* Comp = GetConversationComponent();
	if( Comp )
	{
		Comp->StopDialogBlurb();
	}
}

UBOOL IDisConvSpeakerInterface::SupportsDialogTree( const UDisDialogTree* _pTree ) const
{
	const UDisConversationComponent* Comp = GetConversationComponent();
	return Comp && Comp->InFallbackChain( _pTree );
}
