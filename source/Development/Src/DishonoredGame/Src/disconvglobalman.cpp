// DishonoredGame/src/disconvglobalman.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The game info's conversation manager: the queue every non-immediate dialog
// hook goes through. 2013 rvas (2012 in parentheses; * = found through the caller, unnamed in the 2013 database):
//   InitGlobalConvMan 0x89caa0* (0x8f2610, 498)      Term_GlobalConv (0x8ef920, 92)       TickGlobalConvMan 0x8a71c0 (0x8f5610, 410)
//   QueueFireDialogHook 0x89cc80* (0x8f2830, 449)    ManageGlobalDialogHooks 0x8a5b20* (0x8f2d60, 2899)
//   CalcPendingWeights (0x8f2a00, 860)               GetShortestAudibleDistForSpeaker (0x8e5a00, 480)
//   DisGetConvGlobalMan 0x7bf3e0 (0x8271e0, 6; dishonoredutilities_accessors.cpp)
// Retail's InitGlobalConvMan is called from ADishonoredGameInfo::InitGlobalManagers (2013 0x5e9bd0; the 0x8b2e80 that
// function's comment used to carry is a TArray helper), TickGlobalConvMan from ADishonoredGameInfo::Tick (2012 0x64b533).
// Left out of these bodies, named at the site: DuckNonDominantAudio (0x896cb0*, 935 B) and FDisSubtitlesInfo::Tick
// (0x89e660*, 1712 B) in TickGlobalConvMan, the HUD subtitle resets in Term_GlobalConv, and the audio-cell propagation
// in GetShortestAudibleDistForSpeaker (UDishonoredAudioSystem::GetAudioCellAtPoint / GetMinAudibleDistance), which is the
// plain distance to the player here. Audio is Phase 10 polish.
// Not ported (agentFR.md inventory): OnActorTerminated_ConvGlobalMan 2012 0x8ee9e0, OnSpeakerDestroyed 0x8ec930,
// OnSpeakerStartedTalking 2013 0x89e610, GameSave / GameLoad 2012 0x8ef980 / 0x8efa90, the branch-info accessors.
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "frdialogdiag.h"

UDisConvGlobalMan* UDisConvGlobalMan::s_pInstance = NULL;

UDisConvGlobalMan* DisGetConvGlobalMan()
{
	return UDisConvGlobalMan::s_pInstance;
}

struct FDisHookWeight
{
	FLOAT m_fWeight;
	INT m_iIndex;
};

UDisTweaksBase* UDisConvGlobalMan::GetTweaks_Derived()
{
	return m_pConvSettingTweaks;
}

void UDisConvGlobalMan::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pConvSettingTweaks = Cast<UDisTweaks_ConvSettings>( _pTweaks );
}

UDisTweaks_ConvSettings* UDisConvGlobalMan::GetConvSettings() const
{
	UDisTweaks_ConvSettings* Settings = m_pConvSettingTweaks;
	if( !Settings )
	{
		Settings = (UDisTweaks_ConvSettings*)UDisTweaks_ConvSettings::StaticClass()->GetDefaultObject();
	}
	return Settings;
}

