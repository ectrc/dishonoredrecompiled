// DishonoredGame/src/disdialogtree_ingamebind.cpp
// DISHONORED(port): agent FR (PHASE17 FR). One dialog tree bound to one speaker, and the two running instances it owns.
// 2013 rvas (2012 in parentheses; * = found through the caller, unnamed in the 2013 database):
//   UDisDialogTree_InGameBind: InitAndBind 0x8a0ba0 (0x90d940, 257), BuildFallbackChain (0x90a010, 627),
//     BuildSingletonData_Tree 0x8a0b10 (0x90d8b0, 144), BuildSingletonData_Conv (0x90d770, 305), EvaluateDialogHook 0x8a4690
//     (0x914940, 479), FireDialogHook 0x8a5150 (0x9145a0, 602), FireDialogHook_Helper 0x8a49e0* (0x90a5c0, 223), PlayFromNode
//     0x8a3c10* (0x905f40, 426), TickDialogTree 0x8a66d0 (0x90a6a0, 319), IsSpeakerAvailableForDialog 0x89aec0 (0x914800, 144),
//     ChangeAttentionForSpeaker 0x89a550 (0x902d20, 220), OnOtherActorTerminated 0x8a4620 (0x90a550, 97).
//   FDisDialogRunningInstance: StartInstance 0x88bda0 (0x8fae70, 84), StopInstance 0x8a4410* (0x90a290, 512), StartDialog
//     0x897100* (0x900b20, 166), GetRunningObjects 0x892380 (0x8faed0, 65), GetInGameNode 0x89a3a0* (0x902b80, 411),
//     GetInGameConv 0x89ae10 (0x905e80, 184), GetDialogTree (0x8faf20, 70), FiredFromHookName 0x89ada0 (0x914530, 112).
// 2013 differences honoured: the "no end reason" sentinel is DisConvoEndReason_MAX (14; 13 in 2012), the idle hook is
// DDH_INVALID (126; 122 in 2012), the table has 126 rows.
// Not ported (agentFR.md inventory): ExtractDialogHookText 0x8a4870 / _Helper (debug text), ExtractTextStartingAtNode
// 2012 0x90b3e0, GameSave / GameLoad (agent ED's, dissavegame.cpp).
#include "DishonoredGame.h"
#include "frdialogdiag.h"

static IDisConvSpeakerInterface* DisSpeakerOf( const TScriptInterface<IDisConvSpeakerInterface>& Ref )
{
	return Ref.GetObject() ? (IDisConvSpeakerInterface*)Ref.GetInterface() : NULL;
}

static FString DisSpeakerName( const TScriptInterface<IDisConvSpeakerInterface>& Ref )
{
	return Ref.GetObject() ? Ref.GetObject()->GetName() : FString( TEXT("<none>") );
}

// ---------------------------------------------------------------------------------------------------------------------
// FDisDialogRunningInstance

void FDisDialogRunningInstance::StartInstance( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator, INT _iHookSubIndex, UBOOL _bCheckOnly )
{
	UObject* InitiatorObject = _pInitiator ? _pInitiator->GetUObjectInterfaceDisConvSpeakerInterface() : NULL;
	m_DialogHookThatFiredInst = (BYTE)_Hook;
	m_CurPriority = NOT_IN_A_CONV;
	m_pBoundSpeaker_Initiator.SetObject( InitiatorObject );
	m_pBoundSpeaker_Initiator.SetInterface( InitiatorObject ? _pInitiator : NULL );
	m_bStopWhenDialogStarts = _bCheckOnly ? 1 : 0;
	m_iDialogHookBranchIndex = _iHookSubIndex;
	m_iCurFallbackDepth = 0;
	m_iCurDebugCapture = -1;
	m_CheckOnlyResult = eDisDialogCheckOnly_NotFound;
	m_PendingEndReason = DisConvoEndReason_MAX;
}

