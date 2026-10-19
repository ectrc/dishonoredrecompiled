// DishonoredGame/src/disconversationcomponent.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The direct path of the conversation component, 2013 rvas (2012 in parentheses):
//   Attach 0x893700 (0x8e1a80, 150)            Detach 0x88d260 (0x8de710)             BeginPlay 0x88d2c0 (0x8de770, 124)
//   Tick 0x8a6860* (0x8e3920, 812)              PostSpawned (0x8ea8e0, 630)            AddOneShotAfterSpawned 0x8a2100 (0x8e5820, 103)
//   InstanceAndBindDialogTree (0x8e3d00, 231)   BuildDialogHookSupportedCache 0x894d70 (0x8e1ca0, 90)
//   SupportsDialogHook 0x88b060 (0x8d98c0, 47)  SupportsDialogHook_Slow 0x8937a0 (0x8dec70, 265)
//   FireDialogHook 0x8a6bd0* (0x8f52e0, 804)    FireDialogHookNow 0x8a5670* (0x8dea10, 408)  CheckDialogHook 0x8a6f60 (0x8e1b20, 375)
//   StartDialog (0x8e1d00, 127)                 EndDialog (0x8f0850, 33)               InConversation (0x8debb0, 183)
//   BreakOutOfConversation (0x8ded80)           OnOtherActorTerminated 0x8a4f50 (0x8df270)  ClassifyGlobalFireGroup 0x89a300 (0x8e88d0, 82)
//   InFallbackChain (0x8df160)                  ShouldLowerAlertness (0x8dabd0)
//   (* = found through the caller, unnamed in the 2013 database.) 2013's FireDialogHook / FireDialogHookNow moved the
//   refire-time stamping from the former into the latter; that is the shape ported.
// The pieces of these bodies that are look-at or HUD are the other packages' and are left out, named at the site:
//   Tick's FDisLookAtRequest drive and ADishonoredHUD post-render registration; BeginPlay's FDisLookAtRequest::Initialize;
//   BreakOutOfConversation's HUD unregister.
// Agent ID (PHASE23 ID), the voice of a blurb: PlayDialogBlurb 0x89fbf0 (0x8f7910), StreamDialogBlurb 0x88ba50* (0x8dabf0),
// IsReadyToSpeak (0x8df100, inlined into the interface's 2013 0x88d7e0), IsDonePlayingBlurb 0x89a160 (0x8e8730),
// IsAnyAkEventPlaying 0x896740 (0x8e5890), FindTextBlurb 0x8a2410 (0x8deed0), StopDialogBlurb 0x8a02f0* (0x8ef700),
// StopAkEvents 0x88bae0 (0x8dac70), UnprepareAkEvent 0x88b0b0 (0x8d9960), ConvPrepareEventCB 0x88b090 (0x8d9940),
// DisConvAkEventCB 0x8938b0* (0x8f7840), the ms_VoiceBoneName initialiser 0xb8f290 ("head_jnt").
//   WITH_FACEFX=0, so the three FaceFX calls are compiled out, each named at its site: PlayDialogBlurb's
//   StopFaceFXAnim and its FaceFX branch (GetStoryGroup_Derived -> FxAnimSet::GetAnimGroup -> GetFaceFXAnimStartOffset ->
//   PlayFaceFXAnim, which falls back to the Wwise post when it fails - the path taken here), IsDonePlayingBlurb's
//   IsPlayingFaceFXAnim, StopDialogBlurb's StopFaceFXAnim.
//   DISHONORED(deviation, to be removed by IH2): under DISHONORED_WWISE_SILENT, PlayDialogBlurb gives every voice it posts
//   a length, DishonoredWwise::SetDurationHint(playing id, clamp(characters / 15 cps, 1 s, 12 s)). The silent backend reads
//   only a bank's BKHD chunk, so no media length exists to give instead; without the hint the voice ends on the next audio
//   frame and IsDonePlayingBlurb reports every line done one frame after it starts.
// Not ported (agentFR.md inventory): GetLookTarget 0x88d340, SetLookTarget, GetPsychicConvoPartners 0x89a0a0,
// ExtractDialogHookText, SerializeForGameSave 0x88b9c0, Shutdown, BeginDestroy.
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "frdialogdiag.h"
#include "iddiag.h"

FName UDisConversationComponent::ms_VoiceBoneName( TEXT("head_jnt") );

void DisAkEndOfEventCB( AkCallbackType _Type, AkCallbackInfo* _pInfo );

static TScriptInterface<IDisConvSpeakerInterface> DisToSpeakerInterface( IDisConvSpeakerInterface* Speaker )
{
	TScriptInterface<IDisConvSpeakerInterface> Result;
	if( Speaker )
	{
		UObject* Object = Speaker->GetUObjectInterfaceDisConvSpeakerInterface();
		Result.SetObject( Object );
		Result.SetInterface( Object ? Speaker : NULL );
	}
	return Result;
}

void UDisConversationComponent::Attach()
{
	m_OwnerInterfaceSpeaker.SetObject( NULL );
	m_OwnerInterfaceSpeaker.SetInterface( NULL );
	m_OwnerInterfaceHealth.SetObject( NULL );
	m_OwnerInterfaceHealth.SetInterface( NULL );
	if( Owner )
	{
		IDisConvSpeakerInterface* Speaker = (IDisConvSpeakerInterface*)Owner->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() );
		if( Speaker )
		{
			m_OwnerInterfaceSpeaker.SetObject( Owner );
			m_OwnerInterfaceSpeaker.SetInterface( Speaker );
		}
		IArkHealthInterface* Health = (IArkHealthInterface*)Owner->GetInterfaceAddress( UArkHealthInterface::StaticClass() );
		if( Health )
		{
			m_OwnerInterfaceHealth.SetObject( Owner );
			m_OwnerInterfaceHealth.SetInterface( Health );
		}
	}
	Super::Attach();
}

