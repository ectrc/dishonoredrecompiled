/*=============================================================================
	AkRegistryMgr.h - CAkRegistryMgr / CAkRegisteredObj: the registered game objects and what the game told
	the engine about each (positions, listener mask, environment sends, dry level, scaling, obstruction).

	DISHONORED(port): agent IH1 (wave 26); akregistrymgr.cpp (2012 0x9302f0..0x930c70) and
	akregisteredobj.cpp (0x936e80..0x9372b0). CAkRegisteredObj is the PDB's 0x58 bytes; obstruction and
	occlusion are packed to a byte (value * 255) as retail stores them; game object 0 (the "transport" object)
	is registered by Init at position zero.
	Deviation (named): retail's AK::SoundEngine::RegisterGameObj / SetPosition / ... enqueue a message that the
	audio manager applies on the audio thread (CAkAudioMgr::ProcessMsgQueue); the audio manager is IH2's,
	so the AK::SoundEngine entry points call the registry directly on the game thread, under g_csMain.
	The modified-node list and the switch history are the playback side's (IH2/IH3) and stay empty.
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

/** AkPositionKeeper: the position array plus the packed multi-position type / listener index and mask. */
struct AkPositionKeeper
{
	AkPositionKeeper() : m_aPos( NULL ), m_uNumPos( 0 ), m_eMultiPositionType( 0 ), m_uListenerIndex( 31 ), m_uListenerMask( 1 ) {}

	void SetPosition( const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions );
	void Term();

	AkSoundPosition *	m_aPos;
	AkUInt16			m_uNumPos;
	AkUInt8				m_eMultiPositionType : 3;
	AkUInt8				m_uListenerIndex : 5;
	AkUInt8				m_uListenerMask;
};

class CAkRegisteredObj
{
public:
	explicit CAkRegisteredObj( AkGameObjectID in_GameObjID );
	~CAkRegisteredObj();

	AKRESULT Init();
	void SetPosition( const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions, AK::SoundEngine::MultiPositionType in_eMultiPositionType, AkUInt32 in_ulListenerIndex );
	void SetActiveListeners( AkUInt32 in_uListenerMask ) { m_PosKeep.m_uListenerMask = (AkUInt8)in_uListenerMask; }
	AKRESULT SetGameObjectEnvironmentsValues( const AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues );
	AKRESULT SetObjectObstructionAndOcclusion( AkUInt32 in_uListener, AkReal32 in_fObstructionValue, AkReal32 in_fOcclusionValue );

	void *				m_pListModifiedNodes;
	AkPositionKeeper	m_PosKeep;
	AkEnvironmentValue	m_EnvironmentValues[ 4 ];
	AkUInt8				m_fObstructionValue[ 8 ];
	AkUInt8				m_fOcclusionValue[ 8 ];
	AkReal32			m_fDryLevelValue;
	AkReal32			m_fScalingFactor;
	AkGameObjectID		m_GameObjID;
	AkInt32				m_refCount;
};

class CAkRegistryMgr
{
public:
	AKRESULT Init();
	void Term();

	CAkRegisteredObj * RegisterObject( AkGameObjectID in_GameObjectID, void * in_pMonitorData );
	void UnregisterObject( AkGameObjectID in_GameObjectID );
	void UnregisterAll();
	CAkRegisteredObj * GetObjAndAddref( AkGameObjectID in_GameObjectID );
	CAkRegisteredObj * GetObj( AkGameObjectID in_GameObjectID );
	bool IsGameObjectActive( AkGameObjectID in_GameObjectID );
	AKRESULT SetPosition( AkGameObjectID in_GameObjectID, const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions, AK::SoundEngine::MultiPositionType in_eMultiPositionType, AkUInt32 in_ulListenerIndex );
	AKRESULT SetActiveListeners( AkGameObjectID in_GameObjectID, AkUInt32 in_uListenerMask );
	AKRESULT SetGameObjectEnvironmentsValues( AkGameObjectID in_GameObjectID, const AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues );
	AKRESULT SetGameObjectDryLevelValue( AkGameObjectID in_GameObjectID, AkReal32 in_fControlValue );
	AKRESULT SetGameObjectScalingFactor( AkGameObjectID in_GameObjectID, AkReal32 in_fControlValue );
	AKRESULT SetObjectObstructionAndOcclusion( AkGameObjectID in_GameObjectID, AkUInt32 in_uListener, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel );

	AkUInt32 NumRegistered() const { return m_mapRegisteredObj.Length(); }

	AkHashList<AkGameObjectID, CAkRegisteredObj *, 193> m_mapRegisteredObj;

private:
	static void Destroy( CAkRegisteredObj * in_pObj );
};
