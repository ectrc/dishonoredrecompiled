/*=============================================================================
	AkPath.cpp - CAkPath (akpath.cpp, 2012 0x952f90..0x953900) and CAkPathManager (akpathmanager.cpp,
	0x92f740..0x92fb80); 2013 matched by bytes (ih1_inventory.csv).

	DISHONORED(port): agent IH3 (wave 26). The path manager is created by AK::SoundEngine::Init (PreInit 0x919cc0, after
	the transition manager) and ticked by CAkAudioMgr::Perform 0x925130 once per audio frame.
	Deviations (named):
	  * InitRotationMatricesForNoFollowMode 0x9530c0 writes the product for every listener of the mask into the first
	    nine floats of its array, as the decompile does (the array is sized per listener but never advanced); with one
	    listener (Dishonored) the two readings agree.
=============================================================================*/
#include "AkPath.h"
#include "AkPBI.h"
#include "AkListener.h"
#include "AkAudioMgr.h"
#include <stdlib.h>

CAkPathManager * g_pPathManager = NULL;

// ------------------------------------------------------------------------------------------------------------
// CAkPath
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x953300. */
CAkPath::CAkPath()
	: m_eState( NotRunning )
	, m_pPathsList( NULL )
	, m_pbPlayed( NULL )
	, m_pCurrentList( NULL )
	, m_ulListSize( 0 )
	, m_PathMode( AkStepSequence )
	, m_bWasStarted( false )
	, m_bIsLooping( false )
	, m_iPotentialUsers( 0 )
	, m_iNumUsers( 0 )
	, m_StartTime( 0 )
	, m_EndTime( 0 )
	, m_Duration( 0 )
	, m_TimePaused( 0 )
	, m_ulSoundUniqueID( 0 )
	, m_playingID( 0 )
	, m_pNoFollowOrientationRotation( NULL )
{
}

/** 2012 0x952f90: the vertex moves by a random amount within the list's X / Y (= Z) range. */
void CAkPath::RandomizePosition( AkVector & in_rPosition )
{
	in_rPosition.X = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeX + in_rPosition.X;
	in_rPosition.Z = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeY + in_rPosition.Z;
}

/** 2012 0x953000. */
void CAkPath::Pause( AkUInt32 in_CurrentBufferTick )
{
	m_TimePaused = in_CurrentBufferTick;
	m_eState = Suspended;
}

/** 2012 0x953010: the segment's start and end move by the paused time. */
void CAkPath::Resume( AkUInt32 in_CurrentBufferTick )
{
	const AkUInt32 uPaused = in_CurrentBufferTick - m_TimePaused;
	m_StartTime += uPaused;
	m_EndTime += uPaused;
	m_eState = Running;
	m_fb = -( (AkReal32)m_StartTime * m_fa );
}

/** 2012 0x953050. */
bool CAkPath::IsIdle()
{
	return m_eState == NotRunning;
}

/** 2012 0x953060. */
void CAkPath::ClearPlayedFlags()
{
	if( m_pbPlayed )
	{
		for( AkUInt32 i = 0; i < m_ulListSize; ++i )
		{
			m_pbPlayed[ i ] = false;
		}
	}
}

/** 2012 0x953090. */
void CAkPath::SetSoundUniqueID( AkUniqueID in_soundUniqueID )
{
	m_ulSoundUniqueID = in_soundUniqueID;
}

/** 2012 0x9530a0. */
void CAkPath::SetPlayingID( AkPlayingID in_playingID )
{
	m_playingID = in_playingID;
}

/** 2012 0x9530b0. */
void CAkPath::SetIsLooping( bool in_bIsLooping )
{
	m_bIsLooping = in_bIsLooping;
}

/** 2012 0x9530c0: per active listener, (listener matrix)^T x the default rotation, so a sound that does not follow
    the listener's orientation keeps the orientation the listener had when the path started (see the banner). */
