/*=============================================================================
	AkMath.cpp - port of akmath.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). dBToLinTable is the 98-float table at 2012 .rdata 0xd8d788 (10^((i-97)/20),
	i = 0..97, copied as raw IEEE words). FastLog10 keeps retail's mantissa extraction (bits | 0x3F800000, no mask of the
	exponent first) and dBToLin its round-to-nearest index (fistp under the default control word).
=============================================================================*/
#include "AkMath.h"
#include <math.h>
#include <xmmintrin.h>

namespace
{
	const AkUInt32 dBToLinTable[ 98 ] =
	{
	0x376cfc22, 0x3784f341, 0x37952c46, 0x37a75fe2, 0x37bbcc25, 0x37d2b66d, 0x37ec6c40, 0x3804a2b9,
	0x3814d1c1, 0x3826fa6a, 0x383b5a54, 0x3852368b, 0x386bdce8, 0x3884523f, 0x38947790, 0x38a69529,
	0x38bae8ac, 0x38d1b717, 0x38eb4de2, 0x39040203, 0x39141d87, 0x39263026, 0x393a7750, 0x395137e8,
	0x396abf36, 0x3983b1f7, 0x3993c3b2, 0x39a5cb60, 0x39ba0640, 0x39d0b90b, 0x39ea30dc, 0x3a03621b,
	0x3a136a16, 0x3a2566d4, 0x3a399570, 0x3a503a77, 0x3a69a2d6, 0x3a83126f, 0x3a9310b2, 0x3aa50287,
	0x3ab924e5, 0x3acfbc31, 0x3ae91528, 0x3b02c2f2, 0x3b12b782, 0x3b249e76, 0x3b38b49e, 0x3b4f3e37,
	0x3b6887cf, 0x3b8273a6, 0x3b925e89, 0x3ba43aa2, 0x3bb8449c, 0x3bcec08a, 0x3be7facc, 0x3c02248a,
	0x3c1205c6, 0x3c23d70a, 0x3c37d4dd, 0x3c4e4329, 0x3c676e1e, 0x3c81d59f, 0x3c91ad39, 0x3ca373af,
	0x3cb76563, 0x3ccdc614, 0x3ce6e1c6, 0x3d0186e2, 0x3d1154e1, 0x3d231090, 0x3d36f62b, 0x3d4d494a,
	0x3d6655c3, 0x3d813856, 0x3d90fcbf, 0x3da2adad, 0x3db68738, 0x3dcccccd, 0x3de5ca15, 0x3e00e9f9,
	0x3e10a4d3, 0x3e224b06, 0x3e361887, 0x3e4c509b, 0x3e653ebb, 0x3e809bcc, 0x3e904d1c, 0x3ea1e89b,
	0x3eb5aa1a, 0x3ecbd4b4, 0x3ee4b3b6, 0x3f004dce, 0x3f0ff59a, 0x3f21866c, 0x3f353bef, 0x3f4b5918,
	0x3f642905, 0x3f800000
	};

	inline AkReal32 TableValue( AkInt32 in_iIndex )
	{
		union { AkUInt32 u; AkReal32 f; } v;
		v.u = dBToLinTable[ in_iIndex ];
		return v.f;
	}
}

namespace AkMath
{
	/** 2012 0x91b870; 2013 0x918050. */
	AkReal32 ToRadians( AkReal32 in_fDegrees )
	{
		return ( in_fDegrees - (AkReal32)(AkInt32)( in_fDegrees * 0.0027777778f ) ) * 0.017453292f;
	}

	/** 2012 0x91b8a0; 2013 0x918080. */
	AkReal32 Magnitude( AkVector & in_rVector )
	{
		return sqrtf( in_rVector.Z * in_rVector.Z + in_rVector.Y * in_rVector.Y + in_rVector.X * in_rVector.X );
	}

	/** 2012 0x91b8c0; 2013 0x9180a0. */
	AkReal32 DotProduct( const AkVector & Vector1, const AkVector & Vector2 )
	{
		return Vector1.Z * Vector2.Z + Vector1.Y * Vector2.Y + Vector1.X * Vector2.X;
	}

	/** 2012 0x91b8e0; 2013 0x9180c0. */
	AkVector CrossProduct( AkVector & Vector1, AkVector & Vector2 )
	{
		AkVector result;
		result.X = Vector2.Z * Vector1.Y - Vector2.Y * Vector1.Z;
		result.Y = Vector2.X * Vector1.Z - Vector1.X * Vector2.Z;
		result.Z = Vector1.X * Vector2.Y - Vector2.X * Vector1.Y;
		return result;
	}

	/** 2012 0x91b950; 2013 0x918130. */
	AkReal32 Distance( const AkVector & Position1, const AkVector & Position2 )
	{
		const AkReal32 fZ = Position1.Z - Position2.Z;
		const AkReal32 fY = Position1.Y - Position2.Y;
		const AkReal32 fX = Position1.X - Position2.X;
		return sqrtf( fX * fX + ( fY * fY + fZ * fZ ) );
	}