void FDisDialogRunningInstance::StartDialog( UDisConversation_InGameData* _pConv )
{
	m_pCurPlayingConversation = _pConv;
	INT MyIndex = -1;
	for( INT i = 0; i < 2; i++ )
	{
		if( &m_pTreeGameBinding->m_RunningInstances[i] == this )
		{
			MyIndex = i;
		}
	}
	m_pTreeGameBinding->m_iActiveRunningInstance = MyIndex;
	for( INT i = 0; i < m_NodesToNotifyDialogStarted.Num(); i++ )
	{
		m_NodesToNotifyDialogStarted(i)->OnDialogStarted( this );
	}
	m_NodesToNotifyDialogStarted.Empty();
	FRDIAG( "bind %s: dialog started on %s (instance %d)", *m_pTreeGameBinding->GetPathName(), _pConv ? *_pConv->GetName() : TEXT("<none>"), MyIndex );
}

void FDisDialogRunningInstance::StopInstance( DisConvoEndReasonEnum _ConvoEndReason )
{
	if( m_pCurPlayingConversation )
	{
		FRDIAG( "bind %s: instance stops (reason %d) on %s", *m_pTreeGameBinding->GetPathName(), (INT)_ConvoEndReason, *m_pCurPlayingConversation->GetName() );
		if( _ConvoEndReason != DisConvoEndReason_Finished && m_pCurPlayingNode )
		{
			m_pCurPlayingNode->InterruptTickingNode( this, _ConvoEndReason );
		}
		TArrayNoInit<FDisConvSpeaker_InGame>& Bindings = m_pCurPlayingConversation->m_SpeakerBindings;
		for( INT i = 0; i < Bindings.Num(); i++ )
		{
			FDisConvSpeaker_InGame& Binding = Bindings(i);
			if( Binding.m_bSpeakerEngaged )
			{
				DisConvoEndReasonEnum ActualReason = _ConvoEndReason;
				if( _ConvoEndReason == DisConvoEndReason_Death && !( Binding.m_pBoundSpeaker == m_pTreeGameBinding->m_pBoundSpeaker_Me ) )
				{
					ActualReason = DisConvoEndReason_DeathWitness;
				}
				if( IDisConvSpeakerInterface* Speaker = DisSpeakerOf( Binding.m_pBoundSpeaker ) )
				{
					Speaker->EndDialog( this, ActualReason );
				}
			}
		}
		if( _ConvoEndReason != DisConvoEndReason_Finished )
		{
			UDisConv_Node* ConvNode = m_pCurPlayingConversation->m_pOwnerConvNode;
			if( ConvNode && ConvNode->m_OutputLinks.Num() > 1 && ConvNode->m_OutputLinks(1).m_pLink )
			{
				DisConvoEndReasonEnum Reason = _ConvoEndReason;
				m_pTreeGameBinding->PlayFromNode( ConvNode->m_OutputLinks(1).m_pLink, this, Reason, 1 );
			}
		}
		for( INT i = 0; i < Bindings.Num(); i++ )
		{
			FDisConvSpeaker_InGame& Binding = Bindings(i);
			Binding.m_bSpeakerEngaged = 0;
			Binding.m_SpeakerAvailability = DDA_IGNORED;
			Binding.m_pBoundSpeaker.SetObject( NULL );
			Binding.m_pBoundSpeaker.SetInterface( NULL );
		}
	}
	m_bStopWhenDialogStarts = 0;
	m_pCurPlayingNode = NULL;
	m_pCurPlayingConversation = NULL;
	m_iCurFallbackDepth = 0;
	m_DialogHookThatFiredInst = DDH_INVALID;
	m_iDialogHookBranchIndex = -1;
	m_CheckOnlyResult = eDisDialogCheckOnly_NotFound;
	m_pBoundSpeaker_Initiator.SetObject( NULL );
	m_pBoundSpeaker_Initiator.SetInterface( NULL );
	m_NodesToNotifyDialogStarted.Empty();
	if( m_pTreeGameBinding->m_iActiveRunningInstance != -1 && &m_pTreeGameBinding->m_RunningInstances[m_pTreeGameBinding->m_iActiveRunningInstance] == this )
	{
		m_pTreeGameBinding->m_iActiveRunningInstance = -1;
	}
}

