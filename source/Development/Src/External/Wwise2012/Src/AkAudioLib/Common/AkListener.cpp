/*=============================================================================
	AkListener.cpp - CAkListener's listener state (ak3dlistener.cpp, 2012 0x934cb0..0x935120; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26). m_matRot / m_matIdentity are 2012 .data 0xe36d48 / 0xe36d6c (a 45 degree
	turn about Y for the speaker panning rule, identity for headphones); g_DefaultListenerPosition 0xe36d20 (front
	(0,0,1), top (0,1,0), origin). The 3D volume half is IH3 (see the header).
=============================================================================*/
#include "AkListener.h"
#include "AkLEngine.h"
#include "AkMath.h"

AkListener	CAkListener::m_listeners[ 8 ];
AkReal32	CAkListener::m_matRot[ 3 ][ 3 ] = { { 0.70710677f, 0.f, 0.70710677f }, { 0.f, 1.f, 0.f }, { -0.70710677f, 0.f, 0.70710677f } };
AkReal32	CAkListener::m_matIdentity[ 3 ][ 3 ] = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } };
AkUInt32	CAkListener::m_uAudioMask = 0xFFFFFFFF;
AkUInt32	CAkListener::m_uFeedbackMask = 0;

static const AkListenerPosition g_DefaultListenerPosition = { { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 0.f } };

/** 2012 0x9350a0. */
AKRESULT CAkListener::Init()
{
	for( AkUInt32 i = 0; i < 8; ++i )
	{
		m_listeners[ i ].data.VolumeOffset.Zero();
		m_listeners[ i ].data.bSpatialized = true;
		m_listeners[ i ].fScalingFactor = 1.f;
		SetListenerPosition( i, g_DefaultListenerPosition );
	}
	m_uFeedbackMask = 0;
	m_uAudioMask = 0xFFFFFFFF;
	return AK_Success;
}

/** 2012 0x935120: recomputes every matrix (after a panning rule change). */
void CAkListener::ResetListenerData()
{
	for( AkUInt32 i = 0; i < 8; ++i )
	{
		const AkListenerPosition pos = m_listeners[ i ].data.Pos;
		SetListenerPosition( i, pos );
	}
}

/** 2012 0x934cb0: orthonormalises front against top and builds the rotation matrix the panner uses. */
AKRESULT CAkListener::SetListenerPosition( AkUInt32 in_uListener, const AkListenerPosition & in_Position )
{
	if( in_uListener >= 8 )
	{
		return AK_InvalidParameter;
	}
	AkVector Front = in_Position.OrientationFront;
	AkVector Top = in_Position.OrientationTop;
	AkListener & rListener = m_listeners[ in_uListener ];
	rListener.data.Pos.Position = in_Position.Position;
	const AkReal32 fDot = AkMath::DotProduct( Front, Top );
	Front.X -= Top.X * fDot;
	Front.Y -= Top.Y * fDot;
	Front.Z -= Top.Z * fDot;
	AkMath::Normalise( Front );
	AkMath::Normalise( Top );
	rListener.data.Pos.OrientationFront = Front;
	rListener.data.Pos.OrientationTop = Top;
	const AkVector Side = AkMath::CrossProduct( Top, Front );
	AkReal32 matTmp[ 3 ][ 3 ] =
	{
		{ Side.X, Side.Y, Side.Z },
		{ Top.X, Top.Y, Top.Z },
		{ Front.X, Front.Y, Front.Z }
	};
	if( CAkLEngine::m_panningRule )
	{
		memcpy( rListener.Matrix, matTmp, sizeof( matTmp ) );
	}
	else
	{
		AkMath::MatrixMul3by3( &m_matRot[ 0 ][ 0 ], &matTmp[ 0 ][ 0 ], &rListener.Matrix[ 0 ][ 0 ] );
	}
	return AK_Success;
}

/** 2012 0x934e80. */
AKRESULT CAkListener::GetListenerPosition( AkUInt32 in_uListener, AkListenerPosition & out_rPosition )
{
	if( in_uListener >= 8 )
	{
		return AK_InvalidParameter;
	}
	out_rPosition = m_listeners[ in_uListener ].data.Pos;
	return AK_Success;
}

/** 2012 0x934e60. */
void CAkListener::SetScalingFactor( AkUInt32 in_uListener, AkReal32 in_fScalingFactor )
{
	m_listeners[ in_uListener ].fScalingFactor = in_fScalingFactor;
}

/** 2012 0x934eb0. */
AkReal32 CAkListener::GetScalingFactor( AkUInt32 in_uListener )
{
	return m_listeners[ in_uListener ].fScalingFactor;
}

/** 2012 0x934ed0. */
AKRESULT CAkListener::SetListenerSpatialization( AkUInt32 in_uListener, bool in_bSpatialized, AkSpeakerVolumes * in_pVolumeOffsets )
{
	if( in_uListener >= 8 )
	{
		return AK_InvalidParameter;
	}
	m_listeners[ in_uListener ].data.bSpatialized = in_bSpatialized;
	if( in_pVolumeOffsets )
	{
		m_listeners[ in_uListener ].data.VolumeOffset.Zero();
		m_listeners[ in_uListener ].data.VolumeOffset.volumes = *in_pVolumeOffsets;
	}
	return AK_Success;
}

/** 2012 0x934f50. */
AKRESULT CAkListener::GetListenerSpatialization( AkUInt32 in_uListener, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets )
{
	if( in_uListener >= 8 )
	{
		return AK_InvalidParameter;
	}
	out_rbSpatialized = m_listeners[ in_uListener ].data.bSpatialized;
	out_rVolumeOffsets = m_listeners[ in_uListener ].data.VolumeOffset.volumes;
	return AK_Success;
}

/** 2012 0x934fb0. */
void CAkListener::SetListenerPipeline( AkUInt32 in_uListener, bool in_bAudio, bool in_bFeedback )
{
	const AkUInt32 uMask = 1 << in_uListener;
	m_uAudioMask &= ~uMask;
	if( in_bAudio )
	{
		m_uAudioMask |= uMask;
	}
	m_uFeedbackMask &= ~uMask;
	if( in_bFeedback )
	{
		m_uFeedbackMask |= uMask;
	}
}

/** 2012 0x935080. */
AkReal32 * CAkListener::GetDefaultRotationMatrix()
{
	return CAkLEngine::m_panningRule ? &m_matIdentity[ 0 ][ 0 ] : &m_matRot[ 0 ][ 0 ];
}
