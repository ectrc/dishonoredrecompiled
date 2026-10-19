/*=============================================================================
	AkFPUtilities.h - the SDK's branch-free float helpers (sdk headers akfputilities.h), instantiated in retail as
	AK_FPMin 0x9886a0, AK_FPMax 0x98ddc0, AK_FPSetValGTE 0x993700, AK_FPSetValLT 0x993720, AK_FPSetValGT 0x999ef0 (and
	their per-plug-in duplicates).

	DISHONORED(port): agent IH2 (wave 26).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

inline AkReal32 AK_FPMin( AkReal32 in_fValueA, AkReal32 in_fValueB )
{
	return in_fValueA > in_fValueB ? in_fValueB : in_fValueA;
}

inline AkReal32 AK_FPMax( AkReal32 in_fValueA, AkReal32 in_fValueB )
{
	return in_fValueA <= in_fValueB ? in_fValueB : in_fValueA;
}

inline void AK_FPSetValGTE( AkReal32 in_fComparandA, AkReal32 in_fComparandB, AkReal32 & io_fVariableToSet, AkReal32 in_fValueIfTrue )
{
	if( in_fComparandA >= in_fComparandB )
	{
		io_fVariableToSet = in_fValueIfTrue;
	}
}

inline void AK_FPSetValLT( AkReal32 in_fComparandA, AkReal32 in_fComparandB, AkReal32 & io_fVariableToSet, AkReal32 in_fValueIfTrue )
{
	if( in_fComparandA < in_fComparandB )
	{
		io_fVariableToSet = in_fValueIfTrue;
	}
}

inline void AK_FPSetValGT( AkReal32 in_fComparandA, AkReal32 in_fComparandB, AkReal32 & io_fVariableToSet, AkReal32 in_fValueIfTrue )
{
	if( in_fComparandA > in_fComparandB )
	{
		io_fVariableToSet = in_fValueIfTrue;
	}
}
