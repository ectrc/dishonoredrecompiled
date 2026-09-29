#pragma once
// DishonoredGame/inc/dishonoredutilities_saveload.h
// The save/load layer of DishonoredGame. Retail declares all of DisSaveLoad here (the PDB attributes every
// _DishonoredGetScriptStruct<> instantiation of the module to this header, which is how we know it is the one
// save header the module includes); there is no dissavegame.h in the retail source list.
// Ported by agent CF (PHASE9 CF). Layouts are the 2012 Shipping PDB (DIA dump, build/agentCF/types_saveload.txt);
// bodies are the 2013 retail decompiles, with the 2012 decompile beside them where retail has no match.
//
// The file format this header describes, proved against all 51 retail .sav files on this machine
// (build/agentCF/parse_sav.py, and DisSaveGameSelfTest below):
//   INT     SaveLoadVersion   == GBuiltFromChangeList, must be >= 258971 and <= ours
//   INT     SaveVersion       == 24 (DIS_SAVE_VERSION), only present when SaveLoadVersion > 258971
//   FString SaveDetails       the FMapConfig friendly name, e.g. "1 - Coldridge Prison"
//   INT     DLCMask           UArkDLCManagementBridge's installed-content bits
//   BYTE    bHasObjectMarkers
//   TArray<BYTE> CompressedStringDictionary   (64 KB zlib blocks)
//   TArray<BYTE> CompressedGameState          (64 KB zlib blocks)
//   per level state, in m_LevelStates order:
//     TArray<BYTE> CompressedObjectDictionary
//     TArray<BYTE> CompressedObjectData

// DISHONORED(layout): 2012 PDB enum ESaveLoadLocation (4 bytes). Moved to Core/Inc/UnObjBas.h by agent ED,
// because UObject's five save virtuals take it (vtable slots 66..70); it is not reflected, so it is C++ only.

// DISHONORED(port): 2013 rva 0x614020 (DisSaveLoad::FGameState::Load's version gate). A save older than 15 or
// newer than 24 is refused; 258971 is the changelist below which the version INT is not in the file at all.
#define DIS_SAVE_VERSION			24
#define DIS_SAVE_VERSION_MIN		15
#define DIS_SAVE_CHANGELIST_MIN		258971

// Slot numbering, from the 2013 name builder (rva 0x5fb660), GDisSaveGameSlotNames (2012 0xe34180),
// GetNextAutoSaveSlot (2013 0x60b050, which writes 13 and 14) and GetNextUserSaveSlot (2013 0x5fb930, which
// keys a 40-entry bitmap on Slot - 16). 2013 put three slots in front of the 2012 numbering, which is why
// Dis_Load clamps to 0..55 and not 2012's 0..52:
//   1      OPTIONS                 (the profile blob, not a game save)
//   2, 3   DLC02_LOW_AUTOSAVE, DLC02_HIGH_AUTOSAVE
//   4..12  DisMission0..8          the per-mission checkpoints
//   13, 14 DisAutoSave0, 1
//   15     DisQuickSave
//   16..55 Dishonored0..39         the user slots, and the only ones the load list shows
// Every name above slot 3 also takes the campaign prefix: "" for the base game, "DLC02_" for The Knife of
// Dunwall and "DLC03_" for The Brigmore Witches.
#define DIS_SAVE_SLOT_OPTIONS			1
#define DIS_SAVE_SLOT_DLC02_LOW			2
#define DIS_SAVE_SLOT_DLC02_HIGH		3
#define DIS_SAVE_SLOT_FIRST_NAMED		4
#define DIS_SAVE_SLOT_FIRST_AUTO		13
#define DIS_SAVE_SLOT_QUICK				15
#define DIS_SAVE_SLOT_FIRST_USER		16
#define DIS_SAVE_SLOT_NUM_USER			40
#define DIS_SAVE_SLOT_MAX				(DIS_SAVE_SLOT_FIRST_USER + DIS_SAVE_SLOT_NUM_USER - 1)

/** DISHONORED(layout): 2012 PDB GDisSaveGameSlotNames, const wchar_t*[12] @0xe34180 (slots 1..12) */
extern const TCHAR* GDisSaveGameSlotNames[12];

/** DISHONORED(port): 2012 rva 0x63ed20 - the slot a save file's base name belongs to, 0 when it is not one of ours */
INT GetSaveGameSlot( const TCHAR* _Path );

/** DISHONORED(written): the directory the lister scans and the writer writes: appGameDir() + SaveData\ through the
    file manager's absolute/user mapping (2012 0x64f3b0, 2013 0x614e20) */