void UDisConversationComponent::Detach( UBOOL bWillReattach )
{
	// DISHONORED(bringup): retail finalises m_LookAtRequest here for an NPC owner (FDisLookAtRequest::Finalize) - agent FT's component.
	m_OwnerInterfaceSpeaker.SetObject( NULL );
	m_OwnerInterfaceSpeaker.SetInterface( NULL );
	m_OwnerInterfaceHealth.SetObject( NULL );
	m_OwnerInterfaceHealth.SetInterface( NULL );
	Super::Detach( bWillReattach );
}

void UDisConversationComponent::BeginPlay()
{
	Super::BeginPlay();
	if( m_GenericDialogData.Num() > 0 )
	{
		m_pGenericDialogData_Cached = m_GenericDialogData( appRand() % m_GenericDialogData.Num() );
	}
	// DISHONORED(bringup): retail then initialises m_LookAtRequest on an NPC owner's look-at component
	// (FDisLookAtRequest::Initialize, DisLookAtPriority_Conversation) - agent FT's FArkComponentLookat package.
}

void UDisConversationComponent::Tick( FLOAT DeltaTime )
{
	Super::Tick( DeltaTime );
	m_fBlurbElapsedTime += DeltaTime;
	for( INT i = 0; i < m_OneShotInfo.Num(); i++ )
	{
		if( m_OneShotInfo(i).m_pOneShot_DialogTree )
		{
			m_OneShotInfo(i).m_pOneShot_DialogTree->TickDialogTree( DeltaTime );
		}
	}
	for( INT i = 0; i < m_AdditionalGenericDialogTrees.Num(); i++ )
	{
		if( m_AdditionalGenericDialogTrees(i) )
		{
			m_AdditionalGenericDialogTrees(i)->TickDialogTree( DeltaTime );
		}
	}
	if( m_pGenericDialogTree )
	{
		m_pGenericDialogTree->TickDialogTree( DeltaTime );
	}
	for( INT i = 0; i < m_FailedDialogHooks.Num(); i++ )
	{
		m_FailedDialogHooks(i).m_fTimeToDisplay -= DeltaTime;
		if( m_FailedDialogHooks(i).m_fTimeToDisplay <= 0.0f )
		{
			m_FailedDialogHooks.Remove( i-- );
		}
	}
	// DISHONORED(bringup): retail's tail drives the owner's look-at request towards m_pLookTarget (FDisLookAtRequest::TickRequest /
	// RequestActorTarget / RequestProxyTarget with the per-m_LookType FDisLookAtInfluence) and registers the owner with the HUD's
	// post-render list while m_DebugSpeechText is set - the look-at component is agent FT's, the debug text is the editor's.
}

void UDisConversationComponent::PostSpawned( TArray<UDisDialogVoiceData*>& _rAdditionalGenerics, const TArray<FDisSpeakerOneShotInfo>& _rOneShots )
{
	m_AdditionalGenericDialogDatas = _rAdditionalGenerics;
	if( m_AdditionalGenericDialogDatas.Num() > 0 )
	{
		if( m_AdditionalGenericDialogDatas(0) && m_AdditionalGenericDialogDatas(0)->m_pMatchedGenericVoice )
		{
			m_pGenericDialogData_Cached = m_AdditionalGenericDialogDatas(0)->m_pMatchedGenericVoice;
		}
		m_AdditionalGenericDialogTrees.Empty( m_AdditionalGenericDialogDatas.Num() );
		for( INT i = 0; i < m_AdditionalGenericDialogDatas.Num(); i++ )
		{
			if( m_AdditionalGenericDialogDatas(i) )
			{
				m_AdditionalGenericDialogTrees.AddItem( InstanceAndBindDialogTree( m_AdditionalGenericDialogDatas(i) ) );
			}
		}
	}
	for( INT i = 0; i < _rOneShots.Num(); i++ )
	{
		const FDisSpeakerOneShotInfo& Info = _rOneShots(i);
		FDisDialogOneShotPair& Pair = m_OneShotInfo( m_OneShotInfo.AddZeroed() );
		Pair.m_pOneShot_DialogData = Info.m_pOneShot;
		Pair.m_pOneShot_DialogTree = Info.m_bIsOwner ? InstanceAndBindDialogTree( Info.m_pOneShot ) : NULL;
		if( Pair.m_pOneShot_DialogData && Pair.m_pOneShot_DialogData->m_pMatchedGenericVoice )
		{
			m_pGenericDialogData_Cached = Pair.m_pOneShot_DialogData->m_pMatchedGenericVoice;
		}
	}
	if( m_pGenericDialogData_Cached )
	{
		m_pGenericDialogTree = InstanceAndBindDialogTree( m_pGenericDialogData_Cached );
	}
	BuildDialogHookSupportedCache();
	FRDIAG( "spawned %s owner %s: %d generic(s), %d one-shot(s), %d one-shot tree(s) bound, generic tree %s",
		*GetName(), Owner ? *Owner->GetName() : TEXT("<none>"), _rAdditionalGenerics.Num(), _rOneShots.Num(),
		m_OneShotInfo.Num(), m_pGenericDialogTree ? TEXT("yes") : TEXT("no") );
}

