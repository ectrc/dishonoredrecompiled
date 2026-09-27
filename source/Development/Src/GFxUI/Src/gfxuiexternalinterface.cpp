// GFxUI/Src/gfxuiexternalinterface.cpp - the ActionScript -> UnrealScript path: the two GFx state
// callbacks and the two property converters that marshal across it. Agent BE (PHASE8.md package BE item 2,
// "the external-interface callback path that turns a button press into a script call").
//
// Retail keeps these four in gfxui/src/gfxuiengine.cpp together with FGFxEngine's own body. They are in a
// file of their own here only so that this package and agent BB's runtime-glue package do not edit the same
// unit; the coordinator can fold them back into gfxuiengine.cpp at any time. Function-to-rva map:
//   FGFxExternalInterface::Callback   2013 0x58d510  (2012 0x5e4880, gfxuiengine.cpp:499)
//   FGFxFSCommandHandler::Callback    2013 0x586450  (2012 0x5e3190, gfxuiengine.cpp:444)
//   FGFxEngine::ConvertUPropToGFx     2013 0x58c8f0  (2012 0x5e4080, gfxuiengine.h:396)
//   FGFxEngine::ConvertGFxToUProp                    (2012 0x41630,  gfxuiengine.h:507)
//   FGFxEngine::ReplaceCharsInFString                (2012 0x5b96d0)
//
// This is the whole of what agent AW measured as boundary 4 (gfx_decision.md 2.4): an AS2
// ExternalInterface.call arrives here with a method name and GFxValue arguments, the name is looked up as a
// UFunction on the movie player's ExternalInterface object, the arguments are converted in place and
// ProcessEvent runs the script. The New Game button is exactly this plus two lines of C++.
#include "GFxUI.h"
#include "gfxui_gfx3.h"

