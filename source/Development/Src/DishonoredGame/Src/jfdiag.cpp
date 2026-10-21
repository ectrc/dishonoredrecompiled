// DishonoredGame/src/jfdiag.cpp
// DISHONORED(bringup): agent JF (PHASE24 JF). See Inc/jfdiag.h.

#include "DishonoredGame.h"
#include "jfdiag.h"

static INT GDisJFDiag = -1;

UBOOL DisJFDiagEnabled()
{
	if( GDisJFDiag < 0 )
	{
		GDisJFDiag = ParseParam( appCmdLine(), TEXT("jfdiag") ) ? 1 : 0;
	}
	return GDisJFDiag != 0;
}

FLOAT DisJFDiagNow()
{
	return GWorld && GWorld->GetWorldInfo() ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;
}
