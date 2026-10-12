/*=============================================================================
	frdialogdiag.cpp: agent FR (PHASE17 FR) - the -frdiag trace of the conversation system.

	  frdiag inputs    - a DisSeqAct_DialogInputs activating: its targets, their speaker interface, the initiator
	  frdiag hook      - UDisConversationComponent::FireDialogHook / FireDialogHookNow decisions
	  frdiag queue     - UDisConvGlobalMan queueing and firing
	  frdiag bind      - UDisDialogTree_InGameBind::FireDialogHook / PlayFromNode / speaker acquisition
	  frdiag soiree    - UDisConv_Soiree finding and starting its matinee, and finishing it
	  frdiag oneshot   - ADisDialogOneShot handing its voice data to its speakers' owners
	  frdiag spawned   - a pawn's component instancing its one-shot trees
	  frdiag census    - at every -fkdiag pass: the one-shot actors and the components holding their trees
=============================================================================*/
#include "DishonoredGame.h"
#include "frdialogdiag.h"

static INT GDisFRDiag = -1;

UBOOL DisFRDiagEnabled()
{
	if( GDisFRDiag < 0 )
	{
		GDisFRDiag = ( appStrfind( appCmdLine(), TEXT("-frdiag") ) != NULL ) ? 1 : 0;
	}
	return GDisFRDiag != 0;
}

static const TCHAR* DisFRSpeakerName( const FDisOneOffSpeaker& Speaker )
{
	return Speaker.m_pAttachedActor ? *Speaker.m_pAttachedActor->GetName() : ( Speaker.m_pSpawner ? *Speaker.m_pSpawner->GetName() : TEXT("<none>") );
}

void DisFRReportDialog()
{
	if( !DisFRDiagEnabled() || GWorld == NULL )
	{
		return;
	}
	INT OneShots = 0, Components = 0, Trees = 0, Running = 0;
	for( TObjectIterator<ADisDialogOneShot> It; It; ++It )
	{
		ADisDialogOneShot* OneShot = *It;
		if( OneShot->IsTemplate() )
		{
			continue;
		}
		OneShots++;
		debugf( TEXT("DISHONORED(bringup): frdiag census oneshot %s tree %s notified %d me %s (owner %s) others %d"),
			*OneShot->GetPathName(), OneShot->m_pDialogTree ? *OneShot->m_pDialogTree->GetName() : TEXT("<none>"), OneShot->m_bNotifiedSpeakers ? 1 : 0,
			DisFRSpeakerName( OneShot->m_MeSpeaker ), OneShot->GetOwnerForSpeaker( OneShot->m_MeSpeaker ) ? TEXT("yes") : TEXT("no"), OneShot->m_OtherSpeakers.Num() );
		for( INT i = 0; i < OneShot->m_OtherSpeakers.Num(); i++ )
		{
			debugf( TEXT("DISHONORED(bringup): frdiag census    other %d %s (owner %s)"), i, DisFRSpeakerName( OneShot->m_OtherSpeakers(i) ),
				OneShot->GetOwnerForSpeaker( OneShot->m_OtherSpeakers(i) ) ? TEXT("yes") : TEXT("no") );
		}
	}
	for( TObjectIterator<UDisConversationComponent> It; It; ++It )
	{
		UDisConversationComponent* Comp = *It;
		if( Comp->IsTemplate() || !Comp->IsAttached() )
		{
			continue;
		}
		Components++;
		INT MyTrees = ( Comp->m_pGenericDialogTree ? 1 : 0 ) + Comp->m_AdditionalGenericDialogTrees.Num();
		for( INT i = 0; i < Comp->m_OneShotInfo.Num(); i++ )
		{
			MyTrees += Comp->m_OneShotInfo(i).m_pOneShot_DialogTree ? 1 : 0;
		}
		Trees += MyTrees;
		FDisDialogRunningInstance* Inst = Comp->GetRunningInst();
		Running += ( Inst && Inst->m_pCurPlayingConversation ) ? 1 : 0;
		if( MyTrees > 0 || Inst )
		{
			debugf( TEXT("DISHONORED(bringup): frdiag census component %s owner %s oneshots %d trees %d generic %s running %s"),
				*Comp->GetName(), Comp->GetOwner() ? *Comp->GetOwner()->GetName() : TEXT("<none>"), Comp->m_OneShotInfo.Num(), MyTrees,
				Comp->m_pGenericDialogTree ? TEXT("yes") : TEXT("no"),
				Inst ? ( Inst->m_pCurPlayingConversation ? *Inst->m_pCurPlayingConversation->GetName() : TEXT("<idle>") ) : TEXT("<none>") );
		}
	}
	debugf( TEXT("DISHONORED(bringup): frdiag census: t %.2f %d oneshot actor(s), %d attached component(s), %d bound tree(s), %d running conversation(s)"),
		GWorld->GetTimeSeconds(), OneShots, Components, Trees, Running );
}