/*-----------------------------------------------------------------------------
	FGFxEngine::ConvertUPropToGFx / ConvertGFxToUProp
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x5b96d0
INT FGFxEngine::ReplaceCharsInFString( FString& Text, const TCHAR* Chars, TCHAR Replacement )
{
	INT NumReplaced = 0;
	for( INT Index = 0; Index < Text.Len(); Index++ )
	{
		if( appStrchr( Chars, Text[Index] ) != NULL )
		{
			Text[Index] = Replacement;
			NumReplaced++;
		}
	}
	return NumReplaced;
}

// DISHONORED(port): 2013 rva 0x58c8f0 (2012 0x5e4080). bOverwrite distinguishes "fill a fresh GFx object"
// from "update the one already in Value": a struct or dynamic array with bOverwrite set keeps the AS2 object
// and writes into its members or elements, which is what keeps the HUD from allocating a new object per
// field per frame.
void FGFxEngine::ConvertUPropToGFx( UProperty* Property, BYTE* Address, GFxValue& Value, GFxMovieView* Movie, bool bOverwrite )
{
	// a static C array becomes an AS2 array of the element type
	if( Property->ArrayDim > 1 && Movie )
	{
		Movie->CreateArray( &Value );
		for( INT Index = 0; Index < Property->ArrayDim; Index++ )
		{
			GFxValue Element;
			ConvertUPropToGFx( Property, Address + Index * Property->ElementSize, Element, NULL, false );
			Value.PushBack( Element );
			Element.ReleaseManaged();
		}
		return;
	}

	if( Property->GetClass() == UBoolProperty::StaticClass() )
	{
		const UBOOL bSet = ( *(BITFIELD*)Address & ((UBoolProperty*)Property)->BitMask ) != 0;
		Value.SetBoolean( bSet ? true : false );
		return;
	}


	UArrayProperty* ArrayProperty = Cast<UArrayProperty>( Property );
	if( ArrayProperty && Movie )
	{
		FScriptArray* Array = (FScriptArray*)Address;
		const INT ElementSize = ArrayProperty->Inner->ElementSize;
		if( bOverwrite && Value.IsArray() )
		{
			for( INT Index = 0; Index < Array->Num(); Index++ )
			{
				GFxValue Element;
				Value.GetElement( Index, &Element );
				ConvertUPropToGFx( ArrayProperty->Inner, (BYTE*)Array->GetData() + Index * ElementSize, Element, Movie, true );
				Value.SetElement( Index, Element );
				Element.ReleaseManaged();
			}
		}
		else
		{
			Movie->CreateArray( &Value );
			for( INT Index = 0; Index < Array->Num(); Index++ )
			{
				GFxValue Element;
				ConvertUPropToGFx( ArrayProperty->Inner, (BYTE*)Array->GetData() + Index * ElementSize, Element, Movie, false );
				Value.PushBack( Element );
				Element.ReleaseManaged();
			}
		}
		return;
	}

	UStructProperty* StructProperty = Cast<UStructProperty>( Property );
	if( StructProperty && Movie )
	{
		if( bOverwrite && Value.IsObject() )
		{
			for( TFieldIterator<UProperty> It(StructProperty->Struct); It; ++It )
			{
				GFxValue Member;
				const FTCHARToUTF8 Name( *It->GetName() );
				const UBOOL bHadMember = Value.GetMember( Name, &Member );
				ConvertUPropToGFx( *It, Address + It->Offset, Member, Movie, false );
				if( !bHadMember )
				{
					Value.SetMember( Name, Member );
				}
				Member.ReleaseManaged();
			}
		}
		else
		{
			Movie->CreateObject( &Value, NULL, NULL, 0 );
			for( TFieldIterator<UProperty> It(StructProperty->Struct); It; ++It )
			{
				GFxValue Member;
				ConvertUPropToGFx( *It, Address + It->Offset, Member, Movie, false );
				Value.SetMember( FTCHARToUTF8( *It->GetName() ), Member );
				Member.ReleaseManaged();
			}
		}
		return;
	}

	// the scalars, read through the property so bitfields, byte enums and FName all come out right
	if( Property->GetClass()->ClassCastFlags & CASTCLASS_UByteProperty )
	{
		Value.SetNumber( (DOUBLE)*(BYTE*)Address );
	}
	else if( Property->GetClass()->ClassCastFlags & CASTCLASS_UIntProperty )
	{
		Value.SetNumber( (DOUBLE)*(INT*)Address );
	}
	else if( Property->GetClass()->ClassCastFlags & CASTCLASS_UFloatProperty )
	{
		Value.SetNumber( *(FLOAT*)Address );
	}
	else if( Property->GetClass()->ClassCastFlags & CASTCLASS_UStrProperty )
	{
		Value.SetStringW( **(FString*)Address );
	}
	else if( Property->GetClass()->ClassCastFlags & CASTCLASS_UObjectProperty )
	{
		// only a UGFxObject can cross: it already holds the AS2 value
		UObjectProperty* ObjectProperty = (UObjectProperty*)Property;
		UGFxObject* Object = ( ObjectProperty->PropertyClass
			&& ObjectProperty->PropertyClass->IsChildOf( UGFxObject::StaticClass() ) )
			? (UGFxObject*)*(UObject**)Address : NULL;
		if( Object )
		{
			Value = *Object->GetASValue();
		}
		else
		{
			Value.SetNull();
		}
	}
	else if( Property->GetClass()->ClassCastFlags & CASTCLASS_UBoolProperty )
	{
		Value.SetBoolean( *(INT*)Address != 0 );
	}
	else
	{
		Value.SetUndefined();
	}
}

// DISHONORED(port): 2012 rva 0x41630. The visitor form of the struct case is the nested
// `FGFxEngine::ConvertGFxToUProp'::`49'::ObjVisitor of the PDB (2012 0x60190): AS2 objects are visited by
// member name, because an AS2 object has no field order to walk.
class FGFxToUPropObjVisitor : public GFxValue::ObjectInterface::ObjVisitor
{
public:
	FGFxToUPropObjVisitor( UStructProperty* InStruct, BYTE* InAddress, UGFxMoviePlayer* InMovie )
		: Struct( InStruct ), Address( InAddress ), Movie( InMovie ) {}

	// DISHONORED(port): the second parameter is `const GFxValue&` in the 2012 PDB, not a pointer -
	// agent BB measured it (agentBB.md 2.4a) and the real declaration in External/GFx3/GFxValue.h
	// follows the PDB, so this override does too.
	virtual void Visit( const char* Name, const GFxValue& Value )
	{
		const FString MemberName = FString( FUTF8ToTCHAR( Name ) );
		for( TFieldIterator<UProperty> It(Struct->Struct); It; ++It )
		{
			if( It->GetName() == MemberName )
			{
				FGFxEngine::ConvertGFxToUProp( *It, Address + It->Offset, Value, Movie );
				return;
			}
		}
	}

private:
	UStructProperty* Struct;
	BYTE* Address;
	UGFxMoviePlayer* Movie;
};

void FGFxEngine::ConvertGFxToUProp( UProperty* Property, BYTE* Address, const GFxValue& Value, UGFxMoviePlayer* Movie )
{
	// a static C array is filled from the AS2 array, up to whichever is shorter
	if( Property->ArrayDim > 1 && Value.IsArray() )
	{
		UINT Count = Min<UINT>( Property->ArrayDim, Value.GetArraySize() );
		for( UINT Index = 0; Index < Count; Index++ )
		{
			GFxValue Element;
			Value.GetElement( Index, &Element );
			ConvertGFxToUProp( Property, Address + Index * Property->ElementSize, Element, Movie );
			Element.ReleaseManaged();
		}
		return;
	}

	const DWORD CastFlags = Property->GetClass()->ClassCastFlags;

	if( Value.GetType() == GFxValue::VT_Boolean && ( CastFlags & CASTCLASS_UBoolProperty ) )
	{
		const BITFIELD BitMask = ((UBoolProperty*)Property)->BitMask;
		if( Value.GetBool() )
		{
			*(BITFIELD*)Address |= BitMask;
		}
		else
		{
			*(BITFIELD*)Address &= ~BitMask;
		}
		return;
	}

	if( Value.GetType() == GFxValue::VT_Number )
	{
		if( CastFlags & CASTCLASS_UByteProperty )
		{
			*(BYTE*)Address = (BYTE)Value.GetNumber();
		}
		else if( CastFlags & CASTCLASS_UIntProperty )
		{
			*(INT*)Address = (INT)Value.GetNumber();
		}
		else if( CastFlags & CASTCLASS_UFloatProperty )
		{
			*(FLOAT*)Address = Value.GetNumber();
		}
		return;
	}

	if( CastFlags & CASTCLASS_UStrProperty )
	{
		if( Value.GetType() == GFxValue::VT_String )
		{
			*(FString*)Address = FString( FUTF8ToTCHAR( Value.GetString() ) );
		}
		else if( Value.GetType() == GFxValue::VT_StringW )
		{
			*(FString*)Address = FString( Value.GetStringW() );
		}
		else
		{
			*(FString*)Address = FString();
		}
		return;
	}

	UArrayProperty* ArrayProperty = Cast<UArrayProperty>( Property );
	if( ArrayProperty && Value.GetType() == GFxValue::VT_Array )
	{
		FScriptArray* Array = (FScriptArray*)Address;
		const INT ElementSize = ArrayProperty->Inner->ElementSize;
		Array->Empty( 0, ElementSize );
		const UINT Count = Value.GetArraySize();
		Array->AddZeroed( Count, ElementSize );
		for( UINT Index = 0; Index < Count; Index++ )
		{
			GFxValue Element;
			Value.GetElement( Index, &Element );
			ConvertGFxToUProp( ArrayProperty->Inner, (BYTE*)Array->GetData() + Index * ElementSize, Element, Movie );
			Element.ReleaseManaged();
		}
		return;
	}

	UStructProperty* StructProperty = Cast<UStructProperty>( Property );
	if( StructProperty && Value.IsObject() )
	{
		FGFxToUPropObjVisitor Visitor( StructProperty, Address, Movie );
		Value.VisitMembers( &Visitor );
		return;
	}

	UObjectProperty* ObjectProperty = Cast<UObjectProperty>( Property );
	if( ObjectProperty && ObjectProperty->PropertyClass
		&& ObjectProperty->PropertyClass->IsChildOf( UGFxObject::StaticClass() ) && Movie )
	{
		*(UObject**)Address = Movie->CreateValueAddRef( &Value, ObjectProperty->PropertyClass );
	}
}

/*-----------------------------------------------------------------------------
	FGFxExternalInterface::Callback - AS2's ExternalInterface.call
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x58d510 (2012 0x5e4880). A leading underscore in the method name is the
// data-store protocol, not a script call; everything else is FindFunction on the movie player's
// ExternalInterface object. The FNAME_Find lookup is deliberate: an unknown name must not create an FName.
void FGFxExternalInterface::Callback( GFxMovieView* pMovie, const char* MethodName, const GFxValue* Args, UINT ArgCount )
{
	if( pMovie == NULL || MethodName == NULL || pMovie->GetUserData() == NULL )
	{
		return;
	}
	UGFxMoviePlayer* Movie = (UGFxMoviePlayer*)pMovie->GetUserData();

	if( MethodName[0] == '_' )
	{
		Movie->ProcessDataStoreCall( MethodName, Args, ArgCount );
		return;
	}

	UObject* Target = Movie->ExternalInterface;
	if( Target == NULL || Target->IsPendingKill() || Target->HasAnyFlags( RF_Unreachable ) )
	{
		return;
	}

	const FString MethodNames = FString( FUTF8ToTCHAR( MethodName ) );
	const FName MethodFName( *MethodNames, FNAME_Find );
	if( MethodFName == NAME_None )
	{
		return;
	}
	UFunction* Function = Target->FindFunction( MethodFName );
	if( Function == NULL )
	{
		return;
	}

	BYTE* Parms = (BYTE*)appAlloca( Function->ParmsSize );
	appMemzero( Parms, Function->ParmsSize );

	UINT ArgIndex = 0;
	for( TFieldIterator<UProperty> It(Function); It && ArgIndex < ArgCount
		&& ( It->PropertyFlags & (CPF_Parm|CPF_ReturnParm) ) == CPF_Parm; ++It, ++ArgIndex )
	{
		FGFxEngine::ConvertGFxToUProp( *It, Parms + It->Offset, Args[ArgIndex], Movie );
	}

	Target->ProcessEvent( Function, Parms );

	UProperty* ReturnProperty = Function->GetReturnProperty();
	if( ReturnProperty )
	{
		GFxValue ReturnValue;
		FGFxEngine::ConvertUPropToGFx( ReturnProperty, Parms + Function->ReturnValueOffset, ReturnValue, pMovie );
		if( !ReturnValue.IsUndefined() )
		{
			pMovie->SetExternalInterfaceRetVal( ReturnValue );
		}
		ReturnValue.ReleaseManaged();
	}

	for( TFieldIterator<UProperty> It(Function); It && ( It->PropertyFlags & (CPF_Parm|CPF_ReturnParm) ) == CPF_Parm; ++It )
	{
		It->DestroyValue( Parms + It->Offset );
	}
}

/*-----------------------------------------------------------------------------
	FGFxFSCommandHandler::Callback - AS2's fscommand()
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x586450 (2012 0x5e3190). With a movie user data and a game sequence the
// command goes to every UGFxEvent_FSCommand of that movie whose FSCommand matches; with no user data (or no
// world) it falls back to eventClientMessage on every player controller, which is how a stray fscommand
// still shows up somewhere visible.
void FGFxFSCommandHandler::Callback( GFxMovieView* pMovie, const char* Command, const char* Args )
{
	if( Command == NULL )
	{
		return;
	}
	UGFxMoviePlayer* Movie = pMovie ? (UGFxMoviePlayer*)pMovie->GetUserData() : NULL;
	if( Movie == NULL || GWorld == NULL )
	{
		const FString Message = FString( FUTF8ToTCHAR( Command ) );
		for( AController* Controller = GWorld ? GWorld->GetFirstController() : NULL; Controller; Controller = Controller->NextController )
		{
			APlayerController* PC = Cast<APlayerController>( Controller );
			if( PC )
			{
				PC->eventClientMessage( Message, NAME_None, 0.f );
			}
		}
		return;
	}

	USequence* GameSequence = GWorld->GetGameSequence( NULL );
	if( GameSequence == NULL )
	{
		return;
	}
	TArray<USequenceObject*> Events;
	GameSequence->FindSeqObjectsByClass( UGFxEvent_FSCommand::StaticClass(), Events, TRUE );

	const FString CommandString = FString( FUTF8ToTCHAR( Command ) );
	const FString ArgString = FString( FUTF8ToTCHAR( Args ) );
	for( INT Index = 0; Index < Events.Num(); Index++ )
	{
		UGFxEvent_FSCommand* Event = Cast<UGFxEvent_FSCommand>( Events(Index) );
		if( Event == NULL || Event->Movie != Movie->MovieInfo || Event->FSCommand != CommandString )
		{
			continue;
		}
		if( Event->Handler )
		{
			Event->Handler->eventFSCommand( Movie, Event, CommandString, ArgString );
		}
	}
}

/*-----------------------------------------------------------------------------
	The self-test the package's acceptance asks for: invoke a callback by name
	and see the script side run.
-----------------------------------------------------------------------------*/

