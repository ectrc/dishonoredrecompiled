/*=============================================================================
	AkListener.cpp - CAkListener's listener state (ak3dlistener.cpp, 2012 0x934cb0..0x935120; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26). m_matRot / m_matIdentity are 2012 .data 0xe36d48 / 0xe36d6c (a 45 degree
	turn about Y for the speaker panning rule, identity for headphones); g_DefaultListenerPosition 0xe36d20 (front
	(0,0,1), top (0,1,0), origin).
	The 3D half (Get3DVolumes, GetListenerVolume, ComputeConeEffect, IsInitiallyUnderThreshold,
	DoSpeakerVolumeMatrixCallback) is agent IH3's (wave 26), 2012 0x934ff0..0x936d30.
	Deviations (named):
	  * No feedback (motion) device exists (CAkFeedbackDeviceMgr is never created, CAkPBI::GetFeedbackParameters
	    answers NULL), so Get3DVolumes's AkFeedbackParams branches (ZeroNewVolumes, the per-player volume maxima, the
	    feedback attenuation) and IsInitiallyUnderThreshold's feedback-target branch are not ported.
	  * The user-defined branch indexes CAkPath::m_pNoFollowOrientationRotation nine floats per active listener (the
	    allocation's stride; see AkPath.cpp).
	  * The multi-sources sums are converted back to dB with log10 (retail: fyl2x * log10(2), the same value).
	  * -akdiag: once a second (47 buffer ticks) the game-defined branch logs one 3D PBI: sound, game object, positions,
	    distance, the dry / wet curve values, LPF curve, cone factor, obstruction / occlusion, the first dry gain.
=============================================================================*/
#include "AkListener.h"
#include "AkLEngine.h"
#include "AkMath.h"
#include "AkPBI.h"
#include "AkPath.h"
#include "AkPlayingMgr.h"
#include "AkBusCtx.h"
#include "AkSpeakerPan.h"
#include "AkVPLSrcCbxNode.h"
#include <math.h>

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

// ------------------------------------------------------------------------------------------------------------
// AkSIMDSpeakerVolumes dB helpers (inline in the SDK; expanded in Get3DVolumes 0x935400)
// ------------------------------------------------------------------------------------------------------------

void AkSIMDSpeakerVolumes::dBToLin()
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		aVolumes[ i ] = AkMath::dBToLin( aVolumes[ i ] );
	}
}

void AkSIMDSpeakerVolumes::FastLinTodB()
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		aVolumes[ i ] = AkMath::FastLog10( aVolumes[ i ] ) * 20.f;
	}
}

void AkSIMDSpeakerVolumes::LinTodB()
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		aVolumes[ i ] = (AkReal32)( log10( (AkReal64)aVolumes[ i ] ) * 20.0 );
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkListener: the 3D half
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x934ff0. */
void CAkListener::DoSpeakerVolumeMatrixCallback( AkPlayingID in_playingID, AkUInt32 in_uNumChannels, AkChannelMask in_uChannelMask, bool in_bEnvBus, AkAudioMix * io_pAudioMixWet, AkAudioMix * io_pAudioMixDry )
{
	AkSpeakerVolumeMatrixCallbackInfo info;
	AkUInt32 i = 0;
	for( ; i < in_uNumChannels; ++i )
	{
		info.pVolumesDry[ i ] = &io_pAudioMixDry[ i ].Next.volumes;
	}
	for( ; i < 6; ++i )
	{
		info.pVolumesDry[ i ] = NULL;
	}
	i = 0;
	if( in_bEnvBus )
	{
		for( ; i < in_uNumChannels; ++i )
		{
			info.pVolumesWet[ i ] = &io_pAudioMixWet[ i ].Next.volumes;
		}
	}
	for( ; i < 6; ++i )
	{
		info.pVolumesWet[ i ] = NULL;
	}
	info.uChannelMask = in_uChannelMask;
	info.bIsEnvironmental = in_bEnvBus;
	g_pPlayingMgr->NotifySpeakerVolumeMatrix( in_playingID, &info );
}

/** 2012 0x935180: the angle between the sound's orientation and the direction to the listener, mapped from the
    inside angle (0) to the outside angle (1); the dry volume takes that fraction of the cone's outside volume. */
