/*=============================================================================
	AkSpeakerPan.cpp - port of akspeakerpan.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). Deviations without a change of result:
	  * The 2D-pan scratch AkSpeakerVolumes is zero-initialised (retail multiplies the two unused SIMD lanes of each
	    channel by stack garbage).
	  * GetSpeakerVolumesdB's headphone branch takes the square root of the six speaker lanes only (retail's two
	    _mm_sqrt_ps also touch the two padding lanes of the caller's AkSIMDSpeakerVolumes); every lane but FL/FR is then
	    overwritten with -97 dB as in retail.
	s_uSpreadChannelOrder is the uint16 table retail indexes at 2012 .rdata 0xe371a2 + 10 * channels (5 entries per row,
	read back from the database: 1 [0], 2 [0 1], 3 [0 2 1], 4 [2 0 1 3], 5 [3 0 2 1 4]).
=============================================================================*/
#include "AkSpeakerPan.h"
#include "AkLEngine.h"
#include "AkMath.h"
#include <math.h>
#include <string.h>
#include <xmmintrin.h>

AkReal32 CAkSpeakerPan::m_fSin2[ 256 ];

namespace
{
	const AkUInt16 s_uSpreadChannelOrder[ 6 ][ 5 ] =
	{
		{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },
		{ 0, 1, 0, 0, 0 },
		{ 0, 2, 1, 0, 0 },
		{ 2, 0, 1, 3, 0 },
		{ 3, 0, 2, 1, 4 }
	};

	const AkReal32 ONE_OVER_SQRT_OF_TWO = 0.70710677f;
	const AkReal32 SILENT_DB = -97.f;
	const AkReal32 RADIANS_TO_PAN_ANGLE = 81.487328f;

	inline void MulVolumes( AkSIMDSpeakerVolumes & io_rVolumes, const AkSIMDSpeakerVolumes & in_rGains )
	{
		io_rVolumes.vector[ 0 ] = _mm_mul_ps( in_rGains.vector[ 0 ], io_rVolumes.vector[ 0 ] );
		io_rVolumes.vector[ 1 ] = _mm_mul_ps( io_rVolumes.vector[ 1 ], in_rGains.vector[ 1 ] );
	}

	inline AkReal32 LinToDB( AkReal32 in_fValue )
	{
		return AkMath::FastLog10( in_fValue ) * 20.f;
	}
}

/** 2012 0x948260; 2013 0x944ab0. */
void CAkSpeakerPan::Init()
{
	for( AkUInt32 i = 0; i < 256; ++i )
	{
		const AkReal32 fSin = (AkReal32)sin( (AkReal64)i * 0.01227184664458036 );
		m_fSin2[ i ] = fSin * fSin;
	}
}

