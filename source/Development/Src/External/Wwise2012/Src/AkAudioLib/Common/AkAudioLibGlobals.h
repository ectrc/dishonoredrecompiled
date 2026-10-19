/*=============================================================================
	AkAudioLibGlobals.h - the sound engine's globals (akaudiolib.cpp / akaudiolibglobals) that the bank side
	reads, and the -akdiag log.

	DISHONORED(port): agent IH1 (wave 26). Names, types and defaults are the 2012 PDB's:
	  g_settings / g_PDSettings     AK::SoundEngine::Init copies the caller's settings (2012 0x91ad50)
	  g_csMain                      the main lock every bank reader takes around an object's SetInitialValues
	  g_pIndex, g_pBankManager, g_pRegistryMgr, g_pStateMgr, g_pRTPCMgr, g_pEnvironmentMgr
	                                created by AK::SoundEngine::PreInit (2012 0x919cc0) in that order
	  g_pExternalBankHandlerCallback set by AK::MusicEngine::Init to AkMusicBank::LoadBankItem
	  AkAudioLibSettings::g_pipelineCoreFrequency  48000 (the action delay / bus recovery time multiply by it/1000)
	  g_DefaultSoundPosition        2012 .data 0xe36cf8 = Position (0,0,1), Orientation (0,0,-1); CAkGen3DParams copies it
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <DishonoredWwiseSilent.h>

class CAkBankMgr;
class CAkRegistryMgr;
class CAkStateMgr;
class CAkRTPCMgr;
class CAkEnvironmentsMgr;
class CAkUsageSlot;

namespace AkBank
{
#pragma pack(push, 1)
	/** 5 bytes, packed: the HIRC item header (type, size); dwSectionSize counts the object id. */
	struct AKBKSubHircSection
	{
		AkUInt8		eHircType;
		AkUInt32	dwSectionSize;
	};
#pragma pack(pop)

	struct AkBankHeader
	{
		AkUInt32 dwBankGeneratorVersion;
		AkUInt32 dwSoundBankID;
		AkUInt32 dwLanguageID;
		AkUInt32 bFeedbackInBank;
	};

	struct AkSubchunkHeader
	{
		AkUInt32 dwTag;
		AkUInt32 dwChunkSize;
	};

	struct AKBKHashHeader
	{
		AkUInt32 uiType;
		AkUInt32 uiSize;
	};

	struct MediaHeader
	{
		AkUInt32 id;
		AkUInt32 uOffset;
		AkUInt32 uSize;
	};

	enum AKBKHircType
	{
		HIRCType_State				= 1,
		HIRCType_Sound				= 2,
		HIRCType_Action				= 3,
		HIRCType_Event				= 4,
		HIRCType_RanSeqCntr			= 5,
		HIRCType_SwitchCntr			= 6,
		HIRCType_ActorMixer			= 7,
		HIRCType_Bus				= 8,
		HIRCType_LayerCntr			= 9,
		HIRCType_Segment			= 10,
		HIRCType_Track				= 11,
		HIRCType_MusicSwitch		= 12,
		HIRCType_MusicRanSeq		= 13,
		HIRCType_Attenuation		= 14,
		HIRCType_DialogueEvent		= 15,
		HIRCType_FeedbackBus		= 16,
		HIRCType_FeedbackNode		= 17,
		HIRCType_FxShareSet			= 18,
		HIRCType_FxCustom			= 19,
		HIRCType_Count				= 20
	};

	static const AkUInt32 BankHeaderChunkID			= 0x44484B42;	// 'BKHD'
	static const AkUInt32 BankDataIndexChunkID		= 0x58444944;	// 'DIDX'
	static const AkUInt32 BankDataChunkID			= 0x41544144;	// 'DATA'
	static const AkUInt32 BankHierarchyChunkID		= 0x43524948;	// 'HIRC'
	static const AkUInt32 BankStrMapChunkID			= 0x44495453;	// 'STID'
	static const AkUInt32 BankStateMgrChunkID		= 0x474D5453;	// 'STMG'
	static const AkUInt32 BankEnvSettingChunkID		= 0x53564E45;	// 'ENVS'
}

namespace AK
{
	namespace SoundEngine
	{
		AKRESULT SetVolumeThresholdInternal( AkReal32 in_fVolumeThresholdDB, AkCommandPriority in_eCommandPriority );
		AKRESULT SetMaxNumVoicesLimitInternal( AkUInt16 in_maxNumberVoices, AkCommandPriority in_eCommandPriority );
		void DefaultBankCallbackFunc( AkBankID in_bankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie );
	}
}

namespace AkAudioLibSettings
{
	extern AkUInt32 g_pipelineCoreFrequency;
}

typedef AKRESULT ( *AkExternalBankHandlerCallback )( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot, AkUInt32 in_dwBankID );

extern AkInitSettings					g_settings;
extern AkPlatformInitSettings			g_PDSettings;
extern CAkLock							g_csMain;
extern CAkBankMgr *						g_pBankManager;
extern CAkRegistryMgr *					g_pRegistryMgr;
extern CAkStateMgr *					g_pStateMgr;
extern CAkRTPCMgr *						g_pRTPCMgr;
extern CAkEnvironmentsMgr *				g_pEnvironmentMgr;
extern AkSoundPosition					g_DefaultSoundPosition;
extern AkExternalBankHandlerCallback	g_pExternalBankHandlerCallback;

namespace AkSilent
{
	void Logf( const wchar_t * fmt, ... );
}

/** -akdiag: the bank / query / package log the IH1 acceptance compares against the python dump. */
#define AKDIAG( ... ) do { if( DishonoredWwise::IsDiagnostics() ) { AkSilent::Logf( __VA_ARGS__ ); } } while( 0 )