void UDisConvGlobalMan::InitGlobalConvMan()
{
	s_pInstance = this;
	ApplyTweakChanges();
	UDisTweaks_ConvSettings* Settings = GetConvSettings();
	for( INT i = 0; i < Settings->m_FireGroupConfig.Num(); i++ )
	{
		const FDisFireGroupConfig& Config = Settings->m_FireGroupConfig(i);
		if( Config.m_FireGroup >= eDisDialogHookFireGroup_MAX )
		{
			continue;
		}
		FDisQueuedFireDialogHookGroup_Outer& Outer = m_QueuedHookGroups[Config.m_FireGroup];
		Outer.m_Inner[eDisQueuedFiringGroupType_Loud].m_MaxPlaying_PerGroup = Config.m_MaxPlaying_PerGroup;
		Outer.m_Inner[eDisQueuedFiringGroupType_Loud].m_MaxPlaying_PerInitiator = Config.m_MaxPlaying_PerInitiator;
		Outer.m_Inner[eDisQueuedFiringGroupType_Loud].m_bIgnoreGlobalCap = Config.m_bIgnoreGlobalCap;
		Outer.m_Inner[eDisQueuedFiringGroupType_Loud].m_fMinTimeBetweenFiring = Config.m_fMinTimeBetweenFiring;
		Outer.m_Inner[eDisQueuedFiringGroupType_Quiet].m_MaxPlaying_PerGroup = Config.m_MaxPlaying_PerGroup_Quiet;
		Outer.m_Inner[eDisQueuedFiringGroupType_Quiet].m_MaxPlaying_PerInitiator = Config.m_MaxPlaying_PerInitiator_Quiet;
		Outer.m_Inner[eDisQueuedFiringGroupType_Quiet].m_bIgnoreGlobalCap = Config.m_bIgnoreGlobalCap;
		Outer.m_Inner[eDisQueuedFiringGroupType_Quiet].m_fMinTimeBetweenFiring = Config.m_fMinTimeBetweenFiring;
	}
	TArray<FDisDialogHookInfo>& HookInfo = UDisDialogTree::GetDialogHookInfo();
	for( INT i = 0; i < Settings->m_DialogHookOverrides.Num(); i++ )
	{
		const FDisDialogHookConfigOverride& Override = Settings->m_DialogHookOverrides(i);
		if( Override.m_DialogHookToOverride < HookInfo.Num() )
		{
			FDisDialogHookInfo& Info = HookInfo( Override.m_DialogHookToOverride );
			Info.m_fRefire_DelayMin = Override.m_fRefire_DelayMin;
			Info.m_fRefire_DelayMax = Override.m_fRefire_DelayMax;
			Info.m_fRefire_Delay_Global = Override.m_fRefire_Delay_Global;
			Info.m_FireGroup = Override.m_FireGroup;
		}
	}
	FRDIAG( "queue global manager %s initialised: %d fire group config(s), %d hook override(s), max at once %d / quiet %d",
		*GetName(), Settings->m_FireGroupConfig.Num(), Settings->m_DialogHookOverrides.Num(), Settings->m_MaxDialogHooksAtOnce, Settings->m_MaxDialogHooksAtOnce_Quiet );
}

void UDisConvGlobalMan::Term_GlobalConv()
{
	s_pInstance = NULL;
	m_SubtitlesInfo.m_MainSubtitles.Reset();
	m_SubtitlesInfo.m_SecondarySubtitles.Reset();
	// DISHONORED(bringup): retail then clears both subtitle lines on UDisGFxMoviePlayerHUD (OnMainSubtitleChanged /
	// OnSecondarySubtitleChanged) - the subtitle package's.
}

FLOAT UDisConvGlobalMan::GetShortestAudibleDistForSpeaker( const IDisConvSpeakerInterface* _pCurSpeaker, FLOAT _fMaxDistance )
{
	if( !_pCurSpeaker || !ADishonoredPlayerPawn::s_pInstance )
	{
		return 3.4e38f;
	}
	TArray<FDisConvSpeakerLocInfo> LocInfo;
	_pCurSpeaker->GetSpeakerLocInfo( LocInfo );
	const FVector PlayerEarLoc = ADishonoredPlayerPawn::s_pInstance->Location;
	FLOAT AudibleDist = 3.4e38f;
	for( INT i = 0; i < LocInfo.Num(); i++ )
	{
		const FLOAT Dist = ( LocInfo(i).m_Location - PlayerEarLoc ).Size();
		if( Dist < AudibleDist )
		{
			AudibleDist = Dist;
		}
	}
	return AudibleDist;
}

void UDisConvGlobalMan::CountInitiator( eDisQueuedFiringGroupType _GroupType, const FDisQueuedFireDialogHook& _rHook, INT _Delta )
{
	IDisConvSpeakerInterface* Target = _rHook.m_pTarget.GetObject() ? (IDisConvSpeakerInterface*)_rHook.m_pTarget.GetInterface() : NULL;
	if( !Target || !_rHook.m_pConvComp || _rHook.m_pTarget.GetObject() == _rHook.m_pConvComp->GetOwner() )
	{
		return;
	}
	TMap<IDisConvSpeakerInterface*,INT>& Map = m_InitiatorCountMap[_GroupType].m_Map;
	INT* Count = Map.Find( Target );
	if( Count )
	{
		*Count += _Delta;
	}
	else if( _Delta > 0 )
	{
		Map.Set( Target, _Delta );
	}
}

