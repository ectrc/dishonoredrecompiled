// DishonoredGame/src/disconversation_ingamedata.cpp
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas (2012 in parentheses; * = by vtable slot or caller):
//   UDisConversation_InGameData_Base::InitForConvNode_Derived (0x8e3640, 469), GetInGameDataForNode 0x88d070 (0x8de470, 96)
//   UDisConversation_InGameData::InitForConvNode_Derived 0x8961e0* (0x8f4c90, 534), AcquireSpeakers 0x8a3700* (0x8f4eb0, 879),
//     AcquireSpeakersCheckOnly 0x89e3b0 (0x8e8500, 89), BindAllSpeakers 0x89e2b0 (0x8e3820, 245), BindSpeaker 0x89c6c0
//     (0x8e1770, 772), GetSpeaker 0x88d180 (0x8de580, 98), IsSpeakerEngaged (0x8de5f0, 54), PlayerIsNearEnough 0x896410
//     (0x8e55d0, 485), MarkNodeLabelSeen (0x8f5220, 192), CheckForTickReroute 0x8a5610 (0x8e57c0, 95),
//     RerouteConversationOutput (0x8de660), CalcGlobalHookDominanceWeight 0x899f80 (0x8e8560, 273).
// The 2012 bodies test the conversation's bitfield by mask; the 2013 layout this tree declares inserted
// m_bAlwaysInterruptForCombat and m_bCanStartDuringEnemyAwareness at bits 8-9, so bits 0-7 are read by their 2013 names
// and 2012's 0x100 / 0x200 / 0x400 are m_bPlayerMustBeNear / m_bFireConvOnlyOnce / m_bNotWhenPlayerIsInCombat
// (confirmed on 2013's UpdateOutputLinks, 0x899e00, which tests 0x400 for the "Player Walked Away" output).
// Not ported (agentFR.md inventory): PostGameLoad 2012 0x8de4d0 (agent ER's save layer).
#include "DishonoredGame.h"
#include "frdialogdiag.h"

static IDisConvSpeakerInterface* DisSpeakerOf( const TScriptInterface<IDisConvSpeakerInterface>& Ref )
{
	return Ref.GetObject() ? (IDisConvSpeakerInterface*)Ref.GetInterface() : NULL;
}

static const UDisDialogTree* DisRootTree( const UDisConv_Node* Node )
{
	const UDisConv_Node* Root = Node;
	for( UDisConversation* Owner = Node->m_pOwner; Owner; Owner = Owner->m_pOwner )
	{
		Root = Owner;
	}
	return ConstCast<UDisDialogTree>( Root );
}

// ---------------------------------------------------------------------------------------------------------------------
// UDisConversation_InGameData_Base

void UDisConversation_InGameData_Base::InitForConvNode_Derived( const UDisConv_Node* _pNode )
{
	const UDisConversation* Conv = ConstCast<UDisConversation>( _pNode );
	if( !Conv )
	{
		return;
	}
	const INT NumNodes = Conv->m_Nodes.Num();
	m_Nodes_InGame.Empty( NumNodes );
	m_Nodes_InGame.AddZeroed( NumNodes );
	for( INT i = 0; i < NumNodes; i++ )
	{
		const UDisConv_Node* Node = Conv->m_Nodes(i);
		if( !Node )
		{
			continue;
		}
		FDisConvClassVars ClassVars;
		appMemzero( &ClassVars, sizeof(ClassVars) );
		Node->GetClassVars_Derived( ClassVars );
		UClass* DataClass = ClassVars.m_InGameDataClasses[m_NodeDataType];
		if( !DataClass )
		{
			continue;
		}
		if( Node == _pNode )
		{
			m_Nodes_InGame(i) = this;
		}
		else
		{
			UDisConv_Node_InGameData* Data = ConstructObject<UDisConv_Node_InGameData>( DataClass, this );
			m_Nodes_InGame(i) = Data;
			Data->InitForConvNode( Node );
		}
	}
}