/** 2012 0x948290; 2013 0x944ae0. in_iAngle 0..511 is the full circle. */
void CAkSpeakerPan::AddSpeakerVolumesPower( AkInt32 in_iAngle, AkReal32 in_fDivergenceCenter, AkSpeakerVolumes * out_pVolumes )
{
	if( CAkLEngine::m_panningRule == AkPanningRule_Headphones )
	{
		const AkInt32 iAngle = in_iAngle >= 256 ? 512 - in_iAngle : in_iAngle;
		const AkReal32 fGain = iAngle < 256 ? m_fSin2[ iAngle / 2 ] : 1.f;
		out_pVolumes->fFrontLeft += fGain;
		out_pVolumes->fFrontRight = ( 1.f - fGain ) + out_pVolumes->fFrontRight;
		return;
	}
	if( CAkLEngine::m_panningRule != AkPanningRule_Speakers )
	{
		return;
	}
	if( in_iAngle < 256 )
	{
		const AkReal32 fGain = m_fSin2[ in_iAngle ];
		const AkReal32 fComplement = 1.f - fGain;
		if( in_iAngle < 128 )
		{
			if( in_fDivergenceCenter <= 0.f )
			{
				out_pVolumes->fFrontLeft += fGain;
				out_pVolumes->fFrontRight += fComplement;
			}
			else
			{
				const AkReal32 fCenterGain = m_fSin2[ 2 * in_iAngle ];
				out_pVolumes->fCenter = fCenterGain * in_fDivergenceCenter + out_pVolumes->fCenter;
				const AkReal32 fCenterComplement = ( 1.f - fCenterGain ) * in_fDivergenceCenter;
				const AkReal32 fLeft = ( 1.f - in_fDivergenceCenter ) * fGain;
				const AkReal32 fRight = ( 1.f - in_fDivergenceCenter ) * fComplement;
				if( in_iAngle < 64 )
				{
					out_pVolumes->fFrontLeft = fLeft + out_pVolumes->fFrontLeft;
					out_pVolumes->fFrontRight = ( fRight + fCenterComplement ) + out_pVolumes->fFrontRight;
				}
				else
				{
					out_pVolumes->fFrontRight = fRight + out_pVolumes->fFrontRight;
					out_pVolumes->fFrontLeft = ( fLeft + fCenterComplement ) + out_pVolumes->fFrontLeft;
				}
			}
		}
		else
		{
			out_pVolumes->fFrontLeft += fGain;
			out_pVolumes->fRearLeft += fComplement;
		}
	}
	else
	{
		const AkReal32 fGain = m_fSin2[ in_iAngle - 256 ];
		const AkReal32 fComplement = 1.f - fGain;
		if( in_iAngle < 384 )
		{
			out_pVolumes->fRearLeft += fComplement;
		}
		else
		{
			out_pVolumes->fFrontRight += fComplement;
		}
		out_pVolumes->fRearRight += fGain;
	}
}

/** 2012 0x948420; 2013 0x944c70. */
void CAkSpeakerPan::_GetSpeakerVolumes2DPan( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fCenterPct, AkPanningRule in_panningRule, AkSpeakerVolumes * out_pVolumes )
{
	AkReal32 fLeft;
	AkReal32 fRight;
	if( in_panningRule == AkPanningRule_Speakers )
	{
		const AkReal32 fRearFactor = 1.f - in_fY;
		const AkReal32 fFrontLeft = ( 1.f - in_fX ) * in_fY;
		AkReal32 fCenter;
		fLeft = fFrontLeft;
		fRight = in_fX * in_fY;
		if( in_fCenterPct <= 0.f )
		{
			fCenter = 0.f;
		}
		else if( in_fX < 0.5f )
		{
			fCenter = in_fX * in_fY * 2.f * in_fCenterPct;
			fLeft = ( 1.f - in_fX * 2.f ) * in_fY * in_fCenterPct + ( 1.f - in_fCenterPct ) * fFrontLeft;
			fRight = ( 1.f - in_fCenterPct ) * fRight;
		}
		else
		{
			fCenter = fFrontLeft * 2.f * in_fCenterPct;
			fLeft = ( 1.f - in_fCenterPct ) * fFrontLeft;
			fRight = ( in_fX - 0.5f ) * 2.f * in_fY * in_fCenterPct + ( 1.f - in_fCenterPct ) * fRight;
		}
		out_pVolumes->fCenter = sqrtf( fCenter );
		out_pVolumes->fRearLeft = sqrtf( ( 1.f - in_fX ) * fRearFactor );
		out_pVolumes->fRearRight = sqrtf( fRearFactor * in_fX );
		out_pVolumes->fLfe = 0.f;
	}
	else if( in_panningRule == AkPanningRule_Headphones )
	{
		fRight = in_fX;
		fLeft = 1.f - in_fX;
		out_pVolumes->fRearLeft = 0.f;
		out_pVolumes->fRearRight = 0.f;
		out_pVolumes->fCenter = 0.f;
		out_pVolumes->fLfe = 0.f;
	}
	else
	{
		fLeft = out_pVolumes->fFrontLeft;
		fRight = out_pVolumes->fFrontRight;
	}
	out_pVolumes->fFrontLeft = sqrtf( fLeft );
	out_pVolumes->fFrontRight = sqrtf( fRight );
}