void UDisConvGlobalMan::QueueFireDialogHook( eDisDialogHook _Hook, INT _iHookSubIndex, UDisConversationComponent* _pConvComp, IDisConvSpeakerInterface* _pTarget )
{
	const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _Hook );
	const eDisQueuedFiringGroupType GroupType = _pConvComp->ClassifyGlobalFireGroup();
	FDisQueuedFireDialogHookGroup& Group = m_QueuedHookGroups[Info.m_FireGroup].m_Inner[GroupType];
	UDisTweaks_ConvSettings* Settings = GetConvSettings();
	UObject* TargetObject = _pTarget ? _pTarget->GetUObjectInterfaceDisConvSpeakerInterface() : NULL;
	if( Group.m_bIsLocked )
	{
		FDisFailedDialogHookInfo& Failed = _pConvComp->m_FailedDialogHooks( _pConvComp->m_FailedDialogHooks.AddZeroed() );
		Failed.m_Hook = _Hook;
		Failed.m_ReasonFailed = 3;
		Failed.m_fTimeToDisplay = _pConvComp->m_fFailedDialogHookDisplayTime;
		FRDIAG( "queue hook %d from %s refused: fire group %d (%s) is locked", (INT)_Hook, *_pConvComp->GetOwner()->GetName(), (INT)Info.m_FireGroup, GroupType == eDisQueuedFiringGroupType_Loud ? TEXT("loud") : TEXT("quiet") );
		return;
	}
	if( Group.m_MaxPlaying_PerInitiator > 0 && _pTarget && TargetObject != _pConvComp->GetOwner() )
	{
		if( m_InitiatorCountMap[GroupType].m_Map.FindRef( _pTarget ) >= Group.m_MaxPlaying_PerInitiator )
		{
			FDisDialogRunningInstance* Inst = _pConvComp->GetRunningInst();
			IDisConvSpeakerInterface* RunningInitiator = ( Inst && Inst->m_pBoundSpeaker_Initiator.GetObject() ) ? (IDisConvSpeakerInterface*)Inst->m_pBoundSpeaker_Initiator.GetInterface() : NULL;
			if( RunningInitiator != _pTarget )
			{
				FDisFailedDialogHookInfo& Failed = _pConvComp->m_FailedDialogHooks( _pConvComp->m_FailedDialogHooks.AddZeroed() );
				Failed.m_Hook = _Hook;
				Failed.m_ReasonFailed = 4;
				Failed.m_fTimeToDisplay = _pConvComp->m_fFailedDialogHookDisplayTime;
				FRDIAG( "queue hook %d from %s refused: initiator %s is already at the per-initiator cap", (INT)_Hook, *_pConvComp->GetOwner()->GetName(), *TargetObject->GetName() );
				return;
			}
		}
	}
	FDisQueuedFireDialogHook& NewHook = Group.m_PendingHooks( Group.m_PendingHooks.AddZeroed() );
	NewHook.m_Hook = _Hook;
	NewHook.m_iHookSubIndex = _iHookSubIndex;
	NewHook.m_pConvComp = _pConvComp;
	NewHook.m_pTarget.SetObject( TargetObject );
	NewHook.m_pTarget.SetInterface( TargetObject ? _pTarget : NULL );
	NewHook.m_fCurAudioDuckingWeight = m_CurDominantHook.m_iPlayingHookIndex == -1 ? 1.0f : Settings->m_fNonDominantVolumeScale;
	NewHook.m_HookParams = _pConvComp->m_HookParams;
	NewHook.m_bOwnerAndTargetDiffer = !TargetObject || TargetObject != _pConvComp->GetOwner();
	FRDIAG( "queue hook %d branch %d from %s queued in fire group %d (%s), %d pending", (INT)_Hook, _iHookSubIndex, *_pConvComp->GetOwner()->GetName(), (INT)Info.m_FireGroup,
		GroupType == eDisQueuedFiringGroupType_Loud ? TEXT("loud") : TEXT("quiet"), Group.m_PendingHooks.Num() );
}

