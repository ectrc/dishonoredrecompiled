// DishonoredGame/src/disconv_blurb.cpp
// DISHONORED(port): agent ID (PHASE23 ID). One spoken line of a conversation: it waits for its speaker to be ready, posts the
// voice and the subtitle through the speaker (UDisConversationComponent::PlayDialogBlurb), ticks until the speaker reports
// the line done and then waits out m_fBlurbEndDelay. 2013 rvas (2012 in parentheses; * = by vtable slot of
// ??_7UDisConv_Blurb@@6B@ or by caller, unnamed in the 2013 database):
//   PostLoad 0x897fa0* (0x8dd430, 179; 2013 2300)      InitNode 0x894810* (0x8e0d60, 208)       ExtractText_Derived 0x892ce0 (0x8dd4f0)
//   GetClassVars_Derived 0x8948e0 (0x8ee480, 37)       PlayNode_Derived 0x89fb90 (0x8e4bc0)      TickNode 0x89fbb0 (0x8ea6f0)
//   PlayBlurb_Shared 0x89deb0* (0x8e3270, 509; 2013 574) TickNode_Shared 0x89e0f0 (0x8e7b50)     StartBlurb 0x89bb50* (0x8e4be0)
//   InterruptTickingNode 0x89bb00 (0x8e11b0, 76)       GetBlurbLookTarget 0x89b770* (0x8e0e30, 889)
//   UDisConv_Blurb_InGameData::GetBlurbSpeakerData 0x8946c0 (0x8e0cc0), StopBlurbSpeakerData 0x88c550 (0x8dd3f0)
//   Serialize 0x8988a0 (0x8e7d10).
// Left out, named at the site: PlayBlurb_Shared's 2013 skip of the line "Hey, you in the mask! You look like a clown." when
// the game info is the class behind 2013 0x14721f8 (registered by 0xcec1d0, never named; not the base game's game info) and
// its call into a 3-byte `retn 14h` (0xc715d0, an ICF-folded no-op that took the hook name, priority, both pawns' tweaks and
// the text); StartBlurb's two AI noises (hook flag 8: the speakers' locations heard as dialog; 0x1020: the top enemy) -
// UDisAINoiseManager::MakeNoiseForAI (2013 0x899d80) is unported, and the Kismet hook (flags 0x500) sets neither;
// PostLoad's per-language override of m_Text from Localization\<LANG>\Dialog_*.<LANG> ([section] Text=, 2013 0x7cfbb0
// builds the key) - the INT folder holds 659 files and no Dialog_* one, so on this install it changes nothing.
// Not ported (editor): SearchNode 0x89aa80, ImportText_Derived 0x895890, 2013 slot 83 0x88c590.
#include "DishonoredGame.h"
#include "frdialogdiag.h"
#include "iddiag.h"

static INT GDisIDDiag = -1;

UBOOL DisIDDiagEnabled()
{
	if( GDisIDDiag < 0 )
	{
		GDisIDDiag = ParseParam( appCmdLine(), TEXT("iddiag") ) ? 1 : 0;
	}
	return GDisIDDiag != 0;
}

static IDisConvSpeakerInterface* DisBoundSpeakerOf( const FDisConvSpeaker_InGame& _rBinding )
{
	return _rBinding.m_pBoundSpeaker.GetObject() ? (IDisConvSpeakerInterface*)_rBinding.m_pBoundSpeaker.GetInterface() : NULL;
}

static AActor* DisSpeakerActor( IDisConvSpeakerInterface* _pSpeaker )
{
	return _pSpeaker ? Cast<AActor>( _pSpeaker->GetUObjectInterfaceDisConvSpeakerInterface() ) : NULL;
}

static FString DisSpeakerName( IDisConvSpeakerInterface* _pSpeaker )
{
	AActor* Actor = DisSpeakerActor( _pSpeaker );
	return Actor ? Actor->GetName() : FString( TEXT("<none>") );
}

