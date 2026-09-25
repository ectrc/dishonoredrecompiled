/*=============================================================================
	DishonoredNativeStub.cpp: bring-up body for natives that are not ported yet (see DishonoredNativeStub.h).
=============================================================================*/
#include "CorePrivate.h"
#include "DishonoredNativeStub.h"

static UBOOL GDishonoredStrictNatives = FALSE;
static UBOOL GDishonoredStrictNativesParsed = FALSE;
static TSet<FString> GDishonoredWarnedNatives;

static UFunction* FindStubFunction( UObject* Self, FFrame& Stack, const TCHAR* ExecName )
{
	// exec bodies are named exec<ScriptName>; the script function carries the script name
	const TCHAR* ScriptName = (appStrnicmp( ExecName, TEXT("exec"), 4 ) == 0) ? ExecName + 4 : ExecName;
	FName Name( ScriptName, FNAME_Find );
	if( Name == NAME_None )
	{
		return NULL;
	}
	// called through ProcessEvent: Stack.Node is the function itself (Code points at its EX_NativeParm stubs)
	UFunction* Function = Cast<UFunction>( Stack.Node );
	if( Function && Function->GetFName() == Name )
	{
		return Function;
	}
	return Self ? Self->FindFunction( Name ) : NULL;
}

void DishonoredNativeStub( UObject* Self, FFrame& Stack, void* Result, const TCHAR* Module, const TCHAR* ClassName, const TCHAR* ExecName )
{
	if( !GDishonoredStrictNativesParsed )
	{
		GDishonoredStrictNativesParsed = TRUE;
		GDishonoredStrictNatives = ParseParam( appCmdLine(), TEXT("strictnatives") );
	}
	const FString Key = FString::Printf( TEXT("%s::%s"), ClassName, ExecName );
	if( GDishonoredStrictNatives )
	{
		appErrorf( TEXT("%s native not ported: %s"), Module, *Key );
	}
	if( !GDishonoredWarnedNatives.Contains( Key ) )
	{
		GDishonoredWarnedNatives.Add( Key );
		warnf( TEXT("DISHONORED(bringup): %s native not ported: %s (parameters consumed, result zeroed)"), Module, *Key );
	}

	UFunction* Function = FindStubFunction( Self, Stack, ExecName );
	UProperty* ReturnProperty = NULL;
	if( Function && Stack.Code )
	{
		// consume every parameter the way the real exec body would: one Step per CPF_Parm in declaration order,
		// into a zeroed temporary that is destroyed afterwards (FString/TArray/struct temporaries own memory)
		for( UProperty* Property = Function->PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			if( !(Property->PropertyFlags & CPF_Parm) )
			{
				continue;
			}
			if( Property->PropertyFlags & CPF_ReturnParm )
			{
				ReturnProperty = Property;
				continue;
			}
			const INT Size = Property->ArrayDim * Property->ElementSize;
			BYTE* Temp = (BYTE*)appAlloca( Size + 16 );
			appMemzero( Temp, Size + 16 );
			Stack.Step( Stack.Object, Temp );
			Property->DestroyValue( Temp );
		}
		P_FINISH;
		if( Function->FunctionFlags & FUNC_Iterator )
		{
			// PRE_ITERATOR reads the end offset of the foreach body; a stub iterates nothing and jumps past it
			INT wEndOffset = Stack.ReadWord();
			Stack.Code = &Stack.Node->Script( wEndOffset + 1 );
		}
	}
	else if( Function )
	{
		for( UProperty* Property = Function->PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			if( (Property->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == (CPF_Parm | CPF_ReturnParm) )
			{
				ReturnProperty = Property;
			}
		}
	}
	if( Result && ReturnProperty )
	{
		appMemzero( Result, ReturnProperty->ArrayDim * ReturnProperty->ElementSize );
	}
}