// DISHONORED(bringup): a GFxMovieView test double. It is not a player - it answers GetUserData with the
// movie player under test and records the return value - so FGFxExternalInterface::Callback above can be
// driven without a GFx runtime, which is the whole point: the layer between script and GFx is testable
// before GFx exists. Every other slot of the 73-slot interface is unreachable in this test and says so.
// DISHONORED(bringup, agent DC): this whole block - the GFxMovieView test double and the self-test
// console command that drives it - is compiled out once the runtime is linked. It existed to make the
// script<->AS2 boundary testable before a GFxMovieView existed; there is a real one now
// (GFxMovieRoot, all 73 slots, agent BC) and the double's 73 stand-in signatures are its own rather
// than the PDB's, so keeping it would mean maintaining a second declaration of the interface.
#if !DISHONORED_GFXUI_GFX3_RUNTIME

class FGFxCallbackTestMovieView : public GFxMovieView
{
public:
	FGFxCallbackTestMovieView( UGFxMoviePlayer* InMoviePlayer ) : MoviePlayer( InMoviePlayer ), bGotReturnValue( FALSE ) {}

	virtual void* GetUserData() { return MoviePlayer; }
	virtual void SetUserData( void* InUserData ) { MoviePlayer = (UGFxMoviePlayer*)InUserData; }
	virtual void SetExternalInterfaceRetVal( const GFxValue& InValue ) { ReturnValue = InValue; bGotReturnValue = TRUE; }