AKRESULT CAkPath::InitRotationMatricesForNoFollowMode( AkUInt32 in_uListeners )
{
	if( m_pNoFollowOrientationRotation )
	{
		return AK_Success;
	}
	AkUInt32 uNumListeners = 0;
	for( AkUInt32 uMask = in_uListeners & 0xFF; uMask; uMask >>= 1 )
	{
		uNumListeners += uMask & 1;
	}
	m_pNoFollowOrientationRotation = (AkReal32 *)AK::MemoryMgr::Malloc( g_DefaultPoolId, 36 * uNumListeners );
	if( !m_pNoFollowOrientationRotation )
	{
		return AK_Fail;
	}
	AkReal32 * pOut = m_pNoFollowOrientationRotation;
	AkUInt32 uListener = 0;
	for( AkUInt32 uMask = in_uListeners; uMask; uMask >>= 1, ++uListener )
	{
		if( uMask & 1 )
		{
			const AkReal32 * pDefault = CAkListener::GetDefaultRotationMatrix();
			const AkReal32 ( &L )[ 3 ][ 3 ] = CAkListener::m_listeners[ uListener ].Matrix;
			for( AkUInt32 r = 0; r < 3; ++r )
			{
				for( AkUInt32 c = 0; c < 3; ++c )
				{
					pOut[ r * 3 + c ] = pDefault[ c ] * L[ 0 ][ r ] + pDefault[ 3 + c ] * L[ 1 ][ r ] + pDefault[ 6 + c ] * L[ 2 ][ r ];
				}
			}
		}
	}
	return AK_Success;
}

/** 2012 0x953350: a random list not yet played (all played: the flags are cleared first); true when the cycle wrapped. */
bool CAkPath::PickRandomList()
{
	bool bAllPlayed = true;
	for( AkUInt32 i = 0; i < m_ulListSize; ++i )
	{
		bAllPlayed = bAllPlayed && m_pbPlayed[ i ];
	}
	if( bAllPlayed && m_pbPlayed )
	{
		for( AkUInt32 i = 0; i < m_ulListSize; ++i )
		{
			m_pbPlayed[ i ] = false;
		}
	}
	const AkUInt32 uIndex = rand() % m_ulListSize;
	m_ulCurrentListIndex = (AkUInt16)uIndex;
	if( m_PathMode & AkPathContinuous )
	{
		m_pCurrentList = &m_pPathsList[ uIndex ];
	}
	if( m_pbPlayed )
	{
		m_pbPlayed[ uIndex ] = true;
	}
	return bAllPlayed;
}

/** 2012 0x9533e0: every PBI on the path starts at the start position. */
void CAkPath::UpdateStartPosition()
{
	for( CAkPBI ** it = m_PBIsList.m_pItems; it != m_PBIsList.m_pItemsEnd; ++it )
	{
		( *it )->m_p3DSound->m_Params.m_Position = m_StartPosition;
	}
}

/** 2012 0x953420. */
void CAkPath::Term()
{
	m_eState = NotRunning;
	m_PBIsList.Term();
	if( ( m_PathMode & AkPathContinuous ) && m_pbPlayed )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pbPlayed );
		m_pbPlayed = NULL;
	}
	if( m_pNoFollowOrientationRotation )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pNoFollowOrientationRotation );
	}
}

/** 2012 0x953490: the first vertex (randomised), the direction to the second, the segment's time line in buffer ticks. */
AKRESULT CAkPath::Start( AkUInt32 in_CurrentBufferTick )
{
	if( !m_pCurrentList )
	{
		return AK_Fail;
	}
	m_bWasStarted = true;
	if( m_pCurrentList->iNumVertices <= 0 )
	{
		return AK_PathNoVertices;
	}
	const AkPathVertex * pVertices = m_pCurrentList->pVertices;
	m_uCurrentVertex = 1;
	m_StartPosition = pVertices[ 0 ].Vertex;
	m_Duration = ( pVertices[ 0 ].Duration + AkAudioLibSettings::g_msPerBufferTick - 1 ) / AkAudioLibSettings::g_msPerBufferTick;
	if( !m_Duration )
	{
		m_Duration = 1;
	}
	if( m_pCurrentList->iNumVertices <= 1 )
	{
		m_Direction.X = 0.f;
		m_Direction.Y = 0.f;
		m_Direction.Z = 0.f;
	}
	else
	{
		AkVector To = pVertices[ 1 ].Vertex;
		if( m_StartPosition.X == To.X && m_StartPosition.Y == To.Y && m_StartPosition.Z == To.Z && m_pCurrentList->iNumVertices <= 2 )
		{
			RandomizePosition( m_StartPosition );
			To = m_StartPosition;
		}
		else
		{
			RandomizePosition( m_StartPosition );
			RandomizePosition( To );
		}
		m_Direction.X = To.X - m_StartPosition.X;
		m_Direction.Y = To.Y - m_StartPosition.Y;
		m_Direction.Z = To.Z - m_StartPosition.Z;
	}
	UpdateStartPosition();
	m_StartTime = in_CurrentBufferTick;
	m_EndTime = m_Duration + in_CurrentBufferTick;
	m_fa = 1.f / (AkReal32)m_Duration;
	m_eState = Running;
	m_fb = -( m_fa * (AkReal32)in_CurrentBufferTick );
	return AK_Success;
}