void UDisConvGlobalMan::CalcPendingWeights( FDisQueuedFireDialogHookGroup& _rGroup, eDisQueuedFiringGroupType _GroupType, TArray<FDisHookWeight>& _rWeights )
{
	UDisTweaks_ConvSettings* Settings = GetConvSettings();
	TMap<IDisConvSpeakerInterface*,INT> PendingInitiatorMap;
	const FVector PlayerLoc = ADishonoredPlayerPawn::s_pInstance ? ADishonoredPlayerPawn::s_pInstance->Location : FVector(0,0,0);
	_rWeights.Empty( _rGroup.m_PendingHooks.Num() );
	_rWeights.AddZeroed( _rGroup.m_PendingHooks.Num() );
	for( INT i = 0; i < _rGroup.m_PendingHooks.Num(); i++ )
	{
		const FDisQueuedFireDialogHook& Hook = _rGroup.m_PendingHooks(i);
		FDisHookWeight& Weight = _rWeights(i);
		Weight.m_iIndex = i;
		Weight.m_fWeight = 0.0f;
		const AActor* Owner = Hook.m_pConvComp ? Hook.m_pConvComp->GetOwner() : NULL;
		const FLOAT DistToPlayer = Owner ? ( Owner->Location - PlayerLoc ).Size() : 0.0f;
		const FLOAT MaxDist = Settings->m_fMaxDialogDistance;
		if( MaxDist <= DistToPlayer && Hook.m_Hook != DDH_KISMET_ACTIVATED )
		{
			continue;
		}
		const FLOAT ClampedDist = Clamp( DistToPlayer, 0.0f, MaxDist );
		Weight.m_fWeight = ( MaxDist > 0.0f ? 1.0f - ClampedDist / MaxDist : 1.0f ) + appFrand() * Settings->m_fHookWeightRandRange;
		IDisConvSpeakerInterface* Target = Hook.m_pTarget.GetObject() ? (IDisConvSpeakerInterface*)Hook.m_pTarget.GetInterface() : NULL;
		if( _rGroup.m_MaxPlaying_PerInitiator > 0 && Target && Hook.m_pTarget.GetObject() != Owner )
		{
			const INT Firing = m_InitiatorCountMap[_GroupType].m_Map.FindRef( Target );
			INT* Pending = PendingInitiatorMap.Find( Target );
			if( !Pending )
			{
				PendingInitiatorMap.Set( Target, 1 );
			}
			else
			{
				FDisDialogRunningInstance* Inst = Hook.m_pConvComp->GetRunningInst();
				const UBOOL bAlreadyTalkingToTarget = Inst && Inst->m_pBoundSpeaker_Initiator.GetObject() && Inst->m_pBoundSpeaker_Initiator.GetObject() == Hook.m_pTarget.GetObject();
				if( Firing + *Pending < _rGroup.m_MaxPlaying_PerInitiator || bAlreadyTalkingToTarget )
				{
					( *Pending )++;
				}
				else
				{
					Weight.m_fWeight = 0.0f;
				}
			}
		}
	}
	for( INT i = 1; i < _rWeights.Num(); i++ )
	{
		for( INT j = i; j > 0 && _rWeights(j).m_fWeight > _rWeights(j - 1).m_fWeight; j-- )
		{
			Exchange( _rWeights(j), _rWeights(j - 1) );
		}
	}
}

static UBOOL DisFiringHookStillRunning( const FDisQueuedFireDialogHook& Hook )
{
	const UDisConversationComponent* Comp = Hook.m_pConvComp;
	const FDisDialogRunningInstance* Inst = Comp ? Comp->GetRunningInst() : NULL;
	return Inst && Inst->m_DialogHookThatFiredInst == Hook.m_Hook && Inst->m_iDialogHookBranchIndex == Hook.m_iHookSubIndex
		&& Comp->m_FireHookTag == Hook.m_FireHookTag && Inst->m_pBoundSpeaker_Initiator == Hook.m_pTarget;
}

