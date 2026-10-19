/*=============================================================================
	AkInterpolation.cpp - port of akinterpolation.cpp and akinterpolationtable.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). The tables live in g_DefaultPoolId as retail's. realTodB is fyl2x( x, log10 2 )
	* 20 = 20 * log10( x ); linearMutingTodBMuting96 likewise. CAkInterpolation's ctor (2012 0x934130) is the seven table
	ctors, i.e. the implicit one here. InterpolateDBNoCheck inlines RealTodB's normalisation in retail; it calls it here
	(same arithmetic). g_Interpol itself is defined with akaudiolib.cpp's port (its dynamic initializer, 2012 0xbab060).
=============================================================================*/
#include "AkInterpolation.h"
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <math.h>

extern AkMemPoolId g_DefaultPoolId;

namespace CalculationFunction
{
	/** 2012 0x953b40; 2013 0x950340. */
	AkReal32 AkExp1( AkReal64 in_fFrom )
	{
		return (AkReal32)pow( in_fFrom, 1.41 );
	}

	/** 2012 0x953b50; 2013 0x950350. */
	AkReal32 AkSine( AkReal64 in_fFrom )
	{
		return (AkReal32)( 1.0 - sin( ( 1.0 - in_fFrom ) * 1.570796370506287 ) );
	}

	/** 2012 0x953b70; 2013 0x950370. */
	AkReal32 dBToReal( AkReal64 in_fFrom )
	{
		return (AkReal32)pow( 10.0, in_fFrom * 0.05 );
	}

	/** 2012 0x953b90; 2013 0x950390. */
	AkReal32 realTodB( AkReal64 in_fFrom )
	{
		return (AkReal32)( log10( in_fFrom ) * 20.0 );
	}

	/** 2012 0x953bb0; 2013 0x9503b0. */
	AkReal32 linearMutingTodBMuting96( AkReal64 in_fFrom )
	{
		if( in_fFrom == 0.0 )
		{
			return 0.f;
		}
		if( in_fFrom >= 96.30000305175781 )
		{
			return 96.3f;
		}
		if( in_fFrom <= -96.30000305175781 )
		{
			return -96.3f;
		}
		if( in_fFrom >= 0.0 )
		{
			return (AkReal32)-( log10( ( 96.3 - in_fFrom ) * 0.01038421599169263 ) * 20.0 );
		}
		return (AkReal32)( log10( ( in_fFrom + 96.3 ) * 0.01038421599169263 ) * 20.0 );
	}

	/** 2012 0x953c40; 2013 0x950440. */
	AkReal32 linearToFrequency_20_20000( AkReal64 in_fFrom )
	{
		if( in_fFrom >= 20000.0 )
		{
			return 20000.f;
		}
		if( in_fFrom > 20.0 )
		{
			return (AkReal32)pow( 10.0, ( in_fFrom - 20.0 ) * 0.0001501501501501501 + 1.301029995663981 );
		}
		return 20.f;
	}

	/** 2012 0x953cf0; 2013 0x9504f0. The CRT's integer pow( x, 3 ) by squaring, inlined. */
	AkReal32 AkExp3( AkReal64 in_fFrom )
	{
		return (AkReal32)( in_fFrom * ( in_fFrom * in_fFrom ) );
	}
}

/** 2012 0x9539d0; 2013 0x9501d0. */
CAkInterpolationTable::CAkInterpolationTable()
	: m_ConversionTable( NULL )
	, m_fMaxValue( 0.f )
	, m_fMinValue( 0.f )
	, m_wTableSize( 0 )
	, m_fOneOnScale( 0.f )
{
}