void CAkListener::ComputeConeEffect( CAkGen3DParams * in_pGen3DParams, const AkSoundPosition & in_soundPos, const AkVector & in_direction, AkReal32 in_fDistance, AkReal32 & io_fMinCone, AkReal32 & io_fListenerVolumeDry )
{
	CAkAttenuation * pAttenuation = in_pGen3DParams->m_Params.GetAttenuation();
	AkReal32 fCos = 1.f;
	if( in_fDistance > 0.f )
	{
		fCos = AkMath::DotProduct( in_direction, in_soundPos.Orientation ) / in_fDistance;
		if( fCos >= 1.f )
		{
			fCos = 1.f;
		}
		else if( fCos <= -1.f )
		{
			fCos = -1.f;
		}
	}
	const AkReal32 fAngle = (AkReal32)acos( fCos );
	const AkReal32 fCone = AkMath::Interpolate( pAttenuation->m_ConeParams.fInsideAngle, 0.f, pAttenuation->m_ConeParams.fOutsideAngle, 1.f, fAngle );
	if( fCone <= io_fMinCone )
	{
		io_fMinCone = fCone;
	}
	io_fListenerVolumeDry = in_pGen3DParams->m_Params.m_fConeOutsideVolume * fCone + io_fListenerVolumeDry;
}

/** 2012 0x935270: the scaled distance, the attenuation's dry (curve 0) and wet (curve 1, or the dry curve's value when
    both are the same table) volumes, and the cone for game-defined (or orientation-following user-defined) sounds. */
void CAkListener::GetListenerVolume( CAkGen3DParams * in_pGen3DParams, const AkSoundPosition & in_soundPos, const AkVector & in_vecListenerPos, AkReal32 & io_fMinCone, AkReal32 & io_fDistance, AkReal32 & io_fListenerVolumeDry, AkReal32 & io_fListenerVolumeWet, AkReal32 in_fScalingFactor )
{
	AkVector direction;
	direction.X = in_vecListenerPos.X - in_soundPos.Position.X;
	direction.Y = in_vecListenerPos.Y - in_soundPos.Position.Y;
	direction.Z = in_vecListenerPos.Z - in_soundPos.Position.Z;
	io_fDistance = AkMath::Magnitude( direction ) / in_fScalingFactor;
	CAkAttenuation * pAttenuation = in_pGen3DParams->m_Params.GetAttenuation();
	io_fListenerVolumeDry = 0.f;
	io_fListenerVolumeWet = 0.f;
	if( !pAttenuation )
	{
		io_fMinCone = 0.f;
		return;
	}
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeDry = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeDry );
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeWet = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeWet );
	if( pVolumeDry )
	{
		io_fListenerVolumeDry = pVolumeDry->Convert( io_fDistance );
	}
	if( pVolumeWet == pVolumeDry )
	{
		io_fListenerVolumeWet = io_fListenerVolumeDry;
	}
	else if( pVolumeWet )
	{
		io_fListenerVolumeWet = pVolumeWet->Convert( io_fDistance );
	}
	const AkPositioningType eType = in_pGen3DParams->m_Params.m_eType;
	if( pAttenuation->m_bIsConeEnabled && ( eType == Ak3DGameDef || ( eType == Ak3DUserDef && in_pGen3DParams->m_Params.m_bFollowOrientation ) ) )
	{
		ComputeConeEffect( in_pGen3DParams, in_soundPos, direction, io_fDistance, io_fMinCone, io_fListenerVolumeDry );
	}
	else
	{
		io_fMinCone = 0.f;
	}
}

/** 2012 0x935400: the dry and wet (aux send) mix of a 3D voice, its LPF and its obstruction LPF. Volumes are summed in
    dB (voice volume, attenuation curve, obstruction / occlusion curves, speaker pan) and converted once at the end.
    User-defined: one position relative to the listener, panned through each listener's rotation (or the default).
    Game-defined: every position of the game object for every active listener; single-source / multi-directions keep
    the loudest per speaker, multi-sources add the linear gains of all positions. */