	UBOOL HasReturnValue() const { return bGotReturnValue; }
	const GFxValue& GetReturnValue() const { return ReturnValue; }

	// GFxMovie
	virtual GFxMovieDef* GetMovieDef() { return NULL; }
	virtual UINT GetCurrentFrame() { return 0; }
	virtual bool HasLooped() { return false; }
	virtual void GotoFrame( UINT ) {}
	virtual bool GotoLabeledFrame( const char*, INT ) { return false; }
	virtual void SetPlayState( PlayState ) {}
	virtual PlayState GetPlayState() { return Stopped; }
	virtual void SetVisible( bool ) {}
	virtual bool GetVisible() { return false; }
	virtual bool IsAvailable( const char* ) { return false; }
	virtual void CreateString( GFxValue* Out, const char* ) { Out->SetUndefined(); }
	virtual void CreateStringW( GFxValue* Out, const TCHAR* ) { Out->SetUndefined(); }
	virtual void CreateObject( GFxValue* Out, const char*, const GFxValue*, UINT ) { Out->SetUndefined(); }
	virtual void CreateArray( GFxValue* Out ) { Out->SetUndefined(); }
	virtual void CreateFunction( GFxValue* Out, GFxFunctionHandler*, void* ) { Out->SetUndefined(); }
	virtual bool SetVariable( const char*, const GFxValue&, SetVarType ) { return false; }
	virtual bool GetVariable( GFxValue*, const char* ) { return false; }
	virtual bool SetVariableArray( SetArrayType, const char*, UINT, const void*, UINT, SetVarType ) { return false; }
	virtual bool SetVariableArraySize( const char*, UINT, SetVarType ) { return false; }
	virtual UINT GetVariableArraySize( const char* ) { return 0; }
	virtual bool GetVariableArray( SetArrayType, const char*, UINT, void*, UINT ) { return false; }
	virtual bool Invoke( const char*, GFxValue*, const GFxValue*, UINT ) { return false; }
	virtual bool Invoke( const char*, GFxValue*, const char*, ... ) { return false; }
	virtual bool InvokeArgs( const char*, GFxValue*, const char*, char* ) { return false; }
	// GFxStateBag
	virtual GFxStateBag* GetStateBagImpl() { return this; }
	virtual void SetState( GFxState::StateType, GFxState* ) {}
	virtual GFxState* GetStateAddRef( GFxState::StateType ) { return NULL; }
	virtual void GetStatesAddRef( GFxState**, const GFxState::StateType*, UINT ) {}
	// GFxMovieView
	virtual void SetViewport( const GViewport& ) {}
	virtual void GetViewport( GViewport* Out ) { *Out = GViewport(); }
	virtual void SetViewScaleMode( ScaleModeType ) {}
	virtual ScaleModeType GetViewScaleMode() { return SM_NoScale; }
	virtual void SetViewAlignment( AlignType ) {}
	virtual AlignType GetViewAlignment() { return Align_Center; }
	virtual void GetVisibleFrameRect( FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY ) { MinX = MinY = MaxX = MaxY = 0.f; }
	virtual void SetPerspective3D( const GMatrix3D& ) {}
	virtual void SetView3D( const GMatrix3D& ) {}
	virtual void GetSafeRect( FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY ) { MinX = MinY = MaxX = MaxY = 0.f; }
	virtual void SetSafeRect( FLOAT, FLOAT, FLOAT, FLOAT ) {}
	virtual void Restart() {}
	virtual FLOAT Advance( FLOAT, UINT ) { return 0.f; }
	virtual void Display() {}
	virtual void DisplayPrePass() {}
	virtual void SetPause( bool ) {}
	virtual bool IsPaused() { return true; }
	virtual void SetBackgroundColor( DWORD ) {}
	virtual void SetBackgroundAlpha( FLOAT ) {}
	virtual FLOAT GetBackgroundAlpha() { return 0.f; }
	virtual UINT HandleEvent( const GFxEvent& ) { return HE_NotHandled; }
	virtual void GetMouseState( UINT, FLOAT* X, FLOAT* Y, UINT* Buttons ) { *X = *Y = 0.f; *Buttons = 0; }
	virtual void NotifyMouseState( FLOAT, FLOAT, UINT, UINT ) {}
	virtual bool HitTest( FLOAT, FLOAT, HE_ReturnValueType, UINT ) { return false; }
	virtual bool HitTest3D( void*, FLOAT, FLOAT, UINT ) { return false; }
	virtual bool AttachDisplayCallback( const char*, void (*)(void*), void* ) { return false; }
	virtual bool IsMovieFocused() { return false; }
	virtual bool GetDirtyFlag( bool ) { return false; }
	virtual void SetMouseCursorCount( UINT ) {}
	virtual UINT GetMouseCursorCount() { return 0; }
	virtual void SetControllerCount( UINT ) {}
	virtual UINT GetControllerCount() { return 0; }
	virtual void GetStats( GStatBag*, bool ) {}
	virtual GMemoryHeap* GetHeap() { return NULL; }
	virtual void ForceCollectGarbage() {}

private:
	UGFxMoviePlayer* MoviePlayer;
	GFxValue ReturnValue;
	UBOOL bGotReturnValue;
};

