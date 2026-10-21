// DishonoredGame/src/disconv_playerchoice.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x8d9860  protected: virtual void __thiscall UDisConv_PlayerChoice::GetClassVars_Derived(struct FDisConvClassVars &)const
//   0x8daa00  public: struct FDisPlayerChoice_Optional const * __thiscall UDisConv_PlayerChoice::GetOptionalChoiceForIndex(int, int &)const
//   0x8daac0  public: virtual unsigned int __thiscall UDisConv_PlayerChoice::IsValidChoice(int)const
//   0x8dd8e0  protected: virtual int __thiscall UDisConv_PlayerChoice::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x8dd950  public: virtual int __thiscall UDisConv_PlayerChoice::TickNode(float, struct FDisDialogRunningInstance *)const
//   0x8dd9d0  protected: virtual unsigned int __thiscall UDisConv_PlayerChoice::ImportText_Derived(class FString const &, class FName const &, class FString const &, unsigned int)
//   0x8ddcd0  public: wchar_t const * __thiscall FDisPlayerChoice_Optional::GetChoiceText(void)const
//   0x8dde30  public: wchar_t const * __thiscall FDisPlayerChoice_Static::GetChoiceText(void)const
//   0x8ddf90  public: virtual struct FDisPlayerChoice_Optional & __thiscall UDisConv_PlayerChoice::GetOptionalChoice(int)
//   0x8ddfe0  public: virtual struct FDisPlayerChoice_Optional const & __thiscall UDisConv_PlayerChoice::GetOptionalChoice(int)const
//   0x8de030  public: virtual struct FDisPlayerChoice_Static const & __thiscall UDisConv_PlayerChoice::GetChoice(int)const
//   0x8e8070  protected: void __thiscall UDisConv_PlayerChoice::MatchChoicesToOutputLinks(unsigned int)
//   0x8ea740  public: virtual void __thiscall UDisConv_PlayerChoice::InitNode(class UDisConversation *, unsigned int, unsigned int)
//   0x8ef660  public: static void __cdecl UDisConv_PlayerChoice::InitializePrivateStaticClassUDisConv_PlayerChoice(void)
//   0x8f1590  public: virtual void __thiscall UDisConv_PlayerChoice::Serialize(class FArchive &)
//   0x8f4c00  public: static class UClass * __cdecl UDisConv_PlayerChoice::GetPrivateStaticClassUDisConv_PlayerChoice(wchar_t const *)
//   0x8f6740  public: static class UClass * __cdecl UDisConv_PlayerChoice::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent JF (PHASE24 JF): the conversation node that asks the player to choose (the hide-and-seek question of the Emily greeting,
	DisDialogOneShot_15's DisConversation_14). Its outputs are its static choices, then the optional ones whose story flag holds.
	Playing it hands the node to the player's dialog state - once the state is the current one and the dialog view track has
	finished blending in - and waits; the state pauses the bend time and shows the list, and the node finishes with the chosen
	output once the choice is made and the world's time dilation is back to 1. 2013 bodies.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "jfdiag.h"

static void SerializePerLanguageChoices( FArchive& Ar, UScriptStruct* LocalizedStruct, TArray<FDisPlayerChoiceLocalized>& PerLanguageData, const FName& CurrentLanguage )
{
	INT NumLanguages = PerLanguageData.Num();
	Ar << NumLanguages;
	if( !Ar.IsLoading() )
	{
		for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
		{
			LocalizedStruct->SerializeBin( Ar, (BYTE*)&PerLanguageData(LanguageIndex), sizeof(FDisPlayerChoiceLocalized) );
		}
		return;
	}
	PerLanguageData.Reset( GIsEditor ? NumLanguages : 1 );
	FDisPlayerChoiceLocalized Localized(EC_EventParm);
	for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
	{
		LocalizedStruct->SerializeBin( Ar, (BYTE*)&Localized, sizeof(FDisPlayerChoiceLocalized) );
		if( GIsEditor || Localized.m_Language == CurrentLanguage )
		{
			PerLanguageData.AddItem( Localized );
		}
	}
}

// DISHONORED(written): 2013 rva 0x8a1850 (2012 0x8f1590); non-template choices store the per-language texts of every static
// and optional choice after the tagged properties (version 1); the pre-version m_Choices strings are moved to m_Choices_Static.
void UDisConv_PlayerChoice::Serialize( FArchive& Ar )
{
	if( Ar.IsSaving() && !IsTemplate() )
	{
		m_PlayerChoiceVersionNum = 1;
	}
	Super::Serialize( Ar );
	if( IsTemplate() )
	{
		return;
	}
	UScriptStruct* LocalizedStruct = FindField<UScriptStruct>( GetClass(), TEXT("DisPlayerChoiceLocalized") );
	if( Ar.IsLoading() )
	{
		if( m_Choices.Num() )
		{
			m_Choices_Static.Reset( m_Choices.Num() );
			for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices.Num(); ChoiceIndex++ )
			{
				FDisPlayerChoice_Static* Static = new(m_Choices_Static) FDisPlayerChoice_Static(EC_EventParm);
				Static->m_ChoiceText = m_Choices(ChoiceIndex);
			}
			m_Choices.Reset();
			MarkPackageDirty();
		}
		if( m_PlayerChoiceVersionNum < 1 )
		{
			return;
		}
	}
	const FName CurrentLanguage( UObject::GetLanguage() );
	for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices_Static.Num(); ChoiceIndex++ )
	{
		SerializePerLanguageChoices( Ar, LocalizedStruct, m_Choices_Static(ChoiceIndex).m_PerLanguageData, CurrentLanguage );
	}
	for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices_Optional.Num(); ChoiceIndex++ )
	{
		SerializePerLanguageChoices( Ar, LocalizedStruct, m_Choices_Optional(ChoiceIndex).m_PerLanguageData, CurrentLanguage );
	}
}