FString DisGetSaveGameDir();
/** DISHONORED(written): DisGetSaveGameDir() + the slot's base name + ".sav" (2013 deleter 0x601950) */
FString DisGetSaveGamePath( INT _Slot );
/** DISHONORED(written): the base name of a slot, i.e. GDisSaveGameSlotNames[Slot-1] or Dishonored<Slot-13> */
FString DisGetSaveGameSlotName( INT _Slot );

/** DISHONORED(layout): 2012 PDB FDisSaveGame, sizeof 32. One row of UDishonoredEngine's save list. */
struct FDisSaveGame
{
	INT		m_Slot;
	QWORD	m_Time;
	INT		m_MissionIndex;
	FString	m_MapName;
	UBOOL	m_bIsOwner;

	FDisSaveGame()
	:	m_Slot(0)
	,	m_Time(0)
	,	m_MissionIndex(-1)
	,	m_bIsOwner(FALSE)
	{}
};

/** DISHONORED(port): 2013 rva 0x4790 .. 0x48d0 - Arkane's queued-work base, retail Core/Src/UnAsyncWork.cpp:22.
    Declared here because this tree's Core/Inc/UnAsyncWork.h is the reference one and has no FAsyncWorkBase; fold it
    back into Core when that header is reconciled. */
class FAsyncWorkBase : public FQueuedWork
{
public:
	FAsyncWorkBase( FThreadSafeCounter* InWorkCompletionCounter, const TCHAR* InTaskName );
	virtual ~FAsyncWorkBase();

	virtual void DoThreadedWork();
	virtual void Abandon() {}
	virtual void DoWork() = 0;
	virtual void Dispose();

	UBOOL IsDone();

protected:
	FEvent*				DoneEvent;
	FThreadSafeCounter*	WorkCompletionCounter;
};

/** DISHONORED(layout): 2012 PDB FDisAsyncSaveGameLister, sizeof 40 (2013 allocates 0x40 and also carries the DLC
    campaign file-name prefix, 2013 rva 0x608ef0). Scans the save directory and reads each file's header. */
class FDisAsyncSaveGameLister : public FAsyncWorkBase
{
public:
	FDisAsyncSaveGameLister();

	virtual void DoWork();
	void Wait();

	TArray<FDisSaveGame>	m_SaveGames;
	DOUBLE					m_fStartTime;
	DOUBLE					m_fEndTime;
	/** 2013 only (the two DWORDs its constructor zeroes): the DLC02 chaos-marker files exist */
	UBOOL					m_bDLC02LowChaosFound;
	UBOOL					m_bDLC02HighChaosFound;
};

/** DISHONORED(layout): 2012 PDB FDisAsyncSaveGameDeleter, sizeof 28 */
class FDisAsyncSaveGameDeleter : public FAsyncWorkBase
{
public:
	FDisAsyncSaveGameDeleter( const TCHAR* _SaveGamePath );

	virtual void DoWork();

	FString	m_SaveGamePath;
	UBOOL	m_bSucceeded;
};

namespace DisSaveLoad
{
	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FDeletedActor, sizeof 16 */
	struct FDeletedActor
	{
		FName m_LevelName;
		FName m_ActorName;

		FDeletedActor()
		:	m_LevelName(NAME_None)
		,	m_ActorName(NAME_None)
		{}

		UBOOL operator==( const FDeletedActor& Other ) const
		{
			return m_LevelName == Other.m_LevelName && m_ActorName == Other.m_ActorName;
		}
	};

	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FSubLevelState, sizeof 12 */
	struct FSubLevelState
	{
		FName	m_PackageName;
		BYTE	m_Flags;

		FSubLevelState()
		:	m_PackageName(NAME_None)
		,	m_Flags(0)
		{}
	};

	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FLevelState, sizeof 56 */
	struct FLevelState
	{
		FName				m_LevelName;
		FName				m_PackageName;
		ESaveLoadLocation	m_Location;
		WORD				m_NumObjects;
		WORD				m_NumSavedObjects;
		WORD				m_NumSharedObjects;
		WORD				m_NumSavedSharedObjects;
		TArray<BYTE>		m_CompressedObjectDictionary;
		TArray<BYTE>		m_CompressedObjectData;
		INT					m_SaveVersion;

		FLevelState()
		:	m_LevelName(NAME_None)
		,	m_PackageName(NAME_None)
		,	m_Location(SLL_FILE)
		,	m_NumObjects(0)
		,	m_NumSavedObjects(0)
		,	m_NumSharedObjects(0)
		,	m_NumSavedSharedObjects(0)
		,	m_SaveVersion(DIS_SAVE_VERSION)
		{}
	};

	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FGameStateData, sizeof 36 */
	struct FGameStateData
	{
		FName					m_RootLevelName;
		TArray<FSubLevelState>	m_SubLevels;
		FString					m_SaveDetails;
		INT						m_nMissionIndex;

