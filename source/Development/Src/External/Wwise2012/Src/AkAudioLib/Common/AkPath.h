/*=============================================================================
	AkPath.h - CAkPath (akpath.cpp/.h) and CAkPathManager (akpathmanager.cpp): the user-defined 3D paths a PBI follows.

	DISHONORED(port): agent IH3 (wave 26). Layouts of the 2012 PDB (CAkPath 0x6C bytes, CAkPathManager 0x10 bytes).
	Rvas on the definitions in AkPath.cpp.
=============================================================================*/
#pragma once

#include "AkHierarchy.h"

class CAkPBI;

class CAkPath
{
public:
	enum State
	{
		Idle		= 0,
		NotRunning	= 0,
		Running		= 1,
		Suspended	= 2
	};

	CAkPath();
	void Term();
	AKRESULT SetPathsList( AkPathListItem * in_pPathList, AkUInt32 in_ulListSize, AkPathMode in_PathMode, bool in_bIsLooping, AkPathState * in_pState );
	AKRESULT InitRotationMatricesForNoFollowMode( AkUInt32 in_uListeners );
	void UpdateStartPosition();
	void UpdatePosition( AkUInt32 in_CurrentBufferTick );
	bool IsIdle();
	void SetSoundUniqueID( AkUniqueID in_soundUniqueID );
	void SetPlayingID( AkPlayingID in_playingID );
	void SetIsLooping( bool in_bIsLooping );

	State										m_eState;
	AkPathListItem *							m_pPathsList;
	bool *										m_pbPlayed;
	AkPathListItem *							m_pCurrentList;
	AkUInt16									m_ulCurrentListIndex;
	AkUInt16									m_ulListSize;
	AkUInt16									m_uCurrentVertex;
	AkPathMode									m_PathMode;
	bool										m_bWasStarted;
	bool										m_bIsLooping;
	AkArray<CAkPBI *, CAkPBI *, ArrayPoolDefault, 8>	m_PBIsList;
	AkUInt8										m_iPotentialUsers;
	AkUInt8										m_iNumUsers;
	AkUInt32									m_StartTime;
	AkUInt32									m_EndTime;
	AkUInt32									m_Duration;
	AkReal32									m_fa;
	AkReal32									m_fb;
	AkUInt32									m_TimePaused;
	AkVector									m_StartPosition;
	AkVector									m_Direction;
	AkUniqueID									m_ulSoundUniqueID;
	AkPlayingID									m_playingID;
	AkReal32 *									m_pNoFollowOrientationRotation;

private:
	friend class CAkPathManager;

	void RandomizePosition( AkVector & in_rPosition );
	void Pause( AkUInt32 in_CurrentBufferTick );
	void Resume( AkUInt32 in_CurrentBufferTick );
	void ClearPlayedFlags();
	bool PickRandomList();
	AKRESULT Start( AkUInt32 in_CurrentBufferTick );
	AKRESULT GetNextPathList();
	void NextVertex();
};
static_assert( sizeof( CAkPath ) == 0x6C, "CAkPath: 2012 PDB 0x6C bytes (AddPathToList allocates 0x6C)" );

class CAkPathManager
{
public:
	AKRESULT Init( AkUInt32 in_uMaxNumPath );
	void Term();
	CAkPath * AddPathToList();
	AKRESULT RemovePathFromList( CAkPath * in_pPath );
	AKRESULT AddPathUser( CAkPath * in_pPath, CAkPBI * in_pPBI );
	AKRESULT RemovePathUser( CAkPath * in_pPath, CAkPBI * in_pPBI );
	void AddPotentialUser( CAkPath * in_pPath );
	AKRESULT RemovePotentialUser( CAkPath * in_pPath );
	AKRESULT SetPathsList( CAkPath * in_pPath, AkPathListItem * in_pPathList, AkUInt32 in_ulListSize, AkPathMode in_PathMode, bool in_bIsLooping, AkPathState * in_pState );
	AKRESULT Start( CAkPath * in_pPath, AkPathState * in_pState );
	AKRESULT Pause( CAkPath * in_pPath );
	AKRESULT Resume( CAkPath * in_pPath );
	void ProcessPathsList( AkUInt32 in_uCurrentBufferTick );

	AkArray<CAkPath *, CAkPath *, ArrayPoolDefault, 0>	m_ActivePathsList;
	AkUInt32											m_uMaxPathNumber;
};
static_assert( sizeof( CAkPathManager ) == 0x10, "CAkPathManager: 2012 PDB 0x10 bytes (PreInit allocates 0x10)" );

extern CAkPathManager * g_pPathManager;