void CAkListener::Get3DVolumes( CAkPBI * in_pContext, const AkSoundPositioningParams & in_posParams, AkChannelMask in_uChannelMask, AkAudioMix * out_pAudioMixWet, AkAudioMix * out_pAudioMixDry, AkReal32 * out_pLPF, AkReal32 * out_pObsLPF, bool in_bEnvBus )
{
	const AkUInt32 uNumChannels = AkGetNumChannels( in_uChannelMask );
	const AkUInt32 uNumFullBandChannels = ( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) ? uNumChannels - 1 : uNumChannels;
	const AkReal32 fVolume = in_pContext->m_EffectiveParams.Volume >= 0.f ? 0.f : in_pContext->m_EffectiveParams.Volume;
	CAkGen3DParams * pGen3DParams = in_pContext->m_p3DSound;
	CAkAttenuation * pAttenuation = pGen3DParams->m_Params.GetAttenuation();
	AkReal32 fLPF = in_pContext->m_EffectiveParams.LPF;
	AkReal32 fMinCone = 1.f;
	AkReal32 fDistance = 0.f;
	AkReal32 fListenerVolumeDry = 0.f;
	AkReal32 fListenerVolumeWet = 0.f;
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pSpreadCurve = NULL;
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pLPFCurve = NULL;
	if( pAttenuation )
	{
		pSpreadCurve = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_Spread );
		pLPFCurve = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_LowPassFilter );
	}
	AkReal32 fSpread = 0.f;
	AkSIMDSpeakerVolumes volumes[ AK_VOICE_MAX_NUM_CHANNELS ];

	if( in_posParams.ePosType != Ak3DGameDef )
	{
		const AkSoundPosition * pPos = in_posParams.pPosRef->m_aPos ? in_posParams.pPosRef->m_aPos : &g_DefaultSoundPosition;
		const AkVector origin = { 0.f, 0.f, 0.f };
		GetListenerVolume( pGen3DParams, *pPos, origin, fMinCone, fDistance, fListenerVolumeDry, fListenerVolumeWet, in_pContext->m_pGameObj->m_fScalingFactor );
		if( pGen3DParams->m_Params.m_bIsSpatialized )
		{
			if( pSpreadCurve )
			{
				fSpread = pSpreadCurve->Convert( fDistance );
			}
			if( pLPFCurve )
			{
				const AkReal32 fCurveLPF = pLPFCurve->Convert( fDistance );
				if( fLPF <= fCurveLPF )
				{
					fLPF = fCurveLPF;
				}
			}
			AkReal32 * aMatrices[ 8 ];
			AkReal32 aRotated[ 8 ][ 9 ];
			AkUInt32 uNumMatrices = 0;
			AkUInt32 uListenerMask = in_posParams.pPosRef->m_uListenerMask;
			if( !uListenerMask || pGen3DParams->m_Params.m_bFollowOrientation )
			{
				aMatrices[ 0 ] = CAkLEngine::m_panningRule ? &m_matIdentity[ 0 ][ 0 ] : &m_matRot[ 0 ][ 0 ];
				uNumMatrices = 1;
			}
			else
			{
				for( AkUInt32 uListener = 0; uListenerMask; ++uListener, uListenerMask >>= 1 )
				{
					if( !( uListenerMask & 1 ) )
					{
						continue;
					}
					CAkPath * pPath = in_pContext->m_PathInfo.pPBPath;
					if( pPath )
					{
						AkMath::MatrixMul3by3( &m_listeners[ uListener ].Matrix[ 0 ][ 0 ], &pPath->m_pNoFollowOrientationRotation[ uNumMatrices * 9 ], aRotated[ uNumMatrices ] );
						aMatrices[ uNumMatrices ] = aRotated[ uNumMatrices ];
					}
					else
					{
						aMatrices[ uNumMatrices ] = GetDefaultRotationMatrix();
					}
					++uNumMatrices;
				}
			}
			for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
			{
				volumes[ i ].Set( -97.f );
			}
			for( AkUInt32 m = 0; m < uNumMatrices; ++m )
			{
				const AkReal32 * M = aMatrices[ m ];
				const AkVector & P = pPos->Position;
				const AkReal32 fX = M[ 2 ] * P.Z + M[ 1 ] * P.Y + P.X * M[ 0 ];
				const AkReal32 fY = M[ 8 ] * P.Z + M[ 7 ] * P.Y + M[ 6 ] * P.X;
				AkSIMDSpeakerVolumes listenerVolumes[ AK_VOICE_MAX_NUM_CHANNELS ];
				CAkSpeakerPan::GetSpeakerVolumes( fX, fY, in_posParams.fDivergenceCenter, fSpread, listenerVolumes, uNumFullBandChannels );
				if( pAttenuation && pAttenuation->m_bIsConeEnabled && !pGen3DParams->m_Params.m_bFollowOrientation )
				{
					AkVector direction;
					if( CAkLEngine::m_panningRule )
					{
						direction.X = -fX;
						direction.Z = -fY;
					}
					else
					{
						direction.X = fX * -0.70710677f - fY * -0.70710677f;
						direction.Z = fY * -0.70710677f - fX * 0.70710677f;
					}
					direction.Y = 0.f;
					ComputeConeEffect( pGen3DParams, *pPos, direction, fDistance, fMinCone, fListenerVolumeDry );
				}
				for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
				{
					volumes[ i ].Max( listenerVolumes[ i ] );
				}
			}
		}
		else
		{
			CAkSpeakerPan::GetSpeakerVolumes2DPan( 0.5f, 0.5f, in_posParams.fDivergenceCenter, true, in_uChannelMask, volumes );
			for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
			{
				volumes[ i ].FastLinTodB();
			}
		}
		const AkReal32 fDry = fListenerVolumeDry + fVolume;
		const AkReal32 fWet = fListenerVolumeWet + fVolume;
		if( in_bEnvBus )
		{
			const AkReal32 fDryLevel = in_pContext->m_pGameObj->m_fDryLevelValue;
			for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
			{
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ i ].Next;
				rDry.Copy( volumes[ i ] );
				rDry.Add( fDry );
				rDry.volumes.fLfe = -97.f;
				rDry.dBToLin();
				rDry.Mul( fDryLevel );
				AkSIMDSpeakerVolumes & rWet = out_pAudioMixWet[ i ].Next;
				rWet.Copy( volumes[ i ] );
				rWet.Add( fWet );
				rWet.volumes.fLfe = -97.f;
				rWet.dBToLin();
			}
			if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
			{
				AkSIMDSpeakerVolumes & rWet = out_pAudioMixWet[ uNumChannels - 1 ].Next;
				rWet.Zero();
				rWet.volumes.fLfe = AkMath::dBToLin( fWet );
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ uNumChannels - 1 ].Next;
				rDry.Zero();
				rDry.volumes.fLfe = AkMath::dBToLin( fDry ) * fDryLevel;
			}
		}
		else
		{
			for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
			{
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ i ].Next;
				rDry.Copy( volumes[ i ] );
				rDry.Add( fDry );
				rDry.volumes.fLfe = -97.f;
				rDry.dBToLin();
			}
			if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
			{
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ uNumChannels - 1 ].Next;
				rDry.Zero();
				rDry.volumes.fLfe = AkMath::dBToLin( fDry );
			}
		}
		*out_pObsLPF = 0.f;
	}
	else
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			out_pAudioMixWet[ i ].Next.Set( -97.f );
			out_pAudioMixDry[ i ].Next.Set( -97.f );
		}
		const AkSoundPositionRef & rPosRef = *in_posParams.pPosRef;
		const bool bMultiSources = rPosRef.m_eMultiPositionType == AK::SoundEngine::MultiPositionType_MultiSources;
		const AkSoundPosition * aPositions;
		AkUInt32 uNumPositions;
		if( rPosRef.m_eMultiPositionType == AK::SoundEngine::MultiPositionType_MultiSources || rPosRef.m_eMultiPositionType == AK::SoundEngine::MultiPositionType_MultiDirections )
		{
			if( !rPosRef.m_uNumPos )
			{
				*out_pObsLPF = 0.f;
				for( AkUInt32 i = 0; i < uNumChannels; ++i )
				{
					out_pAudioMixWet[ i ].Next.Zero();
					out_pAudioMixDry[ i ].Next.Zero();
				}
				return;
			}
			aPositions = rPosRef.m_aPos;
			uNumPositions = rPosRef.m_uNumPos;
		}
		else
		{
			aPositions = rPosRef.m_aPos ? rPosRef.m_aPos : &g_DefaultSoundPosition;
			uNumPositions = 1;
		}

		AkReal32 fMinOcclusion = 1.f;
		AkReal32 fMinObstruction = 1.f;
		AkReal32 fMinLPF = 100.f;
		AkReal32 fLastObsOccVolume = 0.f;
		AkUInt32 uListener = 0;
		for( AkUInt32 uListenerMask = rPosRef.m_uListenerMask; uListenerMask; uListenerMask >>= 1, ++uListener )
		{
			if( !( uListenerMask & 1 ) )
			{
				continue;
			}
			const AkListener & rListener = m_listeners[ uListener ];
			AkSIMDSpeakerVolumes aSumDry[ AK_VOICE_MAX_NUM_CHANNELS ];
			AkSIMDSpeakerVolumes aSumWet[ AK_VOICE_MAX_NUM_CHANNELS ];
			if( bMultiSources )
			{
				for( AkUInt32 i = 0; i < AK_VOICE_MAX_NUM_CHANNELS; ++i )
				{
					aSumDry[ i ].Zero();
					aSumWet[ i ].Zero();
				}
			}
			for( AkUInt32 uPos = 0; uPos < uNumPositions; ++uPos )
			{
				const AkSoundPosition & rPos = aPositions[ uPos ];
				GetListenerVolume( pGen3DParams, rPos, rListener.data.Pos.Position, fMinCone, fDistance, fListenerVolumeDry, fListenerVolumeWet, in_pContext->m_pGameObj->m_fScalingFactor * rListener.fScalingFactor );
				const AkReal32 fOcclusion = (AkReal32)in_pContext->m_pGameObj->m_fOcclusionValue[ uListener ] * 0.0039215689f;
				const AkReal32 fOccVolume = g_pEnvironmentMgr->GetCurveValue( CAkEnvironmentsMgr::CurveOcc, CAkEnvironmentsMgr::CurveVol, fOcclusion * 100.f );
				if( fOcclusion <= fMinOcclusion )
				{
					fMinOcclusion = fOcclusion;
				}
				const AkReal32 fObstruction = (AkReal32)in_pContext->m_pGameObj->m_fObstructionValue[ uListener ] * 0.0039215689f;
				const AkReal32 fObsOccVolume = g_pEnvironmentMgr->GetCurveValue( CAkEnvironmentsMgr::CurveObs, CAkEnvironmentsMgr::CurveVol, fObstruction * 100.f ) + fOccVolume;
				if( fObstruction <= fMinObstruction )
				{
					fMinObstruction = fObstruction;
				}
				fLastObsOccVolume = fObsOccVolume;
				if( rListener.data.bSpatialized )
				{
					if( pGen3DParams->m_Params.m_bIsSpatialized )
					{
						const AkVector & L = rListener.data.Pos.Position;
						const AkReal32 dX = rPos.Position.X - L.X;
						const AkReal32 dY = rPos.Position.Y - L.Y;
						const AkReal32 dZ = rPos.Position.Z - L.Z;
						const AkReal32 fX = rListener.Matrix[ 0 ][ 2 ] * dZ + rListener.Matrix[ 0 ][ 1 ] * dY + dX * rListener.Matrix[ 0 ][ 0 ];
						const AkReal32 fY = rListener.Matrix[ 2 ][ 2 ] * dZ + rListener.Matrix[ 2 ][ 1 ] * dY + rListener.Matrix[ 2 ][ 0 ] * dX;
						if( pSpreadCurve )
						{
							fSpread = pSpreadCurve->Convert( fDistance );
						}
						CAkSpeakerPan::GetSpeakerVolumes( fX, fY, in_posParams.fDivergenceCenter, fSpread, volumes, uNumFullBandChannels );
					}
					else
					{
						CAkSpeakerPan::GetSpeakerVolumes2DPan( 0.5f, 0.5f, in_posParams.fDivergenceCenter, true, in_uChannelMask, volumes );
						for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
						{
							volumes[ i ].FastLinTodB();
						}
					}
				}
				else
				{
					for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
					{
						volumes[ i ].Copy( rListener.data.VolumeOffset );
					}
				}
				const AkReal32 fCurveLPF = pLPFCurve ? pLPFCurve->Convert( fDistance ) : 0.f;
				if( fCurveLPF <= fMinLPF )
				{
					fMinLPF = fCurveLPF;
				}
				const AkReal32 fWetdB = fOccVolume + fListenerVolumeWet;
				const AkReal32 fDrydB = fObsOccVolume + fListenerVolumeDry;
				const bool bAudio = !in_pContext->m_bTargetIsFeedback && ( ( 1u << uListener ) & m_uAudioMask );
				for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
				{
					AkSIMDSpeakerVolumes dry = volumes[ i ];
					volumes[ i ].Add( fWetdB );
					dry.Add( fDrydB );
					if( !bAudio )
					{
						continue;
					}
					if( bMultiSources )
					{
						dry.dBToLin();
						volumes[ i ].dBToLin();
						aSumDry[ i ].Add( dry );
						aSumWet[ i ].Add( volumes[ i ] );
					}
					else
					{
						out_pAudioMixDry[ i ].Next.Max( dry );
						out_pAudioMixWet[ i ].Next.Max( volumes[ i ] );
					}
				}
				if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
				{
					const AkUInt32 uLFE = uNumChannels - 1;
					if( bMultiSources )
					{
						aSumDry[ uLFE ].volumes.fLfe += AkMath::dBToLin( fDrydB );
						aSumWet[ uLFE ].volumes.fLfe += AkMath::dBToLin( fWetdB );
					}
					else
					{
						AkReal32 & rDryLfe = out_pAudioMixDry[ uLFE ].Next.volumes.fLfe;
						if( fDrydB > rDryLfe )
						{
							rDryLfe = fDrydB;
						}
						AkReal32 & rWetLfe = out_pAudioMixWet[ uLFE ].Next.volumes.fLfe;
						if( fWetdB > rWetLfe )
						{
							rWetLfe = fWetdB;
						}
					}
				}
			}
			if( bMultiSources )
			{
				for( AkUInt32 i = 0; i < uNumFullBandChannels; ++i )
				{
					aSumDry[ i ].LinTodB();
					aSumWet[ i ].LinTodB();
					out_pAudioMixDry[ i ].Next.Max( aSumDry[ i ] );
					out_pAudioMixWet[ i ].Next.Max( aSumWet[ i ] );
				}
				if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
				{
					const AkUInt32 uLFE = uNumChannels - 1;
					const AkReal32 fDryLfe = (AkReal32)( log10( (AkReal64)aSumDry[ uLFE ].volumes.fLfe ) * 20.0 );
					AkReal32 & rDryLfe = out_pAudioMixDry[ uLFE ].Next.volumes.fLfe;
					if( fDryLfe > rDryLfe )
					{
						rDryLfe = fDryLfe;
					}
					const AkReal32 fWetLfe = (AkReal32)( log10( (AkReal64)aSumWet[ uLFE ].volumes.fLfe ) * 20.0 );
					AkReal32 & rWetLfe = out_pAudioMixWet[ uLFE ].Next.volumes.fLfe;
					if( fWetLfe > rWetLfe )
					{
						rWetLfe = fWetLfe;
					}
				}
			}
		}
		if( fLPF <= fMinLPF )
		{
			fLPF = fMinLPF;
		}
		const AkReal32 fDryOutdB = out_pAudioMixDry[ 0 ].Next.aVolumes[ 0 ] + fVolume;
		if( in_bEnvBus )
		{
			const AkReal32 fDryLevel = in_pContext->m_pGameObj->m_fDryLevelValue;
			for( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ i ].Next;
				rDry.Add( fVolume );
				rDry.dBToLin();
				rDry.Mul( fDryLevel );
				AkSIMDSpeakerVolumes & rWet = out_pAudioMixWet[ i ].Next;
				rWet.Add( fVolume );
				rWet.dBToLin();
			}
		}
		else
		{
			for( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				AkSIMDSpeakerVolumes & rDry = out_pAudioMixDry[ i ].Next;
				rDry.Add( fVolume );
				rDry.dBToLin();
			}
		}
		const AkReal32 fOccLPF = g_pEnvironmentMgr->GetCurveValue( CAkEnvironmentsMgr::CurveOcc, CAkEnvironmentsMgr::CurveLPF, fMinOcclusion * 100.f );
		if( fLPF <= fOccLPF )
		{
			fLPF = fOccLPF;
		}
		*out_pObsLPF = g_pEnvironmentMgr->GetCurveValue( CAkEnvironmentsMgr::CurveObs, CAkEnvironmentsMgr::CurveLPF, fMinObstruction * 100.f );

		if( DishonoredWwise::IsDiagnostics() )
		{
			static AkUInt32 s_uLastDiagTick = 0;
			const AkUInt32 uTick = g_pAudioMgr ? g_pAudioMgr->GetBufferTick() : 0;
			if( uTick - s_uLastDiagTick >= 47 || uTick < s_uLastDiagTick )
			{
				s_uLastDiagTick = uTick;
				AKDIAG( L"akdiag: 3D sound 0x%08x obj %u: %u position(s) type %u, distance %.1f, dry curve %.2f dB, wet curve %.2f dB, LPF %.1f, cone %.3f, obs %.2f occ %.2f (%.2f dB), voice %.2f dB, dry ch0 %.2f dB",
					in_pContext->GetSoundID(), (AkUInt32)in_pContext->m_pGameObj->m_GameObjID, uNumPositions, (AkUInt32)rPosRef.m_eMultiPositionType, fDistance,
					fListenerVolumeDry, fListenerVolumeWet, fLPF, fMinCone, fMinObstruction, fMinOcclusion, fLastObsOccVolume, fVolume, fDryOutdB );
			}
		}
	}

	if( pAttenuation && pAttenuation->m_bIsConeEnabled && pGen3DParams->m_Params.m_fConeLoPass > 0.f )
	{
		const AkReal32 fConeLPF = AkMath::Interpolate( 0.f, 0.f, 1.f, pGen3DParams->m_Params.m_fConeLoPass, fMinCone );
		if( fLPF <= fConeLPF )
		{
			fLPF = fConeLPF;
		}
	}
	*out_pLPF = fLPF;
	if( in_pContext->m_uRegisteredNotif & AK_SpeakerVolumeMatrix )
	{
		DoSpeakerVolumeMatrixCallback( in_pContext->m_UserParams.m_PlayingID, uNumChannels, in_uChannelMask, in_bEnvBus, out_pAudioMixWet, out_pAudioMixDry );
	}
}