		FGameStateData()
		:	m_RootLevelName(NAME_None)
		,	m_nMissionIndex(-1)
		{}
	};

	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FStringDictionary, sizeof 72. Index 0 is always NAME_None: the
	    constructor interns it, which is why every saved index is >= 1. */
	class FStringDictionary
	{
	public:
		FStringDictionary();										// 2013 rva 0x612f30

		WORD GetStringIndex( const FName& _rName );					// 2013 rva 0x611ab0
		void Save( FArchive& _rArchive );							// 2013 rva 0x5fe5a0
		void Load( FArchive& _rArchive );							// 2013 rva 0x611fb0
		void LoadFName( FArchive& _rArchive, FName& _rName );		// 2013 rva 0x5fe700
		void Clear();												// 2013 rva 0x612130

		INT Num() const { return m_Names.Num(); }
		FName GetName( INT _Index ) const;

	private:
		void Init( INT _NumNames );									// 2013 rva 0x611f40

		TArray<INT>		m_Names;
		TMap<INT,WORD>	m_Dictionary;
	};

	/** DISHONORED(layout): 2012 PDB DisSaveLoad::FGameState, sizeof 136. 2013 allocates 0xC0 and adds the three
	    failure flags, the save version and the DLC mask this header keeps at the end (2013 rva 0x614020 writes
	    +136 / +140 / +144 / +152). */
	class FGameState
	{
	public:
		FGameState();
		~FGameState();

		/** 2013 rva 0x612340 - the whole container: header, dictionary, game state, then each level's two blobs */
		void Save( FArchive& _rArchive, const FString& _rSaveDetails );
		/** 2013 rva 0x614020 - the reading half, including retail's changelist / version / content gates */
		UBOOL Load( FArchive& _rArchive );
		/** 2013 rva 0x5ea560 (2012 0x6300f0) - the header-only read the save lister uses */
		static UBOOL LoadMapName( FArchive& _rArchive, FString& _rMapName );

		// DISHONORED: FGameState::SaveLevel (2013 rva 0x613cb0) and SaveGameState (0x602920) are the writing
		// half and are NOT ported: they stand on FLevelSaver, whose object pass needs every GameSave override,
		// and writing a save with 14 of retail's 112 override classes would produce a file that looks valid and
		// is not. Both are decompiled in build/agentED/dec2012/; see resources/docs/agents/agentED.md.
		/** 2013 rva 0x613db0 (2012 0x659b50) - restores the level state whose name matches, then discards it */
		void LoadLevel( ULevel* _pLevel );
		/** 2013 rva 0x5fe820 (2012 0x644280) - is this object covered by a level state we still hold */
		UBOOL ContainsObjectState( UObject* _pObject ) const;

		INT findLevelIndex( const FName& _rLevelName ) const;		// 2013 rva 0x5fe7a0
		void discardLevelState( INT _Index );						// 2013 rva 0x6122c0
		void DiscardLevelState( const FName& _rLevelName );			// 2013 rva 0x613f10
		void DiscardAllLevelStates();

		FStringDictionary&		GetStringDictionary()			{ return m_StringDictionary; }
		FGameStateData&			GetData()						{ return m_GameStateData; }
		const FGameStateData&	GetData() const					{ return m_GameStateData; }
		INT						NumLevelStates() const			{ return m_LevelStates.Num(); }
		FLevelState&			GetLevelState( INT _Index )		{ return *m_LevelStates(_Index); }
		const FLevelState&		GetLevelState( INT _Index ) const{ return *m_LevelStates(_Index); }
		FLevelState&			AddLevelState();
		TArray<FDeletedActor>&	GetDeletedActors()				{ return m_DeletedActors; }

		UBOOL IsCorrupt() const			{ return m_bCorrupt; }
		UBOOL IsNewerThanBuild() const	{ return m_bSaveIsNewer; }
		UBOOL IsMissingContent() const	{ return m_bMissingContent; }
		INT GetSaveVersion() const		{ return m_SaveVersion; }
		INT GetDLCMask() const			{ return m_DLCMask; }

	private:
		FStringDictionary		m_StringDictionary;
		TArray<FLevelState*>	m_LevelStates;
		TArray<FDeletedActor>	m_DeletedActors;
		FGameStateData			m_GameStateData;
		UBOOL					m_bIsLoadingLevelState;
		UBOOL					m_bSaveIsNewer;
		UBOOL					m_bMissingContent;
		UBOOL					m_bCorrupt;
		INT						m_SaveVersion;
		INT						m_DLCMask;
	};

