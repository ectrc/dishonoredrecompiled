/*=============================================================================
	AkCallback.h - Wwise 2012.1 event / bank / music callback surface.

	DISHONORED(layout): AkCallbackType's values and every AkCallbackInfo derivative come from the 2012
	PDB (resources/docs/types/types.json). The two function-pointer typedefs come from the exact
	demangled signatures of the calls Arkane makes:
	  AK::SoundEngine::PostEvent(unsigned long, unsigned int, unsigned long,
	      void (__cdecl *)(enum AkCallbackType, struct AkCallbackInfo *), void *, unsigned long,
	      struct AkExternalSourceInfo *, unsigned long)                      -> AkCallbackFunc
	  AK::SoundEngine::LoadBank(wchar_t const *, void (__cdecl *)(unsigned long, enum AKRESULT, long,
	      void *), void *, long, unsigned long &)                            -> AkBankCallbackFunc
	The bank callback of 2012.1 takes four parameters (bank id, result, memory pool id, cookie); later
	Wwise generations pass an AkBankCallbackInfo struct instead, and that struct is absent from this PDB.
=============================================================================*/
#ifndef _AK_CALLBACK_H_
#define _AK_CALLBACK_H_

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** Bit flags: the caller ORs them into PostEvent's in_uFlags and reads the type back in the callback. */
enum AkCallbackType
{
	AK_EndOfEvent						= 0x0001,
	AK_EndOfDynamicSequenceItem			= 0x0002,
	AK_Marker							= 0x0004,
	AK_Duration							= 0x0008,
	AK_SpeakerVolumeMatrix				= 0x0010,
	AK_MusicPlayStarted					= 0x0080,
	AK_MusicSyncBeat					= 0x0100,
	AK_MusicSyncBar						= 0x0200,
	AK_MusicSyncEntry					= 0x0400,
	AK_MusicSyncExit					= 0x0800,
	AK_MusicSyncGrid					= 0x1000,
	AK_MusicSyncUserCue					= 0x2000,
	AK_MusicSyncPoint					= 0x4000,
	AK_MusicSyncAll						= 0xFF00,
	AK_CallbackBits						= 0xFFFF,
	AK_EnableGetSourcePlayPosition		= 0x00010000,
	AK_EnableGetMusicPlayPosition		= 0x00020000
};

/** DISHONORED(layout): 8 bytes, pCookie @0, gameObjID @4. */
struct AkCallbackInfo
{
	void *			pCookie;
	AkGameObjectID	gameObjID;
};

/** DISHONORED(layout): 16 bytes. */
struct AkEventCallbackInfo : public AkCallbackInfo
{
	AkPlayingID	playingID;
	AkUniqueID	eventID;
};

/** DISHONORED(layout): 28 bytes; strLabel is a narrow string in the bank. */
struct AkMarkerCallbackInfo : public AkEventCallbackInfo
{
	AkUInt32		uIdentifier;
	AkUInt32		uPosition;
	const char *	strLabel;
};

/** DISHONORED(layout): 28 bytes. */
struct AkDurationCallbackInfo : public AkEventCallbackInfo
{
	AkReal32	fDuration;
	AkReal32	fEstimatedDuration;
	AkUniqueID	audioNodeID;
};

/** DISHONORED(layout): 20 bytes. */
struct AkDynamicSequenceItemCallbackInfo : public AkCallbackInfo
{
	AkPlayingID	playingID;
	AkUniqueID	audioNodeID;
	void *		pCustomInfo;
};

/** DISHONORED(layout): 36 bytes; musicSyncType @12 is the AkCallbackType bit that fired. */
struct AkMusicSyncCallbackInfo : public AkCallbackInfo
{
	AkPlayingID		playingID;
	AkCallbackType	musicSyncType;
	AkReal32		fBeatDuration;
	AkReal32		fBarDuration;
	AkReal32		fGridDuration;
	AkReal32		fGridOffset;
	char *			pszUserCueName;
};

/** DISHONORED(layout): 72 bytes; six per-channel AkSpeakerVolumes pointers dry and wet. */
struct AkSpeakerVolumeMatrixCallbackInfo : public AkEventCallbackInfo
{
	AkSpeakerVolumes *	pVolumesDry[6];
	AkSpeakerVolumes *	pVolumesWet[6];
	AkChannelMask		uChannelMask;
	bool				bIsEnvironmental;
};

/** PostEvent / DynamicSequence::Open callback. */
typedef void ( *AkCallbackFunc )( AkCallbackType in_eType, AkCallbackInfo * in_pCallbackInfo );

/** LoadBank / UnloadBank / PrepareBank / PrepareEvent / PrepareGameSyncs callback (2012.1 shape). */
typedef void ( *AkBankCallbackFunc )( AkUInt32 in_bankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie );

/** AK::SoundEngine::RegisterGlobalCallback / AddBehavioralExtension. */
typedef void ( *AkGlobalCallbackFunc )( bool in_bLastCall );

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkCallbackInfo) == 8, "AkCallbackInfo: 2012 PDB sizeof 8");
static_assert(sizeof(AkEventCallbackInfo) == 16, "AkEventCallbackInfo: 2012 PDB sizeof 16");
static_assert(sizeof(AkMarkerCallbackInfo) == 28, "AkMarkerCallbackInfo: 2012 PDB sizeof 28");
static_assert(sizeof(AkDurationCallbackInfo) == 28, "AkDurationCallbackInfo: 2012 PDB sizeof 28");
static_assert(sizeof(AkDynamicSequenceItemCallbackInfo) == 20, "AkDynamicSequenceItemCallbackInfo: 2012 PDB sizeof 20");
static_assert(sizeof(AkMusicSyncCallbackInfo) == 36, "AkMusicSyncCallbackInfo: 2012 PDB sizeof 36");
static_assert(sizeof(AkSpeakerVolumeMatrixCallbackInfo) == 72, "AkSpeakerVolumeMatrixCallbackInfo: 2012 PDB sizeof 72");
#endif

#endif // _AK_CALLBACK_H_