/** 2012 0x936d30: whether a voice is inaudible from its first mix (a mono mix, channel mask 4) - the dry path, and the
    wet path too when its bus sends to an environment. */
bool CAkListener::IsInitiallyUnderThreshold( CAkPBI * in_pCtx )
{
	in_pCtx->CalcEffectiveParams();
	if( !in_pCtx->m_p3DSound )
	{
		return g_fVolumeThresholdDB >= in_pCtx->m_EffectiveParams.Volume;
	}
	AkSoundPositioningParams posParams;
	posParams.pPosRef = NULL;
	in_pCtx->Get3DPositioningParams( posParams );
	const bool bIsEnvironmental = in_pCtx->GetBusContext().IsEnvironmental();
	AkAudioMix mixWet;
	AkAudioMix mixDry;
	AkReal32 fLPF;
	AkReal32 fObsLPF;
	Get3DVolumes( in_pCtx, posParams, AK_SPEAKER_FRONT_CENTER, &mixWet, &mixDry, &fLPF, &fObsLPF, bIsEnvironmental );
	const bool bDryUnder = mixDry.Next.IsUnderThreshold( g_fVolumeThreshold );
	if( bIsEnvironmental && bDryUnder )
	{
		return mixWet.Next.IsUnderThreshold( g_fVolumeThreshold );
	}
	return bDryUnder;
}