	/** DISHONORED(port): 2013 rva 0x5ea500 - a sub-level whose streaming object is not shared between save slots */
	UBOOL IsSubLevelUnshared( const ULevel* _pLevel );

	/** DISHONORED(bringup): the dictionary's spawn-on-load records name a DisTweaks object and a transform;
	    retail spawns through UDisTweaksBase::SpawnActor(eDisTweaksSpawnType_InGame, ...). This spawns the
	    tweaks' m_pSpawnedObjectClass and hands it the tweaks, which is the part the transform needs. */
	class AActor* DisSpawnActorFromTweaks( class UDisTweaksBase* _pTweaks, const FVector& _rLocation, const FRotator& _rRotation );

	/** DISHONORED(port): 2013 rva 0x602760 (2012 0x649130) - re-links a loaded level's Kismet sequence */
	void InitializeSequencePostLoad( USequence* _pSequence );

	/** DISHONORED(written): the object layer's own census, so a load can be measured rather than believed.
	    Every counter is cumulative over the level states of one FGameState::Load, and m_BytesExpected is the
	    uncompressed length of the level's CompressedObjectData, which is what proves the stream stayed in
	    step: a desynchronised read stops early or runs off the end, and either way the two differ. */
	struct FSaveLoadCensus
	{
		INT		m_NumLevels;
		INT		m_DictObjects;			// records read out of the object dictionaries
		INT		m_DictExpected;			// the sum of the level states' m_NumObjects
		INT		m_DictBytesRead;
		INT		m_DictBytesExpected;
		INT		m_ObjectsRestored;		// objects whose GameLoad ran
		INT		m_ObjectsSkipped;		// indices read whose object was NULL or not loadable
		INT		m_DataBytesRead;
		INT		m_DataBytesExpected;
		INT		m_Spawned;				// actors spawned by the dictionary
		INT		m_NotFound;				// dictionary records whose object could not be resolved
		INT		m_DictResolved;			// dictionary records that did resolve to a live object
		INT		m_UnportedClasses;		// objects reached whose class has no GameLoad of its own
		INT		m_NullObjects;			// object-data indices that resolved to no live object
		INT		m_UntrustedSkips;		// objects skipped whose IsSaveable answer this tree cannot vouch for
		INT		m_PartialBodies;		// bodies ported only part of the way, which stop the stream where they end
		INT		m_PostGameLoad;
		UBOOL	m_bDesynchronised;

		FSaveLoadCensus() { Reset(); }
		void Reset() { appMemzero( this, sizeof(FSaveLoadCensus) ); }
		void Log( const TCHAR* _Tag ) const;
	};

	/** DISHONORED(written): the live census of the last FGameState level restore */
	extern FSaveLoadCensus GSaveLoadCensus;

	/** DISHONORED(written): the uncompressed length of one of the save's compressed blobs, read out of the
	    FArchive::SerializeCompressed block summaries (PACKAGE_FILE_TAG, chunk size, summary, chunk infos).
	    This is the "bytes expected" half of the census. */
	INT GetUncompressedSize( const TArray<BYTE>& _rCompressed );

	// DISHONORED: DisSaveLoad::FLevelSaver (2013 rva 0x615730) is NOT ported. It is the writing half of
	// the object layer: its constructor registers the dictionary seeds, walks the level's actors and its
	// Kismet sequence through operator<<(UObject*&), and writes each object's GameSave inline. With 14 of
	// retail's 112 override classes in this tree it would write a save that looks valid and is not, so it
	// is left out rather than left half-right. The 2012 decompiles are in build/agentED/dec2012/ and the
	// flag offsets its level selection needs are resolved in resources/docs/agents/agentED.md.

	/** DISHONORED(port): 2013 rva 0x613070 (2012 0x658d80) - reads one level's objects back. As with the
	    saver, the constructor is the pass: it destroys the save's deleted actors, rebuilds the object table
	    from the dictionary (spawning the actors the dictionary says to spawn), then walks the object data
	    through operator<<(UObject*&) until the terminating index 0. */
	class FLevelLoader : public FArchiveLoadCompressedProxy
	{
	public:
		FLevelLoader( FStringDictionary& _rStringDictionary, TArray<BYTE>& _rCompressedObjectData,
					  TArray<BYTE>& _rCompressedObjectDictionary, ULevel* _pLevel, const FName& _rPackageName,
					  ESaveLoadLocation _Location, WORD _NumObjects, WORD _NumSavedObjects,
					  UBOOL _bReadUnshared, INT _SaveVersion );
		virtual ~FLevelLoader();