FDisDialogRunningObjs& FDisDialogRunningInstance::GetRunningObjects()
{
	return m_FallbackChain( m_iCurFallbackDepth );
}

const UDisDialogTree* FDisDialogRunningInstance::GetDialogTree() const
{
	const FDisDialogRunningObjs& Objs = m_FallbackChain( m_iCurFallbackDepth );
	return Objs.m_pMainDialogTree ? Cast<UDisDialogTree>( Objs.m_pMainDialogTree->m_pOwnerConvNode ) : NULL;
}

UDisConv_Node_InGameData* FDisDialogRunningInstance::GetInGameNode( const UDisConv_Node* _pNode, eDisConvInGameDataType _DataType )
{
	FDisDialogRunningObjs* Objs = NULL;
	if( _DataType == eDisConvInGameDataType_PerNode )
	{
		Objs = &GetRunningObjects();
	}
	else if( _DataType == eDisConvInGameDataType_PerNode_Persistent )
	{
		Objs = &m_pTreeGameBinding->m_FallbackChain_Persist( m_iCurFallbackDepth );
	}
	else if( _DataType == eDisConvInGameDataType_Singleton )
	{
		FDisConvClassVars ClassVars;
		_pNode->GetClassVars( ClassVars );
		const INT* Index = m_pTreeGameBinding->m_Nodes_Singleton_InGameData_Lookup.Find( ClassVars.m_InGameDataClasses[eDisConvInGameDataType_Singleton] );
		return m_pTreeGameBinding->m_Nodes_Singleton_InGameData( Index ? *Index : 0 );
	}
	if( const UDisConversation* Conv = ConstCast<UDisConversation>( _pNode ) )
	{
		if( Conv->m_iConvNodeIndex == -1 )
		{
			return Objs->m_pMainDialogTree;
		}
		return Objs->Conversations()( Conv->m_iConvNodeIndex );
	}
	UDisConversation_InGameData_Base* Parent = Cast<UDisConversation_InGameData_Base>( GetInGameNode( _pNode->m_pOwner, _DataType ) );
	return Parent ? Parent->GetInGameDataForNode( _pNode ) : NULL;
}

UDisConversation_InGameData* FDisDialogRunningInstance::GetInGameConv( const UDisConv_Node* _pNode )
{
	const UDisConversation* Owner = _pNode->m_pOwner;
	if( !Owner )
	{
		return NULL;
	}
	FDisDialogRunningObjs& Objs = GetRunningObjects();
	if( Owner->m_iConvNodeIndex == -1 )
	{
		return Cast<UDisConversation_InGameData>( Objs.m_pMainDialogTree );
	}
	return Objs.Conversations()( Owner->m_iConvNodeIndex );
}

void FDisDialogRunningInstance::FiredFromHookName( FString& _rNameOut ) const
{
	_rNameOut = UDisDialogTree::GetDialogHookInfo()( m_DialogHookThatFiredInst ).m_DisplayName;
}

// ---------------------------------------------------------------------------------------------------------------------
// UDisDialogTree_InGameBind

