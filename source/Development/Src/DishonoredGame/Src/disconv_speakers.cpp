// DishonoredGame/src/disconv_speakers.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The speaker slot nodes. 2013 rvas (2012 in parentheses):
//   UDisConv_Speaker::BindSpeaker (0x904d10, 151), CheckLOSAndRadius (0x9012a0)
//   UDisConv_MeSpeaker::GetSpeakerTree 0x8a7ba0 (0x8f8be0), BindSpeaker_Derived (0x8f9d90)
//   UDisConv_InitiatorSpeaker::GetSpeakerTree 0x61a090 (0x8f8c10), BindSpeaker_Derived (0x8f9db0)
//   UDisConv_OneShotSpeaker::GetSpeakerTree (0x8f8ba0), BindSpeaker_Derived (0x8fc290)
// BindSpeaker's awareness filter (m_bNotWhileInCombat / Suspicious / Fearful against ADishonoredNPCPawn::GetAwarenessLevel)
// and CheckLOSAndRadius's line-of-sight half (FDisComponentVision::CanSeeVisibleThing, DisLineProbe flags 8351) are the AI
// perception package's (agent ET); the radius half is ported and the two are named at the site.
// Not ported (editor): the InitNode / GetSpeakerName / OnDelete_Derived / GetSpeakerEditorActor bodies.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

IDisConvSpeakerInterface* UDisConv_Speaker::BindSpeaker( FDisDialogRunningInstance* _pRunningInstance ) const
{
	IDisConvSpeakerInterface* Bound = BindSpeaker_Derived( _pRunningInstance );
	if( Bound )
	{
		IDisConvSpeakerInterface* Me = _pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me.GetObject() ? (IDisConvSpeakerInterface*)_pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me.GetInterface() : NULL;
		if( !CheckLOSAndRadius( Me, Bound ) )
		{
			FRDIAG( "bind speaker node %s: %s failed the radius / line-of-sight check", *GetPathName(), *Bound->GetUObjectInterfaceDisConvSpeakerInterface()->GetName() );
			return NULL;
		}
	}
	return Bound;
}

UBOOL UDisConv_Speaker::CheckLOSAndRadius( IDisConvSpeakerInterface* _pMe, IDisConvSpeakerInterface* _pBoundSpeaker ) const
{
	if( m_bRequireRadialCheck && _pMe && _pBoundSpeaker )
	{
		TArray<FDisConvSpeakerLocInfo> MeInfo, BoundInfo;
		_pMe->GetSpeakerLocInfo( MeInfo );
		_pBoundSpeaker->GetSpeakerLocInfo( BoundInfo );
		const FLOAT RadiusSq = m_fMustBeWithinRadius * m_fMustBeWithinRadius;
		UBOOL bLegal = FALSE;
		for( INT i = 0; i < MeInfo.Num() && !bLegal; i++ )
		{
			for( INT j = 0; j < BoundInfo.Num() && !bLegal; j++ )
			{
				bLegal = ( BoundInfo(j).m_Location - MeInfo(i).m_Location ).SizeSquared() < RadiusSq;
			}
		}
		if( !bLegal )
		{
			return FALSE;
		}
	}
	if( m_bMustHaveLOS && _pMe && _pBoundSpeaker && GWorld )
	{
		TArray<FDisConvSpeakerLocInfo> MeInfo, BoundInfo;
		_pMe->GetSpeakerLocInfo( MeInfo );
		_pBoundSpeaker->GetSpeakerLocInfo( BoundInfo );
		AActor* MeActor = Cast<AActor>( _pMe->GetUObjectInterfaceDisConvSpeakerInterface() );
		for( INT i = 0; i < MeInfo.Num(); i++ )
		{
			for( INT j = 0; j < BoundInfo.Num(); j++ )
			{
				FCheckResult Hit( 1.0f );
				if( GWorld->SingleLineCheck( Hit, MeActor, BoundInfo(j).m_Location, MeInfo(i).m_Location, TRACE_World | TRACE_StopAtAnyHit ) )
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}
	return TRUE;
}

const UDisDialogTree* UDisConv_MeSpeaker::GetSpeakerTree() const
{
	return GetDialogTree();
}

IDisConvSpeakerInterface* UDisConv_MeSpeaker::BindSpeaker_Derived( FDisDialogRunningInstance* _pRunningInstance ) const
{
	const TScriptInterface<IDisConvSpeakerInterface>& Me = _pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me;
	return Me.GetObject() ? (IDisConvSpeakerInterface*)Me.GetInterface() : NULL;
}

const UDisDialogTree* UDisConv_InitiatorSpeaker::GetSpeakerTree() const
{
	return m_pInitiatorDialogTree;
}

IDisConvSpeakerInterface* UDisConv_InitiatorSpeaker::BindSpeaker_Derived( FDisDialogRunningInstance* _pRunningInstance ) const
{
	const TScriptInterface<IDisConvSpeakerInterface>& Initiator = _pRunningInstance->m_pBoundSpeaker_Initiator;
	return Initiator.GetObject() ? (IDisConvSpeakerInterface*)Initiator.GetInterface() : NULL;
}

const UDisDialogTree* UDisConv_OneShotSpeaker::GetSpeakerTree() const
{
	const UDisDialogTree* Tree = GetDialogTree();
	return ( Tree && Tree->m_pBoundToOneShot ) ? Tree->m_pBoundToOneShot->m_pDialogTree : NULL;
}

IDisConvSpeakerInterface* UDisConv_OneShotSpeaker::BindSpeaker_Derived( FDisDialogRunningInstance* _pRunningInstance ) const
{
	const UDisDialogTree* Tree = GetDialogTree();
	ADisDialogOneShot* OneShot = Tree ? Tree->m_pBoundToOneShot : NULL;
	if( !OneShot || m_iOneShotSpeaker < 0 || m_iOneShotSpeaker >= OneShot->m_OtherSpeakers.Num() )
	{
		return NULL;
	}
	IDisConvOneShotOwnerInterface* Owner = OneShot->GetOwnerForSpeaker( OneShot->m_OtherSpeakers( m_iOneShotSpeaker ) );
	if( !Owner )
	{
		FRDIAG( "bind speaker node %s: one-shot speaker %d of %s has no owner actor", *GetPathName(), m_iOneShotSpeaker, *OneShot->GetName() );
		return NULL;
	}
	const TScriptInterface<IDisConvSpeakerInterface>& Me = _pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me;
	return Owner->GetSpeakerFromOneShotOwner( Me.GetObject() ? (IDisConvSpeakerInterface*)Me.GetInterface() : NULL );
}
