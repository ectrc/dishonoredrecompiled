// DishonoredGame/src/jediag.cpp
// DISHONORED(bringup): agent JE (PHASE24 JE). See Inc/jediag.h.

#include "DishonoredGame.h"
#include "jediag.h"

static INT GDisJEDiag = -1;

UBOOL DisJEDiagEnabled()
{
	if( GDisJEDiag < 0 )
	{
		GDisJEDiag = ParseParam( appCmdLine(), TEXT("jediag") ) ? 1 : 0;
	}
	return GDisJEDiag != 0;
}

FLOAT DisJEDiagNow()
{
	return GWorld && GWorld->GetWorldInfo() ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;
}
