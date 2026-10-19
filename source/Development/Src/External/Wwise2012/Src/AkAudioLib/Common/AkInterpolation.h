/*=============================================================================
	AkInterpolation.h - CAkInterpolationTable (akinterpolationtable.cpp) and CAkInterpolation (akinterpolation.cpp): the
	curve tables behind every fade (CAkTransition), RTPC conversion and dB <-> linear lookup; one global, g_Interpol,
	built by AK::SoundEngine::PreInit with Init( 0x14D, 0x14D, AkITable_All ) and released by Term.

	DISHONORED(port): agent IH2 (wave 26). Layouts of the 2012 PDB (CAkInterpolationTable 20 bytes, CAkInterpolation
	seven tables in this order).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

enum AkITable
{
	AkITable_None	= 0x0,
	AkITable_Exp1	= 0x1,
	AkITable_Exp2	= 0x2,
	AkITable_Exp3	= 0x4,
	AkITable_All	= 0xFF
};

namespace CalculationFunction
{
	AkReal32 AkExp1( AkReal64 in_fFrom );
	AkReal32 AkSine( AkReal64 in_fFrom );
	AkReal32 dBToReal( AkReal64 in_fFrom );
	AkReal32 realTodB( AkReal64 in_fFrom );
	AkReal32 linearMutingTodBMuting96( AkReal64 in_fFrom );
	AkReal32 linearToFrequency_20_20000( AkReal64 in_fFrom );
	AkReal32 AkExp3( AkReal64 in_fFrom );
}

class CAkInterpolationTable
{
public:
	CAkInterpolationTable();

	AKRESULT Init( AkReal32 ( *in_pfCalcFunc )( AkReal64 ), AkReal32 in_fMinValue, AkReal32 in_fMaxValue, AkUInt32 in_wTableSize );
	void Term();
	AkReal32 GetNoCheck( AkReal32 in_fKey );
	AkReal32 Get( AkReal32 in_fKey );

	AkReal32 *	m_ConversionTable;
	AkReal32	m_fMaxValue;
	AkReal32	m_fMinValue;
	AkUInt32	m_wTableSize;
	AkReal32	m_fOneOnScale;
};

class CAkInterpolation
{
public:
	AKRESULT Init( AkUInt32 in_wTableSize, AkUInt32 in_wdBTableSize, AkITable in_eITable );
	void Term();
	AkReal32 InterpolateNoCheck( AkReal32 in_fTimeRatio, AkReal32 in_fInitialVal, AkReal32 in_fTargetVal, AkCurveInterpolation in_eFadeCurve );
	AkReal32 RealTodB( AkReal32 in_fReal );
	AkReal32 InterpolateDBNoCheck( AkReal32 in_fTimeRatio, AkReal32 in_fInitialVal, AkReal32 in_fTargetVal, AkCurveInterpolation in_eFadeCurve );

	CAkInterpolationTable m_tableDBtoReal;
	CAkInterpolationTable m_tableRealToDB;
	CAkInterpolationTable m_tableExp1;
	CAkInterpolationTable m_tableSine;
	CAkInterpolationTable m_tableExp3;
	CAkInterpolationTable m_tableLinearMutingTodBMuting96;
	CAkInterpolationTable m_tableLinearToFrequency_20_20000;
};

extern CAkInterpolation g_Interpol;