// DISHONORED(written): 2013 rva 0x8988a0 (2012 0x8e7d10); non-template blurbs store one FDisBlurbLangInfo per language
// after the tagged properties, and outside the editor only the current language is kept.
void UDisConv_Blurb::Serialize( FArchive& Ar )
{
	if( Ar.IsSaving() && !IsTemplate() )
	{
		m_BlurbVersionNum = 2;
	}
	Super::Serialize( Ar );
	if( IsTemplate() )
	{
		return;
	}
	const FName CurrentLanguage( UObject::GetLanguage() );
	UScriptStruct* LangInfoStruct = FindField<UScriptStruct>( GetClass(), TEXT("DisBlurbLangInfo") );
	if( Ar.IsLoading() )
	{
		if( m_BlurbVersionNum < 2 )
		{
			m_PerLanguageData.Empty();
			return;
		}
		INT NumLanguages = 0;
		Ar << NumLanguages;
		m_PerLanguageData.Empty( GIsEditor ? NumLanguages : 1 );
		FDisBlurbLangInfo LangInfo(EC_EventParm);
		for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
		{
			LangInfoStruct->SerializeBin( Ar, (BYTE*)&LangInfo, sizeof(FDisBlurbLangInfo) );
			if( GIsEditor || LangInfo.m_Language == CurrentLanguage )
			{
				m_PerLanguageData.AddItem( LangInfo );
			}
		}
		return;
	}
	INT NumLanguages = m_PerLanguageData.Num();
	Ar << NumLanguages;
	for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
	{
		LangInfoStruct->SerializeBin( Ar, (BYTE*)&m_PerLanguageData(LanguageIndex), sizeof(FDisBlurbLangInfo) );
	}
}

// 2013 0x894760 trims the tail, FString::Trim the head, 0x7c8aa0 replaces the three entities of its table at 0x1162f90.
static void DisCleanBlurbText( FString& _rText )
{
	INT Last = _rText.Len() - 1;
	while( Last >= 0 && ( (*_rText)[Last] == TEXT('\n') || (*_rText)[Last] == TEXT(' ') || (*_rText)[Last] == TEXT('\t') ) )
	{
		Last--;
	}
	if( Last < _rText.Len() - 1 )
	{
		_rText = _rText.Left( Last + 1 );
	}
	_rText = _rText.Trim();
	static const TCHAR* Entities[][2] = { { TEXT("&quot;"), TEXT("\"") }, { TEXT("&nbsp;"), TEXT(" ") }, { TEXT("&#45;"), TEXT("-") } };
	for( INT i = 0; i < ARRAY_COUNT(Entities); i++ )
	{
		_rText = _rText.Replace( Entities[i][0], Entities[i][1] );
	}
}

void UDisConv_Blurb::PostLoad()
{
	Super::PostLoad();
	UBOOL bDirty = FALSE;
	if( m_bTurnToFace )
	{
		m_LookType = eDisConvoLookType_TurnToFace;
		m_bTurnToFace = FALSE;
		bDirty = TRUE;
	}
	for( INT i = 0; i < m_OnlyHeardBy.Num(); i++ )
	{
		if( m_OnlyHeardBy(i).m_bTurnToFace )
		{
			m_OnlyHeardBy(i).m_LookType = eDisConvoLookType_TurnToFace;
			m_OnlyHeardBy(i).m_bTurnToFace = FALSE;
			bDirty = TRUE;
		}
	}
	DisCleanBlurbText( m_Text );
	if( GIsEditor )
	{
		for( INT i = 0; i < m_PerLanguageData.Num(); i++ )
		{
			DisCleanBlurbText( m_PerLanguageData(i).m_LocalizedText );
		}
	}
	if( bDirty )
	{
		MarkPackageDirty();
	}
}

void UDisConv_Blurb::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	Super::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	if( !m_iBlurbGUID.IsValid() )
	{
		m_iBlurbGUID = appCreateGuid();
	}
	if( !m_bSubtitleTypeSet )
	{
		m_bMainSubtitle = ( _pOwner && _pOwner->IsA( UDisDialogTree::StaticClass() ) ) ? FALSE : TRUE;
		m_bSubtitleTypeSet = TRUE;
	}
}

void UDisConv_Blurb::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	_rVarsOut.m_bIsStartOfDialog = TRUE;
	_rVarsOut.m_InGameDataClasses[eDisConvInGameDataType_Singleton] = UDisConv_Blurb_InGameData::StaticClass();
}

const TCHAR* UDisConv_Blurb::ExtractText_Derived() const
{
	if( appStricmp( UObject::GetLanguage(), TEXT("INT") ) == 0 )
	{
		return *m_Text;
	}
	if( m_PerLanguageData.Num() == 1 )
	{
		return *m_PerLanguageData(0).m_LocalizedText;
	}
	return TEXT("<Lang not found!>");
}