/**
 * DISHONORED(bringup): the acceptance test of this package - invoke a callback by name and see the script
 * side run. GFXCALLBACK <Object> <Function> [arg ...] does one call on demand;
 * -gfxcallbacktest runs a fixed suite once on the first world tick and reports it as a census line, so the
 * harness can see it without a console.
 *
 * The suite drives the real FGFxExternalInterface::Callback against a GFxMovieView test double, which is the
 * only thing standing in for GFx - the name lookup, the GFxValue -> UProperty marshalling, ProcessEvent and
 * the return value going back through SetExternalInterfaceRetVal are all the ported code. Cases:
 *   1. a name AS2 could send that no UFunction answers -> nothing runs and nothing crashes;
 *   2. DishonoredGameInfo.GetChangelist -> a native with no parameters and an int return: proves the return
 *      value reaches SetExternalInterfaceRetVal;
 *   3. the movie player's own GFxMoviePlayer.SetVariableString with two string arguments -> proves two
 *      GFxValues are marshalled into a UFunction's parameters and that ProcessEvent reaches one of this
 *      package's own ported natives (it returns without a movie, which is the point: the call arrived);
 *   4. GFxMoviePlayer.SetVariableBool with a string and a number -> proves a GFxValue number becomes a
 *      UnrealScript bool and a GFxValue string becomes an FString.
 */