/** 2012 0x9485a0; 2013 0x944df0. */
void CAkSpeakerPan::_GetSpeakerVolumes2DPanSourceHasCenter( AkReal32 in_fX, AkReal32 in_fY, AkSpeakerVolumes * out_pVolumes )
{
	const AkReal32 fFrontLeft = ( 2.f - in_fX * 2.f ) * 0.33333334f;
	const AkReal32 fRear = in_fY < 0.5f ? 1.f - in_fY * 1.2f : ( 4.f - in_fY * 4.f ) * 0.2f;
	const AkReal32 fFront = 1.f - fRear;
	out_pVolumes->fLfe = 0.f;
	out_pVolumes->fCenter = sqrtf( fFront * 0.33333334f );
	out_pVolumes->fFrontLeft = sqrtf( fFront * fFrontLeft );
	out_pVolumes->fFrontRight = sqrtf( fFront * ( 1.f - ( fFrontLeft + 0.33333334f ) ) );
	out_pVolumes->fRearLeft = sqrtf( ( 1.f - in_fX ) * fRear );
	out_pVolumes->fRearRight = sqrtf( fRear * in_fX );
}

/** 2012 0x948680; 2013 0x944ed0. */
void CAkSpeakerPan::GetSpeakerVolumesdB( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fDivergenceCenter, AkSpeakerVolumes * out_pVolumes )
{
	const AkReal32 fDistanceSqr = in_fX * in_fX + in_fY * in_fY;
	if( CAkLEngine::m_panningRule == AkPanningRule_Headphones )
	{
		if( fDistanceSqr == 0.f )
		{
			out_pVolumes->fFrontLeft = -3.f;
			out_pVolumes->fFrontRight = -3.f;
		}
		else
		{
			AkReal32 fCos = (AkReal32)( in_fX / sqrt( fDistanceSqr ) );
			if( fCos < -1.f )
			{
				fCos = -1.f;
			}
			else if( fCos > 1.f )
			{
				fCos = 1.f;
			}
			out_pVolumes->fFrontLeft = 0.f;
			out_pVolumes->fFrontRight = 0.f;
			AddSpeakerVolumesPower( (AkInt32)( acos( fCos ) * RADIANS_TO_PAN_ANGLE + 0.5 ), in_fDivergenceCenter, out_pVolumes );
			out_pVolumes->fFrontLeft = LinToDB( sqrtf( out_pVolumes->fFrontLeft ) );
			out_pVolumes->fFrontRight = LinToDB( sqrtf( out_pVolumes->fFrontRight ) );
		}
		out_pVolumes->fLfe = SILENT_DB;
		out_pVolumes->fCenter = SILENT_DB;
		out_pVolumes->fRearLeft = SILENT_DB;
		out_pVolumes->fRearRight = SILENT_DB;
		return;
	}
	if( CAkLEngine::m_panningRule != AkPanningRule_Speakers )
	{
		return;
	}
	out_pVolumes->fLfe = SILENT_DB;
	out_pVolumes->fCenter = SILENT_DB;
	if( fDistanceSqr == 0.f )
	{
		out_pVolumes->fFrontLeft = -6.f;
		out_pVolumes->fFrontRight = -6.f;
		out_pVolumes->fRearLeft = -6.f;
		out_pVolumes->fRearRight = -6.f;
		return;
	}
	const AkReal32 fInvDistance = (AkReal32)( 1.0 / sqrt( fDistanceSqr ) );
	const AkReal32 fX = fInvDistance * in_fX;
	const AkReal32 fY = fInvDistance * in_fY;
	if( in_fX <= 0.f )
	{
		if( in_fY > 0.f )
		{
			out_pVolumes->fFrontRight = SILENT_DB;
			out_pVolumes->fFrontLeft = LinToDB( fY );
			out_pVolumes->fRearLeft = LinToDB( -fX );
			out_pVolumes->fRearRight = SILENT_DB;
			return;
		}
		out_pVolumes->fFrontLeft = SILENT_DB;
		out_pVolumes->fFrontRight = SILENT_DB;
		out_pVolumes->fRearLeft = LinToDB( -fX );
	}
	else
	{
		if( in_fY > 0.f )
		{
			if( in_fDivergenceCenter <= 0.f )
			{
				out_pVolumes->fFrontLeft = LinToDB( fY );
				out_pVolumes->fFrontRight = LinToDB( fX );
			}
			else
			{
				const AkReal32 fNonCenter = 1.f - in_fDivergenceCenter;
				const AkReal32 fCos2 = ( fX * fX * 2.f - 1.f ) * ( fX * fX * 2.f - 1.f );
				const AkReal32 fSqrtNonCenter = sqrtf( fNonCenter );
				out_pVolumes->fCenter = LinToDB( sqrtf( 1.f - fCos2 ) * sqrtf( in_fDivergenceCenter ) );
				if( in_fX <= in_fY )
				{
					out_pVolumes->fFrontLeft = LinToDB( sqrtf( fNonCenter * fY * fY + fCos2 * in_fDivergenceCenter ) );
					out_pVolumes->fFrontRight = LinToDB( fSqrtNonCenter * fX );
				}
				else
				{
					out_pVolumes->fFrontLeft = LinToDB( fSqrtNonCenter * fY );
					out_pVolumes->fFrontRight = LinToDB( sqrtf( fNonCenter * fX * fX + fCos2 * in_fDivergenceCenter ) );
				}
			}
			out_pVolumes->fRearLeft = SILENT_DB;
			out_pVolumes->fRearRight = SILENT_DB;
			return;
		}
		out_pVolumes->fFrontLeft = SILENT_DB;
		out_pVolumes->fRearLeft = SILENT_DB;
		out_pVolumes->fFrontRight = LinToDB( fX );
	}
	out_pVolumes->fRearRight = LinToDB( -fY );
}