static void BuildFallbackChain( UDisDialogTree_InGameBind* _pTreeBind, const UDisDialogTree* _pTree, TArrayNoInit<FDisDialogRunningObjs>& _rFallbacks, UClass* _pConvInGameClass, eDisConvInGameDataType _DataType )
{
	INT NumTrees = 0;
	for( const UDisDialogTree* Tree = _pTree; Tree; Tree = Tree->m_pFallbackTree )
	{
		NumTrees++;
	}
	_rFallbacks.Empty( NumTrees );
	_rFallbacks.AddZeroed( NumTrees );
	INT Cur = 0;
	for( const UDisDialogTree* Tree = _pTree; Tree; Tree = Tree->m_pFallbackTree, Cur++ )
	{
		FDisDialogRunningObjs& Objs = _rFallbacks( Cur );
		Objs.m_pMainDialogTree = (UDisConversation_InGameData_Base*)ConstructObject<UDisConv_Node_InGameData>( _pConvInGameClass, _pTreeBind );
		Objs.m_pMainDialogTree->InitForConvNode( Tree );
		TArrayNoInit<UDisConversation_InGameData*>& Convs = Objs.Conversations();
		Convs.Empty( Tree->m_Conversations.Num() );
		Convs.AddZeroed( Tree->m_Conversations.Num() );
		for( INT i = 0; i < Tree->m_Conversations.Num(); i++ )
		{
			const UDisConversation* Conv = Tree->m_Conversations(i);
			FDisConvClassVars ClassVars;
			Conv->GetClassVars( ClassVars );
			UDisConv_Node_InGameData* Data = ConstructObject<UDisConv_Node_InGameData>( ClassVars.m_InGameDataClasses[_DataType], _pTreeBind );
			Convs(i) = (UDisConversation_InGameData*)Data;
			Data->InitForConvNode( Conv );
		}
	}
}

static void BuildSingletonData_Conv( UDisDialogTree_InGameBind* _pInGameBind, const UDisConversation* _pConv )
{
	for( INT i = 0; i < _pConv->m_Nodes.Num(); i++ )
	{
		const UDisConv_Node* Node = _pConv->m_Nodes(i);
		if( !Node )
		{
			continue;
		}
		FDisConvClassVars ClassVars;
		Node->GetClassVars( ClassVars );
		UClass* SingletonClass = ClassVars.m_InGameDataClasses[eDisConvInGameDataType_Singleton];
		if( SingletonClass && !_pInGameBind->m_Nodes_Singleton_InGameData_Lookup.Find( SingletonClass ) )
		{
			UDisConv_Node_InGameData* Data = ConstructObject<UDisConv_Node_InGameData>( SingletonClass, _pInGameBind );
			Data->InitForConvNode( Node );
			_pInGameBind->m_Nodes_Singleton_InGameData.AddItem( Data );
			_pInGameBind->m_Nodes_Singleton_InGameData_Lookup.Set( SingletonClass, _pInGameBind->m_Nodes_Singleton_InGameData.Num() - 1 );
		}
	}
}

static void BuildSingletonData_Tree( UDisDialogTree_InGameBind* _pInGameBind, const UDisDialogTree* _pTree )
{
	for( const UDisDialogTree* Tree = _pTree; Tree; Tree = Tree->m_pFallbackTree )
	{
		BuildSingletonData_Conv( _pInGameBind, Tree );
		for( INT i = 0; i < Tree->m_Conversations.Num(); i++ )
		{
			if( Tree->m_Conversations(i) )
			{
				BuildSingletonData_Conv( _pInGameBind, Tree->m_Conversations(i) );
			}
		}
	}
}

