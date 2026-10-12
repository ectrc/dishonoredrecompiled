// DishonoredGame/src/disconv_soiree.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The node that starts a scripted scene. 2013 rvas (2012 in parentheses; * = by
// vtable slot of ??_7UDisConv_Soiree@@6B@ at 0x119ed30, unnamed in the 2013 database):
//   PlayNode_Derived 0x8a7a60 (0x8f8ab0, 91)      PlayNodeMatinee 0x8b61f0* (0x904c10, 245)   PlayNodePreview 0x8acff0* (0x904a50, 444)
//   TickNode 0x8a7ac0 (0x8f8b10, 72)              TickNodeMatinee 0x8a95c0* (0x904750, 255)   TickNodePreview 0x8acea0* (0x904580, 459)
//   InterruptTickingNode 0x8a9640* (0x904850, 263) IsTickingNodeInterruptible 0x8a96c0* (0x904960, 233)
//   FindMatinee 0x8b5de0 (0x900d80, 328)           FindMatineeLinked (0x900ed0, 186)           GetClassVars_Derived 0x8ad130 (0x900210, 43)
// 2013 differences honoured: TickNodeMatinee tests USeqAct_Interp::m_bMatineeHasBeenRejected (bit 23 of the flag word at
// +400; 2012 tested bit 22) and ends the conversation with DisConvoEndReason_SoireeRejection (12; 11 in 2012's enum).
// The matinee's skip surface (USeqAct_Interp::OnSkipRequest 2013 0x223b80 311 B, IsSkippable 2012 0x22a8b0, StopFromNPC
// 2012 0x22a990 13 B) is agent FS's UnInterpolation.cpp; the two callers here use Stop() for StopFromNPC and name the
// other two. Not ported (editor): OnDelete_Derived 2013 0x8b6260, GetActionText 0x8b6000, FindBlurbs 2012 0x8fbdc0.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

UDisConv_Soiree_InGameData* UDisConv_Soiree::GetSoireeData( FDisDialogRunningInstance* _pRunningInstance ) const
{
	return Cast<UDisConv_Soiree_InGameData>( _pRunningInstance->GetInGameNode( this, eDisConvInGameDataType_PerNode ) );
}

void UDisConv_Soiree::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	UDisConv_Action::GetClassVars_Derived( _rVarsOut );
	_rVarsOut.m_bIsStartOfDialog = TRUE;
	_rVarsOut.m_InGameDataClasses[eDisConvInGameDataType_PerNode] = UDisConv_Soiree_InGameData::StaticClass();
}

INT UDisConv_Soiree::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	FRDIAG( "soiree %s: owner %s blocking %d blockingPlayer %d preview %d guid %s", *GetPathName(), m_pOwner ? *m_pOwner->GetName() : TEXT("<none>"),
		m_pOwner ? ( m_pOwner->m_bIsBlocking ? 1 : 0 ) : -1, m_pOwner ? ( m_pOwner->m_bIsBlocking_Player ? 1 : 0 ) : -1, m_UsePreviewBlurbsOnly ? 1 : 0, *m_SoireeGUID.String() );
	if( !m_pOwner || !m_pOwner->m_bIsBlocking )
	{
		FRDIAG( "soiree %s: owner conversation %s is not blocking, so the node passes straight through", *GetPathName(), m_pOwner ? *m_pOwner->GetName() : TEXT("<none>") );
		return 0;
	}
	return m_UsePreviewBlurbsOnly ? PlayNodePreview( _rbNotFinished, _pRunningInstance ) : PlayNodeMatinee( _rbNotFinished, _pRunningInstance );
}

INT UDisConv_Soiree::TickNode( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	return m_UsePreviewBlurbsOnly ? TickNodePreview( _fDeltaTime, _pRunningInstance ) : TickNodeMatinee( _fDeltaTime, _pRunningInstance );
}

