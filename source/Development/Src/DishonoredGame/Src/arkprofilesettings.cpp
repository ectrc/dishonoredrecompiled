// DishonoredGame/src/arkprofilesettings.cpp
// Engine.ArkProfileSettings is an Engine-package shim declared by the generated DishonoredGameEngineShims.h; retail keeps its code
// in Engine/src/arksettings.cpp (2012 PDB).

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x1cd3a0 = UOnlinePlayerStorage::execSetToDefaults (one body in the retail native table for four
// classes). The UArkProfileSettings::SetToDefaults override (2012 rva 0x57c1b0, Engine) is not ported: the reference
// UOnlineProfileSettings::SetToDefaults runs.
void UArkProfileSettings::execSetToDefaults( FFrame& Stack, RESULT_DECL )
{
	UOnlinePlayerStorage::execSetToDefaults( Stack, Result );
}