void UDisDialogTree_InGameBind::InitAndBind( const UDisDialogTree* _pTree, IDisConvSpeakerInterface* _pMe )
{
	m_pDialogTree = const_cast<UDisDialogTree*>( _pTree );
	UObject* MeObject = _pMe ? _pMe->GetUObjectInterfaceDisConvSpeakerInterface() : NULL;
	IDisConvSpeakerInterface* Me = MeObject ? (IDisConvSpeakerInterface*)MeObject->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() ) : NULL;
	m_pBoundSpeaker_Me.SetObject( Me ? MeObject : NULL );
	m_pBoundSpeaker_Me.SetInterface( Me );
	m_iActiveRunningInstance = -1;
	for( INT i = 0; i < 2; i++ )
	{
		FDisDialogRunningInstance& Inst = m_RunningInstances[i];
		Inst.m_pTreeGameBinding = this;
		Inst.m_bStopWhenDialogStarts = 0;
		Inst.m_iCurFallbackDepth = 0;
		Inst.m_DialogHookThatFiredInst = DDH_INVALID;
		Inst.m_iDialogHookBranchIndex = -1;
		Inst.m_pCurPlayingNode = NULL;
		Inst.m_pCurPlayingConversation = NULL;
		Inst.m_CheckOnlyResult = eDisDialogCheckOnly_NotFound;
		Inst.m_pBoundSpeaker_Initiator.SetObject( NULL );
		Inst.m_pBoundSpeaker_Initiator.SetInterface( NULL );
		BuildFallbackChain( this, m_pDialogTree, Inst.m_FallbackChain, UDisConversation_InGameData::StaticClass(), eDisConvInGameDataType_PerNode );
	}
	BuildFallbackChain( this, _pTree, m_FallbackChain_Persist, UDisConversation_InGameData_Base::StaticClass(), eDisConvInGameDataType_PerNode_Persistent );
	BuildSingletonData_Tree( this, _pTree );
	FRDIAG( "spawned binding %s: tree %s bound to %s (%d conversation(s), %d hook slot(s))", *GetName(), *_pTree->GetPathName(), *DisSpeakerName( m_pBoundSpeaker_Me ),
		_pTree->m_Conversations.Num(), _pTree->m_DialogHooks.Num() );
}

UBOOL UDisDialogTree_InGameBind::EvaluateDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator, INT& _rHookSubIndex, UBOOL _bSkipAvailabilityTest )
{
	FDisDialogRunningInstance& Inst = m_RunningInstances[( m_iActiveRunningInstance + 1 ) % 2];
	_rHookSubIndex = -1;
	Inst.StartInstance( _Hook, _pInitiator, -1, FALSE );
	UBOOL bFoundNode = FALSE;
	for( const UDisDialogTree* Tree = m_pDialogTree; Tree; Tree = Tree->m_pFallbackTree, Inst.m_iCurFallbackDepth++ )
	{
		UDisConv_DialogHook* HookNode = ( _Hook >= 0 && _Hook < Tree->m_DialogHooks.Num() ) ? Tree->m_DialogHooks( _Hook ) : NULL;
		if( !HookNode )
		{
			continue;
		}
		bFoundNode = TRUE;
		const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _Hook );
		const UBOOL bClassMatches = Info.m_pHookClass ? ( Info.m_pHookClass == HookNode->GetClass() ) : ( HookNode->GetClass() == UDisConv_DialogHook::StaticClass() );
		if( bClassMatches )
		{
			const INT Branch = HookNode->EvaluateForFiring( &Inst );
			if( Branch != -1 )
			{
				if( _bSkipAvailabilityTest || IsSpeakerAvailableForDialog( DisSpeakerOf( m_pBoundSpeaker_Me ), NOT_INTERRUPTABLE_CONV, (eDisDialogHook)Inst.m_DialogHookThatFiredInst, 4 ) == DDA_AVAILABLE )
				{
					if( Branch >= 0 && Branch < HookNode->m_OutputLinks.Num() )
					{
						_rHookSubIndex = Branch;
					}
				}
				else
				{
					FRDIAG( "bind %s: hook %d found but %s is not available for dialog", *GetName(), (INT)_Hook, *DisSpeakerName( m_pBoundSpeaker_Me ) );
				}
			}
		}
		break;
	}
	Inst.StopInstance( DisConvoEndReason_Finished );
	return bFoundNode;
}

