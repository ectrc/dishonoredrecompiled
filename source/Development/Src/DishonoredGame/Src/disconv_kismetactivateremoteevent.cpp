// DishonoredGame/src/disconv_kismetactivateremoteevent.cpp
// DISHONORED(port): agent FR (PHASE17 FR). PlayNode_Derived 2012 rva 0x9074f0 (703 B): every enabled
// DisSeqEvent_DialogOutputs whose Originator is this tree's one-shot (or whose m_pDialogTree is this tree) has the output
// link named m_EventName activated, with the one-shot (or the world info) as originator and the node's speaker (or the
// world info) as instigator. Retail calls USequenceEvent::PublishLinkedVariableValues (2012 vtable slot 90) before
// CheckActivate (slot 98). GetActionText 0x8feb70 and OnDelete_Derived 0x8fec30 are editor-only.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

INT UDisConv_KismetActivateRemoteEvent::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	if( m_EventName == NAME_None || !GWorld )
	{
		return 0;
	}
	AActor* SpeakerActor = NULL;
	if( UDisConversation_InGameData* ConvData = _pRunningInstance->GetInGameConv( this ) )
	{
		if( IDisConvSpeakerInterface* Speaker = ConvData->GetSpeaker( m_iSpeaker ) )
		{
			SpeakerActor = Cast<AActor>( Speaker->GetUObjectInterfaceDisConvSpeakerInterface() );
		}
	}
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	const UDisDialogTree* Tree = GetDialogTree();
	AActor* Originator = ( Tree && Tree->m_pBoundToOneShot ) ? (AActor*)Tree->m_pBoundToOneShot : (AActor*)WorldInfo;
	AActor* Instigator = SpeakerActor ? SpeakerActor : (AActor*)WorldInfo;
	USequence* GameSequence = GWorld->GetGameSequence();
	if( !GameSequence )
	{
		return 0;
	}
	TArray<USequenceObject*> DialogEvents;
	GameSequence->FindSeqObjectsByClass( UDisSeqEvent_DialogOutputs::StaticClass(), DialogEvents, TRUE );
	INT Activated = 0;
	for( INT i = 0; i < DialogEvents.Num(); i++ )
	{
		UDisSeqEvent_DialogOutputs* Event = Cast<UDisSeqEvent_DialogOutputs>( DialogEvents(i) );
		if( !Event || !Event->bEnabled )
		{
			continue;
		}
		const UBOOL bOurOneShot = Tree && Tree->m_pBoundToOneShot && Event->Originator == Tree->m_pBoundToOneShot;
		if( !bOurOneShot && Tree != Event->m_pDialogTree )
		{
			continue;
		}
		for( INT j = 0; j < Event->OutputLinks.Num(); j++ )
		{
			if( FName( *Event->OutputLinks(j).LinkDesc, FNAME_Add ) == m_EventName )
			{
				TArray<INT> ActiveIndices;
				ActiveIndices.AddItem( j );
				Event->PublishLinkedVariableValues();
				Event->CheckActivate( Originator, Instigator, FALSE, &ActiveIndices, FALSE );
				Activated++;
				break;
			}
		}
	}
	FRDIAG( "bind %s: remote event '%s' activated %d DisSeqEvent_DialogOutputs of %d", *GetPathName(), *m_EventName.ToString(), Activated, DialogEvents.Num() );
	return 0;
}