INT UDisConv_Blurb::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	return PlayBlurb_Shared( _rbNotFinished, _pRunningInstance, FALSE, -1.0f );
}

INT UDisConv_Blurb::TickNode( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	return TickNode_Shared( _fDeltaTime, _pRunningInstance, FALSE );
}

INT UDisConv_Blurb::PlayBlurb_Shared( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance, UBOOL _bSkipLookTargets, FLOAT _fSeekTime ) const
{
	UDisConv_Blurb_InGameData* Data = (UDisConv_Blurb_InGameData*)_pRunningInstance->GetInGameNode( this, eDisConvInGameDataType_Singleton );
	UDisConversation_InGameData* OwnerInGame = _pRunningInstance->GetInGameConv( this );
	IDisConvSpeakerInterface* Speaker = OwnerInGame ? OwnerInGame->GetSpeaker( m_iSpeaker ) : NULL;
	if( !Data )
	{
		// DISHONORED(written): retail dereferences the singleton unguarded; a tree bound before its blurbs had in-game data
		// would crash there, this one is reported and finishes the node instead.
		IDDIAG( "blurb %s: no UDisConv_Blurb_InGameData singleton, the line is skipped", *GetPathName() );
		return 0;
	}
	Data->m_bBlurbHasEnded = FALSE;
	Data->m_fRemainingBlurbDelay = m_fBlurbEndDelay;
	if( !Speaker )
	{
		IDDIAG( "blurb %s: speaker %d is not bound, output 0", *GetName(), m_iSpeaker );
		return 0;
	}
	FDisBlurbPerSpeakerInstance& SpeakerData = Data->GetBlurbSpeakerData( Speaker );
	UDisDialogVoiceData* VoiceData = NULL;
	FDisVoiceTextBlurb* VoiceBlurb = Speaker->FindTextBlurb( GetDialogTree(), m_iBlurbGUID, VoiceData );
	if( VoiceBlurb )
	{
		Speaker->StreamDialogBlurb( VoiceBlurb );
	}
	if( !_bSkipLookTargets )
	{
		BYTE LookType = eDisConvoLookType_HeadAndTorso;
		UBOOL bUseAttentionProxy = FALSE;
		IDisConvSpeakerInterface* LookTarget = GetBlurbLookTarget( OwnerInGame, _pRunningInstance, LookType, bUseAttentionProxy );
		AActor* LookActor = DisSpeakerActor( LookTarget );
		if( LookActor )
		{
			Speaker->SetLookTarget( LookActor, LookType, bUseAttentionProxy );
		}
	}
	_rbNotFinished = TRUE;
	SpeakerData.m_BlurbStage = 0;
	SpeakerData.m_fSeekTime = _fSeekTime;
	IDDIAG( "blurb play %s speaker %s voice %s event %s", *GetPathName(), *DisSpeakerName( Speaker ),
		VoiceData ? *VoiceData->GetName() : TEXT("<none>"), VoiceBlurb ? *VoiceBlurb->m_AkEventName : TEXT("<none>") );
	return -1;
}

