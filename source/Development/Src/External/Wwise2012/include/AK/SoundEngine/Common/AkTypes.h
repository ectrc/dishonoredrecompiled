/*=============================================================================
	AkTypes.h - Wwise 2012.1 scalar types, ids, enums and plain structs.

	DISHONORED(layout): every typedef, enumerator value, struct member and struct size below comes from
	the 2012 symbolized build's PDB, extracted into resources/docs/types/types.json
	(resources/tools/pdb/dia_dump.py) and cross-checked against the exact demangled signatures in
	resources/docs/symbols/functions.csv. The scalar widths follow from the signatures, not from a
	guess: AK::SoundEngine::GetIDFromString(wchar_t const *) returns `unsigned long`, so AkUInt32 is
	`unsigned long` (Wwise's Win32 AkWinTypes.h), AK::SoundEngine::LoadBank(wchar_t const *, long,
	unsigned long &) takes AkMemPoolId = `long` = AkInt32, and
	AK::SoundEngine::RegisterGameObj(unsigned int, char const *) takes AkGameObjectID = `unsigned int`
	= AkUIntPtr (32-bit build), not AkUInt32.

	Packing: Wwise's own libraries are built with the default 8-byte packing while every UE3 module is
	compiled /Zp4 (cmake/DishonoredDefines.cmake). The pack push/pop below keeps these structs at their
	real Wwise sizes in both worlds, which is what the static_asserts at the end check.
=============================================================================*/
#ifndef _AK_TYPES_H_
#define _AK_TYPES_H_

#include <AK/AkWwiseSDKVersion.h>

#include <stddef.h>		// NULL and size_t, as the SDK's own AkTypes.h pulls them in

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

// ----------------------------------------------------------------------------------------------------
// Scalars (AkWinTypes.h)
// ----------------------------------------------------------------------------------------------------

typedef unsigned char		AkUInt8;			///< DISHONORED(layout): 1 byte
typedef unsigned short		AkUInt16;			///< DISHONORED(layout): 2 bytes
typedef unsigned long		AkUInt32;			///< DISHONORED(layout): GetIDFromString returns `unsigned long`
typedef unsigned __int64	AkUInt64;
typedef char				AkInt8;				///< DISHONORED(layout): SetBankLoadIOSettings(float, char) takes AkPriority
typedef short				AkInt16;
typedef long				AkInt32;			///< DISHONORED(layout): AkMemPoolId / AkTimeMs demangle as `long`
typedef __int64				AkInt64;
typedef float				AkReal32;
typedef double				AkReal64;
typedef unsigned int		AkUIntPtr;			///< pointer-sized unsigned (32-bit build)
typedef int					AkIntPtr;
typedef wchar_t				AkOSChar;			///< Windows: the OS character type is wide
typedef wchar_t				AkTChar;
typedef void *				AkThread;
typedef unsigned long		AkThreadID;
typedef void *				AkEvent;
typedef void *				AkSemaphore;
typedef void *				AkFileHandle;		///< DISHONORED(layout): AkFileDesc::hFile is `void *`

// ----------------------------------------------------------------------------------------------------
// Identifiers
// ----------------------------------------------------------------------------------------------------

typedef AkUInt32			AkUniqueID;			///< event / audio-node id (Wwise name hash)
typedef AkUInt32			AkStateID;
typedef AkUInt32			AkStateGroupID;
typedef AkUInt32			AkPlayingID;		///< DISHONORED(layout): PostEvent returns `unsigned long`
typedef AkInt32				AkTimeMs;			///< DISHONORED(layout): StopPlayingID(unsigned long, long, AkCurveInterpolation)
typedef AkUInt16			AkPortNumber;
typedef AkReal32			AkPitchValue;
typedef AkReal32			AkVolumeValue;
typedef AkUIntPtr			AkGameObjectID;		///< DISHONORED(layout): RegisterGameObj(unsigned int, char const *)
typedef AkUInt8				AkLPFType;
typedef AkInt32				AkMemPoolId;		///< DISHONORED(layout): LoadBank(wchar_t const *, long, unsigned long &)
typedef AkUInt32			AkPluginID;
typedef AkUInt32			AkCodecID;
typedef AkUInt32			AkAuxBusID;
typedef AkInt8				AkPriority;			///< DISHONORED(layout): AkIoHeuristics::priority is `char`
typedef AkUInt16			AkDataCompID;
typedef AkInt8				AkDataTypeID;
typedef AkUInt8				AkDataInterleaveID;
typedef AkUInt32			AkSwitchGroupID;
typedef AkUInt32			AkSwitchStateID;
typedef AkUInt32			AkRtpcID;
typedef AkReal32			AkRtpcValue;
typedef AkUInt32			AkBankID;			///< DISHONORED(layout): LoadBank's out parameter is `unsigned long &`
typedef AkUInt32			AkFileID;			///< DISHONORED(layout): CAkUnrealIOHookBlocking::Open mangles its id parameter as K (unsigned long)
typedef AkUInt32			AkDeviceID;
typedef AkUInt32			AkTriggerID;
typedef AkUInt32			AkArgumentValueID;
typedef AkUInt32			AkChannelMask;
typedef AkUInt32			AkModulatorID;

