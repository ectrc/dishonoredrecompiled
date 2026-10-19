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

class CAkParameterNodeBase;

/** AkSoundPositionRef (PDB, 8 bytes): the position array, the listener mask, the packed multi-position type and the
    listener the object is attached to (m_iListenerIdx, signed 5 bits, -1 = none; the decompile reads byte +7 >> 3). */
struct AkSoundPositionRef
{
	AkSoundPosition *	m_aPos;
	AkUInt16			m_uNumPos;
	AkUInt8				m_uListenerMask;
	AkUInt8				m_eMultiPositionType : 3;
	AkInt8				m_iListenerIdx : 5;
};

struct AkPositionKeeper : public AkSoundPositionRef
{
	AkPositionKeeper()
	{
		m_aPos = NULL;
		m_uNumPos = 0;
		m_uListenerMask = 1;
		m_eMultiPositionType = AK::SoundEngine::MultiPositionType_SingleSource;
		m_iListenerIdx = -1;
	}

	AKRESULT SetPosition( const AkSoundPosition * in_aPositions, AkUInt16 in_uNumPositions );
	void Term();
};
static_assert( sizeof( AkSoundPositionRef ) == 8, "AkSoundPositionRef: 2012 PDB 8 bytes" );

/** AkSwitchHistItem (akregistrymgr.h): what a switch container last played for a game object, for its "play only the
    first time" / continue behaviours. */
struct AkSwitchHistItem
{
	AkUInt32	LastSwitch;
	AkUInt32	NumPlayBack;
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
	bool CreateModifiedNodeList();
	AKRESULT SetNodeAsModified( CAkParameterNodeBase * in_pNode );

	CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> *	m_pListModifiedNodes;
	AkPositionKeeper	m_PosKeep;
	AkEnvironmentValue	m_EnvironmentValues[ 4 ];
	CAkKeyArray<AkUniqueID, AkSwitchHistItem, 5>	m_SwitchHist;
	AkUInt8				m_fObstructionValue[ 8 ];
	AkUInt8				m_fOcclusionValue[ 8 ];
	AkReal32			m_fDryLevelValue;
	AkReal32			m_fScalingFactor;
	AkGameObjectID		m_GameObjID;
	AkInt32				m_refCount;

private:
	void FreeModifiedNodes();
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
	void SetNodeIDAsModified( CAkParameterNodeBase * in_pNode );
	AkSwitchHistItem GetSwitchHistItem( CAkRegisteredObj * in_pGameObj, AkUniqueID in_SwitchContID );
	AKRESULT SetSwitchHistItem( CAkRegisteredObj * in_pGameObj, AkUniqueID in_SwitchContID, const AkSwitchHistItem & in_SwitchHistItem );
	AKRESULT ClearSwitchHist( AkUniqueID in_SwitchContID, CAkRegisteredObj * in_pGameObj = NULL );
	AKRESULT GetActiveGameObjects( AkArray<AkGameObjectID, AkGameObjectID, ArrayPoolDefault, 32> & io_GameObjectList );

	CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>	m_listModifiedNodes;
	AkHashList<AkGameObjectID, CAkRegisteredObj *, 193> m_mapRegisteredObj;

private:
	static void Destroy( CAkRegisteredObj * in_pObj );
};