INT UDisConv_Blurb::TickNode_Shared( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance, UBOOL _bSkipLookTargets ) const
{
	UDisConversation_InGameData* OwnerInGame = _pRunningInstance->GetInGameConv( this );
	IDisConvSpeakerInterface* Speaker = OwnerInGame ? OwnerInGame->GetSpeaker( m_iSpeaker ) : NULL;
	UDisConv_Blurb_InGameData* Data = (UDisConv_Blurb_InGameData*)_pRunningInstance->GetInGameNode( this, eDisConvInGameDataType_Singleton );
	if( !Speaker || !Data )
	{
		// DISHONORED(written): retail ticks a blurb only after PlayBlurb_Shared found both; a speaker destroyed mid-line
		// would crash retail here, it ends the node instead.
		IDDIAG( "blurb %s: speaker or in-game data gone while ticking, output 0", *GetName() );
		return 0;
	}
	FDisBlurbPerSpeakerInstance& SpeakerData = Data->GetBlurbSpeakerData( Speaker );
	if( SpeakerData.m_BlurbStage == 0 )
	{
		if( Speaker->IsReadyToSpeak() )
		{
			StartBlurb( _pRunningInstance, SpeakerData, Speaker, _bSkipLookTargets );
		}
		return -1;
	}
	if( SpeakerData.m_BlurbStage != 1 )
	{
		return -1;
	}
	if( Speaker->IsDonePlayingBlurb() )
	{
		if( !Data->m_bBlurbHasEnded )
		{
			Data->StopBlurbSpeakerData( Speaker );
			Speaker->StopDialogBlurb();
			Data->m_bBlurbHasEnded = TRUE;
			IDDIAG( "blurb done %s speaker %s, end delay %.2f", *GetPathName(), *DisSpeakerName( Speaker ), m_fBlurbEndDelay );
		}
		Data->m_fRemainingBlurbDelay -= _fDeltaTime;
		if( Data->m_fRemainingBlurbDelay <= 0.0f )
		{
			return 0;
		}
	}
	if( !m_bUseOnlyHeardBy )
	{
		return -1;
	}
	for( INT i = 0; i < m_OnlyHeardBy.Num(); i++ )
	{
		const FDisBlurbListener& Listener = m_OnlyHeardBy(i);
		BYTE& ListenStage = SpeakerData.m_BlurbListenerInfo(i).m_ListenStage;
		if( !ListenStage && Listener.m_Listener != -1 && Listener.m_Listener != m_iSpeaker )
		{
			IDisConvSpeakerInterface* ListenerSpeaker = OwnerInGame->GetSpeaker( Listener.m_Listener );
			if( ListenerSpeaker && ListenerSpeaker->IsReadyToSpeak() )
			{
				ListenStage = 1;
			}
		}
	}
	return -1;
}

UBOOL UDisConv_Blurb::StartBlurb( FDisDialogRunningInstance* _pRunningInstance, FDisBlurbPerSpeakerInstance& _rSpeakerData, IDisConvSpeakerInterface* _pSpeaker, UBOOL _bSkipLookTargets ) const
{
	UDisDialogVoiceData* VoiceData = NULL;
	FDisVoiceTextBlurb* VoiceBlurb = m_bDirtiedWithoutExport ? NULL : _pSpeaker->FindTextBlurb( GetDialogTree(), m_iBlurbGUID, VoiceData );
	_rSpeakerData.m_BlurbStage = 1;
	const TCHAR* Text = ExtractText_Derived();
	UDisConversation_InGameData* OwnerInGame = _pRunningInstance->GetInGameConv( this );
	IDDIAG( "blurb start %s speaker %s event %s text '%s' radius %.0f main %d seek %.2f", *GetPathName(),
		*DisSpeakerName( _pSpeaker ), VoiceBlurb ? *VoiceBlurb->m_AkEventName : TEXT("<none>"),
		Text, m_fVoiceRadius, (INT)m_bMainSubtitle, _rSpeakerData.m_fSeekTime );
	_pSpeaker->PlayDialogBlurb( VoiceData, VoiceBlurb, Text, m_fBlurbEndDelay, m_bShowDebugBubble || !VoiceBlurb, m_fVoiceRadius, m_bMainSubtitle, _rSpeakerData.m_fSeekTime );
	if( m_bUseOnlyHeardBy )
	{
		const INT NumListeners = m_OnlyHeardBy.Num();
		_rSpeakerData.m_BlurbListenerInfo.Empty();
		_rSpeakerData.m_BlurbListenerInfo.AddZeroed( NumListeners );
		for( INT i = 0; i < NumListeners; i++ )
		{
			if( !_bSkipLookTargets )
			{
				const FDisBlurbListener& Listener = m_OnlyHeardBy(i);
				if( Listener.m_Listener != -1 && Listener.m_Listener != m_iSpeaker )
				{
					IDisConvSpeakerInterface* ListenerSpeaker = OwnerInGame->GetSpeaker( Listener.m_Listener );
					if( ListenerSpeaker )
					{
						ListenerSpeaker->SetLookTarget( DisSpeakerActor( _pSpeaker ), Listener.m_LookType, FALSE );
					}
				}
			}
			_rSpeakerData.m_BlurbListenerInfo(i).m_ListenStage = 0;
		}
	}
	else if( !_bSkipLookTargets )
	{
		for( INT i = 0; i < OwnerInGame->m_SpeakerBindings.Num(); i++ )
		{
			const FDisConvSpeaker_InGame& Binding = OwnerInGame->m_SpeakerBindings(i);
			IDisConvSpeakerInterface* Bound = DisBoundSpeakerOf( Binding );
			if( Binding.m_bSpeakerEngaged && Bound != _pSpeaker )
			{
				AActor* SpeakerActor = DisSpeakerActor( _pSpeaker );
				if( SpeakerActor )
				{
					Bound->SetLookTarget( SpeakerActor, eDisConvoLookType_HeadAndTorso, FALSE );
				}
			}
		}
	}
	const FDisDialogHookInfo& HookInfo = UDisDialogTree::GetDialogHookInfo()( _pRunningInstance->m_DialogHookThatFiredInst );
	if( HookInfo.m_Flags & 0x1028 )
	{
		IDDIAG( "blurb %s: hook %d flags 0x%x would make an AI noise (UDisAINoiseManager::MakeNoiseForAI is not ported)", *GetName(),
			(INT)_pRunningInstance->m_DialogHookThatFiredInst, HookInfo.m_Flags );
	}
	return FALSE;
}