#define AK_INVALID_POOL_ID					(-1)
#define AK_DEFAULT_POOL_ID					(-1)
#define AK_INVALID_DEVICE_ID				((AkDeviceID)-1)
#define AK_INVALID_UNIQUE_ID				(0)
#define AK_INVALID_PLAYING_ID				(0)
#define AK_INVALID_BANK_ID					(0)
#define AK_INVALID_GAME_OBJECT				((AkGameObjectID)-1)
#define AK_INVALID_FILE_ID					(0xFFFFFFFF)
#define AK_INVALID_AUX_ID					(0)
#define AK_INVALID_PLUGINID					((AkPluginID)-1)
#define AK_DEFAULT_SWITCH_STATE				(0)
#define AK_DEFAULT_PRIORITY					(50)
#define AK_MIN_PRIORITY						(0)
#define AK_MAX_PRIORITY						(100)
#define AK_DEFAULT_BANK_IO_PRIORITY			(AK_DEFAULT_PRIORITY)
#define AK_DEFAULT_BANK_THROUGHPUT			(1024.f * 1024.f / 1000.f)
#define AK_DEFAULT_PATH_LENGTH				(260)

/** Speaker bits of AkChannelMask (AK::SoundEngine::GetSpeakerConfiguration / AkSpeakerVolumes order). */
#define AK_SPEAKER_FRONT_LEFT				0x1
#define AK_SPEAKER_FRONT_RIGHT				0x2
#define AK_SPEAKER_FRONT_CENTER				0x4
#define AK_SPEAKER_LOW_FREQUENCY			0x8
#define AK_SPEAKER_BACK_LEFT				0x10
#define AK_SPEAKER_BACK_RIGHT				0x20
#define AK_SPEAKER_SETUP_MONO				(AK_SPEAKER_FRONT_CENTER)
#define AK_SPEAKER_SETUP_STEREO				(AK_SPEAKER_FRONT_LEFT | AK_SPEAKER_FRONT_RIGHT)
#define AK_SPEAKER_SETUP_5POINT1			(AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_FRONT_CENTER | AK_SPEAKER_LOW_FREQUENCY | AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT)

// ----------------------------------------------------------------------------------------------------
// Enums. DISHONORED(layout): enumerator values verbatim from the 2012 PDB enum table.
// ----------------------------------------------------------------------------------------------------