/** 2012 0x953620: a list with a kept state (continuous mode, shared by the node) or a fresh played-flags array. */
AKRESULT CAkPath::SetPathsList( AkPathListItem * in_pPathList, AkUInt32 in_ulListSize, AkPathMode in_PathMode, bool in_bIsLooping, AkPathState * in_pState )
{
	if( m_eState != NotRunning )
	{
		return AK_Fail;
	}
	m_pPathsList = in_pPathList;
	m_ulListSize = (AkUInt16)in_ulListSize;
	m_PathMode = in_PathMode;
	m_bIsLooping = in_bIsLooping;
	if( in_pState->pbPlayed )
	{
		m_pbPlayed = in_pState->pbPlayed;
		m_ulCurrentListIndex = (AkUInt16)in_pState->ulCurrentListIndex;
		m_pCurrentList = &in_pPathList[ (AkUInt16)in_pState->ulCurrentListIndex ];
		return AK_Success;
	}
	m_ulCurrentListIndex = 0;
	m_pCurrentList = in_pPathList;
	m_pbPlayed = (bool *)AK::MemoryMgr::Malloc( g_DefaultPoolId, in_ulListSize );
	if( !m_pbPlayed )
	{
		return AK_Fail;
	}
	ClearPlayedFlags();
	if( in_PathMode & AkPathRandom )
	{
		PickRandomList();
	}
	return AK_Success;
}

/** 2012 0x9536d0: the next list (random or in sequence); AK_NoMoreData (17) at the end of a non-looping continuous path. */
AKRESULT CAkPath::GetNextPathList()
{
	if( !m_pCurrentList )
	{
		return AK_NoMoreData;
	}
	bool bWrapped;
	if( m_PathMode & AkPathRandom )
	{
		bWrapped = PickRandomList();
	}
	else
	{
		++m_ulCurrentListIndex;
		bWrapped = false;
		if( m_ulCurrentListIndex >= m_ulListSize )
		{
			if( m_PathMode & AkPathContinuous )
			{
				m_pCurrentList = m_pPathsList;
			}
			m_ulCurrentListIndex = 0;
			bWrapped = true;
		}
		else if( m_PathMode & AkPathContinuous )
		{
			++m_pCurrentList;
		}
	}
	if( !( m_PathMode & AkPathContinuous ) || ( bWrapped && !m_bIsLooping ) )
	{
		return AK_NoMoreData;
	}
	m_uCurrentVertex = 0;
	return AK_Success;
}

/** 2012 0x953750: the next segment of the current list, else the next list, else the path stops. */
void CAkPath::NextVertex()
{
	if( m_uCurrentVertex < m_pCurrentList->iNumVertices )
	{
		const AkPathVertex & rVertex = m_pCurrentList->pVertices[ m_uCurrentVertex ];
		++m_uCurrentVertex;
		m_StartPosition = rVertex.Vertex;
		m_StartPosition.X = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeX + m_StartPosition.X;
		m_StartPosition.Z = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeY + m_StartPosition.Z;
		m_Duration = ( rVertex.Duration + AkAudioLibSettings::g_msPerBufferTick - 1 ) / AkAudioLibSettings::g_msPerBufferTick;
		if( !m_Duration )
		{
			m_Duration = 1;
		}
	}
	if( m_uCurrentVertex < m_pCurrentList->iNumVertices || GetNextPathList() == AK_Success )
	{
		const AkPathVertex & rTo = m_pCurrentList->pVertices[ m_uCurrentVertex ];
		const AkReal32 fToX = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeX + rTo.Vertex.X;
		const AkReal32 fToZ = ( (AkReal32)rand() * 0.000061038882f - 1.f ) * m_pCurrentList->fRangeY + rTo.Vertex.Z;
		m_Direction.X = fToX - m_StartPosition.X;
		m_Direction.Y = rTo.Vertex.Y - m_StartPosition.Y;
		m_Direction.Z = fToZ - m_StartPosition.Z;
		m_StartTime = m_EndTime;
		m_EndTime = m_Duration + m_EndTime;
		m_fa = 1.f / (AkReal32)m_Duration;
		m_fb = -( m_fa * (AkReal32)m_StartTime );
	}
	else
	{
		m_eState = NotRunning;
	}
}