UBOOL UDisDialogTree_InGameBind::FireDialogHook_Helper( eDisDialogHook _Hook, INT _iHookSubIndex, FDisDialogRunningInstance* _pRunningInstance, UBOOL& _bHookFiredNoContent, DisConvoEndReasonEnum& _rEndReason, UBOOL _bFirstTreeTried )
{
	UDisConversation_InGameData* ConvInGame = Cast<UDisConversation_InGameData>( _pRunningInstance->m_FallbackChain( _pRunningInstance->m_iCurFallbackDepth ).m_pMainDialogTree );
	UDisDialogTree* Tree = ConvInGame ? Cast<UDisDialogTree>( ConvInGame->m_pOwnerConvNode ) : NULL;
	UDisConv_DialogHook* HookNode = ( Tree && _Hook >= 0 && _Hook < Tree->m_DialogHooks.Num() ) ? Tree->m_DialogHooks( _Hook ) : NULL;
	if( HookNode )
	{
		_bHookFiredNoContent = TRUE;
		ConvInGame->BindAllSpeakers( _pRunningInstance, FALSE );
		if( PlayFromNode( HookNode, _pRunningInstance, _rEndReason, -1 ) )
		{
			return TRUE;
		}
	}
	return _pRunningInstance->IsCheckOnly() && _pRunningInstance->m_CheckOnlyResult != eDisDialogCheckOnly_NotFound;
}

UBOOL UDisDialogTree_InGameBind::FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator, INT _iHookSubIndex, UBOOL& _bHookFiredNoContent, eDisDialogCheckOnly* _pCheckOnly_ResultOut )
{
	FDisDialogRunningInstance& Inst = m_RunningInstances[( m_iActiveRunningInstance + 1 ) % 2];
	if( _pCheckOnly_ResultOut )
	{
		*_pCheckOnly_ResultOut = eDisDialogCheckOnly_NotFound;
	}
	Inst.StartInstance( _Hook, _pInitiator, _iHookSubIndex, _pCheckOnly_ResultOut != NULL );
	DisConvoEndReasonEnum EndReason = DisConvoEndReason_Finished;
	UBOOL bFirstTried = TRUE;
	UBOOL bRet = FALSE;
	UBOOL bFound = FALSE;
	for( const UDisDialogTree* Tree = m_pDialogTree; Tree; Tree = Tree->m_pFallbackTree, Inst.m_iCurFallbackDepth++ )
	{
		if( _Hook >= 0 && _Hook < Tree->m_DialogHooks.Num() && Tree->m_DialogHooks( _Hook ) )
		{
			Inst.m_bTryFallbackTree = 0;
			if( FireDialogHook_Helper( _Hook, _iHookSubIndex, &Inst, _bHookFiredNoContent, EndReason, bFirstTried ) )
			{
				bFound = TRUE;
				break;
			}
			if( _bHookFiredNoContent && !Inst.m_bTryFallbackTree )
			{
				break;
			}
			bFirstTried = FALSE;
		}
	}
	if( !bFound )
	{
		if( _bHookFiredNoContent && Inst.m_bTryFallbackTree )
		{
			_bHookFiredNoContent = FALSE;
		}
	}
	else if( _pCheckOnly_ResultOut )
	{
		*_pCheckOnly_ResultOut = (eDisDialogCheckOnly)Inst.m_CheckOnlyResult;
		bRet = TRUE;
	}
	else
	{
		const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _Hook );
		bRet = TRUE;
		if( ( Info.m_Flags & 0x4000 ) && !( Inst.m_pBoundSpeaker_Initiator == m_pBoundSpeaker_Me ) )
		{
			m_fRemainingInitatorBustedTimer = Info.m_fBustTimeWhenFired;
			ChangeAttentionForSpeaker( DisSpeakerOf( m_pBoundSpeaker_Me ), DisSpeakerOf( Inst.m_pBoundSpeaker_Initiator ), DAL_Busted );
		}
		else
		{
			m_fRemainingInitatorBustedTimer = 0.0f;
		}
	}
	FRDIAG( "bind %s: FireDialogHook hook %d branch %d on %s -> %s, end reason %d%s", *GetName(), (INT)_Hook, _iHookSubIndex, *DisSpeakerName( m_pBoundSpeaker_Me ),
		bRet ? TEXT("fired") : TEXT("not fired"), (INT)EndReason, EndReason == DisConvoEndReason_MAX ? TEXT(" (still running)") : TEXT("") );
	if( EndReason != DisConvoEndReason_MAX )
	{
		Inst.StopInstance( EndReason );
	}
	return bRet;
}