/** Every Wwise entry point returns one of these; AK_Success is 1, not 0. */
enum AKRESULT
{
	AK_NotImplemented				= 0,
	AK_Success						= 1,
	AK_Fail							= 2,
	AK_PartialSuccess				= 3,
	AK_NotCompatible				= 4,
	AK_AlreadyConnected				= 5,
	AK_NameNotSet					= 6,
	AK_InvalidFile					= 7,
	AK_CorruptedFile				= 8,
	AK_MaxReached					= 9,
	AK_InputsInUsed					= 10,
	AK_OutputsInUsed				= 11,
	AK_InvalidName					= 12,
	AK_NameAlreadyInUse				= 13,
	AK_InvalidID					= 14,
	AK_IDNotFound					= 15,
	AK_InvalidInstanceID			= 16,
	AK_NoMoreData					= 17,
	AK_NoSourceAvailable			= 18,
	AK_StateGroupAlreadyExists		= 19,
	AK_InvalidStateGroup			= 20,
	AK_ChildAlreadyHasAParent		= 21,
	AK_InvalidLanguage				= 22,
	AK_CannotAddItseflAsAChild		= 23,
	AK_TransitionNotFound			= 24,
	AK_TransitionNotStartable		= 25,
	AK_TransitionNotRemovable		= 26,
	AK_UsersListFull				= 27,
	AK_UserAlreadyInList			= 28,
	AK_UserNotInList				= 29,
	AK_NoTransitionPoint			= 30,
	AK_InvalidParameter				= 31,
	AK_ParameterAdjusted			= 32,
	AK_IsA3DSound					= 33,
	AK_NotA3DSound					= 34,
	AK_ElementAlreadyInList			= 35,
	AK_PathNotFound					= 36,
	AK_PathNoVertices				= 37,
	AK_PathNotRunning				= 38,
	AK_PathNotPaused				= 39,
	AK_PathNodeAlreadyInList		= 40,
	AK_PathNodeNotInList			= 41,
	AK_VoiceNotFound				= 42,
	AK_DataNeeded					= 43,
	AK_NoDataNeeded					= 44,
	AK_DataReady					= 45,
	AK_NoDataReady					= 46,
	AK_NoMoreSlotAvailable			= 47,
	AK_SlotNotFound					= 48,
	AK_ProcessingOnly				= 49,
	AK_MemoryLeak					= 50,
	AK_CorruptedBlockList			= 51,
	AK_InsufficientMemory			= 52,
	AK_Cancelled					= 53,
	AK_UnknownBankID				= 54,
	AK_IsProcessing					= 55,
	AK_BankReadError				= 56,
	AK_InvalidSwitchType			= 57,
	AK_VoiceDone					= 58,
	AK_UnknownEnvironment			= 59,
	AK_EnvironmentInUse				= 60,
	AK_UnknownObject				= 61,
	AK_NoConversionNeeded			= 62,
	AK_FormatNotReady				= 63,
	AK_WrongBankVersion				= 64,
	AK_DataReadyNoProcess			= 65,
	AK_FileNotFound					= 66,
	AK_DeviceNotReady				= 67,
	AK_CouldNotCreateSecBuffer		= 68,
	AK_BankAlreadyLoaded			= 69,
	// 70 has no enumerator in the 2012 PDB
	AK_RenderedFX					= 71,
	AK_ProcessNeeded				= 72,
	AK_ProcessDone					= 73,
	AK_MemManagerNotInitialized		= 74,
	AK_StreamMgrNotInitialized		= 75,
	AK_SSEInstructionsNotSupported	= 76,
	AK_Busy							= 77,
	AK_UnsupportedChannelConfig		= 78,
	AK_PluginMediaNotAvailable		= 79,
	AK_MustBeVirtualized			= 80,
	AK_CommandTooLarge				= 81
};

enum AkCurveInterpolation
{
	AkCurveInterpolation_Log3			= 0,
	AkCurveInterpolation_Sine			= 1,
	AkCurveInterpolation_Log1			= 2,
	AkCurveInterpolation_InvSCurve		= 3,
	AkCurveInterpolation_Linear			= 4,
	AkCurveInterpolation_SCurve			= 5,
	AkCurveInterpolation_Exp1			= 6,
	AkCurveInterpolation_SineRecip		= 7,
	AkCurveInterpolation_Exp3			= 8,
	AkCurveInterpolation_LastFadeCurve	= 8,
	AkCurveInterpolation_Constant		= 9
};

enum AkGroupType
{
	AkGroupType_Switch	= 0,
	AkGroupType_State	= 1
};

enum AkPanningRule
{
	AkPanningRule_Speakers		= 0,
	AkPanningRule_Headphones	= 1
};

enum AkSoundQuality
{
	AkSoundQuality_High	= 0,
	AkSoundQuality_Low	= 1
};

enum AkMemPoolAttributes
{
	AkNoAlloc				= 0,
	AkMalloc				= 1,
	AkVirtualAlloc			= 2,
	AkAllocMask				= 3,
	AkFixedSizeBlocksMode	= 8,
	AkBlockMgmtMask			= 8
};

enum AkPluginType
{
	AkPluginTypeNone			= 0,
	AkPluginTypeCodec			= 1,
	AkPluginTypeSource			= 2,
	AkPluginTypeEffect			= 3,
	AkPluginTypeMotionDevice	= 4,
	AkPluginTypeMotionSource	= 5,
	AkPluginTypeMask			= 15
};

enum AkPositioningType
{
	AkUndefined			= 0,
	Ak2DPositioning		= 1,
	Ak3DUserDef			= 2,
	Ak3DGameDef			= 3
};

enum AkOpenMode
{
	AK_OpenModeRead			= 0,
	AK_OpenModeWrite		= 1,
	AK_OpenModeWriteOvrwr	= 2,
	AK_OpenModeReadWrite	= 3
};

enum AkMoveMethod
{
	AK_MoveBegin	= 0,
	AK_MoveCurrent	= 1,
	AK_MoveEnd		= 2
};

enum AkStmStatus
{
	AK_StmStatusIdle		= 0,
	AK_StmStatusCompleted	= 1,
	AK_StmStatusPending		= 2,
	AK_StmStatusCancelled	= 3,
	AK_StmStatusError		= 4
};

// ----------------------------------------------------------------------------------------------------
// Plain structs
// ----------------------------------------------------------------------------------------------------

/** DISHONORED(layout): 12 bytes, X/Y/Z. UAkAudioDevice::SetListener writes (-X, Z, Y) of the UE vector. */
struct AkVector
{
	AkReal32 X;
	AkReal32 Y;
	AkReal32 Z;
};