UDisConv_Node_InGameData* UDisConversation_InGameData_Base::GetInGameDataForNode( const UDisConv_Node* _pNode ) const
{
	const INT Index = _pNode->m_iConvNodeIndex;
	if( Index >= 0 && Index < m_Nodes_InGame.Num() )
	{
		return m_Nodes_InGame( Index );
	}
	return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// UDisConversation_InGameData

void UDisConversation_InGameData::InitForConvNode_Derived( const UDisConv_Node* _pNode )
{
	UDisConversation_InGameData_Base::InitForConvNode_Derived( _pNode );
	const UDisConversation* Conv = ConstCast<UDisConversation>( _pNode );
	if( !Conv )
	{
		return;
	}
	const INT NumSpeakers = Conv->m_Speakers.Num();
	m_SpeakerBindings.Empty( NumSpeakers );
	m_SpeakerBindings.AddZeroed( NumSpeakers );
	for( INT i = 0; i < NumSpeakers; i++ )
	{
		m_SpeakerBindings(i).m_pSpeaker = (FPointer)&Conv->m_Speakers(i);
		m_SpeakerBindings(i).m_fMaxVoiceRadius = 0.0f;
	}
	for( INT i = 0; i < Conv->m_Nodes.Num(); i++ )
	{
		const UDisConv_Blurb* Blurb = ConstCast<UDisConv_Blurb>( Conv->m_Nodes(i) );
		if( Blurb && Blurb->m_iSpeaker >= 0 && Blurb->m_iSpeaker < m_SpeakerBindings.Num() )
		{
			FDisConvSpeaker_InGame& Binding = m_SpeakerBindings( Blurb->m_iSpeaker );
			if( Blurb->m_fVoiceRadius > Binding.m_fMaxVoiceRadius )
			{
				Binding.m_fMaxVoiceRadius = Blurb->m_fVoiceRadius;
			}
		}
	}
}

IDisConvSpeakerInterface* UDisConversation_InGameData::GetSpeaker( INT _iIndex ) const
{
	if( _iIndex >= 0 && _iIndex < m_SpeakerBindings.Num() )
	{
		return DisSpeakerOf( m_SpeakerBindings( _iIndex ).m_pBoundSpeaker );
	}
	return NULL;
}

UBOOL UDisConversation_InGameData::IsSpeakerEngaged( INT _iIndex ) const
{
	if( _iIndex < 0 || _iIndex >= m_SpeakerBindings.Num() )
	{
		return FALSE;
	}
	const FDisConvSpeaker_InGame& Binding = m_SpeakerBindings( _iIndex );
	return Binding.m_pBoundSpeaker.GetObject() && Binding.m_pBoundSpeaker.GetInterface() && Binding.m_bSpeakerEngaged;
}

UBOOL UDisConversation_InGameData::PlayerIsNearEnough( FDisDialogRunningInstance* _pRunningInstance ) const
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	if( !Conv || !Conv->m_bPlayerMustBeNear || !ADishonoredPlayerPawn::s_pInstance )
	{
		return TRUE;
	}
	IDisConvSpeakerInterface* Me = DisSpeakerOf( _pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me );
	if( !Me )
	{
		return TRUE;
	}
	TArray<FDisConvSpeakerLocInfo> LocInfo;
	Me->GetSpeakerLocInfo( LocInfo );
	const FVector PlayerLoc = ADishonoredPlayerPawn::s_pInstance->Location;
	const FLOAT RadiusSq = Conv->m_fPlayerNearRadius * Conv->m_fPlayerNearRadius;
	for( INT i = 0; i < LocInfo.Num(); i++ )
	{
		if( ( LocInfo(i).m_Location - PlayerLoc ).SizeSquared() < RadiusSq )
		{
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL UDisConversation_InGameData::BindSpeaker( FDisConvSpeaker_InGame& _rSpeaker, FDisDialogRunningInstance* _pRunningInstance, UBOOL _bMustBeAvailable )
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	const FDisConvSpeaker* Speaker = (const FDisConvSpeaker*)_rSpeaker.m_pSpeaker;
	UDisDialogTree_InGameBind* Binding = _pRunningInstance->m_pTreeGameBinding;
	_rSpeaker.m_bSpeakerEngaged = 0;
	_rSpeaker.m_SpeakerAvailability = DDA_IGNORED;
	if( !Conv || !Speaker )
	{
		return FALSE;
	}
	const UDisDialogTree* SpeakerTree = Speaker->m_pLinkedNode ? Speaker->m_pLinkedNode->GetSpeakerTree() : NULL;
	switch( Speaker->m_Type )
	{
	case 0:
		if( Speaker->m_pLinkedNode )
		{
			IDisConvSpeakerInterface* Bound = Speaker->m_pLinkedNode->BindSpeaker( _pRunningInstance );
			_rSpeaker.m_pBoundSpeaker.SetObject( Bound ? Bound->GetUObjectInterfaceDisConvSpeakerInterface() : NULL );
			_rSpeaker.m_pBoundSpeaker.SetInterface( _rSpeaker.m_pBoundSpeaker.GetObject() ? Bound : NULL );
		}
		else
		{
			_rSpeaker.m_pBoundSpeaker = Binding->m_pBoundSpeaker_Me;
		}
		break;
	case 1:
		if( Speaker->m_pLinkedNode )
		{
			IDisConvSpeakerInterface* Bound = Speaker->m_pLinkedNode->BindSpeaker( _pRunningInstance );
			_rSpeaker.m_pBoundSpeaker.SetObject( Bound ? Bound->GetUObjectInterfaceDisConvSpeakerInterface() : NULL );
			_rSpeaker.m_pBoundSpeaker.SetInterface( _rSpeaker.m_pBoundSpeaker.GetObject() ? Bound : NULL );
		}
		else
		{
			_rSpeaker.m_pBoundSpeaker = _pRunningInstance->m_pBoundSpeaker_Initiator;
		}
		break;
	case 2:
		_rSpeaker.m_pBoundSpeaker.SetObject( ADishonoredPlayerPawn::s_pInstance );
		_rSpeaker.m_pBoundSpeaker.SetInterface( ADishonoredPlayerPawn::s_pInstance ? (IDisConvSpeakerInterface*)ADishonoredPlayerPawn::s_pInstance : NULL );
		break;
	case 3:
		if( Speaker->m_pLinkedNode )
		{
			IDisConvSpeakerInterface* Bound = Speaker->m_pLinkedNode->BindSpeaker( _pRunningInstance );
			_rSpeaker.m_pBoundSpeaker.SetObject( Bound ? Bound->GetUObjectInterfaceDisConvSpeakerInterface() : NULL );
			_rSpeaker.m_pBoundSpeaker.SetInterface( _rSpeaker.m_pBoundSpeaker.GetObject() ? Bound : NULL );
		}
		break;
	default:
		break;
	}
	IDisConvSpeakerInterface* Bound = DisSpeakerOf( _rSpeaker.m_pBoundSpeaker );
	if( SpeakerTree && Bound && !Bound->SupportsDialogTree( SpeakerTree ) )
	{
		FRDIAG( "bind %s: speaker %s does not support tree %s", *GetName(), *_rSpeaker.m_pBoundSpeaker.GetObject()->GetName(), *SpeakerTree->GetName() );
		_rSpeaker.m_pBoundSpeaker.SetObject( NULL );
		_rSpeaker.m_pBoundSpeaker.SetInterface( NULL );
		Bound = NULL;
	}
	if( Bound )
	{
		const UBOOL bIsPlayer = _rSpeaker.m_pBoundSpeaker.GetObject()->IsA( ADishonoredPlayerPawn::StaticClass() );
		INT StartDialogFlags = 0;
		if( bIsPlayer ? Conv->m_bIsBlocking_Player : Conv->m_bIsBlocking )
		{
			StartDialogFlags |= 2;
		}
		if( !_pRunningInstance->HasDialogStarted() )
		{
			StartDialogFlags |= 4;
		}
		if( Conv->m_bAllowPlayerToCarryBody )
		{
			StartDialogFlags |= 1;
		}
		if( Conv->m_bStopBlockingForAINotice )
		{
			StartDialogFlags |= 8;
		}
		_rSpeaker.m_SpeakerAvailability = Binding->IsSpeakerAvailableForDialog( Bound, Conv->GetFiringPriority( _pRunningInstance ), (eDisDialogHook)_pRunningInstance->m_DialogHookThatFiredInst, StartDialogFlags );
		if( _rSpeaker.m_SpeakerAvailability == DDA_AVAILABLE )
		{
			for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
			{
				const FDisConvSpeaker_InGame& Other = m_SpeakerBindings(i);
				if( &Other != &_rSpeaker && Other.m_bSpeakerEngaged && Other.m_pBoundSpeaker == _rSpeaker.m_pBoundSpeaker )
				{
					_rSpeaker.m_SpeakerAvailability = DDA_ALREADY_ENGAGED_IN_THIS_CONVO;
					break;
				}
			}
		}
		if( _rSpeaker.m_SpeakerAvailability != DDA_AVAILABLE )
		{
			FRDIAG( "bind %s: speaker %s (type %d) availability %d with flags %d", *GetName(), *_rSpeaker.m_pBoundSpeaker.GetObject()->GetName(), (INT)Speaker->m_Type, (INT)_rSpeaker.m_SpeakerAvailability, StartDialogFlags );
		}
	}
	else
	{
		FRDIAG( "bind %s: speaker slot type %d (%s) bound nothing", *GetName(), (INT)Speaker->m_Type, Speaker->m_pLinkedNode ? *Speaker->m_pLinkedNode->GetName() : TEXT("no linked node") );
	}
	if( _bMustBeAvailable )
	{
		if( _rSpeaker.m_SpeakerAvailability != DDA_AVAILABLE )
		{
			_rSpeaker.m_pBoundSpeaker.SetObject( NULL );
			_rSpeaker.m_pBoundSpeaker.SetInterface( NULL );
		}
		else
		{
			_rSpeaker.m_bSpeakerEngaged = 1;
		}
	}
	return _rSpeaker.m_pBoundSpeaker.GetObject() && _rSpeaker.m_pBoundSpeaker.GetInterface();
}

UBOOL UDisConversation_InGameData::BindAllSpeakers( FDisDialogRunningInstance* _pRunningInstance, UBOOL _bMustBeAvailable )
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
	{
		FDisConvSpeaker_InGame& Binding = m_SpeakerBindings(i);
		const FDisConvSpeaker* Speaker = (const FDisConvSpeaker*)Binding.m_pSpeaker;
		if( !Speaker )
		{
			continue;
		}
		UBOOL bMustBeAvailable = _bMustBeAvailable;
		if( Speaker->m_Type == 2 )
		{
			bMustBeAvailable = _bMustBeAvailable && Conv && Conv->m_bIsBlocking_Player;
		}
		else if( Speaker->m_Type == 1 )
		{
			bMustBeAvailable = _bMustBeAvailable && Speaker->m_pLinkedNode != NULL;
		}
		else if( Speaker->m_Type == 3 && !Speaker->m_pLinkedNode )
		{
			continue;
		}
		if( !BindSpeaker( Binding, _pRunningInstance, bMustBeAvailable ) )
		{
			if( !Speaker->m_pLinkedNode || !Speaker->m_pLinkedNode->m_bOptional )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

UBOOL UDisConversation_InGameData::AcquireSpeakersCheckOnly( FDisDialogRunningInstance* _pRunningInstance )
{
	if( _pRunningInstance->m_pCurPlayingNode && _pRunningInstance->GetInGameConv( _pRunningInstance->m_pCurPlayingNode ) == this )
	{
		return FALSE;
	}
	if( !PlayerIsNearEnough( _pRunningInstance ) )
	{
		return FALSE;
	}
	return BindAllSpeakers( _pRunningInstance, TRUE );
}

UBOOL UDisConversation_InGameData::AcquireSpeakers( FDisDialogRunningInstance* _pRunningInstance )
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	const UDisDialogTree* Tree = DisRootTree( m_pOwnerConvNode );
	if( _pRunningInstance->m_pCurPlayingNode && _pRunningInstance->GetInGameConv( _pRunningInstance->m_pCurPlayingNode ) == this )
	{
		return FALSE;
	}
	if( !PlayerIsNearEnough( _pRunningInstance ) )
	{
		FRDIAG( "bind %s: the player is not near enough", *GetName() );
		return FALSE;
	}
	if( !BindAllSpeakers( _pRunningInstance, TRUE ) )
	{
		for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
		{
			m_SpeakerBindings(i).m_bSpeakerEngaged = 0;
		}
		return FALSE;
	}
	_pRunningInstance->m_CurPriority = (BYTE)Conv->GetFiringPriority( _pRunningInstance );
	const INT FiredDialogHook = _pRunningInstance->m_DialogHookThatFiredInst;
	for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
	{
		FDisConvSpeaker_InGame& Binding = m_SpeakerBindings(i);
		IDisConvSpeakerInterface* Speaker = DisSpeakerOf( Binding.m_pBoundSpeaker );
		if( !Binding.m_bSpeakerEngaged || !Speaker )
		{
			continue;
		}
		const UBOOL bIsPlayer = Binding.m_pBoundSpeaker.GetObject()->IsA( ADishonoredPlayerPawn::StaticClass() );
		const UBOOL bBlocking = bIsPlayer ? Conv->m_bIsBlocking_Player : Conv->m_bIsBlocking;
		INT StartDialogFlags = ( Conv->m_bAllowPlayerToCarryBody ? 1 : 0 ) | ( Conv->m_bStopBlockingForAINotice ? 8 : 0 );
		Speaker->StartDialog( _pRunningInstance, bBlocking, StartDialogFlags, FiredDialogHook );
		if( Tree != m_pOwnerConvNode )
		{
			if( FDisConvSaveData* SaveData = Speaker->GetDialogSaveData() )
			{
				SaveData->FindConvData( Tree, Conv, TRUE )->m_NumTimesFired++;
			}
		}
	}
	if( Tree != m_pOwnerConvNode && ADishonoredPlayerPawn::s_pInstance )
	{
		ADishonoredPlayerPawn::s_pInstance->GetDialogSaveData_Global()->FindConvData( Tree, Conv, TRUE )->m_NumTimesFired++;
	}
	if( UDisConversation_InGameData* Previous = _pRunningInstance->m_pCurPlayingConversation )
	{
		for( INT i = 0; i < Previous->m_SpeakerBindings.Num(); i++ )
		{
			FDisConvSpeaker_InGame& Old = Previous->m_SpeakerBindings(i);
			IDisConvSpeakerInterface* OldSpeaker = DisSpeakerOf( Old.m_pBoundSpeaker );
			if( !Old.m_bSpeakerEngaged || !OldSpeaker )
			{
				continue;
			}
			UBOOL bStillSpeaking = FALSE;
			for( INT j = 0; j < m_SpeakerBindings.Num() && !bStillSpeaking; j++ )
			{
				bStillSpeaking = m_SpeakerBindings(j).m_bSpeakerEngaged && m_SpeakerBindings(j).m_pBoundSpeaker == Old.m_pBoundSpeaker;
			}
			if( !bStillSpeaking )
			{
				OldSpeaker->EndDialog( _pRunningInstance, DisConvoEndReason_Finished );
				Old.m_bSpeakerEngaged = 0;
				Old.m_SpeakerAvailability = DDA_IGNORED;
				Old.m_pBoundSpeaker.SetObject( NULL );
				Old.m_pBoundSpeaker.SetInterface( NULL );
			}
		}
	}
	m_bDidPlayerWalkAwayCheck = 0;
	_pRunningInstance->StartDialog( this );
	return TRUE;
}

void UDisConversation_InGameData::MarkNodeLabelSeen( const UDisConv_Node* _pNode )
{
	const UDisDialogTree* Tree = DisRootTree( _pNode );
	const UDisConversation* Conv = _pNode->m_pOwner;
	for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
	{
		const FDisConvSpeaker_InGame& Binding = m_SpeakerBindings(i);
		IDisConvSpeakerInterface* Speaker = DisSpeakerOf( Binding.m_pBoundSpeaker );
		if( Speaker && Binding.m_bSpeakerEngaged )
		{
			if( FDisConvSaveData* SaveData = Speaker->GetDialogSaveData() )
			{
				SaveData->FindLabelData( Tree, Conv, _pNode->m_Label, TRUE )->m_NumTimesFired++;
			}
		}
	}
}

UBOOL UDisConversation_InGameData::RerouteConversationOutput( FDisDialogRunningInstance* _pRunningInstance, INT _OutputIndex )
{
	UDisConv_Node* ConvNode = m_pOwnerConvNode;
	if( _OutputIndex < 0 || _OutputIndex >= ConvNode->m_OutputLinks.Num() || !ConvNode->m_OutputLinks( _OutputIndex ).m_pLink )
	{
		return FALSE;
	}
	if( _pRunningInstance->m_pCurPlayingNode )
	{
		_pRunningInstance->m_pCurPlayingNode->InterruptTickingNode( _pRunningInstance, DisConvoEndReason_PlayerWalkedAway );
	}
	_pRunningInstance->m_pCurPlayingNode = NULL;
	DisConvoEndReasonEnum EndReason = DisConvoEndReason_MAX;
	_pRunningInstance->m_pTreeGameBinding->PlayFromNode( ConvNode->m_OutputLinks( _OutputIndex ).m_pLink, _pRunningInstance, EndReason, _OutputIndex );
	if( EndReason != DisConvoEndReason_MAX )
	{
		_pRunningInstance->StopInstance( EndReason );
	}
	return TRUE;
}

UBOOL UDisConversation_InGameData::CheckForTickReroute( FDisDialogRunningInstance* _pRunningInstance )
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	if( !Conv || Conv->m_bIsBlocking_Player || m_bDidPlayerWalkAwayCheck )
	{
		return FALSE;
	}
	if( PlayerIsNearEnough( _pRunningInstance ) )
	{
		return FALSE;
	}
	UBOOL bRerouted = RerouteConversationOutput( _pRunningInstance, 2 );
	if( !bRerouted )
	{
		_pRunningInstance->StopInstance( DisConvoEndReason_PlayerWalkedAway );
		bRerouted = TRUE;
	}
	m_bDidPlayerWalkAwayCheck = 1;
	return bRerouted;
}

FLOAT UDisConversation_InGameData::CalcGlobalHookDominanceWeight() const
{
	const UDisConversation* Conv = Cast<UDisConversation>( m_pOwnerConvNode );
	if( !Conv )
	{
		return 0.0f;
	}
	if( Conv->m_bIsBlocking_Player )
	{
		return 1.0f;
	}
	FLOAT Weight = 0.0f;
	if( Conv->m_bMakeOtherDialogQuiet && UDisConvGlobalMan::s_pInstance )
	{
		const UDisTweaks_ConvSettings* Settings = UDisConvGlobalMan::s_pInstance->GetConvSettings();
		for( INT i = 0; i < m_SpeakerBindings.Num(); i++ )
		{
			const FDisConvSpeaker_InGame& Binding = m_SpeakerBindings(i);
			if( !Binding.m_bSpeakerEngaged )
			{
				continue;
			}
			const FLOAT DominantDist = ( Settings ? Settings->m_fDominantMaxDistance_Percent : 1.0f ) * Binding.m_fMaxVoiceRadius;
			const FLOAT AudibleDist = UDisConvGlobalMan::GetShortestAudibleDistForSpeaker( DisSpeakerOf( Binding.m_pBoundSpeaker ), DominantDist );
			if( DominantDist >= AudibleDist && DominantDist > 0.0f && 1.0f - AudibleDist / DominantDist > Weight )
			{
				Weight = 1.0f - AudibleDist / DominantDist;
			}
		}
	}
	return Weight;
}