INT UDisConv_Soiree::PlayNodeMatinee( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	if( !Data )
	{
		UDisConv_Node_InGameData* Raw = _pRunningInstance->GetInGameNode( this, eDisConvInGameDataType_PerNode );
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisConv_Soiree::PlayNodeMatinee: %s has no UDisConv_Soiree_InGameData (got %s, node index %d, owner %s index %d)"),
			*GetPathName(), Raw ? *Raw->GetClass()->GetName() : TEXT("NULL"), m_iConvNodeIndex, m_pOwner ? *m_pOwner->GetName() : TEXT("<none>"), m_pOwner ? m_pOwner->m_iConvNodeIndex : -2 );
		_rbNotFinished = FALSE;
		return 0;
	}
	USeqAct_Interp* Matinee = FindMatinee();
	if( Matinee )
	{
		Data->m_PlayingMatinee = Matinee;
		Data->m_PlayingNode = const_cast<UDisConv_Soiree*>( this );
		Matinee->StartFromDialog( _pRunningInstance, this, m_pOwner->m_bIsBlocking );
		FRDIAG( "soiree %s: StartFromDialog on %s (blocking %d, activations %d)", *GetPathName(), *Matinee->GetPathName(), m_pOwner->m_bIsBlocking ? 1 : 0, Matinee->ActivateCount );
	}
	else
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisConv_Soiree::PlayNodeMatinee: %s names no loaded USeqAct_Interp (guid %s)"), *GetPathName(), *m_SoireeGUID.String() );
	}
	_rbNotFinished = Data->m_PlayingMatinee != NULL;
	return Data->m_PlayingMatinee ? -1 : 0;
}

INT UDisConv_Soiree::TickNodeMatinee( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	USeqAct_Interp* Matinee = Data ? Data->m_PlayingMatinee : NULL;
	if( !Matinee || Matinee->bIsPlaying || Matinee->GetConversationNodePointer() != this )
	{
		return -1;
	}
	if( Matinee->m_bMatineeHasBeenRejected )
	{
		FRDIAG( "soiree %s: %s was rejected", *GetPathName(), *Matinee->GetName() );
		_pRunningInstance->m_PendingEndReason = DisConvoEndReason_SoireeRejection;
		return 0;
	}
	if( Matinee->Position <= 0.0f )
	{
		return -1;
	}
	FRDIAG( "soiree %s: %s finished at position %.2f", *GetPathName(), *Matinee->GetName(), Matinee->Position );
	Data->m_PlayingMatinee = NULL;
	Data->m_PlayingNode = NULL;
	return 0;
}

INT UDisConv_Soiree::PlayNodePreview( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	_rbNotFinished = FALSE;
	if( !Data || m_OutputLinks.Num() <= 1 || !m_OutputLinks(1).m_pLink )
	{
		return 0;
	}
	UDisConv_Node* Node = m_OutputLinks(1).m_pLink;
	Data->m_PlayingNode = Node;
	while( TRUE )
	{
		UBOOL bNotFinished = FALSE;
		const INT Output = Node->PlayNode( bNotFinished, _pRunningInstance );
		if( bNotFinished )
		{
			_rbNotFinished = TRUE;
			return -1;
		}
		if( Output < 0 || Output >= Node->m_OutputLinks.Num() || !Node->m_OutputLinks( Output ).m_pLink )
		{
			Data->m_PlayingNode = NULL;
			return 0;
		}
		Node = Node->m_OutputLinks( Output ).m_pLink;
		Data->m_PlayingNode = Node;
	}
}

