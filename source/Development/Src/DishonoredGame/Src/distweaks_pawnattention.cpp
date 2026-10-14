// DishonoredGame/src/distweaks_pawnattention.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10): the four below plus the six class-registration helpers of the two classes.
//
// ---- agent GI ports (PHASE20 GI): the pawn-attention tweaks' own code ----
// Resolved by hand in both databases (build/agentGI/idb). FDisVisCalcTuningParams' bodies live here too: retail attributes
// them to dishonoredpawn_attentiontarget.cpp's neighbour (0x7495d0 .. 0x74d6f0, UDisAttentionDefs' unit), and this is the
// first unit that needs them.

#include "DishonoredGame.h"
#include "disattentioninfo.h"

void DishonoredLinkUDisTweaks_PawnAttention()
{
}

// DISHONORED(port): 2012 rva 0x764dc0 / 2013 0x701550 (24 B)
FDisAttentionIncrease::FDisAttentionIncrease( FLOAT _fByAmount, BYTE _ToThreshold )
{
	m_fByAmount = _fByAmount;
	m_ToThreshold = _ToThreshold;
}

// DISHONORED(port): 2012 rva 0x7673e0 / 2013 0x701570 (90 B): stop + uniform( -variance, +variance ) through rand() / 32768.
FLOAT FDisAttentionLevelThreshold::CalculateStoppingThreshold() const
{
	const FLOAT fLow = -m_fStoppingVariance;
	return m_fAttentionLevelToStop + ( (FLOAT)rand() * ( m_fStoppingVariance - fLow ) * ( 1.f / 32768.f ) + fLow );
}

// DISHONORED(port): 2012 rva 0x7674a0 / 2013 0x70fc00
UBOOL UDisTweaks_PawnAttention::FixupDefaults_Derived()
{
	m_DistanceValueTuning.FixupDefaults_DisVisCalcTuningParams();
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x749470 (GetRelevantVisCalcPoints): the two table points bracketing the parameter, only the
// last when past the end, only the first when before the start.
void FDisVisCalcTuningParams::GetRelevantVisCalcPoints( FLOAT _fParam, const FDisVisCalcTuningParamsCurvePoint*& _rpOut_Point0, const FDisVisCalcTuningParamsCurvePoint*& _rpOut_Point1 ) const
{
	_rpOut_Point0 = NULL;
	_rpOut_Point1 = NULL;
	const TArray<FDisVisCalcTuningParamsCurvePoint>& Pts = Points();
	if( Pts.Num() <= 0 )
	{
		return;
	}
	if( _fParam <= Pts(0).m_fInputValue )
	{
		_rpOut_Point0 = &Pts(0);
		return;
	}
	if( _fParam >= Pts( Pts.Num() - 1 ).m_fInputValue )
	{
		_rpOut_Point1 = &Pts( Pts.Num() - 1 );
		return;
	}
	for( INT i = 1; i < Pts.Num(); i++ )
	{
		const FDisVisCalcTuningParamsCurvePoint& P0 = Pts(i - 1);
		const FDisVisCalcTuningParamsCurvePoint& P1 = Pts(i);
		if( _fParam >= P0.m_fInputValue && _fParam <= P1.m_fInputValue )
		{
			_rpOut_Point0 = &P0;
			_rpOut_Point1 = &P1;
			return;
		}
	}
}

// DISHONORED(port): 2012 rva 0x7495d0 / 2013 0x70b350 (203 B): the factor is (param^exponent + bias) * scalar, or the table's
// interpolation; op 0 adds it to the value, op 1 multiplies, anything else returns the value untouched.
FLOAT FDisVisCalcTuningParams::Apply( FLOAT _fParam, FLOAT _fApplyToThisValue ) const
{
	FLOAT fFactor = 0.f;
	if( !m_bUsePointsInsteadOfFormula )
	{
		fFactor = ( appPow( _fParam, m_fExponent ) + m_fBias ) * m_fScalar;
	}
	else
	{
		const FDisVisCalcTuningParamsCurvePoint* P0 = NULL;
		const FDisVisCalcTuningParamsCurvePoint* P1 = NULL;
		GetRelevantVisCalcPoints( _fParam, P0, P1 );
		if( P0 && P1 )
		{
			const FLOAT fRange = P1->m_fInputValue - P0->m_fInputValue;
			const FLOAT fAlpha = fRange > 0.f ? ( _fParam - P0->m_fInputValue ) / fRange : 0.f;
			fFactor = Lerp( P0->m_fOutputValue, P1->m_fOutputValue, fAlpha );
		}
		else if( P0 )
		{
			fFactor = P0->m_fOutputValue;
		}
		else if( P1 )
		{
			fFactor = P1->m_fOutputValue;
		}
	}
	switch( m_ApplyOp )
	{
	case 0: return fFactor + _fApplyToThisValue;
	case 1: return fFactor * _fApplyToThisValue;
	default: return _fApplyToThisValue;
	}
}

// DISHONORED(port): 2012 rva 0x74d6a0 / 2013 0x70eca0 (65 B): applied to the identity of the op (0 for add, 1 for multiply).
FLOAT FDisVisCalcTuningParams::Apply( FLOAT _fParam ) const
{
	return Apply( _fParam, m_ApplyOp == 1 ? 1.f : 0.f );
}

// DISHONORED(port): 2012 rva 0x74d6f0: the points sorted by input value.
IMPLEMENT_COMPARE_CONSTREF( FDisVisCalcTuningParamsCurvePoint, distweaks_pawnattention, { return A.m_fInputValue < B.m_fInputValue ? -1 : ( A.m_fInputValue > B.m_fInputValue ? 1 : 0 ); } )

void FDisVisCalcTuningParams::FixupDefaults_DisVisCalcTuningParams()
{
	TArray<FDisVisCalcTuningParamsCurvePoint>& Pts = *reinterpret_cast< TArray<FDisVisCalcTuningParamsCurvePoint>* >( m_Points );
	if( Pts.Num() > 1 )
	{
		Sort<USE_COMPARE_CONSTREF( FDisVisCalcTuningParamsCurvePoint, distweaks_pawnattention )>( &Pts(0), Pts.Num() );
	}
}