/** 2012 0x953900: every PBI on the path moves along the segment; past its end the next one starts. */
void CAkPath::UpdatePosition( AkUInt32 in_CurrentBufferTick )
{
	AkReal32 fdP = (AkReal32)in_CurrentBufferTick * m_fa + m_fb;
	if( fdP >= 1.f )
	{
		fdP = 1.f;
	}
	else if( fdP <= 0.f )
	{
		fdP = 0.f;
	}
	AkVector Position;
	Position.X = m_Direction.X * fdP + m_StartPosition.X;
	Position.Y = m_Direction.Y * fdP + m_StartPosition.Y;
	Position.Z = m_Direction.Z * fdP + m_StartPosition.Z;
	for( CAkPBI ** it = m_PBIsList.m_pItems; it != m_PBIsList.m_pItemsEnd; ++it )
	{
		( *it )->m_p3DSound->m_Params.m_Position = Position;
	}
	if( in_CurrentBufferTick >= m_EndTime )
	{
		NextVertex();
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkPathManager
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x92f870: room for in_uMaxNumPath paths (255 when 0). */
AKRESULT CAkPathManager::Init( AkUInt32 in_uMaxNumPath )
{
	m_uMaxPathNumber = in_uMaxNumPath ? in_uMaxNumPath : 255;
	return m_ActivePathsList.Reserve( m_uMaxPathNumber );
}

/** 2012 0x92f8f0. */
void CAkPathManager::Term()
{
	for( CAkPath ** it = m_ActivePathsList.m_pItems; it != m_ActivePathsList.m_pItemsEnd; ++it )
	{
		CAkPath * pPath = *it;
		pPath->Term();
		pPath->~CAkPath();
		AK::MemoryMgr::Free( g_DefaultPoolId, pPath );
	}
	m_ActivePathsList.Term();
}

/** 2012 0x92fa90. */
CAkPath * CAkPathManager::AddPathToList()
{
	if( m_ActivePathsList.Length() >= m_uMaxPathNumber )
	{
		return NULL;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkPath ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkPath * pPath = AkPlacementNew( pMem ) CAkPath();
	if( !m_ActivePathsList.AddLast( pPath ) )
	{
		pPath->~CAkPath();
		AK::MemoryMgr::Free( g_DefaultPoolId, pPath );
		return NULL;
	}
	return pPath;
}

/** 2012 0x92f960. */
AKRESULT CAkPathManager::RemovePathFromList( CAkPath * in_pPath )
{
	for( CAkPath ** it = m_ActivePathsList.m_pItems; it != m_ActivePathsList.m_pItemsEnd; ++it )
	{
		if( *it == in_pPath )
		{
			if( m_ActivePathsList.Length() > 1 )
			{
				*it = *( m_ActivePathsList.m_pItemsEnd - 1 );
			}
			--m_ActivePathsList.m_pItemsEnd;
			in_pPath->Term();
			in_pPath->~CAkPath();
			AK::MemoryMgr::Free( g_DefaultPoolId, in_pPath );
			return AK_Success;
		}
	}
	return AK_PathNotFound;
}

/** 2012 0x92fb10: at most eight PBIs per path. */
AKRESULT CAkPathManager::AddPathUser( CAkPath * in_pPath, CAkPBI * in_pPBI )
{
	if( in_pPath->m_PBIsList.Length() >= 8 )
	{
		return AK_Fail;
	}
	if( in_pPath->m_PBIsList.Exists( in_pPBI ) || !in_pPath->m_PBIsList.AddLast( in_pPBI ) )
	{
		return AK_PathNodeAlreadyInList;
	}
	++in_pPath->m_iNumUsers;
	return AK_Success;
}

/** 2012 0x92fb80: the last user (and no potential user) frees the path. */
AKRESULT CAkPathManager::RemovePathUser( CAkPath * in_pPath, CAkPBI * in_pPBI )
{
	if( in_pPath->m_PBIsList.RemoveSwap( in_pPBI ) != AK_Success )
	{
		return AK_PathNodeNotInList;
	}
	--in_pPath->m_iNumUsers;
	if( in_pPath->m_iPotentialUsers || in_pPath->m_iNumUsers )
	{
		return AK_Success;
	}
	return RemovePathFromList( in_pPath );
}

/** 2012 0x92f740. */
void CAkPathManager::AddPotentialUser( CAkPath * in_pPath )
{
	++in_pPath->m_iPotentialUsers;
}

/** 2012 0x92f9d0. */
AKRESULT CAkPathManager::RemovePotentialUser( CAkPath * in_pPath )
{
	if( in_pPath->m_iPotentialUsers )
	{
		--in_pPath->m_iPotentialUsers;
	}
	if( in_pPath->m_iPotentialUsers || in_pPath->m_iNumUsers )
	{
		return AK_Success;
	}
	return RemovePathFromList( in_pPath );
}

/** 2012 0x92f750. */
AKRESULT CAkPathManager::SetPathsList( CAkPath * in_pPath, AkPathListItem * in_pPathList, AkUInt32 in_ulListSize, AkPathMode in_PathMode, bool in_bIsLooping, AkPathState * in_pState )
{
	if( !in_pPathList || !in_ulListSize )
	{
		return AK_InvalidParameter;
	}
	return in_pPath->SetPathsList( in_pPathList, in_ulListSize, in_PathMode, in_bIsLooping, in_pState );
}

/** 2012 0x92f790: a started path only moves its PBIs to the start; a step path advances the node's list state. */
AKRESULT CAkPathManager::Start( CAkPath * in_pPath, AkPathState * in_pState )
{
	if( in_pPath->m_eState != CAkPath::NotRunning )
	{
		return AK_Fail;
	}
	if( in_pPath->m_bWasStarted )
	{
		in_pPath->UpdateStartPosition();
		return AK_Fail;
	}
	if( in_pState->pbPlayed )
	{
		in_pPath->m_pbPlayed = in_pState->pbPlayed;
		in_pPath->m_ulCurrentListIndex = (AkUInt16)in_pState->ulCurrentListIndex;
		in_pPath->m_pCurrentList = &in_pPath->m_pPathsList[ in_pState->ulCurrentListIndex ];
	}
	const AKRESULT eResult = in_pPath->Start( g_pAudioMgr->GetBufferTick() );
	if( !( in_pPath->m_PathMode & AkPathContinuous ) )
	{
		in_pPath->GetNextPathList();
		in_pState->ulCurrentListIndex = in_pPath->m_ulCurrentListIndex;
		in_pState->pbPlayed = in_pPath->m_pbPlayed;
	}
	return eResult;
}

/** 2012 0x92f810. */
AKRESULT CAkPathManager::Pause( CAkPath * in_pPath )
{
	if( in_pPath->m_eState != CAkPath::Running )
	{
		return AK_PathNotRunning;
	}
	in_pPath->Pause( g_pAudioMgr->GetBufferTick() );
	return AK_Success;
}

/** 2012 0x92f840. */
AKRESULT CAkPathManager::Resume( CAkPath * in_pPath )
{
	if( in_pPath->m_eState != CAkPath::Suspended )
	{
		return AK_PathNotPaused;
	}
	in_pPath->Resume( g_pAudioMgr->GetBufferTick() );
	return AK_Success;
}

/** 2012 0x92f8c0. */
void CAkPathManager::ProcessPathsList( AkUInt32 in_uCurrentBufferTick )
{
	for( CAkPath ** it = m_ActivePathsList.m_pItems; it != m_ActivePathsList.m_pItemsEnd; ++it )
	{
		if( ( *it )->m_eState == CAkPath::Running )
		{
			( *it )->UpdatePosition( in_uCurrentBufferTick );
		}
	}
}