void UDisConv_Blurb::InterruptTickingNode( FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const
{
	if( _InterruptReason == DisConvoEndReason_PlayerSkipWhole )
	{
		return;
	}
	UDisConversation_InGameData* OwnerInGame = _pRunningInstance->GetInGameConv( this );
	IDisConvSpeakerInterface* Speaker = OwnerInGame ? OwnerInGame->GetSpeaker( m_iSpeaker ) : NULL;
	if( Speaker )
	{
		UDisConv_Blurb_InGameData* Data = (UDisConv_Blurb_InGameData*)_pRunningInstance->GetInGameNode( this, eDisConvInGameDataType_Singleton );
		Data->StopBlurbSpeakerData( Speaker );
		Speaker->StopDialogBlurb();
		IDDIAG( "blurb interrupt %s speaker %s reason %d", *GetPathName(), *DisSpeakerName( Speaker ), (INT)_InterruptReason );
	}
}

IDisConvSpeakerInterface* UDisConv_Blurb::GetBlurbLookTarget( UDisConversation_InGameData* _pOwner_InGame, FDisDialogRunningInstance* _pRunningInstance, BYTE& _rLookType, UBOOL& _rbUseAttentionProxy ) const
{
	IDisConvSpeakerInterface* LookAt = NULL;
	IDisConvSpeakerInterface* Speaker = _pOwner_InGame->GetSpeaker( m_iSpeaker );
	_rLookType = m_LookType;
	_rbUseAttentionProxy = FALSE;
	if( !Speaker )
	{
		return NULL;
	}
	if( m_SpokenTo != -1 )
	{
		LookAt = _pOwner_InGame->GetSpeaker( m_SpokenTo );
		_rbUseAttentionProxy = !_pOwner_InGame->IsSpeakerEngaged( m_SpokenTo );
	}
	BYTE OldLookType = eDisConvoLookType_Default;
	const AActor* OldTargetActor = Speaker->GetConvLookTarget( OldLookType );
	IDisConvSpeakerInterface* OldLookTarget = OldTargetActor ? (IDisConvSpeakerInterface*)const_cast<AActor*>( OldTargetActor )->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() ) : NULL;
	if( !LookAt )
	{
		UBOOL bKeepOld = OldLookTarget != NULL;
		if( !bKeepOld )
		{
			if( _rLookType == eDisConvoLookType_Default && m_pOwner && ( m_pOwner->m_bIsBlocking || m_pOwner->m_bIsBlocking_Player ) )
			{
				_rLookType = eDisConvoLookType_TurnToFace;
			}
			const TArray<FDisConvSpeaker_InGame>& Bindings = _pOwner_InGame->m_SpeakerBindings;
			for( INT i = 0; i < Bindings.Num(); i++ )
			{
				const FDisConvSpeaker* Slot = (const FDisConvSpeaker*)Bindings(i).m_pSpeaker;
				if( Slot->m_Type == 2 )
				{
					if( Bindings(i).m_bSpeakerEngaged )
					{
						LookAt = DisBoundSpeakerOf( Bindings(i) );
						if( LookAt )
						{
							break;
						}
					}
					break;
				}
			}
			if( !LookAt )
			{
				for( INT i = 0; i < Bindings.Num(); i++ )
				{
					UBOOL bCanHearBlurb = TRUE;
					if( m_bUseOnlyHeardBy )
					{
						bCanHearBlurb = FALSE;
						for( INT j = 0; j < m_OnlyHeardBy.Num(); j++ )
						{
							if( m_OnlyHeardBy(j).m_Listener == i )
							{
								bCanHearBlurb = TRUE;
								break;
							}
						}
					}
					IDisConvSpeakerInterface* Bound = DisBoundSpeakerOf( Bindings(i) );
					if( Bindings(i).m_bSpeakerEngaged && Bound != Speaker )
					{
						LookAt = Bound;
						if( bCanHearBlurb )
						{
							break;
						}
					}
				}
			}
			if( !LookAt )
			{
				bKeepOld = TRUE;
				const FDisDialogHookInfo& HookInfo = UDisDialogTree::GetDialogHookInfo()( _pRunningInstance->m_DialogHookThatFiredInst );
				if( !( HookInfo.m_Flags & 0x2000 ) )
				{
					for( INT i = 0; i < Bindings.Num(); i++ )
					{
						const FDisConvSpeaker* Slot = (const FDisConvSpeaker*)Bindings(i).m_pSpeaker;
						if( Slot->m_Type == 1 )
						{
							IDisConvSpeakerInterface* Bound = DisBoundSpeakerOf( Bindings(i) );
							if( Bound && Bound != Speaker )
							{
								LookAt = Bound;
								_rbUseAttentionProxy = TRUE;
								bKeepOld = FALSE;
							}
							break;
						}
					}
				}
			}
		}
		if( bKeepOld )
		{
			LookAt = OldLookTarget;
			_rLookType = OldLookType;
		}
	}
	if( _rLookType == eDisConvoLookType_Default )
	{
		_rLookType = eDisConvoLookType_HeadAndTorso;
	}
	return LookAt;
}