void UDisConversationComponent::AddOneShotAfterSpawned( const FDisSpeakerOneShotInfo& _rOneShot )
{
	FDisDialogOneShotPair& Pair = m_OneShotInfo( m_OneShotInfo.AddZeroed() );
	Pair.m_pOneShot_DialogData = _rOneShot.m_pOneShot;
	Pair.m_pOneShot_DialogTree = _rOneShot.m_bIsOwner ? InstanceAndBindDialogTree( _rOneShot.m_pOneShot ) : NULL;
	BuildDialogHookSupportedCache();
}

UDisDialogTree_InGameBind* UDisConversationComponent::InstanceAndBindDialogTree( UDisDialogVoiceData* _pDialogVoiceData )
{
	if( !_pDialogVoiceData || !_pDialogVoiceData->m_pDialogTree )
	{
		return NULL;
	}
	const INT NumHooks = UDisDialogTree::GetDialogHookInfo().Num();
	m_DialogHookInfo_InGame.Empty( NumHooks );
	m_DialogHookInfo_InGame.AddZeroed( NumHooks );
	UDisDialogTree_InGameBind* Binding = ConstructObject<UDisDialogTree_InGameBind>( UDisDialogTree_InGameBind::StaticClass(), this );
	Binding->InitAndBind( _pDialogVoiceData->m_pDialogTree, (IDisConvSpeakerInterface*)m_OwnerInterfaceSpeaker.GetInterface() );
	return Binding;
}

void UDisConversationComponent::BuildDialogHookSupportedCache()
{
	appMemzero( m_IsDialogHookSupported_Cache, sizeof(m_IsDialogHookSupported_Cache) );
	for( INT Hook = 0; Hook < DDH_INVALID; Hook++ )
	{
		if( SupportsDialogHook_Slow( (eDisDialogHook)Hook ) )
		{
			m_IsDialogHookSupported_Cache[Hook >> 5] |= 1 << ( Hook & 31 );
		}
	}
}

UBOOL UDisConversationComponent::SupportsDialogHook( eDisDialogHook _Hook ) const
{
	return ( m_IsDialogHookSupported_Cache[( (BYTE)_Hook ) >> 5] & ( 1 << ( _Hook & 31 ) ) ) != 0;
}

UBOOL UDisConversationComponent::SupportsDialogHook_Slow( eDisDialogHook _Hook ) const
{
	for( INT i = 0; i < m_OneShotInfo.Num(); i++ )
	{
		const FDisDialogOneShotPair& Pair = m_OneShotInfo(i);
		if( Pair.m_pOneShot_DialogTree && Pair.m_pOneShot_DialogData && Pair.m_pOneShot_DialogData->m_pDialogTree && Pair.m_pOneShot_DialogData->m_pDialogTree->SupportsDialogHook( _Hook ) )
		{
			return TRUE;
		}
	}
	for( INT i = 0; i < m_AdditionalGenericDialogDatas.Num(); i++ )
	{
		if( m_AdditionalGenericDialogDatas(i) && m_AdditionalGenericDialogDatas(i)->m_pDialogTree && m_AdditionalGenericDialogDatas(i)->m_pDialogTree->SupportsDialogHook( _Hook ) )
		{
			return TRUE;
		}
	}
	return m_pGenericDialogTree && m_pGenericDialogData_Cached && m_pGenericDialogData_Cached->m_pDialogTree && m_pGenericDialogData_Cached->m_pDialogTree->SupportsDialogHook( _Hook );
}

UBOOL UDisConversationComponent::InFallbackChain( const UDisDialogTree* _pTree ) const
{
	for( INT i = 0; i < m_OneShotInfo.Num(); i++ )
	{
		const FDisDialogOneShotPair& Pair = m_OneShotInfo(i);
		// a one-shot this speaker only takes part in (no binding of its own) still makes the speaker support its tree (0x8df160)
		if( Pair.m_pOneShot_DialogData && Pair.m_pOneShot_DialogData->m_pDialogTree && Pair.m_pOneShot_DialogData->m_pDialogTree->InFallbackChain( _pTree ) )
		{
			return TRUE;
		}
	}
	for( INT i = 0; i < m_AdditionalGenericDialogDatas.Num(); i++ )
	{
		if( m_AdditionalGenericDialogDatas(i) && m_AdditionalGenericDialogDatas(i)->m_pDialogTree && m_AdditionalGenericDialogDatas(i)->m_pDialogTree->InFallbackChain( _pTree ) )
		{
			return TRUE;
		}
	}
	return m_pGenericDialogTree && m_pGenericDialogData_Cached && m_pGenericDialogData_Cached->m_pDialogTree && m_pGenericDialogData_Cached->m_pDialogTree->InFallbackChain( _pTree );
}

UBOOL UDisConversationComponent::ShouldLowerAlertness() const
{
	FDisDialogRunningInstance* Inst = GetRunningInst();
	if( Inst && Inst->m_pCurPlayingConversation )
	{
		UDisConversation* Conv = Cast<UDisConversation>( Inst->m_pCurPlayingConversation->m_pOwnerConvNode );
		return Conv && Conv->m_bLowerAIAlertness;
	}
	return FALSE;
}