/** 2012 0x948aa0; 2013 0x9452f0. */
void CAkSpeakerPan::GetSpeakerVolumes2DPan( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fCenterPct, bool in_bIsPannerEnabled, AkChannelMask in_uChannelMask, AkSIMDSpeakerVolumes * out_pVolumes )
{
	AkSIMDSpeakerVolumes gains;
	gains.Zero();
	const AkPanningRule ePanningRule = CAkLEngine::m_panningRule;
	switch( in_uChannelMask & ~AK_SPEAKER_LOW_FREQUENCY )
	{
	case AK_SPEAKER_SETUP_STEREO:
		memset( out_pVolumes, 0, 2 * sizeof( AkSIMDSpeakerVolumes ) );
		out_pVolumes[ 0 ].volumes.fFrontLeft = 1.f;
		out_pVolumes[ 1 ].volumes.fFrontRight = 1.f;
		if( in_bIsPannerEnabled )
		{
			out_pVolumes[ 0 ].volumes.fRearLeft = 1.f;
			out_pVolumes[ 1 ].volumes.fRearRight = 1.f;
			_GetSpeakerVolumes2DPan( in_fX, in_fY, 0.f, ePanningRule, &gains.volumes );
			MulVolumes( out_pVolumes[ 0 ], gains );
			MulVolumes( out_pVolumes[ 1 ], gains );
		}
		return;
	case AK_SPEAKER_FRONT_CENTER:
		if( in_bIsPannerEnabled )
		{
			_GetSpeakerVolumes2DPan( in_fX, in_fY, in_fCenterPct, ePanningRule, &out_pVolumes->volumes );
		}
		else
		{
			_GetSpeakerVolumes2DPan( 0.5f, 1.f, in_fCenterPct, ePanningRule, &out_pVolumes->volumes );
		}
		return;
	case AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_FRONT_CENTER:
		memset( out_pVolumes, 0, 3 * sizeof( AkSIMDSpeakerVolumes ) );
		out_pVolumes[ 0 ].volumes.fFrontLeft = 1.f;
		out_pVolumes[ 1 ].volumes.fFrontRight = 1.f;
		if( ePanningRule == AkPanningRule_Speakers )
		{
			out_pVolumes[ 2 ].volumes.fCenter = 1.f;
		}
		else if( ePanningRule == AkPanningRule_Headphones )
		{
			out_pVolumes[ 2 ].volumes.fFrontLeft = 0.5f;
			out_pVolumes[ 2 ].volumes.fFrontRight = 0.5f;
		}
		if( in_bIsPannerEnabled )
		{
			if( ePanningRule == AkPanningRule_Speakers )
			{
				out_pVolumes[ 0 ].volumes.fRearLeft = 1.f;
				out_pVolumes[ 1 ].volumes.fRearRight = 1.f;
				out_pVolumes[ 2 ].volumes.fRearLeft = 0.5f;
				out_pVolumes[ 2 ].volumes.fRearRight = 0.5f;
				_GetSpeakerVolumes2DPanSourceHasCenter( in_fX, in_fY, &gains.volumes );
				MulVolumes( out_pVolumes[ 0 ], gains );
				MulVolumes( out_pVolumes[ 1 ], gains );
				MulVolumes( out_pVolumes[ 2 ], gains );
			}
			else if( ePanningRule == AkPanningRule_Headphones )
			{
				_GetSpeakerVolumes2DPanSourceHasCenter( in_fX, in_fY, &gains.volumes );
				const AkSpeakerVolumes & v = gains.volumes;
				const AkReal32 fRearLeftHalf = v.fRearLeft * v.fRearLeft * 0.5f;
				out_pVolumes[ 0 ].volumes.fFrontLeft = sqrtf( v.fFrontLeft * v.fFrontLeft + fRearLeftHalf );
				const AkReal32 fRearRightHalf = 0.5f * ( v.fRearRight * v.fRearRight );
				out_pVolumes[ 1 ].volumes.fFrontRight = sqrtf( v.fFrontRight * v.fFrontRight + fRearRightHalf );
				const AkReal32 fCenter2 = v.fCenter * v.fCenter;
				out_pVolumes[ 2 ].volumes.fFrontRight = sqrtf( fRearRightHalf + fCenter2 ) * ONE_OVER_SQRT_OF_TWO;
				out_pVolumes[ 2 ].volumes.fFrontLeft = ONE_OVER_SQRT_OF_TWO * sqrtf( fRearLeftHalf + fCenter2 );
			}
		}
		return;
	case AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT:
		memset( out_pVolumes, 0, 4 * sizeof( AkSIMDSpeakerVolumes ) );
		out_pVolumes[ 0 ].volumes.fFrontLeft = 1.f;
		out_pVolumes[ 1 ].volumes.fFrontRight = 1.f;
		if( ePanningRule == AkPanningRule_Speakers )
		{
			out_pVolumes[ 2 ].volumes.fRearLeft = 1.f;
			out_pVolumes[ 3 ].volumes.fRearRight = 1.f;
		}
		else if( ePanningRule == AkPanningRule_Headphones )
		{
			out_pVolumes[ 2 ].volumes.fFrontLeft = ONE_OVER_SQRT_OF_TWO;
			out_pVolumes[ 3 ].volumes.fFrontRight = ONE_OVER_SQRT_OF_TWO;
		}
		if( in_bIsPannerEnabled )
		{
			_GetSpeakerVolumes2DPan( in_fX, in_fY, 0.f, AkPanningRule_Speakers, &gains.volumes );
			if( ePanningRule != AkPanningRule_Speakers )
			{
				out_pVolumes[ 0 ].volumes.fFrontLeft = gains.volumes.fFrontLeft;
				out_pVolumes[ 1 ].volumes.fFrontRight = gains.volumes.fFrontRight;
				out_pVolumes[ 2 ].volumes.fFrontLeft = gains.volumes.fRearLeft * ONE_OVER_SQRT_OF_TWO;
				out_pVolumes[ 3 ].volumes.fFrontRight = gains.volumes.fRearRight * ONE_OVER_SQRT_OF_TWO;
			}
			else
			{
				MulVolumes( out_pVolumes[ 0 ], gains );
				MulVolumes( out_pVolumes[ 1 ], gains );
				MulVolumes( out_pVolumes[ 2 ], gains );
				MulVolumes( out_pVolumes[ 3 ], gains );
			}
		}
		return;
	case AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_FRONT_CENTER | AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT:
		memset( out_pVolumes, 0, 5 * sizeof( AkSIMDSpeakerVolumes ) );
		out_pVolumes[ 0 ].volumes.fFrontLeft = 1.f;
		out_pVolumes[ 1 ].volumes.fFrontRight = 1.f;
		if( ePanningRule == AkPanningRule_Speakers )
		{
			out_pVolumes[ 2 ].volumes.fCenter = 1.f;
			out_pVolumes[ 3 ].volumes.fRearLeft = 1.f;
			out_pVolumes[ 4 ].volumes.fRearRight = 1.f;
		}
		else if( ePanningRule == AkPanningRule_Headphones )
		{
			out_pVolumes[ 3 ].volumes.fFrontLeft = ONE_OVER_SQRT_OF_TWO;
			out_pVolumes[ 4 ].volumes.fFrontRight = ONE_OVER_SQRT_OF_TWO;
			out_pVolumes[ 2 ].volumes.fFrontLeft = ONE_OVER_SQRT_OF_TWO;
			out_pVolumes[ 2 ].volumes.fFrontRight = ONE_OVER_SQRT_OF_TWO;
		}
		if( in_bIsPannerEnabled )
		{
			_GetSpeakerVolumes2DPanSourceHasCenter( in_fX, in_fY, &gains.volumes );
			if( ePanningRule != AkPanningRule_Speakers )
			{
				out_pVolumes[ 0 ].volumes.fFrontLeft = gains.volumes.fFrontLeft;
				out_pVolumes[ 1 ].volumes.fFrontRight = gains.volumes.fFrontRight;
				const AkReal32 fCenter = gains.volumes.fCenter * ONE_OVER_SQRT_OF_TWO;
				out_pVolumes[ 2 ].volumes.fFrontRight = fCenter;
				out_pVolumes[ 2 ].volumes.fFrontLeft = fCenter;
				out_pVolumes[ 3 ].volumes.fFrontLeft = gains.volumes.fRearLeft * ONE_OVER_SQRT_OF_TWO;
				out_pVolumes[ 4 ].volumes.fFrontRight = gains.volumes.fRearRight * ONE_OVER_SQRT_OF_TWO;
			}
			else
			{
				for( AkUInt32 i = 0; i < 5; ++i )
				{
					MulVolumes( out_pVolumes[ i ], gains );
				}
			}
		}
		return;
	default:
		return;
	}
}