FDisBlurbPerSpeakerInstance& UDisConv_Blurb_InGameData::GetBlurbSpeakerData( IDisConvSpeakerInterface* _pSpeaker )
{
	FDisBlurbPerSpeakerInstance* Found = NULL;
	FDisBlurbPerSpeakerInstance* FirstFree = NULL;
	for( INT i = 0; i < ARRAY_COUNT(m_PerSpeakerInfo); i++ )
	{
		FDisBlurbPerSpeakerInstance& Slot = m_PerSpeakerInfo[i];
		IDisConvSpeakerInterface* SlotSpeaker = Slot.m_pSpeaker.GetObject() ? (IDisConvSpeakerInterface*)Slot.m_pSpeaker.GetInterface() : NULL;
		if( SlotSpeaker == _pSpeaker )
		{
			Found = &Slot;
			break;
		}
		if( ( !Slot.m_pSpeaker.GetObject() || !Slot.m_pSpeaker.GetInterface() ) && !FirstFree )
		{
			FirstFree = &Slot;
		}
	}
	FDisBlurbPerSpeakerInstance* Result = Found ? Found : FirstFree;
	check( Result );
	UObject* Object = _pSpeaker ? _pSpeaker->GetUObjectInterfaceDisConvSpeakerInterface() : NULL;
	void* Interface = Object ? Object->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() ) : NULL;
	Result->m_pSpeaker.SetObject( Interface ? Object : NULL );
	Result->m_pSpeaker.SetInterface( Interface );
	return *Result;
}

void UDisConv_Blurb_InGameData::StopBlurbSpeakerData( IDisConvSpeakerInterface* _pSpeaker )
{
	for( INT i = 0; i < ARRAY_COUNT(m_PerSpeakerInfo); i++ )
	{
		FDisBlurbPerSpeakerInstance& Slot = m_PerSpeakerInfo[i];
		IDisConvSpeakerInterface* SlotSpeaker = Slot.m_pSpeaker.GetObject() ? (IDisConvSpeakerInterface*)Slot.m_pSpeaker.GetInterface() : NULL;
		if( SlotSpeaker == _pSpeaker )
		{
			Slot.m_pSpeaker.SetObject( NULL );
			Slot.m_pSpeaker.SetInterface( NULL );
			return;
		}
	}
}