// DISHONORED(bringup): retail asks the owner's IArkHealthInterface::ArkIsDeadOrDestroyed (vtable +12); the pawns' override of
// that pure virtual is not ported in this tree (the generated stub appErrorfs), so the pawn's Health is read instead.
static UBOOL DisOwnerIsDeadForHook( const UDisConversationComponent* Comp, eDisDialogHook Hook )
{
	const APawn* Pawn = Cast<APawn>( Comp->m_OwnerInterfaceHealth.GetObject() );
	if( !Pawn )
	{
		return FALSE;
	}
	return Pawn->Health <= 0 && Hook != DDH_COMBAT_DYING && Hook != DDH_COMBAT_BACKSTABBED && Hook != DDH_COMBAT_STEALTHKILL_PROJECTILE;
}

void UDisConversationComponent::FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator, UBOOL _bImmediate )
{
	if( !SupportsDialogHook( _Hook ) )
	{
		FRDIAG( "hook %s on %s: hook %d not supported by any bound tree", Owner ? *Owner->GetName() : *GetName(), *GetName(), (INT)_Hook );
		return;
	}
	if( !_pInitiator )
	{
		_pInitiator = m_OwnerInterfaceSpeaker.GetObject() ? (IDisConvSpeakerInterface*)m_OwnerInterfaceSpeaker.GetInterface() : NULL;
	}
	if( DisOwnerIsDeadForHook( this, _Hook ) )
	{
		return;
	}
	INT HookSubIndex = -1;
	UBOOL bFound = FALSE;
	for( INT i = 0; i < m_OneShotInfo.Num() && !bFound; i++ )
	{
		bFound = m_OneShotInfo(i).m_pOneShot_DialogTree && m_OneShotInfo(i).m_pOneShot_DialogTree->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	for( INT i = 0; i < m_AdditionalGenericDialogTrees.Num() && !bFound; i++ )
	{
		bFound = m_AdditionalGenericDialogTrees(i) && m_AdditionalGenericDialogTrees(i)->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	if( !bFound && m_pGenericDialogTree )
	{
		m_pGenericDialogTree->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	if( HookSubIndex == -1 )
	{
		FRDIAG( "hook %s: hook %d evaluated by %d one-shot / %d generic tree(s), no branch", Owner ? *Owner->GetName() : *GetName(), (INT)_Hook, m_OneShotInfo.Num(), m_AdditionalGenericDialogTrees.Num() + ( m_pGenericDialogTree ? 1 : 0 ) );
		return;
	}
	UDisConvGlobalMan* GlobalMan = DisGetConvGlobalMan();
	const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _Hook );
	FDisDialogHookInfo_InGame& MyInfo = m_DialogHookInfo_InGame( _Hook );
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FLOAT Epsilon = 0.00000001f;
	if( Now - MyInfo.m_fLastFireTime + Epsilon <= Info.m_fRefire_DelayMin && MyInfo.m_fLastFireTime >= Epsilon )
	{
		FDisFailedDialogHookInfo& Failed = m_FailedDialogHooks( m_FailedDialogHooks.AddZeroed() );
		Failed.m_Hook = _Hook;
		Failed.m_ReasonFailed = 0;
		Failed.m_fTimeToDisplay = m_fFailedDialogHookDisplayTime;
		FRDIAG( "hook %s: hook %d branch %d refused by its own refire delay (last %.2f, min %.2f)", Owner ? *Owner->GetName() : *GetName(), (INT)_Hook, HookSubIndex, MyInfo.m_fLastFireTime, Info.m_fRefire_DelayMin );
		return;
	}
	const FLOAT GlobalLast = GlobalMan ? GlobalMan->m_DialogHookInfo_InGame[_Hook].m_fLastFireTime : 0.0f;
	if( Now - GlobalLast + Epsilon > Info.m_fRefire_Delay_Global || Epsilon > GlobalLast )
	{
		FRDIAG( "hook %s: hook %d branch %d %s (initiator %s)", Owner ? *Owner->GetName() : *GetName(), (INT)_Hook, HookSubIndex,
			_bImmediate || !GlobalMan ? TEXT("fires now") : TEXT("queued on the global manager"),
			_pInitiator && _pInitiator->GetUObjectInterfaceDisConvSpeakerInterface() ? *_pInitiator->GetUObjectInterfaceDisConvSpeakerInterface()->GetName() : TEXT("<none>") );
		if( _bImmediate || !GlobalMan )
		{
			FireDialogHookNow( _Hook, HookSubIndex, _pInitiator, NULL );
		}
		else
		{
			GlobalMan->QueueFireDialogHook( _Hook, HookSubIndex, this, _pInitiator );
		}
	}
	else
	{
		FDisFailedDialogHookInfo& Failed = m_FailedDialogHooks( m_FailedDialogHooks.AddZeroed() );
		Failed.m_Hook = _Hook;
		Failed.m_ReasonFailed = 1;
		Failed.m_fTimeToDisplay = m_fFailedDialogHookDisplayTime;
		FRDIAG( "hook %s: hook %d branch %d refused by the global refire delay", Owner ? *Owner->GetName() : *GetName(), (INT)_Hook, HookSubIndex );
	}
}

UBOOL UDisConversationComponent::FireDialogHookNow( eDisDialogHook _Hook, INT _iHookSubIndex, IDisConvSpeakerInterface* _pInitiator, eDisDialogCheckOnly* _pCheckOnly_ResultOut )
{
	if( DisOwnerIsDeadForHook( this, _Hook ) )
	{
		return FALSE;
	}
	UBOOL bHookFireNoContent = FALSE;
	UBOOL bFired = FALSE;
	for( INT i = 0; i < m_OneShotInfo.Num() && !bFired; i++ )
	{
		bFired = m_OneShotInfo(i).m_pOneShot_DialogTree && m_OneShotInfo(i).m_pOneShot_DialogTree->FireDialogHook( _Hook, _pInitiator, _iHookSubIndex, bHookFireNoContent, _pCheckOnly_ResultOut );
	}
	if( !bFired && !bHookFireNoContent )
	{
		for( INT i = 0; i < m_AdditionalGenericDialogTrees.Num() && !bFired; i++ )
		{
			bFired = m_AdditionalGenericDialogTrees(i) && m_AdditionalGenericDialogTrees(i)->FireDialogHook( _Hook, _pInitiator, _iHookSubIndex, bHookFireNoContent, _pCheckOnly_ResultOut );
		}
		if( !bFired && !bHookFireNoContent && m_pGenericDialogTree )
		{
			bFired = m_pGenericDialogTree->FireDialogHook( _Hook, _pInitiator, _iHookSubIndex, bHookFireNoContent, _pCheckOnly_ResultOut );
		}
	}
	if( bFired && !_pCheckOnly_ResultOut )
	{
		m_FireHookTag++;
		const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _Hook );
		const FLOAT Now = GWorld->GetTimeSeconds();
		if( _Hook < m_DialogHookInfo_InGame.Num() )
		{
			m_DialogHookInfo_InGame( _Hook ).m_fLastFireTime = Now + appFrand() * ( Info.m_fRefire_DelayMax - Info.m_fRefire_DelayMin );
		}
		if( UDisConvGlobalMan* GlobalMan = DisGetConvGlobalMan() )
		{
			GlobalMan->m_DialogHookInfo_InGame[_Hook].m_fLastFireTime = Now;
		}
	}
	FRDIAG( "hook %s: FireDialogHookNow hook %d branch %d -> %s%s", Owner ? *Owner->GetName() : *GetName(), (INT)_Hook, _iHookSubIndex,
		bFired ? TEXT("fired") : TEXT("nothing fired"), bHookFireNoContent ? TEXT(" (hook present, no content)") : TEXT("") );
	return bFired;
}

eDisDialogCheckOnly UDisConversationComponent::CheckDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator ) const
{
	eDisDialogCheckOnly Result = eDisDialogCheckOnly_NotFound;
	if( !_pInitiator )
	{
		_pInitiator = m_OwnerInterfaceSpeaker.GetObject() ? (IDisConvSpeakerInterface*)m_OwnerInterfaceSpeaker.GetInterface() : NULL;
	}
	if( DisOwnerIsDeadForHook( this, _Hook ) )
	{
		return Result;
	}
	INT HookSubIndex = -1;
	UBOOL bFound = FALSE;
	for( INT i = 0; i < m_OneShotInfo.Num() && !bFound; i++ )
	{
		bFound = m_OneShotInfo(i).m_pOneShot_DialogTree && m_OneShotInfo(i).m_pOneShot_DialogTree->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	for( INT i = 0; i < m_AdditionalGenericDialogTrees.Num() && !bFound; i++ )
	{
		bFound = m_AdditionalGenericDialogTrees(i) && m_AdditionalGenericDialogTrees(i)->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	if( !bFound && m_pGenericDialogTree )
	{
		m_pGenericDialogTree->EvaluateDialogHook( _Hook, _pInitiator, HookSubIndex, FALSE );
	}
	if( HookSubIndex != -1 )
	{
		const_cast<UDisConversationComponent*>( this )->FireDialogHookNow( _Hook, HookSubIndex, _pInitiator, &Result );
	}
	return Result;
}

void UDisConversationComponent::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bIsBlocking, INT _FiringHookIndex )
{
	FDisDialogRunningInstance* Current = GetRunningInst();
	if( Current && Current != _pDialogRunningInst )
	{
		const BYTE Hook = _pDialogRunningInst->m_DialogHookThatFiredInst;
		const UBOOL bDeath = Hook == DDH_COMBAT_DYING || Hook == DDH_COMBAT_BACKSTABBED || Hook == DDH_COMBAT_STEALTHKILL_PROJECTILE;
		BreakOutOfConversation( bDeath ? DisConvoEndReason_Death : DisConvoEndReason_NewConvo, eDisDialogHookFireGroup_MAX );
	}
	m_bIsBlocking = _bIsBlocking ? 1 : 0;
	m_iRunningInstID++;
	m_pDialogRunningInst = _pDialogRunningInst;
	m_fBlurbCueDuration = 0.0f;
	m_pLookTarget = NULL;
	FRDIAG( "bind %s starts dialog (blocking %d, hook %d, running id %d)", Owner ? *Owner->GetName() : *GetName(), _bIsBlocking ? 1 : 0, _FiringHookIndex, m_iRunningInstID );
}

void UDisConversationComponent::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
	StopDialogBlurb();
	m_bIsBlocking = 0;
	m_pDialogRunningInst = NULL;
	m_pLookTarget = NULL;
	FRDIAG( "bind %s ends dialog (reason %d)", Owner ? *Owner->GetName() : *GetName(), (INT)_ConvoEndReason );
}