/** 2012 0x9539f0; 2013 0x9501f0. */
AKRESULT CAkInterpolationTable::Init( AkReal32 ( *in_pfCalcFunc )( AkReal64 ), AkReal32 in_fMinValue, AkReal32 in_fMaxValue, AkUInt32 in_wTableSize )
{
	const AkReal64 dblScale = ( in_fMaxValue - in_fMinValue ) / (AkReal64)( in_wTableSize - 1 );
	m_fOneOnScale = (AkReal32)( 1.0 / dblScale );
	m_ConversionTable = (AkReal32 *)AK::MemoryMgr::Malloc( g_DefaultPoolId, in_wTableSize * sizeof( AkReal32 ) );
	if( !m_ConversionTable )
	{
		return AK_InsufficientMemory;
	}
	for( AkUInt32 i = 0; i < in_wTableSize; ++i )
	{
		m_ConversionTable[ i ] = in_pfCalcFunc( (AkReal64)i * dblScale + in_fMinValue );
	}
	m_fMaxValue = in_fMaxValue;
	m_fMinValue = in_fMinValue;
	m_wTableSize = in_wTableSize;
	return AK_Success;
}

/** 2012 0x953ac0; 2013 0x9502c0. */
void CAkInterpolationTable::Term()
{
	if( m_ConversionTable )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_ConversionTable );
		m_ConversionTable = NULL;
	}
}

/** 2012 0x953ae0; 2013 0x9502e0. */
AkReal32 CAkInterpolationTable::GetNoCheck( AkReal32 in_fKey )
{
	const AkReal32 fIndex = ( in_fKey - m_fMinValue ) * m_fOneOnScale;
	const AkUInt32 uIndex = (AkUInt32)(AkInt64)fIndex;
	const AkReal32 * pEntry = &m_ConversionTable[ uIndex ];
	return ( fIndex - (AkReal32)uIndex ) * ( pEntry[ 1 ] - pEntry[ 0 ] ) + pEntry[ 0 ];
}

/** 2012 0x953c90; 2013 0x950490. */
AkReal32 CAkInterpolationTable::Get( AkReal32 in_fKey )
{
	if( in_fKey >= m_fMaxValue )
	{
		return m_ConversionTable[ m_wTableSize - 1 ];
	}
	if( m_fMinValue < in_fKey )
	{
		return GetNoCheck( in_fKey );
	}
	return m_ConversionTable[ 0 ];
}

/** 2012 0x934170; 2013 0x930a70. */
AKRESULT CAkInterpolation::Init( AkUInt32 in_wTableSize, AkUInt32 in_wdBTableSize, AkITable in_eITable )
{
	if( ( in_eITable & AkITable_Exp1 ) && m_tableExp1.Init( CalculationFunction::AkExp1, 0.f, 1.f, in_wTableSize ) != AK_Success )
	{
		return AK_Fail;
	}
	if( ( in_eITable & AkITable_Exp2 ) && m_tableSine.Init( CalculationFunction::AkSine, 0.f, 1.f, in_wTableSize ) != AK_Success )
	{
		return AK_Fail;
	}
	if( ( in_eITable & AkITable_Exp3 ) && m_tableExp3.Init( CalculationFunction::AkExp3, 0.f, 1.f, in_wTableSize ) != AK_Success )
	{
		return AK_Fail;
	}
	if( m_tableDBtoReal.Init( CalculationFunction::dBToReal, -96.3f, 96.3f, in_wdBTableSize ) != AK_Success
		|| m_tableRealToDB.Init( CalculationFunction::realTodB, 1.f, 10.f, in_wdBTableSize ) != AK_Success
		|| m_tableLinearMutingTodBMuting96.Init( CalculationFunction::linearMutingTodBMuting96, -96.3f, 96.3f, in_wdBTableSize ) != AK_Success
		|| m_tableLinearToFrequency_20_20000.Init( CalculationFunction::linearToFrequency_20_20000, 20.f, 20000.f, in_wdBTableSize ) != AK_Success )
	{
		return AK_Fail;
	}
	return AK_Success;
}