	/** 2012 0x91b980; 2013 0x918160. */
	AkReal32 Interpolate( AkReal32 in_fLowerX, AkReal32 in_fLowerY, AkReal32 in_fUpperX, AkReal32 in_fUpperY, AkReal32 in_fX )
	{
		if( in_fLowerX >= in_fX )
		{
			return in_fLowerY;
		}
		if( in_fX < in_fUpperX )
		{
			return ( ( in_fX - in_fLowerX ) / ( in_fUpperX - in_fLowerX ) ) * ( in_fUpperY - in_fLowerY ) + in_fLowerY;
		}
		return in_fUpperY;
	}

	/** 2012 0x91b9f0; 2013 0x9181d0. */
	void MatrixMul3by3( AkReal32 * pA, AkReal32 * pB, AkReal32 * pC )
	{
		pC[ 0 ] = pA[ 0 ] * pB[ 0 ] + pA[ 1 ] * pB[ 3 ] + pB[ 6 ] * pA[ 2 ];
		pC[ 1 ] = pA[ 1 ] * pB[ 4 ] + pB[ 7 ] * pA[ 2 ] + pB[ 1 ] * pA[ 0 ];
		pC[ 2 ] = pA[ 1 ] * pB[ 5 ] + pB[ 8 ] * pA[ 2 ] + pB[ 2 ] * pA[ 0 ];
		pC[ 3 ] = pA[ 4 ] * pB[ 3 ] + pA[ 3 ] * pB[ 0 ] + pA[ 5 ] * pB[ 6 ];
		pC[ 4 ] = pA[ 5 ] * pB[ 7 ] + pA[ 4 ] * pB[ 4 ] + pA[ 3 ] * pB[ 1 ];
		pC[ 5 ] = pA[ 5 ] * pB[ 8 ] + pA[ 4 ] * pB[ 5 ] + pA[ 3 ] * pB[ 2 ];
		pC[ 6 ] = pA[ 6 ] * pB[ 0 ] + pB[ 6 ] * pA[ 8 ] + pA[ 7 ] * pB[ 3 ];
		pC[ 7 ] = pB[ 7 ] * pA[ 8 ] + pB[ 4 ] * pA[ 7 ] + pB[ 1 ] * pA[ 6 ];
		pC[ 8 ] = pB[ 8 ] * pA[ 8 ] + pB[ 5 ] * pA[ 7 ] + pB[ 2 ] * pA[ 6 ];
	}

	/** 2012 0x91bb80; 2013 0x918360. */
	AkReal32 dBToLin( AkReal32 in_fdB )
	{
		if( in_fdB <= -96.3f )
		{
			return 0.f;
		}
		if( in_fdB >= 0.f )
		{
			return 1.f;
		}
		const AkReal32 fInterpLoc = in_fdB + 97.f;
		const AkInt32 ulLeftIndex = _mm_cvtss_si32( _mm_set_ss( fInterpLoc - 0.5f ) );
		const AkReal32 fFrac = fInterpLoc - (AkReal32)(AkUInt32)ulLeftIndex;
		return fFrac * TableValue( ulLeftIndex + 1 ) + ( 1.f - fFrac ) * TableValue( ulLeftIndex );
	}

	/** 2012 0x91bc00; 2013 0x9183e0. */
	AkReal32 FastLog10( AkReal32 fX )
	{
		union { AkReal32 f; AkUInt32 u; } in, mantissa;
		in.f = fX;
		mantissa.u = in.u | 0x3F800000;
		const AkReal32 fZ = ( mantissa.f - 1.f ) / ( mantissa.f + 1.f );
		const AkReal32 fLog2Mantissa = fZ * ( fZ * fZ * 0.33333334f + 1.f );
		const AkInt32 iExponent = (AkInt32)( ( in.u >> 23 ) & 0xFF );
		return ( fLog2Mantissa + fLog2Mantissa + ( (AkReal32)iExponent - 127.f ) * 0.69314718f ) * 0.43429449f;
	}

	/** 2012 0x91bc70; 2013 0x918450. */
	void Normalise( AkVector & io_Vector )
	{
		const AkReal32 fMagnitude = sqrtf( io_Vector.Z * io_Vector.Z + io_Vector.Y * io_Vector.Y + io_Vector.X * io_Vector.X );
		if( fMagnitude != 0.f )
		{
			io_Vector.X = ( 1.f / fMagnitude ) * io_Vector.X;
			io_Vector.Y = ( 1.f / fMagnitude ) * io_Vector.Y;
			io_Vector.Z = ( 1.f / fMagnitude ) * io_Vector.Z;
		}
	}
}