eDisConvPriority UDisConversationComponent::InConversation( UBOOL& _rIsBlocking_Out, UBOOL* _pIsNonInterruptable, const INT* _pMatchRunningInstID ) const
{
	FDisDialogRunningInstance* Inst = GetRunningInst();
	if( Inst && Inst->m_pCurPlayingConversation && ( !_pMatchRunningInstID || *_pMatchRunningInstID == m_iRunningInstID ) )
	{
		_rIsBlocking_Out = m_bIsBlocking;
		if( _pIsNonInterruptable )
		{
			*_pIsNonInterruptable = ( UDisDialogTree::GetDialogHookInfo()( Inst->m_DialogHookThatFiredInst ).m_Flags & 4 ) != 0;
		}
		return (eDisConvPriority)Inst->m_CurPriority;
	}
	_rIsBlocking_Out = FALSE;
	if( _pIsNonInterruptable )
	{
		*_pIsNonInterruptable = FALSE;
	}
	return NOT_IN_A_CONV;
}

void UDisConversationComponent::BreakOutOfConversation( DisConvoEndReasonEnum _ConvoEndReason, eDisDialogHookFireGroup _FiringGroup )
{
	FDisDialogRunningInstance* Inst = GetRunningInst();
	if( bAttached && Inst )
	{
		const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( Inst->m_DialogHookThatFiredInst );
		if( ( Info.m_Flags & 4 ) == 0 || _ConvoEndReason == DisConvoEndReason_Destroyed )
		{
			UBOOL bStop = _FiringGroup == eDisDialogHookFireGroup_MAX;
			if( !bStop )
			{
				bStop = _FiringGroup == Info.m_FireGroup;
				UDisConversation* Conv = Inst->m_pCurPlayingConversation ? Cast<UDisConversation>( Inst->m_pCurPlayingConversation->m_pOwnerConvNode ) : NULL;
				if( Conv && Conv->m_bLowerAIAlertness )
				{
					bStop = _FiringGroup == eDisDialogHookFireGroup_Casual;
				}
			}
			if( bStop )
			{
				Inst->StopInstance( _ConvoEndReason );
			}
		}
	}
	// DISHONORED(bringup): on DisConvoEndReason_Destroyed retail also unregisters the owner from ADishonoredHUD's post-render list.
}