/** 2012 0x9342d0; 2013 0x930bd0. */
void CAkInterpolation::Term()
{
	m_tableExp1.Term();
	m_tableSine.Term();
	m_tableExp3.Term();
	m_tableDBtoReal.Term();
	m_tableRealToDB.Term();
	m_tableLinearMutingTodBMuting96.Term();
	m_tableLinearToFrequency_20_20000.Term();
}

/** 2012 0x934310; 2013 0x930c10. */
AkReal32 CAkInterpolation::InterpolateNoCheck( AkReal32 in_fTimeRatio, AkReal32 in_fInitialVal, AkReal32 in_fTargetVal, AkCurveInterpolation in_eFadeCurve )
{
	const AkReal32 fRange = in_fTargetVal - in_fInitialVal;
	switch( in_eFadeCurve )
	{
	case AkCurveInterpolation_Log3:
		return ( 1.f - m_tableExp3.GetNoCheck( 1.f - in_fTimeRatio ) ) * fRange + in_fInitialVal;
	case AkCurveInterpolation_Sine:
		return ( 1.f - m_tableSine.GetNoCheck( 1.f - in_fTimeRatio ) ) * fRange + in_fInitialVal;
	case AkCurveInterpolation_Log1:
		return ( 1.f - m_tableExp1.GetNoCheck( 1.f - in_fTimeRatio ) ) * fRange + in_fInitialVal;
	case AkCurveInterpolation_InvSCurve:
		if( in_fTimeRatio >= 0.5f )
		{
			return ( m_tableSine.GetNoCheck( ( in_fTimeRatio - 0.5f ) * 2.f ) * 0.5f + 0.5f ) * fRange + in_fInitialVal;
		}
		return ( 1.f - m_tableSine.GetNoCheck( 1.f - in_fTimeRatio * 2.f ) ) * 0.5f * fRange + in_fInitialVal;
	case AkCurveInterpolation_Linear:
		return in_fTimeRatio * fRange + in_fInitialVal;
	case AkCurveInterpolation_SCurve:
		if( in_fTimeRatio >= 0.5f )
		{
			return ( ( 1.f - m_tableSine.GetNoCheck( 1.f - ( in_fTimeRatio - 0.5f ) * 2.f ) ) * 0.5f + 0.5f ) * fRange + in_fInitialVal;
		}
		return m_tableSine.GetNoCheck( in_fTimeRatio * 2.f ) * 0.5f * fRange + in_fInitialVal;
	case AkCurveInterpolation_Exp1:
		return m_tableExp1.GetNoCheck( in_fTimeRatio ) * fRange + in_fInitialVal;
	case AkCurveInterpolation_SineRecip:
		return m_tableSine.GetNoCheck( in_fTimeRatio ) * fRange + in_fInitialVal;
	case AkCurveInterpolation_Exp3:
		return m_tableExp3.GetNoCheck( in_fTimeRatio ) * fRange + in_fInitialVal;
	default:
		return 0.f * fRange + in_fInitialVal;
	}
}

/** 2012 0x934590; 2013 0x930e90. */
AkReal32 CAkInterpolation::RealTodB( AkReal32 in_fReal )
{
	if( in_fReal <= 0.f )
	{
		return -96.3f;
	}
	AkReal32 fCorrector = 0.f;
	while( in_fReal > 10.f )
	{
		in_fReal *= 0.1f;
		fCorrector += 20.f;
	}
	while( in_fReal < 1.f )
	{
		in_fReal *= 10.f;
		fCorrector -= 20.f;
	}
	return m_tableRealToDB.GetNoCheck( in_fReal ) + fCorrector;
}

/** 2012 0x934630; 2013 0x930f30. */
AkReal32 CAkInterpolation::InterpolateDBNoCheck( AkReal32 in_fTimeRatio, AkReal32 in_fInitialVal, AkReal32 in_fTargetVal, AkCurveInterpolation in_eFadeCurve )
{
	return RealTodB( InterpolateNoCheck( in_fTimeRatio, in_fInitialVal, in_fTargetVal, in_eFadeCurve ) );
}
