/*=============================================================================
	DishonoredNativeStub.cpp: bring-up body for natives that are not ported yet (see DishonoredNativeStub.h).
=============================================================================*/
#include "CorePrivate.h"
#include "DishonoredNativeStub.h"

extern UFunction* GDishonoredCallingFunction;  // UnCorSc.cpp: the UFunction UObject::CallFunction is dispatching

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

static void ConsumeAndZero( FFrame& Stack, void* Result, UFunction* Function )
{
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

static void WarnOrAbort( const TCHAR* Module, const FString& Key )
{
	if( !GDishonoredStrictNativesParsed )
	{
		GDishonoredStrictNativesParsed = TRUE;
		GDishonoredStrictNatives = ParseParam( appCmdLine(), TEXT("strictnatives") );
	}
	if( GDishonoredStrictNatives )
	{
		appErrorf( TEXT("%s native not ported: %s"), Module, *Key );
	}
	if( !GDishonoredWarnedNatives.Contains( Key ) )
	{
		GDishonoredWarnedNatives.Add( Key );
		warnf( TEXT("DISHONORED(bringup): %s native not ported: %s (parameters consumed, result zeroed)"), Module, *Key );
	}
}

void DishonoredNativeStub( UObject* Self, FFrame& Stack, void* Result, const TCHAR* Module, const TCHAR* ClassName, const TCHAR* ExecName )
{
	WarnOrAbort( Module, FString::Printf( TEXT("%s::%s"), ClassName, ExecName ) );
	ConsumeAndZero( Stack, Result, FindStubFunction( Self, Stack, ExecName ) );
}

/**
 * DISHONORED(bringup): body bound by UFunction::Bind to every native the retail scripts declare but our
 * Core/Engine/GameFramework code does not implement yet (147 as of wave 3, agent Z's list). Script calls reach
 * it through UObject::CallFunction (which records the UFunction in GDishonoredCallingFunction); ProcessEvent
 * calls carry the function in Stack.Node.
 */
void UObject::execDishonoredUnboundNative( FFrame& Stack, RESULT_DECL )
{
	UFunction* Function = Cast<UFunction>( Stack.Node );
	if( !Function || Function->Func != &UObject::execDishonoredUnboundNative )
	{
		Function = GDishonoredCallingFunction;
	}
	const FString Key = Function ? Function->GetPathName() : FString(TEXT("<unknown>"));
	WarnOrAbort( TEXT("Engine (no C++ body)"), Key );
	ConsumeAndZero( Stack, Result, Function );
}
