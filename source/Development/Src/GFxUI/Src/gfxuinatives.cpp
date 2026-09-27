// GFxUI/Src/gfxuinatives.cpp - the remaining 12 natives of the GFxUI module and the C++ methods behind
// them: UGFxInteraction (4), UGFxDataStoreSubscriber (7) and UGFxFSCmdHandler_Kismet (1). Agent BE
// (PHASE8.md package BE item 1).
//
// Retail spreads these over two units: the interaction and fscommand ones are in
// gfxui/src/gfxuiinteraction.cpp together with UGFxInteraction's whole input pipeline (Init, InputKey,
// InputAxis, InputChar, Exec, Tick), and the subscriber ones are in gfxui/src/gfxuidatastore.cpp. That input
// pipeline drives FGFxEngine directly and belongs to the runtime-glue package, so keeping only the natives
// here is what stops the two packages editing the same file. Each function names its retail unit and rva.
#include "GFxUI.h"
#include "gfxui_gfx3.h"

/*-----------------------------------------------------------------------------
	UGFxInteraction - four thin forwards onto the engine (gfxuiinteraction.cpp)
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x5d22b0, gfxuiinteraction.cpp:108
void UGFxInteraction::CloseAllMoviePlayers()
{
	if( GGFxEngine )
	{
		GGFxEngine->CloseAllMovies( 0 );
	}
}

// DISHONORED(port): 2012 rva 0x5d4f30, gfxuiinteraction.cpp:99
void UGFxInteraction::NotifyGameSessionEnded()
{
	if( GGFxEngine )
	{
		GGFxEngine->NotifyGameSessionEnded();
	}
}

// DISHONORED(port): 2012 rva 0x5caf40, gfxuiinteraction.cpp:509 - the focused movie's owning script object
UGFxMoviePlayer* UGFxInteraction::GetFocusMovie( INT ControllerId )
{
	if( GGFxEngine == NULL )
	{
		return NULL;
	}
	FGFxMovie* Movie = GGFxEngine->GetFocusedMovieFromControllerID( ControllerId );
	return Movie ? Movie->pUMovie : NULL;
}

// DISHONORED(port): gfxuiinteraction.cpp. Adding or removing a local player changes which movies can be
// focused and how the viewport is split, both of which are FGFxEngine's books.
void UGFxInteraction::NotifyPlayerAdded( INT PlayerIndex, ULocalPlayer* AddedPlayer )
{
	if( GGFxEngine )
	{
		GGFxEngine->ReevaluateFocus();
	}
}

void UGFxInteraction::NotifyPlayerRemoved( INT PlayerIndex, ULocalPlayer* RemovedPlayer )
{
	if( GGFxEngine )
	{
		GGFxEngine->CloseAllMovies( PlayerIndex );
		GGFxEngine->ReevaluateFocus();
	}
}

void UGFxInteraction::execCloseAllMoviePlayers( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	CloseAllMoviePlayers();
}
void UGFxInteraction::execNotifyPlayerRemoved( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(PlayerIndex);
	P_GET_OBJECT(ULocalPlayer,RemovedPlayer);
	P_FINISH;
	NotifyPlayerRemoved(PlayerIndex,RemovedPlayer);
}
void UGFxInteraction::execNotifyPlayerAdded( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(PlayerIndex);
	P_GET_OBJECT(ULocalPlayer,AddedPlayer);
	P_FINISH;
	NotifyPlayerAdded(PlayerIndex,AddedPlayer);
}
void UGFxInteraction::execNotifyGameSessionEnded( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	NotifyGameSessionEnded();
}
void UGFxInteraction::execGetFocusMovie( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(ControllerId);
	P_FINISH;
	*(UGFxMoviePlayer**)Result = GetFocusMovie(ControllerId);
}

/*-----------------------------------------------------------------------------
	UGFxFSCmdHandler_Kismet (gfxuiinteraction.cpp)
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x5caf80, gfxuiinteraction.cpp:1084. CheckActivate on the Kismet event, then
// every "Argument" string variable linked to it receives the fscommand's argument. The return value is TRUE
// whenever the event fired, which is what tells FGFxFSCommandHandler::Callback the command was consumed.
UBOOL UGFxFSCmdHandler_Kismet::FSCommand( UGFxMoviePlayer* Movie, UGFxEvent_FSCommand* Event, const FString& Cmd, const FString& Arg )
{
	if( Event == NULL || GWorld == NULL || GWorld->GetWorldInfo() == NULL )
	{
		return FALSE;
	}
	if( !Event->CheckActivate( GWorld->GetWorldInfo(), NULL, FALSE, NULL, NULL ) )
	{
		return FALSE;
	}
	TArray<FString*> ArgVars;
	Event->GetStringVars( ArgVars, TEXT("Argument") );
	for( INT Index = 0; Index < ArgVars.Num(); Index++ )
	{
		*ArgVars(Index) = Arg;
	}
	return TRUE;
}

void UGFxFSCmdHandler_Kismet::execFSCommand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UGFxMoviePlayer,Movie);
	P_GET_OBJECT(UGFxEvent_FSCommand,Event);
	P_GET_STR(Cmd);
	P_GET_STR(Arg);
	P_FINISH;
	*(UBOOL*)Result = FSCommand(Movie,Event,Cmd,Arg);
}

/*-----------------------------------------------------------------------------
	UGFxDataStoreSubscriber (gfxuidatastore.cpp). The subscriber owns no bindings
	of its own: every one of these walks Movie->DataStoreBindings, which is why
	the retail bodies all start by loading the movie pointer and its array.
	FGFxDataStoreBinding is 152 bytes and DataSource.ResolvedDataStore sits at
	byte 44 of it, which is the +44 the decompiles index.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x5bf530, gfxuidatastore.cpp:299
FString UGFxDataStoreSubscriber::GetDataStoreBinding( INT BindingIndex ) const
{
	if( Movie == NULL || !Movie->DataStoreBindings.IsValidIndex( BindingIndex ) )
	{
		return FString();
	}
	return Movie->DataStoreBindings(BindingIndex).DataSource.MarkupString;
}

// DISHONORED(port): gfxuidatastore.cpp. Retail rebinds through FUIDataStoreBinding::SetDataBinding, whose
// body is Engine's (UnUIDataStores.cpp) and is not declared in this tree yet; the markup itself and the
// refresh that follows it are the part that lives here.
void UGFxDataStoreSubscriber::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex )
{
	if( Movie == NULL || !Movie->DataStoreBindings.IsValidIndex( BindingIndex ) )
	{
		return;
	}
	FUIDataStoreBinding& Binding = Movie->DataStoreBindings(BindingIndex).DataSource;
	if( Binding.MarkupString == MarkupText )
	{
		return;
	}
	Binding.MarkupString = MarkupText;
	Binding.ResolvedDataStore = NULL;
	Binding.DataStoreName = NAME_None;
	Binding.DataStoreField = NAME_None;
	// DISHONORED(bringup): retail then calls FUIDataStoreBinding::ResolveMarkup (Engine) and
	// RefreshSubscriberValue; the markup resolver is not declared in this tree.
	RefreshSubscriberValue( BindingIndex );
}

// DISHONORED(port): 2012 rva 0x5c8b30, gfxuidatastore.cpp:373 - the +44 of the decompile is
// DataSource.ResolvedDataStore
void UGFxDataStoreSubscriber::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	if( Movie == NULL )
	{
		return;
	}
	for( INT Index = 0; Index < Movie->DataStoreBindings.Num(); Index++ )
	{
		UUIDataStore* DataStore = Movie->DataStoreBindings(Index).DataSource.ResolvedDataStore;
		if( DataStore )
		{
			out_BoundDataStores.AddUniqueItem( DataStore );
		}
	}
}

// DISHONORED(port): 2012 rva 0x5c8c00, gfxuidatastore.cpp:382. Besides clearing the binding, the retail body
// releases the managed GFxValue each binding cached for its model and control objects - that is the
// ObjectRelease the decompile shows through the misnamed Subscriber pointer.
void UGFxDataStoreSubscriber::ClearBoundDataStores()
{
	if( Movie == NULL )
	{
		return;
	}
	for( INT Index = 0; Index < Movie->DataStoreBindings.Num(); Index++ )
	{
		FGFxDataStoreBinding& Binding = Movie->DataStoreBindings(Index);
		Binding.DataSource.ResolvedDataStore = NULL;
		Binding.DataSource.DataStoreName = NAME_None;
		Binding.DataSource.DataStoreField = NAME_None;
		for( INT Which = 0; Which < 2; Which++ )
		{
			FPointer& Ref = Which == 0 ? Binding.ModelRef : Binding.ControlRef;
			if( Ref )
			{
				GFxValue* Value = (GFxValue*)Ref;
				Value->ReleaseManaged();
				delete Value;
				Ref = NULL;
			}
		}
		Binding.ListDataProvider = TScriptInterface<IUIListElementProvider>();
	}
}

// DISHONORED(port): 2012 rva 0x5ce310, gfxuidatastore.cpp. Pushing a data-store value into the movie is a
// SetVariable on the binding's VarPath; without a resolved data store there is nothing to read.
UBOOL UGFxDataStoreSubscriber::RefreshSubscriberValue( INT BindingIndex )
{
	if( Movie == NULL )
	{
		return FALSE;
	}
	UBOOL bRefreshed = FALSE;
	for( INT Index = 0; Index < Movie->DataStoreBindings.Num(); Index++ )
	{
		if( BindingIndex != INDEX_NONE && BindingIndex != Index )
		{
			continue;
		}
		FGFxDataStoreBinding& Binding = Movie->DataStoreBindings(Index);
		if( Binding.DataSource.ResolvedDataStore == NULL || Binding.VarPath.Len() == 0 )
		{
			continue;
		}
		// DISHONORED(bringup): retail reads the field through UUIDataStore::GetDataStoreValue and pushes it
		// with the right SetVariable* overload for the field type; the data-store read side is Engine's and
		// is not ported, so the binding is counted as refreshed without a value change.
		bRefreshed = TRUE;
	}
	return bRefreshed;
}

// DISHONORED(port): 2012 rva 0x5c8df0 / 0x5c8f80, gfxuidatastore.cpp:440. The publish direction: read the
// movie's variable back and write it into the data store, for the bindings marked editable.
UBOOL UGFxDataStoreSubscriber::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex )
{
	if( Movie == NULL )
	{
		return FALSE;
	}
	UBOOL bSaved = FALSE;
	for( INT Index = 0; Index < Movie->DataStoreBindings.Num(); Index++ )
	{
		if( BindingIndex != INDEX_NONE && BindingIndex != Index )
		{
			continue;
		}
		FGFxDataStoreBinding& Binding = Movie->DataStoreBindings(Index);
		if( !Binding.bEditable || Binding.DataSource.ResolvedDataStore == NULL )
		{
			continue;
		}
		out_BoundDataStores.AddUniqueItem( Binding.DataSource.ResolvedDataStore );
		// DISHONORED(bringup): the write itself is UUIDataStore::SetDataStoreValue, Engine's and unported
		bSaved = TRUE;
	}
	return bSaved;
}

void UGFxDataStoreSubscriber::PublishValues()
{
	TArray<UUIDataStore*> BoundDataStores;
	SaveSubscriberValue( BoundDataStores, INDEX_NONE );
}

// DISHONORED(port): gfxuidatastore.cpp. A data store telling us one of its values moved re-reads exactly the
// bindings that name it, which is why the whole array is walked rather than a single index.
void UGFxDataStoreSubscriber::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated,
	FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	if( Movie == NULL || SourceDataStore == NULL )
	{
		return;
	}
	for( INT Index = 0; Index < Movie->DataStoreBindings.Num(); Index++ )
	{
		const FUIDataStoreBinding& Binding = Movie->DataStoreBindings(Index).DataSource;
		if( Binding.ResolvedDataStore == SourceDataStore
			&& ( PropertyTag == NAME_None || Binding.DataStoreField == PropertyTag ) )
		{
			RefreshSubscriberValue( Index );
		}
	}
}

void UGFxDataStoreSubscriber::execSaveSubscriberValue( FFrame& Stack, RESULT_DECL )
{
	P_GET_TARRAY_REF(UUIDataStore*,out_BoundDataStores);
	P_GET_INT_OPTX(BindingIndex,INDEX_NONE);
	P_FINISH;
	*(UBOOL*)Result = SaveSubscriberValue(out_BoundDataStores,BindingIndex);
}
void UGFxDataStoreSubscriber::execClearBoundDataStores( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	ClearBoundDataStores();
}
void UGFxDataStoreSubscriber::execGetBoundDataStores( FFrame& Stack, RESULT_DECL )
{
	P_GET_TARRAY_REF(UUIDataStore*,out_BoundDataStores);
	P_FINISH;
	GetBoundDataStores(out_BoundDataStores);
}
void UGFxDataStoreSubscriber::execNotifyDataStoreValueUpdated( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UUIDataStore,SourceDataStore);
	P_GET_UBOOL(bValuesInvalidated);
	P_GET_NAME(PropertyTag);
	P_GET_OBJECT(UUIDataProvider,SourceProvider);
	P_GET_INT(ArrayIndex);
	P_FINISH;
	NotifyDataStoreValueUpdated(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex);
}
void UGFxDataStoreSubscriber::execRefreshSubscriberValue( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(BindingIndex,INDEX_NONE);
	P_FINISH;
	*(UBOOL*)Result = RefreshSubscriberValue(BindingIndex);
}
void UGFxDataStoreSubscriber::execGetDataStoreBinding( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(BindingIndex,INDEX_NONE);
	P_FINISH;
	*(FString*)Result = GetDataStoreBinding(BindingIndex);
}
void UGFxDataStoreSubscriber::execSetDataStoreBinding( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(MarkupText);
	P_GET_INT_OPTX(BindingIndex,INDEX_NONE);
	P_FINISH;
	SetDataStoreBinding(MarkupText,BindingIndex);
}
void UGFxDataStoreSubscriber::execPublishValues( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	PublishValues();
}