UBOOL UDisDialogTree_InGameBind::PlayFromNode( const UDisConv_Node* _pNode, FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum& _rEndReason, INT _iLastKnownOutputIndex )
{
	_rEndReason = DisConvoEndReason_MAX;
	UBOOL bRet = _pRunningInstance->m_pCurPlayingConversation != NULL;
	const UDisConv_Node* Node = _pNode;
	UBOOL bNotFinished = FALSE;
	while( TRUE )
	{
		if( !_pRunningInstance->m_pCurPlayingConversation )
		{
			FDisConvClassVars ClassVars;
			Node->GetClassVars( ClassVars );
			if( ClassVars.m_bIsStartOfDialog )
			{
				UDisConversation_InGameData* Conv = _pRunningInstance->GetInGameConv( Node );
				if( !Conv )
				{
					_rEndReason = DisConvoEndReason_Error;
					return bRet;
				}
				if( _pRunningInstance->IsCheckOnly() )
				{
					_pRunningInstance->m_CheckOnlyResult = Conv->AcquireSpeakersCheckOnly( _pRunningInstance ) ? eDisDialogCheckOnly_Found : eDisDialogCheckOnly_Found_But_Busy;
					return bRet;
				}
				if( !Conv->AcquireSpeakers( _pRunningInstance ) )
				{
					FRDIAG( "bind %s: %s could not acquire its speakers at %s", *GetName(), *Conv->GetName(), *Node->GetName() );
					return bRet;
				}
			}
		}
		bRet = TRUE;
		bNotFinished = FALSE;
		const INT Output = Node->PlayNode( bNotFinished, _pRunningInstance );
		FRDIAG( "bind %s: played %s (%s) -> output %d%s", *GetName(), *Node->GetName(), *Node->GetClass()->GetName(), Output, bNotFinished ? TEXT(", ticking") : TEXT("") );
		if( Node->m_Label != NAME_None && !Node->IsA( UDisConversation::StaticClass() ) )
		{
			if( UDisConversation_InGameData* Conv = _pRunningInstance->GetInGameConv( Node ) )
			{
				Conv->MarkNodeLabelSeen( Node );
			}
		}
		if( _pRunningInstance->m_PendingEndReason != DisConvoEndReason_MAX )
		{
			_rEndReason = (DisConvoEndReasonEnum)_pRunningInstance->m_PendingEndReason;
			_pRunningInstance->m_PendingEndReason = DisConvoEndReason_MAX;
			return bRet;
		}
		if( Output == -1 )
		{
			break;
		}
		if( Output < 0 || Output >= Node->m_OutputLinks.Num() )
		{
			_rEndReason = DisConvoEndReason_Error;
			return bRet;
		}
		Node = Node->m_OutputLinks( Output ).m_pLink;
		if( !Node )
		{
			_rEndReason = DisConvoEndReason_Finished;
			return bRet;
		}
	}
	if( bNotFinished )
	{
		_pRunningInstance->m_pCurPlayingNode = const_cast<UDisConv_Node*>( Node );
	}
	else
	{
		_rEndReason = DisConvoEndReason_Finished;
	}
	return bRet;
}