class FGFxCallbackSelfTest : public FSelfRegisteringExec, public FTickableObject
{
public:
	FGFxCallbackSelfTest() : bHasRun( FALSE ) {}

	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		if( !ParseCommand( &Cmd, TEXT("GFXCALLBACK") ) )
		{
			return FALSE;
		}
		const FString ObjectName = ParseToken( Cmd, 0 );
		const FString FunctionName = ParseToken( Cmd, 0 );
		UGFxMoviePlayer* MoviePlayer = MakeMoviePlayer();
		const UBOOL bRan = Run( MoviePlayer, ObjectName, FunctionName, Cmd, Ar );
		MoviePlayer->RemoveFromRoot();
		Ar.Logf( TEXT("DISHONORED(bringup): GFx callback census: %d calls, %s.%s"), bRan ? 1 : 0, *ObjectName, *FunctionName );
		return TRUE;
	}

	virtual void Tick( FLOAT DeltaTime )
	{
		bHasRun = TRUE;
		UGFxMoviePlayer* MoviePlayer = MakeMoviePlayer();
		INT Ran = 0, Expected = 0;

		// 1. a name nothing answers
		Expected++;
		if( !Run( MoviePlayer, TEXT("DishonoredGameInfo"), TEXT("ThisIsNotAnExternalInterfaceMethod"), TEXT(""), *GLog ) )
		{
			Ran++;
		}
		// 2. no parameters, an int return
		Expected++;
		if( Run( MoviePlayer, TEXT("DishonoredGameInfo"), TEXT("GetChangelist"), TEXT(""), *GLog ) && bLastHadReturnValue )
		{
			Ran++;
		}
		// 3. two strings into one of this package's own natives
		Expected++;
		if( Run( MoviePlayer, TEXT("GFxMoviePlayer"), TEXT("SetVariableString"), TEXT("_root.testPath agentBE"), *GLog ) )
		{
			Ran++;
		}
		// 4. a string and a number, where the number has to become a bool
		Expected++;
		if( Run( MoviePlayer, TEXT("GFxMoviePlayer"), TEXT("SetVariableBool"), TEXT("_root.testFlag 1"), *GLog ) )
		{
			Ran++;
		}
		MoviePlayer->RemoveFromRoot();
		GLog->Logf( TEXT("DISHONORED(bringup): GFx callback census: %d calls, %d of %d cases passed, external interface path %s"),
			Ran, Ran, Expected, Ran == Expected ? TEXT("OK") : TEXT("FAILED") );
	}

	virtual UBOOL IsTickable() const
	{
		return !bHasRun && ParseParam( appCmdLine(), TEXT("gfxcallbacktest") );
	}

	virtual UBOOL IsTickableWhenPaused() const { return TRUE; }

