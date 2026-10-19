// DishonoredGame/src/disconv_speakers.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The speaker slot nodes. 2013 rvas (2012 in parentheses):
//   UDisConv_Speaker::BindSpeaker (0x904d10, 151), CheckLOSAndRadius (0x9012a0)
//   UDisConv_MeSpeaker::GetSpeakerTree 0x8a7ba0 (0x8f8be0), BindSpeaker_Derived (0x8f9d90)
//   UDisConv_InitiatorSpeaker::GetSpeakerTree 0x61a090 (0x8f8c10), BindSpeaker_Derived (0x8f9db0)
//   UDisConv_OneShotSpeaker::GetSpeakerTree (0x8f8ba0), BindSpeaker_Derived (0x8fc290)
// BindSpeaker's awareness filter (m_bNotWhileInCombat / Suspicious / Fearful against ADishonoredNPCPawn::GetAwarenessLevel)
// and CheckLOSAndRadius's line-of-sight half (FDisComponentVision::CanSeeVisibleThing, DisLineProbe flags 8351) are the AI
// perception package's (agent ET); the radius half is ported and the two are named at the site.
// Not ported (editor): the InitNode / OnDelete_Derived / GetSpeakerEditorActor bodies.
// Agent IE (PHASE23 IE): GetSpeakerName of the four speaker classes, by vtable slot 86 (+344) of their 2013 vtables
// (0x11a0d70 / 0x11a0ed8 / 0x11a1040 / 0x11a11a8): UDisConv_Speaker 0x8ad320 (2012 0x8fc460), UDisConv_MeSpeaker 0x8b1cc0
// (0x8ff210), UDisConv_InitiatorSpeaker 0x8b1e80 (0x8ff3d0), UDisConv_OneShotSpeaker 0x8b6380 (0x8fc0e0; 2013 gathers the
// one-shot's other speakers through ADisDialogOneShot's virtual at +940, 0x655f00, which lists m_OtherSpeakers in order, so
// the index reads m_OtherSpeakers directly here). Retail's OneShot body dereferences the tree's one-shot unchecked; a tree
// bound to none answers like an out-of-range index here.
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

void UDisConv_Speaker::GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const
{
	_rNameOut = m_NickName.Len() == 0 && _bUseDecorations ? FString( TEXT("<???>") ) : FString( m_NickName );
}

void UDisConv_MeSpeaker::GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const
{
	FString NickName = m_NickName;
	const UDisDialogTree* Tree = GetDialogTree();
	if( NickName.Len() == 0 )
	{
		NickName = Tree->m_OwnerNickname;
	}
	if( NickName.Len() > 0 )
	{
		_rNameOut = _bUseDecorations ? FString::Printf( TEXT("%s, <Me>"), *NickName ) : NickName;
	}
	else if( Tree->m_pBoundToOneShot && Tree->m_pBoundToOneShot->m_MeSpeaker.m_SpeakerNickName != NAME_None )
	{
		const FString OneShotNickName = Tree->m_pBoundToOneShot->m_MeSpeaker.m_SpeakerNickName.GetNameString();
		_rNameOut = _bUseDecorations ? FString::Printf( TEXT("%s, <Me>"), *OneShotNickName ) : OneShotNickName;
	}
	else
	{
		_rNameOut = _bUseDecorations ? TEXT("<Me>") : TEXT("");
	}
}

void UDisConv_InitiatorSpeaker::GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const
{
	if( m_NickName.Len() == 0 )
	{
		_rNameOut = _bUseDecorations ? TEXT("<Initiator>") : TEXT("");
	}
	else
	{
		_rNameOut = _bUseDecorations ? FString::Printf( TEXT("%s, <Initiator>"), *m_NickName ) : FString( m_NickName );
	}
}

void UDisConv_OneShotSpeaker::GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const
{
	if( m_NickName.Len() > 0 )
	{
		_rNameOut = m_NickName;
		return;
	}
	const ADisDialogOneShot* OneShot = GetDialogTree()->m_pBoundToOneShot;
	if( OneShot && m_iOneShotSpeaker >= 0 && m_iOneShotSpeaker < OneShot->m_OtherSpeakers.Num() )
	{
		_rNameOut = OneShot->m_OtherSpeakers(m_iOneShotSpeaker).GetOneShotName( _bUseDecorations );
	}
	else
	{
		_rNameOut = _bUseDecorations ? TEXT("<???>") : TEXT("");
	}
}