void UDisDialogTree_InGameBind::TickDialogTree( FLOAT _fDeltaSeconds )
{
	if( m_iActiveRunningInstance == -1 )
	{
		return;
	}
	FDisDialogRunningInstance& Inst = m_RunningInstances[m_iActiveRunningInstance];
	if( !Inst.m_pCurPlayingNode )
	{
		return;
	}
	if( m_fRemainingInitatorBustedTimer > 0.0f )
	{
		m_fRemainingInitatorBustedTimer -= _fDeltaSeconds;
		ChangeAttentionForSpeaker( DisSpeakerOf( m_pBoundSpeaker_Me ), DisSpeakerOf( Inst.m_pBoundSpeaker_Initiator ), DAL_Busted );
	}
	if( Inst.m_pCurPlayingConversation && Inst.m_pCurPlayingConversation->CheckForTickReroute( &Inst ) )
	{
		return;
	}
	const INT Output = Inst.m_pCurPlayingNode->TickNode( _fDeltaSeconds, &Inst );
	if( Output == -1 )
	{
		return;
	}
	UDisConv_Node* Node = Inst.m_pCurPlayingNode;
	Inst.m_pCurPlayingNode = NULL;
	if( Output < 0 || Output >= Node->m_OutputLinks.Num() )
	{
		Inst.StopInstance( DisConvoEndReason_Error );
		return;
	}
	UDisConv_Node* Next = Node->m_OutputLinks( Output ).m_pLink;
	if( !Next )
	{
		Inst.StopInstance( DisConvoEndReason_Finished );
		return;
	}
	DisConvoEndReasonEnum EndReason = DisConvoEndReason_MAX;
	PlayFromNode( Next, &Inst, EndReason, Output );
	if( EndReason != DisConvoEndReason_MAX )
	{
		Inst.StopInstance( EndReason );
	}
}

eDisDialogAvailability UDisDialogTree_InGameBind::IsSpeakerAvailableForDialog( const IDisConvSpeakerInterface* _pSpeaker, eDisConvPriority _Priority, eDisDialogHook _FromHook, INT _StartDialogFlags ) const
{
	if( !_pSpeaker )
	{
		return DDA_IGNORED;
	}
	const eDisDialogAvailability Availability = _pSpeaker->IsAvailableForDialog( _Priority, _StartDialogFlags );
	const FDisDialogHookInfo& Info = UDisDialogTree::GetDialogHookInfo()( _FromHook );
	if( Availability == DDA_IN_NON_INTERRUPTABLE_CONVO || ( Info.m_Flags & 2 ) == 0 )
	{
		return Availability;
	}
	return DDA_AVAILABLE;
}

// DISHONORED(bringup): retail raises FAIStimStruct_DialogAttentionChange( target, threshold ) on the speaker's brain through
// UDishonoredAIBrain::HandleAIStim (0x89a550). The stim struct is declared and the brain's HandleAIStim template exists, but
// the attention stim's handler is the AI attention package's; the call is named here and made once that lands.
void UDisDialogTree_InGameBind::ChangeAttentionForSpeaker( IDisConvSpeakerInterface* _pSpeakerWithBrain, IDisConvSpeakerInterface* _pTargetSpeaker, EDisAttentionLevel _ToThreshold )
{
	if( !_pSpeakerWithBrain || !_pTargetSpeaker )
	{
		return;
	}
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisDialogTree_InGameBind::ChangeAttentionForSpeaker: the dialog attention stim is not ported (agent FR, inventory)") );
	}
}

void UDisDialogTree_InGameBind::OnOtherActorTerminated( const AActor& _rOther )
{
	IDisConvSpeakerInterface* Speaker = (IDisConvSpeakerInterface*)const_cast<AActor&>( _rOther ).GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() );
	if( !Speaker )
	{
		return;
	}
	for( INT i = 0; i < 2; i++ )
	{
		if( DisSpeakerOf( m_RunningInstances[i].m_pBoundSpeaker_Initiator ) == Speaker )
		{
			m_RunningInstances[i].StopInstance( DisConvoEndReason_Destroyed );
		}
	}
}
