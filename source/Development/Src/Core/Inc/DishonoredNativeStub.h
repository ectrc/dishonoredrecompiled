/*=============================================================================
	DishonoredNativeStub.h: bring-up body for natives that are not ported yet.

	DISHONORED(bringup): the generated <Module>NativeStubs.cpp units give every unported native an exec body
	that would otherwise abort the process. DISHONORED_NATIVE_STUB consumes the call's parameters exactly
	like a real exec body (one Stack.Step per CPF_Parm, then P_FINISH, iterator bodies skipped), zeroes the
	return value, and warns once per native. -strictnatives turns the warning back into appErrorf so a
	golden run can prove that the startup path hits no stub.
=============================================================================*/
#pragma once

void DishonoredNativeStub( UObject* Self, FFrame& Stack, void* Result, const TCHAR* Module, const TCHAR* ClassName, const TCHAR* ExecName );

/** DISHONORED(bringup): agent GE (PHASE19 GE). The same stub for a native listed in <Module>NativeStubs.logparams.txt: every
    call logs the parameters it consumed, each exported as text (object path, class, vector, number), so a rare event such
    as the player's PlayDying_Native names its killer, damage type and hit location in the log instead of warning once. */
void DishonoredNativeStubLogParams( UObject* Self, FFrame& Stack, void* Result, const TCHAR* Module, const TCHAR* ClassName, const TCHAR* ExecName );

#define DISHONORED_NATIVE_STUB(Module, Class, Exec) DishonoredNativeStub( this, Stack, Result, TEXT(#Module), TEXT(#Class), TEXT(#Exec) )
#define DISHONORED_NATIVE_STUB_LOGPARAMS(Module, Class, Exec) DishonoredNativeStubLogParams( this, Stack, Result, TEXT(#Module), TEXT(#Class), TEXT(#Exec) )