private:
	/** a movie player with no movie, which is what makes the test independent of GFx */
	static UGFxMoviePlayer* MakeMoviePlayer()
	{
		UGFxMoviePlayer* MoviePlayer = ConstructObject<UGFxMoviePlayer>( UGFxMoviePlayer::StaticClass() );
		MoviePlayer->AddToRoot();
		return MoviePlayer;
	}

	/** the object a callback is delivered to; a class name means that class's default object */
	static UObject* ResolveTarget( const FString& ObjectName, UGFxMoviePlayer* MoviePlayer )
	{
		if( ObjectName == TEXT("GFxMoviePlayer") )
		{
			return MoviePlayer;
		}
		UObject* Object = FindObject<UObject>( ANY_PACKAGE, *ObjectName );
		UClass* Class = Cast<UClass>( Object );
		return Class ? Class->GetDefaultObject() : Object;
	}

	/** @return TRUE when a UFunction answered the name and ProcessEvent ran */
	UBOOL Run( UGFxMoviePlayer* MoviePlayer, const FString& ObjectName, const FString& FunctionName,
		const TCHAR* Arguments, FOutputDevice& Ar )
	{
		bLastHadReturnValue = FALSE;
		UObject* Target = ResolveTarget( ObjectName, MoviePlayer );
		if( Target == NULL )
		{
			Ar.Logf( TEXT("DISHONORED(bringup): GFx callback: object '%s' not found"), *ObjectName );
			return FALSE;
		}
		MoviePlayer->ExternalInterface = Target;
		FGFxCallbackTestMovieView TestView( MoviePlayer );

		TArray<FString> Tokens;
		const TCHAR* Cursor = Arguments;
		for( FString Token = ParseToken( Cursor, 0 ); Token.Len(); Token = ParseToken( Cursor, 0 ) )
		{
			Tokens.AddItem( Token );
		}
		const UINT ArgCount = Tokens.Num();
		AutoGFxValueArray( CallArgs, ArgCount );
		for( UINT Index = 0; Index < ArgCount; Index++ )
		{
			if( Tokens(Index).IsNumeric() )
			{
				CallArgs(Index).SetNumber( appAtof( *Tokens(Index) ) );
			}
			else
			{
				CallArgs(Index).SetStringW( *Tokens(Index) );
			}
		}

		const FName MethodFName( *FunctionName, FNAME_Find );
		UFunction* Function = MethodFName != NAME_None ? Target->FindFunction( MethodFName ) : NULL;

		FGFxExternalInterface Interface;
		Interface.Callback( &TestView, FTCHARToUTF8( *FunctionName ), CallArgs, ArgCount );

		bLastHadReturnValue = TestView.HasReturnValue();
		FString ReturnText = TEXT("(void)");
		if( bLastHadReturnValue )
		{
			const GFxValue& Value = TestView.GetReturnValue();
			switch( Value.GetType() )
			{
			case GFxValue::VT_StringW: ReturnText = Value.GetStringW(); break;
			case GFxValue::VT_String:  ReturnText = FString( FUTF8ToTCHAR( Value.GetString() ) ); break;
			case GFxValue::VT_Number:  ReturnText = FString::Printf( TEXT("%.0f"), Value.GetNumber() ); break;
			case GFxValue::VT_Boolean: ReturnText = Value.GetBool() ? TEXT("true") : TEXT("false"); break;
			default:                   ReturnText = TEXT("(undefined)"); break;
			}
		}
		Ar.Logf( TEXT("DISHONORED(bringup): GFx callback %s.%s: resolved %d, %d args, returned %s"),
			*Target->GetName(), *FunctionName, Function ? 1 : 0, ArgCount, *ReturnText );
		return Function != NULL;
	}

	UBOOL bHasRun;
	UBOOL bLastHadReturnValue;
};

static FGFxCallbackSelfTest GGFxCallbackSelfTest;

#endif // !DISHONORED_GFXUI_GFX3_RUNTIME