static IDisConvSpeakerInterface* DisOwnerSpeaker( const UDisConversationComponent* _pComp )
{
	return _pComp->m_OwnerInterfaceSpeaker.GetObject() ? (IDisConvSpeakerInterface*)_pComp->m_OwnerInterfaceSpeaker.GetInterface() : NULL;
}

static void ConvPrepareEventCB( AkUInt32 _EventID, AKRESULT, AkMemPoolId, void* _pCookie )
{
	( (UDisConversationComponent*)_pCookie )->m_ReadyAkEventID = (INT)_EventID;
}

static void DisConvAkEventCB( AkCallbackType _Type, AkCallbackInfo* _pInfo )
{
	if( _Type == AK_EndOfEvent )
	{
		DisAkEndOfEventCB( AK_EndOfEvent, _pInfo );
		return;
	}
	const FLOAT Duration = ( (AkDurationCallbackInfo*)_pInfo )->fDuration * 0.001f;
	AActor* Owner = ( (UAkComponent*)_pInfo->gameObjID )->GetOwner();
	ADisSpeaker_PA* PASpeaker = Cast<ADisSpeaker_PA>( Owner );
	if( PASpeaker )
	{
		for( INT i = 0; i < PASpeaker->m_OwningSpeakerGroups.Num(); i++ )
		{
			ADisSpeakerGroup_PA* Group = PASpeaker->m_OwningSpeakerGroups(i);
			IDisConvSpeakerInterface* GroupSpeaker = Group ? (IDisConvSpeakerInterface*)Group->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() ) : NULL;
			UDisConversationComponent* Comp = GroupSpeaker ? GroupSpeaker->GetConversationComponent() : NULL;
			if( Comp )
			{
				Comp->m_fPlayingAudioDuration = Duration;
			}
		}
		return;
	}
	IDisConvSpeakerInterface* Speaker = Owner ? (IDisConvSpeakerInterface*)Owner->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() ) : NULL;
	UDisConversationComponent* Comp = Speaker ? Speaker->GetConversationComponent() : NULL;
	if( Comp )
	{
		Comp->m_fPlayingAudioDuration = Duration;
		IDDIAG( "voice duration %.2f s on %s", Duration, *Owner->GetName() );
	}
}