		virtual FArchive& operator<<( FName& _rName );				// 2013 rva 0x5fe780
		virtual FArchive& operator<<( UObject*& _rpObject );			// 2013 rva 0x60c930
		/** DISHONORED(written): after an abort, reads return zeros and the stream is not touched again */
		virtual void Serialize( void* _pData, INT _Count );

		void NotifyPostGameLoad();									// 2013 rva 0x60bbc0

		/** DISHONORED(written): a ported body that reaches a branch this tree does not have stops the stream
		    through this rather than reading on. Retail has nothing like it: retail cannot be missing a branch. */
		void Abort() { m_bAborted = TRUE; }

		/** a dictionary record whose object was not in memory when the dictionary was read */
		struct FObjectRef
		{
			UClass*	m_pClass;
			WORD	m_OuterIndex;
			FName	m_Name;

			FObjectRef() : m_pClass(NULL), m_OuterIndex(0), m_Name(NAME_None) {}
		};

	private:
		UBOOL ShouldLoadObject( UObject* _pObject );				// 2013 rva 0x607b30
		/** DISHONORED(written): one dictionary record's object, constructing it and its outers from what the
		    record remembers when this session does not have them. Retail reads m_Objects directly. */
		UObject* ResolveRecord( WORD _Index );
		/** DISHONORED(written): add one dictionary record to the three parallel arrays */
		void AddRecord( UObject* _pObject, UClass* _pRecordClass, UBOOL _bIsClass );

		FStringDictionary&		m_rStringDictionary;
		TArray<UObject*>		m_Objects;
		/** DISHONORED(written): parallel to m_Objects - the class each dictionary record declared, and whether
		    the record was a UClass. The record's shape is read from these rather than from the live object,
		    so a lookup that misses cannot change how many bytes the next record is. */
		TArray<UClass*>			m_RecordClasses;
		TArray<UBOOL>			m_RecordIsClass;
		/** DISHONORED(written): how many dictionary records resolved to a live object */
		INT						m_NumResolved;
		TMap<WORD,FObjectRef>	m_NotFoundObjects;
		TSet<UObject*>			m_LoadedObjects;
		ESaveLoadLocation		m_Location;
		/** DISHONORED(written): set when an object is reached whose GameLoad this tree has not ported. The
		    stream carries no length prefix, so the only safe thing left is to stop. */
		UBOOL					m_bAborted;
		/** DISHONORED(written): -disdictdebug=<n> logs the first n dictionary records and how each resolved */
		INT						m_DebugRecords;
		/** DISHONORED(written): the uncompressed length of the object-data blob. Retail never needs it: its
		    stream is always in step, so the terminating index 0 arrives before the end. A tree with a partial
		    override set can run off the end instead, and FArchiveLoadCompressedProxy asserts there. */
		INT						m_DataSize;
		/** DISHONORED(written): -disstreamdebug=<n> logs the first n object references the data stream reads,
		    each with the byte offset it was read at and how it resolved. The gap between two lines is the
		    previous object's body, which is what says where a partial override set lost step. */
		INT						m_DebugStreamObjects;
		INT						m_DebugStreamSeen;
	};

} // namespace DisSaveLoad

/** DISHONORED(port): 2013 rva 0x7e6ce0 (2012 0x8245e0) - the level's package name, or the persistent level's */
FName DisGetLevelName( class ULevel* _pLevel );
/** DISHONORED(port): 2013 rva 0x7ef1e0 (2012 0x82f2e0) */
class ULevel* DisFindLevelFromName( const FName& _rLevelName );
/** DISHONORED(port): 2013 rva 0x7ef0c0 (2012 0x82f1c0) */
class ULevel* DisGetCurrentLevel();

/** DISHONORED(written): the current installed-content mask retail writes into the save header, from
    UGameEngine::DLCManagementBridge (2013 rva 0xb7aa0 over the bridge's per-entry state bytes) */
INT DisGetInstalledContentMask();

/** DISHONORED(port): 2013 rva 0x7e6cc0 (2012 0x8245c0), dishonoredutilities_saveload.cpp - every script
    property of the object's own class, binary and untagged. Most of the small GameSave/GameLoad bodies are
    nothing but this. */
void DisSaveLoadObject( FArchive& _rArchive, class UObject* _pObject );

/** DISHONORED(written): -savetest - reads every save in the directory through the ported container and, with
    -savetestwrite, writes one back out and re-reads it. Defined in dishonoredengine.cpp. */
void DisSaveGameSelfTest();