void UDisConvGlobalMan::ManageGlobalDialogHooks( FLOAT _fDeltaSeconds )
{
	UDisTweaks_ConvSettings* Settings = GetConvSettings();
	INT NumHooksAvailGlobal[eDisQueuedFiringGroupType_MAX] = { Settings->m_MaxDialogHooksAtOnce, Settings->m_MaxDialogHooksAtOnce_Quiet };
	TArray<FDisHookWeight> PendingWeights;
	for( INT GroupIndex = eDisDialogHookFireGroup_MAX - 1; GroupIndex >= 0; GroupIndex-- )
	{
		FDisQueuedFireDialogHookGroup_Outer& Outer = m_QueuedHookGroups[GroupIndex];
		for( INT Type = 0; Type < eDisQueuedFiringGroupType_MAX; Type++ )
		{
			TArrayNoInit<FDisQueuedFireDialogHook>& Firing = Outer.m_Inner[Type].m_FiringHooks;
			for( INT i = 0; i < Firing.Num(); i++ )
			{
				if( !Firing(i).m_pConvComp )
				{
					continue;
				}
				const eDisQueuedFiringGroupType NowType = Firing(i).m_pConvComp->ClassifyGlobalFireGroup();
				if( NowType != Type )
				{
					FDisQueuedFireDialogHook Moved = Firing(i);
					CountInitiator( (eDisQueuedFiringGroupType)Type, Moved, -1 );
					CountInitiator( NowType, Moved, +1 );
					Outer.m_Inner[NowType].m_FiringHooks.AddItem( Moved );
					Firing.Remove( i-- );
				}
			}
		}
		for( INT Type = 0; Type < eDisQueuedFiringGroupType_MAX; Type++ )
		{
			FDisQueuedFireDialogHookGroup& Group = Outer.m_Inner[Type];
			INT& GlobalAvail = NumHooksAvailGlobal[Type];
			if( Group.m_bIsLocked )
			{
				Group.m_fGroupLockoutTimer -= _fDeltaSeconds;
				if( Group.m_fGroupLockoutTimer <= 0.0f )
				{
					Group.m_bIsLocked = 0;
				}
			}
			const UBOOL bUnlimited = Group.m_MaxPlaying_PerGroup <= 0;
			INT Capacity = bUnlimited ? 0 : Group.m_MaxPlaying_PerGroup - Group.m_FiringHooks.Num();
			if( !Group.m_bIgnoreGlobalCap )
			{
				GlobalAvail -= Group.m_FiringHooks.Num();
			}
			const INT NumPending = Group.m_PendingHooks.Num();
			if( NumPending > 0 && ( bUnlimited || Capacity > 0 ) && ( GlobalAvail > 0 || Group.m_bIgnoreGlobalCap ) )
			{
				CalcPendingWeights( Group, (eDisQueuedFiringGroupType)Type, PendingWeights );
				INT Cur = 0;
				for( ; Cur < NumPending && ( bUnlimited || Capacity > 0 ); Cur++ )
				{
					if( PendingWeights(Cur).m_fWeight < 0.0001f )
					{
						break;
					}
					FDisQueuedFireDialogHook& Hook = Group.m_PendingHooks( PendingWeights(Cur).m_iIndex );
					UDisConversationComponent* Comp = Hook.m_pConvComp;
					Comp->m_HookParams = Hook.m_HookParams;
					IDisConvSpeakerInterface* Target = Hook.m_pTarget.GetObject() ? (IDisConvSpeakerInterface*)Hook.m_pTarget.GetInterface() : NULL;
					FRDIAG( "queue firing hook %d branch %d on %s (fire group %d %s, weight %.2f)", (INT)Hook.m_Hook, Hook.m_iHookSubIndex, *Comp->GetOwner()->GetName(), GroupIndex,
						Type == eDisQueuedFiringGroupType_Loud ? TEXT("loud") : TEXT("quiet"), PendingWeights(Cur).m_fWeight );
					if( Comp->FireDialogHookNow( (eDisDialogHook)Hook.m_Hook, Hook.m_iHookSubIndex, Target, NULL ) )
					{
						Hook.m_FireHookTag = Comp->m_FireHookTag;
						Group.m_FiringHooks.AddItem( Hook );
						CountInitiator( (eDisQueuedFiringGroupType)Type, Hook, +1 );
						if( !Group.m_bIgnoreGlobalCap )
						{
							GlobalAvail--;
							if( GlobalAvail <= 0 )
							{
								Cur++;
								break;
							}
						}
					}
					Capacity--;
				}
				for( ; Cur < NumPending; Cur++ )
				{
					FDisQueuedFireDialogHook& Hook = Group.m_PendingHooks( PendingWeights(Cur).m_iIndex );
					FDisFailedDialogHookInfo& Failed = Hook.m_pConvComp->m_FailedDialogHooks( Hook.m_pConvComp->m_FailedDialogHooks.AddZeroed() );
					Failed.m_Hook = Hook.m_Hook;
					Failed.m_ReasonFailed = 5;
					Failed.m_fTimeToDisplay = Hook.m_pConvComp->m_fFailedDialogHookDisplayTime;
				}
			}
			else
			{
				for( INT i = 0; i < NumPending; i++ )
				{
					FDisQueuedFireDialogHook& Hook = Group.m_PendingHooks(i);
					if( Hook.m_pConvComp )
					{
						FDisFailedDialogHookInfo& Failed = Hook.m_pConvComp->m_FailedDialogHooks( Hook.m_pConvComp->m_FailedDialogHooks.AddZeroed() );
						Failed.m_Hook = Hook.m_Hook;
						Failed.m_ReasonFailed = 5;
						Failed.m_fTimeToDisplay = Hook.m_pConvComp->m_fFailedDialogHookDisplayTime;
					}
				}
				if( NumPending > 0 )
				{
					FRDIAG( "queue fire group %d %s: %d pending hook(s) dropped (capacity %d, global %d)", GroupIndex, Type == eDisQueuedFiringGroupType_Loud ? TEXT("loud") : TEXT("quiet"), NumPending, Capacity, GlobalAvail );
				}
			}
			Group.m_PendingHooks.Empty();
		}
	}
	for( INT GroupIndex = 0; GroupIndex < eDisDialogHookFireGroup_MAX; GroupIndex++ )
	{
		for( INT Type = 0; Type < eDisQueuedFiringGroupType_MAX; Type++ )
		{
			FDisQueuedFireDialogHookGroup& Group = m_QueuedHookGroups[GroupIndex].m_Inner[Type];
			for( INT i = 0; i < Group.m_FiringHooks.Num(); i++ )
			{
				if( DisFiringHookStillRunning( Group.m_FiringHooks(i) ) )
				{
					continue;
				}
				CountInitiator( (eDisQueuedFiringGroupType)Type, Group.m_FiringHooks(i), -1 );
				Group.m_FiringHooks.Remove( i-- );
				if( Group.m_fMinTimeBetweenFiring > 0.0001f )
				{
					Group.m_bIsLocked = 1;
					Group.m_fGroupLockoutTimer = Group.m_fMinTimeBetweenFiring;
				}
			}
		}
	}
}