void UDisConversationComponent::PlayDialogBlurb( UDisDialogVoiceData* _pVoiceData, FDisVoiceTextBlurb* _pBlurb, const TCHAR* _pText, FLOAT _fBlurbEndDelay,
	UBOOL _bShowDebugText, FLOAT _fVoiceRadius, UBOOL _bMainSubtitle, FLOAT _fSeekTime )
{
	if( _pBlurb )
	{
		UDisConvGlobalMan* GlobalMan = DisGetConvGlobalMan();
		UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
		TArray<FDisConvSpeakerLocInfo> LocInfo;
		DisOwnerSpeaker( this )->GetSpeakerLocInfo( LocInfo );
		TArray<FDisAkCompAndPlayingID> Speakers;
		m_fCurVoiceRadius = _fVoiceRadius;
		FLOAT AttenuationScale = _fVoiceRadius * 0.00033333333f;
		if( GlobalMan && GlobalMan->m_CurDominantHook.m_iPlayingHookIndex != -1 )
		{
			AttenuationScale *= GlobalMan->GetConvSettings()->m_fNonDominantVolumeScale;
		}
		m_fPlayingAudioDuration = -1.0f;
		for( INT i = 0; i < LocInfo.Num() && AudioDevice; i++ )
		{
			AActor* EventOwner = LocInfo(i).m_pAKEventOwner;
			UDisAkComponent* AkComp = (UDisAkComponent*)AudioDevice->GetAkComponent( EventOwner, ms_VoiceBoneName, FALSE );
			if( !AkComp )
			{
				continue;
			}
			if( AkComp->m_PlayingRadii.Num() > 0 )
			{
				AkComp->Stop();
			}
			// WITH_FACEFX=0: USkeletalMeshComponent::StopFaceFXAnim on LocInfo(i).m_pSkelMeshCompForFaceFX, then the FaceFX
			// branch (an event name of 2+ characters, a mesh with a FaceFX asset and the voice data's m_pFaceFXAnimSet: the
			// pending blurb / text / main flag are stored and PlayFaceFXAnim starts the line). Without FaceFX that call fails,
			// which is retail's own fall-through to the Wwise post below.
			AK::SoundEngine::SetAttenuationScalingFactor( AkComp->GetAkGameObjectID(), AttenuationScale );
			const AkPlayingID PlayingID = AudioDevice->PostEvent( _pBlurb->m_AkEventName, _fVoiceRadius, EventOwner, ms_VoiceBoneName,
				AK_EndOfEvent | AK_Duration, DisConvAkEventCB, GWorld ? GWorld->m_pAudioSystem : NULL, FALSE );
			if( _fSeekTime > 0.0f )
			{
				AudioDevice->SeekOnEvent( _pBlurb->m_AkEventName, EventOwner, ms_VoiceBoneName, (AkTimeMs)( _fSeekTime * 1000.0f ) );
			}
#if DISHONORED_WWISE_SILENT
			if( PlayingID != AK_INVALID_PLAYING_ID )
			{
				const FLOAT HintSeconds = Clamp<FLOAT>( appStrlen( _pText ) / 15.0f, 1.0f, 12.0f );
				DishonoredWwise::SetDurationHint( PlayingID, HintSeconds );
				IDDIAG( "voice hint %.2f s (source: %d characters / 15 cps, clamped 1..12; the silent backend has no media length) playing id %u event %s on %s",
					HintSeconds, appStrlen( _pText ), PlayingID, *_pBlurb->m_AkEventName, *EventOwner->GetName() );
			}
#endif
			if( PlayingID != AK_INVALID_PLAYING_ID && *_pText )
			{
				FDisAkCompAndPlayingID& Entry = Speakers( Speakers.AddZeroed() );
				Entry.m_pAkComp = AkComp;
				Entry.m_PlayingID = (INT)PlayingID;
			}
		}
		if( Speakers.Num() > 0 && GlobalMan && !GEngine->bSubtitlesForcedOff && GEngine->bSubtitlesEnabled && ( _bMainSubtitle || !GEngine->bHideSecondarySubtitles ) )
		{
			GlobalMan->m_SubtitlesInfo.AddSubtitle( this, Speakers, _pText, _fVoiceRadius, _bMainSubtitle );
		}
		else
		{
			IDDIAG( "no subtitle for '%s': %d voice(s), subtitles enabled %d forced off %d hide secondary %d main %d", _pText, Speakers.Num(),
				(INT)GEngine->bSubtitlesEnabled, (INT)GEngine->bSubtitlesForcedOff, (INT)GEngine->bHideSecondarySubtitles, (INT)_bMainSubtitle );
		}
	}
	m_fBlurbElapsedTime = Max( _fSeekTime, 0.0f );
}

void UDisConversationComponent::StreamDialogBlurb( FDisVoiceTextBlurb* _pBlurb )
{
	const AkUniqueID EventID = AK::SoundEngine::GetIDFromString( *_pBlurb->m_AkEventName );
	if( m_PendingAkEventID )
	{
		UnprepareAkEvent();
	}
	m_PendingAkEventID = (INT)EventID;
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( GIsEditor || !AudioDevice )
	{
		// DISHONORED(written): retail has a device in game; with none (-nosound) the line must not wait for ever.
		m_ReadyAkEventID = (INT)EventID;
		return;
	}
	AudioDevice->PrepareEvent( AK::SoundEngine::Preparation_Load, EventID, ConvPrepareEventCB, this );
}

void UDisConversationComponent::UnprepareAkEvent()
{
	if( !m_PendingAkEventID )
	{
		return;
	}
	AK::SoundEngine::CancelBankCallbackCookie( this );
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !GIsEditor && AudioDevice )
	{
		AudioDevice->PrepareEvent( AK::SoundEngine::Preparation_Unload, (AkUniqueID)m_PendingAkEventID, NULL, NULL );
	}
	m_ReadyAkEventID = 0;
	m_PendingAkEventID = 0;
}

UBOOL UDisConversationComponent::IsReadyToSpeak() const
{
	UBOOL bReady = TRUE;
	ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( Owner );
	UDishonoredAIBrain* Brain = NPC ? NPC->GetAIBrain() : NULL;
	UDisBehaviorBlockingDialog* BlockingDialog = Brain ? Cast<UDisBehaviorBlockingDialog>( Brain->m_pCurrentBehavior ) : NULL;
	if( BlockingDialog )
	{
		bReady = BlockingDialog->IsReadyToSpeak();
	}
	return ( m_ReadyAkEventID || !m_PendingAkEventID ) ? bReady : FALSE;
}

UBOOL UDisConversationComponent::IsDonePlayingBlurb() const
{
	if( m_fBlurbCueDuration >= 0.00000001f )
	{
		return m_fBlurbCueDuration > m_fBlurbElapsedTime ? FALSE : TRUE;
	}
	// WITH_FACEFX=0: USkeletalMeshComponent::IsPlayingFaceFXAnim on the first speaker location's mesh is compiled out.
	return !IsAnyAkEventPlaying();
}