INT UDisConv_Soiree::TickNodePreview( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	UDisConv_Node* Node = Data ? Data->m_PlayingNode : NULL;
	if( !Node )
	{
		return 0;
	}
	INT Output = Node->TickNode( _fDeltaTime, _pRunningInstance );
	if( Output < 0 )
	{
		return -1;
	}
	while( Output >= 0 && Output < Node->m_OutputLinks.Num() && Node->m_OutputLinks( Output ).m_pLink )
	{
		Node = Node->m_OutputLinks( Output ).m_pLink;
		Data->m_PlayingNode = Node;
		UBOOL bNotFinished = FALSE;
		Output = Node->PlayNode( bNotFinished, _pRunningInstance );
		if( bNotFinished )
		{
			return -1;
		}
	}
	Data->m_PlayingNode = NULL;
	return 0;
}

void UDisConv_Soiree::InterruptTickingNode( FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const
{
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	if( !Data )
	{
		return;
	}
	if( _InterruptReason == DisConvoEndReason_PlayerSkipWhole )
	{
		if( m_pOwner && m_pOwner->m_bIsBlocking_Player && Data->m_PlayingMatinee && Data->m_PlayingMatinee->bIsPlaying )
		{
			// DISHONORED(bringup): retail asks the matinee to skip (USeqAct_Interp::OnSkipRequest( TRUE ), 2013 rva 0x223b80) - agent FS's.
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisConv_Soiree::InterruptTickingNode: USeqAct_Interp::OnSkipRequest is not ported; %s keeps playing"), *Data->m_PlayingMatinee->GetName() );
		}
		return;
	}
	if( Data->m_PlayingMatinee )
	{
		if( Data->m_PlayingMatinee->bIsPlaying )
		{
			// DISHONORED(bringup): retail's USeqAct_Interp::StopFromNPC( 0, 0 ) (2012 rva 0x22a990, 13 B) - Stop() is the engine half of it.
			Data->m_PlayingMatinee->Stop();
		}
		Data->m_PlayingMatinee->ClearDialogConvNode();
		Data->m_PlayingMatinee = NULL;
	}
	Data->m_PlayingNode = NULL;
}

UBOOL UDisConv_Soiree::IsTickingNodeInterruptible( FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const
{
	if( _InterruptReason != DisConvoEndReason_PlayerSkipWhole || !m_pOwner || !m_pOwner->m_bIsBlocking_Player )
	{
		return FALSE;
	}
	UDisConv_Soiree_InGameData* Data = GetSoireeData( _pRunningInstance );
	USeqAct_Interp* Matinee = Data ? Data->m_PlayingMatinee : NULL;
	// DISHONORED(bringup): retail asks USeqAct_Interp::IsSkippable( TRUE ) (2012 rva 0x22a8b0); the flag is what it reads.
	return Matinee && Matinee->bIsPlaying && Matinee->m_bIsSkippable;
}

USeqAct_Interp* UDisConv_Soiree::FindMatinee() const
{
	if( !( m_SoireeGUID.A | m_SoireeGUID.B | m_SoireeGUID.C | m_SoireeGUID.D ) )
	{
		return NULL;
	}
	for( TObjectIterator<USeqAct_Interp> It; It; ++It )
	{
		USeqAct_Interp* Matinee = *It;
		if( Matinee->m_MatineeGUID == m_SoireeGUID )
		{
			if( !GIsEditor || !GIsGame || !Matinee->GetOutermost()->GetName().StartsWith( TEXT("UEDPIE") ) )
			{
				return Matinee;
			}
		}
	}
	return NULL;
}

void UDisConv_Soiree::FindMatineeLinked( TArray<USeqAct_Interp*>& _lMatinee ) const
{
	for( TObjectIterator<USeqAct_Interp> It; It; ++It )
	{
		USeqAct_Interp* Matinee = *It;
		if( Matinee->GetOutermost()->PackageFlags & PKG_PlayInEditor )
		{
			continue;
		}
		UInterpData* Data = Matinee->FindInterpDataFromVariable();
		if( Data && Data->m_Data && Data->m_Data->m_RunData.m_ConversationNodeGUID == m_NodeGUID )
		{
			_lMatinee.AddItem( Matinee );
		}
	}
}