/** DISHONORED(layout): 24 bytes, Position @0, Orientation @12. */
struct AkSoundPosition
{
	AkVector Position;
	AkVector Orientation;
};

/** DISHONORED(layout): 36 bytes, OrientationFront @0, OrientationTop @12, Position @24. */
struct AkListenerPosition
{
	AkVector OrientationFront;
	AkVector OrientationTop;
	AkVector Position;
};

/** DISHONORED(layout): 24 bytes, one AkReal32 per speaker in AK_SPEAKER_* order. */
struct AkSpeakerVolumes
{
	AkReal32 fFrontLeft;
	AkReal32 fFrontRight;
	AkReal32 fCenter;
	AkReal32 fRearLeft;
	AkReal32 fRearRight;
	AkReal32 fLfe;
};

/** DISHONORED(layout): 8 bytes. */
struct AkEnvironmentValue
{
	AkAuxBusID	EnvID;
	AkReal32	fControlValue;
};

/** DISHONORED(layout): 12 bytes. */
struct AkObjectInfo
{
	AkUniqueID	objID;
	AkUniqueID	parentID;
	AkInt32		iDepth;
};

/** DISHONORED(layout): 44 bytes; positioningType @4 is AkPositioningType, the three bools pack at 8..11. */
struct AkPositioningInfo
{
	AkReal32			fCenterPct;
	AkPositioningType	positioningType;
	bool				bUpdateEachFrame;
	bool				bUseSpatialization;
	bool				bUseAttenuation;
	bool				bUseConeAttenuation;
	AkReal32			fInnerAngle;
	AkReal32			fOuterAngle;
	AkReal32			fConeMaxAttenuation;
	AkLPFType			LPFCone;
	AkReal32			fMaxDistance;
	AkReal32			fVolDryAtMaxDist;
	AkReal32			fVolWetAtMaxDist;
	AkLPFType			LPFValueAtMaxDist;
};

/** DISHONORED(layout): 20 bytes, five AkInt32 in milliseconds (AK::MusicEngine::GetPlayingSegmentInfo). */
struct AkSegmentInfo
{
	AkTimeMs iCurrentPosition;
	AkTimeMs iPreEntryDuration;
	AkTimeMs iActiveDuration;
	AkTimeMs iPostExitDuration;
	AkTimeMs iRemainingLookAheadTime;
};

/** DISHONORED(layout): 24 bytes; szFile is wchar_t* on Windows. */
struct AkExternalSourceInfo
{
	AkUInt32	iExternalSrcCookie;
	AkCodecID	idCodec;
	AkOSChar *	szFile;
	void *		pInMemory;
	AkUInt32	uiMemorySize;
	AkFileID	idFile;
};

struct AkExternalSourceArray;

/** DISHONORED(layout): 16 bytes, align 8 (AK::SoundEngine::PostEvent's AkCustomParamType overload). */
struct AkCustomParamType
{
	AkInt64					customParam;
	AkUInt32				ui32Reserved;
	AkExternalSourceArray *	pExternalSrcs;
};

/** DISHONORED(layout): 12 bytes (AkInitSettings / AkDeviceSettings embed three of them). */
struct AkThreadProperties
{
	AkInt32		nPriority;
	AkUInt32	dwAffinityMask;
	AkUInt32	uStackSize;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkVector) == 12, "AkVector: 2012 PDB sizeof 12");
static_assert(sizeof(AkSoundPosition) == 24, "AkSoundPosition: 2012 PDB sizeof 24");
static_assert(sizeof(AkListenerPosition) == 36, "AkListenerPosition: 2012 PDB sizeof 36");
static_assert(sizeof(AkSpeakerVolumes) == 24, "AkSpeakerVolumes: 2012 PDB sizeof 24");
static_assert(sizeof(AkEnvironmentValue) == 8, "AkEnvironmentValue: 2012 PDB sizeof 8");
static_assert(sizeof(AkObjectInfo) == 12, "AkObjectInfo: 2012 PDB sizeof 12");
static_assert(sizeof(AkPositioningInfo) == 44, "AkPositioningInfo: 2012 PDB sizeof 44");
static_assert(sizeof(AkSegmentInfo) == 20, "AkSegmentInfo: 2012 PDB sizeof 20");
static_assert(sizeof(AkExternalSourceInfo) == 24, "AkExternalSourceInfo: 2012 PDB sizeof 24");
static_assert(sizeof(AkCustomParamType) == 16, "AkCustomParamType: 2012 PDB sizeof 16");
static_assert(sizeof(AkThreadProperties) == 12, "AkThreadProperties: 2012 PDB sizeof 12");
#endif

#endif // _AK_TYPES_H_