void UDisConvGlobalMan::TickGlobalConvMan( FLOAT _fDeltaSeconds )
{
	ManageGlobalDialogHooks( _fDeltaSeconds );
	FLOAT MaxDominantWeight = 0.00000001f;
	m_CurDominantHook.m_iPlayingHookIndex = -1;
	for( INT GroupIndex = 0; GroupIndex < eDisDialogHookFireGroup_MAX; GroupIndex++ )
	{
		for( INT Type = 0; Type < eDisQueuedFiringGroupType_MAX; Type++ )
		{
			const TArrayNoInit<FDisQueuedFireDialogHook>& Firing = m_QueuedHookGroups[GroupIndex].m_Inner[Type].m_FiringHooks;
			for( INT i = 0; i < Firing.Num(); i++ )
			{
				const FDisDialogRunningInstance* Inst = Firing(i).m_pConvComp ? Firing(i).m_pConvComp->GetRunningInst() : NULL;
				const FLOAT Weight = ( Inst && Inst->m_pCurPlayingConversation ) ? Inst->m_pCurPlayingConversation->CalcGlobalHookDominanceWeight() : 0.0f;
				if( Weight > MaxDominantWeight )
				{
					MaxDominantWeight = Weight;
					m_CurDominantHook.m_iGroupIndex = GroupIndex;
					m_CurDominantHook.m_GroupType = (BYTE)Type;
					m_CurDominantHook.m_iPlayingHookIndex = i;
				}
			}
		}
	}
	// DISHONORED(bringup): retail then ducks the non-dominant hooks' audio (DuckNonDominantAudio) and ticks the subtitles
	// (FDisSubtitlesInfo::Tick) with the dominant conversation - the voice and subtitle packages.
}