UBOOL UDisConversationComponent::IsAnyAkEventPlaying() const
{
	TArray<FDisConvSpeakerLocInfo> LocInfo;
	DisOwnerSpeaker( this )->GetSpeakerLocInfo( LocInfo );
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	for( INT i = 0; i < LocInfo.Num() && AudioDevice; i++ )
	{
		UAkComponent* AkComp = AudioDevice->GetAkComponent( LocInfo(i).m_pAKEventOwner, ms_VoiceBoneName, FALSE );
		if( AkComp && AkComp->m_PlayingRadii.Num() > 0 )
		{
			return TRUE;
		}
	}
	return FALSE;
}

FDisVoiceTextBlurb* UDisConversationComponent::FindTextBlurb( const UDisDialogTree* _pTree, const FGuid& _rBlurbGUID, UDisDialogVoiceData*& _rpVoiceDataOut )
{
	for( INT i = 0; i < m_OneShotInfo.Num(); i++ )
	{
		UDisDialogVoiceData* VoiceData = m_OneShotInfo(i).m_pOneShot_DialogData;
		FDisVoiceTextBlurb* Blurb = VoiceData ? VoiceData->FindTextBlurb( _pTree, _rBlurbGUID ) : NULL;
		if( Blurb )
		{
			_rpVoiceDataOut = VoiceData;
			return Blurb;
		}
	}
	for( INT i = 0; i < m_AdditionalGenericDialogDatas.Num(); i++ )
	{
		FDisVoiceTextBlurb* Blurb = m_AdditionalGenericDialogDatas(i)->FindTextBlurb( _pTree, _rBlurbGUID );
		if( Blurb )
		{
			_rpVoiceDataOut = m_AdditionalGenericDialogDatas(i);
			return Blurb;
		}
	}
	if( !m_pGenericDialogData_Cached )
	{
		return NULL;
	}
	_rpVoiceDataOut = m_pGenericDialogData_Cached;
	return m_pGenericDialogData_Cached->FindTextBlurb( _pTree, _rBlurbGUID );
}

UDisAkComponent* UDisConversationComponent::StopAkEvents( AActor* _pActor )
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	UAkComponent* AkComp = AudioDevice ? AudioDevice->GetAkComponent( _pActor, ms_VoiceBoneName, FALSE ) : NULL;
	if( AkComp && AkComp->m_PlayingRadii.Num() > 0 )
	{
		AkComp->Stop();
	}
	return (UDisAkComponent*)AkComp;
}

void UDisConversationComponent::StopDialogBlurb()
{
	UnprepareAkEvent();
	if( Owner )
	{
		TArray<FDisConvSpeakerLocInfo> LocInfo;
		DisOwnerSpeaker( this )->GetSpeakerLocInfo( LocInfo );
		for( INT i = 0; i < LocInfo.Num(); i++ )
		{
			StopAkEvents( LocInfo(i).m_pAKEventOwner );
			// WITH_FACEFX=0: USkeletalMeshComponent::StopFaceFXAnim on LocInfo(i).m_pSkelMeshCompForFaceFX is compiled out.
		}
	}
	m_fBlurbCueDuration = 0.0f;
	m_DebugSpeechText = TEXT("");
	UDisConvGlobalMan* GlobalMan = DisGetConvGlobalMan();
	if( GlobalMan )
	{
		GlobalMan->m_SubtitlesInfo.KillSubtitle( this );
	}
}

void UDisConversationComponent::OnOtherActorTerminated( const AActor& _rOtherActor )
{
	for( INT i = 0; i < m_OneShotInfo.Num(); i++ )
	{
		if( m_OneShotInfo(i).m_pOneShot_DialogTree )
		{
			m_OneShotInfo(i).m_pOneShot_DialogTree->OnOtherActorTerminated( _rOtherActor );
		}
	}
	for( INT i = 0; i < m_AdditionalGenericDialogTrees.Num(); i++ )
	{
		if( m_AdditionalGenericDialogTrees(i) )
		{
			m_AdditionalGenericDialogTrees(i)->OnOtherActorTerminated( _rOtherActor );
		}
	}
	if( m_pGenericDialogTree )
	{
		m_pGenericDialogTree->OnOtherActorTerminated( _rOtherActor );
	}
	if( m_pLookTarget == &_rOtherActor )
	{
		m_pLookTarget = NULL;
	}
	if( m_HookParams.m_pCorpse.GetObject() == &_rOtherActor )
	{
		m_HookParams.m_pCorpse.SetObject( NULL );
		m_HookParams.m_pCorpse.SetInterface( NULL );
	}
}

eDisQueuedFiringGroupType UDisConversationComponent::ClassifyGlobalFireGroup() const
{
	const IDisConvSpeakerInterface* Speaker = m_OwnerInterfaceSpeaker.GetObject() ? (const IDisConvSpeakerInterface*)m_OwnerInterfaceSpeaker.GetInterface() : NULL;
	const UDisTweaks_ConvSettings* Settings = UDisConvGlobalMan::s_pInstance ? UDisConvGlobalMan::s_pInstance->GetConvSettings() : NULL;
	const FLOAT LoudDistance = Settings ? Settings->m_fDialogDistanceConsideredLoud : 0.0f;
	return LoudDistance <= UDisConvGlobalMan::GetShortestAudibleDistForSpeaker( Speaker, LoudDistance ) ? eDisQueuedFiringGroupType_Quiet : eDisQueuedFiringGroupType_Loud;
}