// 2013 0x8934c0 (347 B; 2012 0x8dde30): the choice's text in the game's language - the authored text for INT, the one localized
// entry a cooked package keeps otherwise (Serialize keeps only the current language's)
const TCHAR* FDisPlayerChoice_Static::GetChoiceText() const
{
	if( appStricmp( *FString( UObject::GetLanguage() ), TEXT("INT") ) == 0 )
	{
		return *m_ChoiceText;
	}
	if( m_PerLanguageData.Num() == 1 )
	{
		return *m_PerLanguageData(0).m_LocalizedChoiceText;
	}
	return TEXT("<Lang not found!>");
}

// 2013 0x893360 (347 B; 2012 0x8ddcd0): the same for an optional choice
const TCHAR* FDisPlayerChoice_Optional::GetChoiceText() const
{
	if( appStricmp( *FString( UObject::GetLanguage() ), TEXT("INT") ) == 0 )
	{
		return *m_ChoiceText;
	}
	if( m_PerLanguageData.Num() == 1 )
	{
		return *m_PerLanguageData(0).m_LocalizedChoiceText;
	}
	return TEXT("<Lang not found!>");
}

// 2013 0x88aff0 (13 B; 2012 0x8d9860): a choice node starts a dialog
void UDisConv_PlayerChoice::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	_rVarsOut.m_bIsStartOfDialog = TRUE;
}

// 2013 0x89ac90 (40 B; 2012 0x8ea740)
void UDisConv_PlayerChoice::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	UDisConv_Node::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	MatchChoicesToOutputLinks( _bForEditor );
}

// 2013 0x8999f0 (842 B; 2012 0x8e8070): one output link per static then optional choice, added when missing, extra ones removed;
// the editor names each link after its choice
void UDisConv_PlayerChoice::MatchChoicesToOutputLinks( UBOOL _bForEditor )
{
	const INT NumLinks = GetNumChoices() + GetNumOptionalChoices();
	const INT NumChoices = GetNumChoices();
	for( INT ChoiceIndex = 0; ChoiceIndex < NumChoices; ChoiceIndex++ )
	{
		const FString Text( GetChoice( ChoiceIndex ).GetChoiceText() );
		if( ChoiceIndex >= m_OutputLinks.Num() )
		{
			m_OutputLinks.AddItem( FDisConvNodeLink(EC_EventParm) );
		}
		if( _bForEditor )
		{
			m_OutputLinks(ChoiceIndex).m_Description = Text;
		}
	}
	const INT NumOptional = GetNumOptionalChoices();
	for( INT OptionalIndex = 0; OptionalIndex < NumOptional; OptionalIndex++ )
	{
		const FDisPlayerChoice_Optional& Optional = GetOptionalChoice( OptionalIndex );
		if( OptionalIndex + GetNumChoices() >= m_OutputLinks.Num() )
		{
			m_OutputLinks.AddItem( FDisConvNodeLink(EC_EventParm) );
		}
		if( _bForEditor )
		{
			m_OutputLinks(OptionalIndex + GetNumChoices()).m_Description = Optional.m_ChoiceText;
		}
	}
	if( NumLinks < m_OutputLinks.Num() )
	{
		m_OutputLinks.Remove( NumLinks, m_OutputLinks.Num() - NumLinks );
	}
}

// 2013 0x88cbd0 / 0x88cb80 (78 B each; 2012 0x8ddf90 / 0x8ddfe0)
FDisPlayerChoice_Optional& UDisConv_PlayerChoice::GetOptionalChoice( INT _iChoice )
{
	return m_Choices_Optional(_iChoice);
}

const FDisPlayerChoice_Optional& UDisConv_PlayerChoice::GetOptionalChoice( INT _iChoice ) const
{
	return m_Choices_Optional(_iChoice);
}

// 2013 0x88cc20 (78 B; 2012 0x8de030)
const FDisPlayerChoice_Static& UDisConv_PlayerChoice::GetChoice( INT _iChoice ) const
{
	return m_Choices_Static(_iChoice);
}