/** 2012 0x949030; 2013 0x945880. One dB volume set per full-band channel, spread over the circle around the direction. */
void CAkSpeakerPan::GetSpeakerVolumes( AkReal32 in_fX, AkReal32 in_fY, AkReal32 in_fDivergenceCenter, AkReal32 in_fSpread, AkSIMDSpeakerVolumes * out_pVolumes, AkUInt32 in_uNumFullBandChannels )
{
	if( !in_uNumFullBandChannels )
	{
		return;
	}
	if( in_fSpread == 0.f )
	{
		GetSpeakerVolumesdB( in_fX, in_fY, in_fDivergenceCenter, &out_pVolumes->volumes );
		if( in_uNumFullBandChannels > 1 )
		{
			const __m128 vAttenuation = _mm_set1_ps( AkMath::FastLog10( (AkReal32)( 1.0 / (AkReal64)in_uNumFullBandChannels ) ) * 10.f );
			out_pVolumes->vector[ 0 ] = _mm_add_ps( out_pVolumes->vector[ 0 ], vAttenuation );
			out_pVolumes->vector[ 1 ] = _mm_add_ps( out_pVolumes->vector[ 1 ], vAttenuation );
			for( AkUInt32 i = 1; i < in_uNumFullBandChannels; ++i )
			{
				out_pVolumes[ i ] = out_pVolumes[ 0 ];
			}
		}
		return;
	}
	AkReal32 fAngle;
	if( in_fX == 0.f )
	{
		if( in_fY > 0.f )
		{
			fAngle = 1.5707964f;
		}
		else if( in_fY == 0.f )
		{
			fAngle = 0.78539819f;
		}
		else
		{
			fAngle = 4.712389f;
		}
	}
	else
	{
		fAngle = (AkReal32)atan2( in_fY, in_fX );
		if( fAngle < 0.f )
		{
			fAngle = fAngle + 6.2831855f;
		}
	}
	const AkReal32 fChannelSpread = (AkReal32)( in_fSpread * 5.1199999 / (AkReal64)in_uNumFullBandChannels );
	AkReal32 fChannelAngle = (AkReal32)( (AkReal64)( in_uNumFullBandChannels >> 1 ) * fChannelSpread + fAngle * (AkReal64)RADIANS_TO_PAN_ANGLE );
	if( !( in_uNumFullBandChannels & 1 ) )
	{
		fChannelAngle = fChannelAngle - fChannelSpread * 0.5f;
	}
	const AkUInt32 uNumPoints = (AkUInt32)(AkInt64)ceil( fChannelSpread * 0.0078125 ) + 1;
	const AkReal32 fOneOverNumPoints = (AkReal32)( 1.0 / (AkReal64)( in_uNumFullBandChannels * uNumPoints ) );
	const AkReal32 fPointSpacing = (AkReal32)( fChannelSpread / (AkReal64)uNumPoints );
	AkReal32 fStartAngle = fChannelAngle;
	if( !( uNumPoints & 1 ) )
	{
		fStartAngle = fChannelAngle - fPointSpacing * 0.5f;
		fChannelAngle = fStartAngle;
	}
	const AkReal32 fHalfPointsSpan = (AkReal32)( fPointSpacing * (AkReal64)( uNumPoints >> 1 ) );
	const __m128 vOneOverNumPoints = _mm_set1_ps( fOneOverNumPoints );
	for( AkUInt32 uChannel = 0; uChannel < in_uNumFullBandChannels; ++uChannel )
	{
		AkSIMDSpeakerVolumes & volumes = out_pVolumes[ s_uSpreadChannelOrder[ in_uNumFullBandChannels ][ uChannel ] ];
		AkReal32 fPointAngle = fHalfPointsSpan + fStartAngle;
		volumes.Zero();
		for( AkUInt32 uPoint = 0; uPoint < uNumPoints; ++uPoint )
		{
			AddSpeakerVolumesPower( (AkInt32)( fPointAngle + 0.5f ) & 0x1FF, in_fDivergenceCenter, &volumes.volumes );
			fPointAngle = fPointAngle - fPointSpacing;
		}
		volumes.vector[ 0 ] = _mm_sqrt_ps( _mm_mul_ps( vOneOverNumPoints, volumes.vector[ 0 ] ) );
		volumes.vector[ 1 ] = _mm_sqrt_ps( _mm_mul_ps( volumes.vector[ 1 ], vOneOverNumPoints ) );
		volumes.volumes.fFrontLeft = LinToDB( volumes.volumes.fFrontLeft );
		volumes.volumes.fFrontRight = LinToDB( volumes.volumes.fFrontRight );
		volumes.volumes.fCenter = LinToDB( volumes.volumes.fCenter );
		volumes.volumes.fLfe = LinToDB( volumes.volumes.fLfe );
		volumes.volumes.fRearLeft = LinToDB( volumes.volumes.fRearLeft );
		volumes.volumes.fRearRight = LinToDB( volumes.volumes.fRearRight );
		fChannelAngle = fChannelAngle - fChannelSpread;
		fStartAngle = fChannelAngle;
	}
}