// 2013 0x88b8d0 (33 B; 2012 0x8daac0)
UBOOL UDisConv_PlayerChoice::IsValidChoice( INT _iChoice ) const
{
	return _iChoice >= 0 && _iChoice < m_Choices_Static.Num();
}

// 2013 0x88b810 (182 B; 2012 0x8daa00): the optional choice shown at list index _iIndex (the static choices come first) - an
// optional choice is shown when its story flag set and flag are named and the flag is set (clear, with m_bStoryFlagMustBeFalse) on
// the player - and its output link index
const FDisPlayerChoice_Optional* UDisConv_PlayerChoice::GetOptionalChoiceForIndex( INT _iIndex, INT& _rOutputIndex ) const
{
	INT CurIndex = GetNumChoices();
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	_rOutputIndex = -1;
	const INT NumOptional = GetNumOptionalChoices();
	for( INT OptionalIndex = 0; OptionalIndex < NumOptional; OptionalIndex++ )
	{
		const FDisPlayerChoice_Optional& Optional = GetOptionalChoice( OptionalIndex );
		const FGuid& Flag = Optional.m_StoryFlag;
		if( !Optional.m_pStoryFlagSet || !( Flag.A | Flag.B | Flag.C | Flag.D ) || Player->CheckStoryFlag( Optional.m_pStoryFlagSet, Flag ) != !Optional.m_bStoryFlagMustBeFalse )
		{
			continue;
		}
		if( CurIndex == _iIndex )
		{
			_rOutputIndex = OptionalIndex + GetNumChoices();
			return &Optional;
		}
		CurIndex++;
	}
	return NULL;
}

/**
 * 2013 0x89c570 (148 B; 2012 0x8dd8e0, 99 B): a node without choices plays through; otherwise the player's dialog state takes it -
 * in 2013 only once the state is the current one and the dialog's view track has finished blending in (TickNode retries) - and the
 * node waits.
 */
INT UDisConv_PlayerChoice::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	if( !GetNumChoices() )
	{
		return 0;
	}
	_pRunningInstance->GetInGameConv( this );
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	if( !Player )
	{
		return 0;
	}
	UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( Player->m_pPlayerMasterFSM->GetLogicalState() );
	if( !InDialog )
	{
		JFDIAG( "%s PlayNode: player not in the dialog state (%s), plays through", *GetName(),
			Player->m_pPlayerMasterFSM->GetLogicalState() ? *Player->m_pPlayerMasterFSM->GetLogicalState()->GetClass()->GetName() : TEXT("none") );
		return 0;
	}
	const UBOOL bBlending = InDialog->IsDialogViewTrackBlending();
	const UBOOL bActive = InDialog == Player->m_pPlayerMasterFSM->GetCurrentlyActiveState();
	if( !bBlending && bActive )
	{
		InDialog->SetActivePlayerChoice( this );
	}
	JFDIAG( "%s PlayNode: %d static + %d optional choice(s), dialog state active %d, view track blending %d -> %s, waiting", *GetName(), GetNumChoices(),
		GetNumOptionalChoices(), bActive, bBlending, ( !bBlending && bActive ) ? TEXT("choice handed to the state") : TEXT("retried in TickNode") );
	_rbNotFinished = TRUE;
	return -1;
}

/**
 * 2013 0x89c610 (174 B, vtable slot 74; 2012 0x8dd950, 117 B): until the dialog state holds a choice node the hand-over is retried;
 * then the node waits while the choice is up or the world is still bent, and finishes with the chosen output - a static choice's
 * index, or an optional choice's link.
 * DISHONORED(deviation): retail reads the logical state as the dialog state without a test; a state that is not one (the dialog
 * ended under the node) waits here instead.
 */
INT UDisConv_PlayerChoice::TickNode( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	_pRunningInstance->GetInGameConv( this );
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	UStatePlayerMasterInDialog* InDialog = Player ? Cast<UStatePlayerMasterInDialog>( Player->m_pPlayerMasterFSM->GetLogicalState() ) : NULL;
	if( !InDialog )
	{
		return -1;
	}
	if( !InDialog->m_pActivePlayerChoice )
	{
		if( !InDialog->IsDialogViewTrackBlending() && InDialog == Player->m_pPlayerMasterFSM->GetCurrentlyActiveState() )
		{
			InDialog->SetActivePlayerChoice( this );
			JFDIAG( "%s TickNode: choice handed to the dialog state", *GetName() );
		}
		return -1;
	}
	if( InDialog->m_DialogState == 1 || GWorld->GetWorldInfo()->TimeDilation <= 0.9999f )
	{
		return -1;
	}
	INT Output = InDialog->m_iCurChoice;
	if( Output < 0 || Output >= m_Choices_Static.Num() )
	{
		INT OptionalOutput = -1;
		if( !GetOptionalChoiceForIndex( Output, OptionalOutput ) )
		{
			return -1;
		}
		Output = OptionalOutput;
	}
	JFDIAG( "%s TickNode: choice %d made -> output %d", *GetName(), InDialog->m_iCurChoice, Output );
	return Output;
}
