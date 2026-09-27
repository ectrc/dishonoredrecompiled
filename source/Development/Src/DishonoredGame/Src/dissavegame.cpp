// DishonoredGame/src/dissavegame.cpp
// DisSaveLoad: the save-game container and its string dictionary. Ported by agent CF (PHASE9 CF) from the
// 2013 retail decompiles, with the 2012 Shipping decompile (which carries the PDB names) beside each one.
//
// What is here is the whole file format: the header, the name dictionary, the game-state table (sub-levels,
// level states, mission index) and each level's two already-compressed object blobs. What is NOT here is the
// object layer - FLevelSaver / FLevelLoader and FGameState::SaveLevel / LoadLevel - because it stands on
// UObject::GameSave / GameLoad / IsSaveable / IsRefSaveable / PostGameLoad, which this tree does not declare
// on UObject at all: retail has 105 GameSave and 108 GameLoad overrides, and the object stream is not
// self-delimiting (operator<<(UObject*&) writes a WORD index and then the object's own GameSave inline), so a
// single missing override desynchronises everything after it. See resources/docs/agents/agentCF.md section 5.
//
// PDB functions attributed to this file (30), with their 2013 addresses where the matcher has one:
//   0x6300c0 / 0x5ea500  DisSaveLoad::IsSubLevelUnshared
//   0x6300f0 / 0x5ea560  DisSaveLoad::FGameState::LoadMapName
//   0x643ff0 / 0x5fe5a0  DisSaveLoad::FStringDictionary::Save
//   0x644160 / 0x5fe700  DisSaveLoad::FStringDictionary::LoadFName
//   0x6441e0 / 0x5fe780  DisSaveLoad::FLevelLoader::operator<<(FName&)                 [object layer, not ported]
//   0x644200 / 0x5fe7a0  DisSaveLoad::FGameState::findLevelIndex
//   0x644280 / 0x5fe820  DisSaveLoad::FGameState::ContainsObjectState                  [object layer, not ported]
//   0x649130 / 0x602760  DisSaveLoad::InitializeSequencePostLoad                       [object layer, not ported]
//   0x6492e0 / 0x602920  DisSaveLoad::FGameState::SaveGameState                        [object layer, not ported]
//   0x64d0b0 / 0x607ac0  DisSaveLoad::FLevelSaver::ShouldSaveObject                    [object layer, not ported]
//   0x64d120 / 0x607b30  DisSaveLoad::FLevelLoader::ShouldLoadObject                   [object layer, not ported]
//   0x64ff80 / 0x609d70  DisSaveLoad::FLevelSaver::SerializeLevel                      [object layer, not ported]
//   0x6516f0 / 0x60bbc0  DisSaveLoad::FLevelLoader::NotifyPostGameLoad                 [object layer, not ported]
//   0x653700 / 0x60c930  DisSaveLoad::FLevelLoader::operator<<(UObject*&)              [object layer, not ported]
//   0x6562a0 / 0x611ab0  DisSaveLoad::FStringDictionary::GetStringIndex
//   0x656320 / 0x611b30  DisSaveLoad::FLevelSaver::operator<<(FName&)                  [object layer, not ported]
//   0x656370 / 0x611b80  DisSaveLoad::FLevelSaver::GetObjectIndex                      [object layer, not ported]
//   0x6573f0 / 0x611f40  DisSaveLoad::FStringDictionary::Init
//   0x657460 / 0x611fb0  DisSaveLoad::FStringDictionary::Load
//   0x6575e0 / 0x612130  DisSaveLoad::FStringDictionary::Clear
//   0x6576c0 / 0x612190  DisSaveLoad::FLevelSaver::operator<<(UObject*&)               [object layer, not ported]
//   0x6577f0 / 0x6122c0  DisSaveLoad::FGameState::discardLevelState
//   0x657870 / 0x612340  DisSaveLoad::FGameState::Save
//   0x658630 / 0x612f30  DisSaveLoad::FStringDictionary::FStringDictionary
//   0x6586d0 / 0x615730  DisSaveLoad::FLevelSaver::FLevelSaver                         [object layer, not ported]
//   0x658d80 / 0x613070  DisSaveLoad::FLevelLoader::FLevelLoader                       [object layer, not ported]
//   0x659a60 / -         DisSaveLoad::FGameState::SaveLevel                            [object layer, not ported]
//   0x659b50 / 0x613db0  DisSaveLoad::FGameState::LoadLevel                            [object layer, not ported]
//   0x659cb0 / 0x613f10  DisSaveLoad::FGameState::DiscardLevelState
//   0x659db0 / 0x614020  DisSaveLoad::FGameState::Load

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "dishonoredutilities_saveload.h"

namespace DisSaveLoad
{

/*-----------------------------------------------------------------------------
	FStringDictionary
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x612f30 (2012 0x658630) - an empty dictionary already holds NAME_None at index 0
FStringDictionary::FStringDictionary()
{
	Init( 1 );
}

// DISHONORED(port): 2013 rva 0x611f40 (2012 0x6573f0)
void FStringDictionary::Init( INT _NumNames )
{
	m_Names.Empty( _NumNames );
	m_Dictionary.Empty( _NumNames );
	GetStringIndex( FName(NAME_None) );
}

// DISHONORED(port): 2013 rva 0x612130 (2012 0x6575e0)
void FStringDictionary::Clear()
{
	Init( 1 );
}

// DISHONORED(port): 2013 rva 0x611ab0 (2012 0x6562a0). Keyed on FName::Index only, so every Number of a name
// shares one dictionary slot and the Number travels beside the index in the stream.
WORD FStringDictionary::GetStringIndex( const FName& _rName )
{
	const INT NameIdx = _rName.GetIndex();
	WORD* Existing = m_Dictionary.Find( NameIdx );
	if( Existing )
	{
		return *Existing;
	}
	const WORD NewIndex = (WORD)m_Names.AddItem( NameIdx );
	m_Dictionary.Set( NameIdx, NewIndex );
	return NewIndex;
}

// the dictionary stores raw FName::Index values, exactly as retail does, so the base name comes back through
// FName's (EName, Number) constructor - the same one ULinkerLoad uses for a package's name table
FName FStringDictionary::GetName( INT _Index ) const
{
	if( !m_Names.IsValidIndex( _Index ) )
	{
		return FName(NAME_None);
	}
	return FName( (EName)m_Names(_Index), NAME_NO_NUMBER_INTERNAL );
}

// DISHONORED(port): 2013 rva 0x5fe5a0 (2012 0x643ff0). Index 0 is NAME_None and is never written; each name
// goes out as its ANSI characters plus the terminator, so the stream is NUL separated and not length prefixed.
void FStringDictionary::Save( FArchive& _rArchive )
{
	INT NumNames = m_Names.Num();
	_rArchive.ByteOrderSerialize( &NumNames, sizeof(NumNames) );
	for( INT NameIdx = 1; NameIdx < NumNames; NameIdx++ )
	{
		const FString NameString = GetName( NameIdx ).GetNameString();
		_rArchive.Serialize( (void*)(const ANSICHAR*)TCHAR_TO_ANSI(*NameString), NameString.Len() + 1 );
	}
}

// DISHONORED(port): 2013 rva 0x611fb0 (2012 0x657460). FNAME_Add, so a save's names enter this run's name table.
void FStringDictionary::Load( FArchive& _rArchive )
{
	INT NumNames = 0;
	_rArchive.ByteOrderSerialize( &NumNames, sizeof(NumNames) );
	Init( NumNames );
	if( NumNames > 1 )
	{
		m_Names.Add( NumNames - 1 );
	}
	ANSICHAR TmpStr[1024];
	for( INT NameIdx = 1; NameIdx < NumNames; NameIdx++ )
	{
		ANSICHAR* Cursor = TmpStr;
		do
		{
			_rArchive.Serialize( Cursor, 1 );
		}
		while( *Cursor++ && Cursor < TmpStr + ARRAY_COUNT(TmpStr) - 1 );
		*Cursor = 0;

		const FName Name( ANSI_TO_TCHAR(TmpStr), FNAME_Add, TRUE );
		m_Names(NameIdx) = Name.GetIndex();
		m_Dictionary.Set( (INT)Name.GetIndex(), (WORD)NameIdx );
	}
}

// DISHONORED(port): 2013 rva 0x5fe700 (2012 0x644160) - WORD dictionary index then WORD FName::Number
void FStringDictionary::LoadFName( FArchive& _rArchive, FName& _rName )
{
	WORD StrIndex = 0;
	WORD NameNumber = 0;
	_rArchive.ByteOrderSerialize( &StrIndex, sizeof(StrIndex) );
	_rArchive.ByteOrderSerialize( &NameNumber, sizeof(NameNumber) );
	// FName::GetNumber() is the raw instance number in this engine, so the WORD in the file is that field
	_rName = m_Names.IsValidIndex( StrIndex ) ? FName( (EName)m_Names(StrIndex), (INT)NameNumber ) : FName(NAME_None);
}

/*-----------------------------------------------------------------------------
	FGameState
-----------------------------------------------------------------------------*/

FGameState::FGameState()
:	m_bIsLoadingLevelState(FALSE)
,	m_bSaveIsNewer(FALSE)
,	m_bMissingContent(FALSE)
,	m_bCorrupt(FALSE)
,	m_SaveVersion(DIS_SAVE_VERSION)
,	m_DLCMask(0)
{
}

FGameState::~FGameState()
{
	DiscardAllLevelStates();
}

// DISHONORED(port): 2013 rva 0x5fe7a0 (2012 0x644200)
INT FGameState::findLevelIndex( const FName& _rLevelName ) const
{
	for( INT LevelIdx = 0; LevelIdx < m_LevelStates.Num(); LevelIdx++ )
	{
		if( m_LevelStates(LevelIdx)->m_LevelName == _rLevelName )
		{
			return LevelIdx;
		}
	}
	return INDEX_NONE;
}

// DISHONORED(port): 2013 rva 0x6122c0 (2012 0x6577f0) - the dictionary is cleared once the last state is gone
void FGameState::discardLevelState( INT _Index )
{
	if( !m_LevelStates.IsValidIndex( _Index ) )
	{
		return;
	}
	delete m_LevelStates(_Index);
	m_LevelStates.Remove( _Index, 1 );
	if( m_LevelStates.Num() == 0 )
	{
		m_StringDictionary.Clear();
	}
}

// DISHONORED(port): 2013 rva 0x613f10 (2012 0x659cb0) - NAME_None means "all of them"
void FGameState::DiscardLevelState( const FName& _rLevelName )
{
	if( _rLevelName != NAME_None )
	{
		const INT LevelIndex = findLevelIndex( _rLevelName );
		if( LevelIndex != INDEX_NONE )
		{
			discardLevelState( LevelIndex );
		}
		return;
	}
	DiscardAllLevelStates();
}

void FGameState::DiscardAllLevelStates()
{
	for( INT LevelIdx = 0; LevelIdx < m_LevelStates.Num(); LevelIdx++ )
	{
		delete m_LevelStates(LevelIdx);
	}
	m_LevelStates.Empty();
	m_StringDictionary.Clear();
}

FLevelState& FGameState::AddLevelState()
{
	FLevelState* State = new FLevelState();
	m_LevelStates.AddItem( State );
	return *State;
}

// DISHONORED(port): 2013 rva 0x5ea560 (2012 0x6300f0). The save lister reads nothing but this: the changelist,
// then - only when the file is new enough to carry one - the save version, then the friendly map name.
// 2012 stopped at the changelist equality test; 2013 accepts the whole supported range, which is why its saves
// carry the extra INT. Kept as 2013 has it, because a retail file is a 2013 file.
UBOOL FGameState::LoadMapName( FArchive& _rArchive, FString& _rMapName )
{
	INT SaveLoadVersion = 0;
	_rArchive.ByteOrderSerialize( &SaveLoadVersion, sizeof(SaveLoadVersion) );
	if( SaveLoadVersion < DIS_SAVE_CHANGELIST_MIN || SaveLoadVersion > GBuiltFromChangeList )
	{
		return FALSE;
	}
	INT SaveVersion = -1;
	if( SaveLoadVersion > DIS_SAVE_CHANGELIST_MIN )
	{
		_rArchive.ByteOrderSerialize( &SaveVersion, sizeof(SaveVersion) );
		if( SaveVersion > DIS_SAVE_VERSION || SaveVersion < DIS_SAVE_VERSION_MIN )
		{
			return FALSE;
		}
	}
	_rArchive << _rMapName;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x612340 (2012 0x657870). The game state is compressed first because writing it
// is what interns the names, and the dictionary is written to the file first because reading it is what turns
// those indices back into names.
void FGameState::Save( FArchive& _rArchive, const FString& _rSaveDetails )
{
	m_GameStateData.m_SaveDetails = _rSaveDetails;

	INT SaveLoadVersion = GBuiltFromChangeList;
	_rArchive.ByteOrderSerialize( &SaveLoadVersion, sizeof(SaveLoadVersion) );
	INT SaveVersion = DIS_SAVE_VERSION;
	_rArchive.ByteOrderSerialize( &SaveVersion, sizeof(SaveVersion) );
	FString SaveDetails = _rSaveDetails;
	_rArchive << SaveDetails;
	INT DLCMask = DisGetInstalledContentMask();
	_rArchive.ByteOrderSerialize( &DLCMask, sizeof(DLCMask) );
	BYTE bHasObjectMarkers = 0;
	_rArchive.Serialize( &bHasObjectMarkers, sizeof(bHasObjectMarkers) );

	TArray<BYTE> CompressedGameState;
	{
		FArchiveSaveCompressedProxy GameStateSaver( CompressedGameState, (ECompressionFlags)(COMPRESS_ZLIB|COMPRESS_BiasSpeed), 0x10000 );
		GameStateSaver.SetVer( SaveVersion );

		BYTE NumSubLevels = (BYTE)m_GameStateData.m_SubLevels.Num();
		GameStateSaver.Serialize( &NumSubLevels, sizeof(NumSubLevels) );
		for( INT SubLevelIdx = 0; SubLevelIdx < NumSubLevels; SubLevelIdx++ )
		{
			FSubLevelState& SubLevel = m_GameStateData.m_SubLevels(SubLevelIdx);
			WORD StrIndex = m_StringDictionary.GetStringIndex( SubLevel.m_PackageName );
			WORD NameNumber = (WORD)SubLevel.m_PackageName.GetNumber();
			GameStateSaver.ByteOrderSerialize( &StrIndex, sizeof(StrIndex) );
			GameStateSaver.ByteOrderSerialize( &NameNumber, sizeof(NameNumber) );
			GameStateSaver.Serialize( &SubLevel.m_Flags, sizeof(SubLevel.m_Flags) );
		}

		BYTE NumLevelStates = (BYTE)m_LevelStates.Num();
		GameStateSaver.Serialize( &NumLevelStates, sizeof(NumLevelStates) );
		for( INT LevelIdx = 0; LevelIdx < NumLevelStates; LevelIdx++ )
		{
			FLevelState& LevelState = *m_LevelStates(LevelIdx);
			WORD StrIndex = m_StringDictionary.GetStringIndex( LevelState.m_LevelName );
			WORD NameNumber = (WORD)LevelState.m_LevelName.GetNumber();
			GameStateSaver.ByteOrderSerialize( &StrIndex, sizeof(StrIndex) );
			GameStateSaver.ByteOrderSerialize( &NameNumber, sizeof(NameNumber) );
			StrIndex = m_StringDictionary.GetStringIndex( LevelState.m_PackageName );
			NameNumber = (WORD)LevelState.m_PackageName.GetNumber();
			GameStateSaver.ByteOrderSerialize( &StrIndex, sizeof(StrIndex) );
			GameStateSaver.ByteOrderSerialize( &NameNumber, sizeof(NameNumber) );
			BYTE SaveLoadLocation = (BYTE)LevelState.m_Location;
			GameStateSaver.Serialize( &SaveLoadLocation, sizeof(SaveLoadLocation) );
			GameStateSaver.ByteOrderSerialize( &LevelState.m_NumObjects, sizeof(LevelState.m_NumObjects) );
			GameStateSaver.ByteOrderSerialize( &LevelState.m_NumSavedObjects, sizeof(LevelState.m_NumSavedObjects) );
			GameStateSaver.ByteOrderSerialize( &LevelState.m_NumSharedObjects, sizeof(LevelState.m_NumSharedObjects) );
			GameStateSaver.ByteOrderSerialize( &LevelState.m_NumSavedSharedObjects, sizeof(LevelState.m_NumSavedSharedObjects) );
			GameStateSaver.ByteOrderSerialize( &LevelState.m_SaveVersion, sizeof(LevelState.m_SaveVersion) );
			// the two reserved DWORDs retail writes as literal zeroes and reads into a scratch INT
			INT Zero = 0;
			GameStateSaver.ByteOrderSerialize( &Zero, sizeof(Zero) );
			GameStateSaver.ByteOrderSerialize( &Zero, sizeof(Zero) );
		}
		GameStateSaver.ByteOrderSerialize( &m_GameStateData.m_nMissionIndex, sizeof(m_GameStateData.m_nMissionIndex) );
	}

	TArray<BYTE> CompressedStringDictionary;
	{
		FArchiveSaveCompressedProxy DictionarySaver( CompressedStringDictionary, (ECompressionFlags)(COMPRESS_ZLIB|COMPRESS_BiasSpeed), 0x10000 );
		DictionarySaver.SetVer( SaveVersion );
		m_StringDictionary.Save( DictionarySaver );
	}

	_rArchive << CompressedStringDictionary;
	_rArchive << CompressedGameState;

	for( INT LevelIdx = 0; LevelIdx < m_LevelStates.Num(); LevelIdx++ )
	{
		FLevelState& LevelState = *m_LevelStates(LevelIdx);
		_rArchive << LevelState.m_CompressedObjectDictionary;
		_rArchive << LevelState.m_CompressedObjectData;
	}
}

// DISHONORED(port): 2013 rva 0x614020 (2012 0x659db0). The three failure modes are retail's own and are what
// UDisGFxMoviePlayerGlobal::OnSaveGameLoadCorrupt / OnSaveGameOwnerFailure (2013 0x5f6e80 / 0x5f6df0) report.
UBOOL FGameState::Load( FArchive& _rArchive )
{
	DiscardAllLevelStates();
	m_bSaveIsNewer = FALSE;
	m_bMissingContent = FALSE;
	m_bCorrupt = FALSE;

	INT SaveLoadVersion = 0;
	_rArchive.ByteOrderSerialize( &SaveLoadVersion, sizeof(SaveLoadVersion) );
	m_SaveVersion = -1;
	if( SaveLoadVersion > DIS_SAVE_CHANGELIST_MIN )
	{
		_rArchive.ByteOrderSerialize( &m_SaveVersion, sizeof(m_SaveVersion) );
	}
	_rArchive << m_GameStateData.m_SaveDetails;

	if( m_SaveVersion > DIS_SAVE_VERSION )
	{
		m_bSaveIsNewer = TRUE;
		return FALSE;
	}
	if( SaveLoadVersion < DIS_SAVE_CHANGELIST_MIN || SaveLoadVersion > GBuiltFromChangeList )
	{
		warnf( TEXT("Failed to read savegame!  File version did not match.  Expected [cl:%d] but found [cl:%d]"), GBuiltFromChangeList, SaveLoadVersion );
		m_bCorrupt = TRUE;
		return FALSE;
	}
	if( m_SaveVersion < DIS_SAVE_VERSION_MIN )
	{
		warnf( TEXT("Failed to read savegame!  File version did not match.  Expected at least v%d but found [v %d (cl %d)]"), DIS_SAVE_VERSION_MIN, m_SaveVersion, SaveLoadVersion );
		m_bCorrupt = TRUE;
		return FALSE;
	}
	if( SaveLoadVersion != GBuiltFromChangeList )
	{
		warnf( TEXT("Warning Save have been made with an old build and may result in hazardous behaviour. save made with CL[%d] but loaded with CL[%d]"), SaveLoadVersion, GBuiltFromChangeList );
	}

	const INT CurrentContentMask = DisGetInstalledContentMask();
	_rArchive.ByteOrderSerialize( &m_DLCMask, sizeof(m_DLCMask) );
	// retail's escape hatch here is a build-configuration query (2013 rva 0xbbf150); only the low four bits,
	// i.e. the campaign packs, gate a load
	if( ((BYTE)m_DLCMask & (BYTE)~(BYTE)CurrentContentMask & 0x0f) != 0 )
	{
		m_bMissingContent = TRUE;
		return FALSE;
	}

	BYTE bHasObjectMarkers = 0;
	_rArchive.Serialize( &bHasObjectMarkers, sizeof(bHasObjectMarkers) );

	{
		TArray<BYTE> CompressedStringDictionary;
		_rArchive << CompressedStringDictionary;
		FArchiveLoadCompressedProxy DictionaryLoader( CompressedStringDictionary, (ECompressionFlags)(COMPRESS_ZLIB|COMPRESS_BiasSpeed), 0x10000 );
		DictionaryLoader.SetVer( m_SaveVersion );
		m_StringDictionary.Load( DictionaryLoader );
	}

	INT NumLevelStates = 0;
	{
		TArray<BYTE> CompressedGameState;
		_rArchive << CompressedGameState;
		FArchiveLoadCompressedProxy GameStateLoader( CompressedGameState, (ECompressionFlags)(COMPRESS_ZLIB|COMPRESS_BiasSpeed), 0x10000 );
		GameStateLoader.SetVer( m_SaveVersion );

		BYTE NumSubLevels = 0;
		GameStateLoader.Serialize( &NumSubLevels, sizeof(NumSubLevels) );
		m_GameStateData.m_SubLevels.Empty( NumSubLevels );
		m_GameStateData.m_SubLevels.Add( NumSubLevels );
		for( INT SubLevelIdx = 0; SubLevelIdx < NumSubLevels; SubLevelIdx++ )
		{
			FSubLevelState& SubLevel = m_GameStateData.m_SubLevels(SubLevelIdx);
			m_StringDictionary.LoadFName( GameStateLoader, SubLevel.m_PackageName );
			GameStateLoader.Serialize( &SubLevel.m_Flags, sizeof(SubLevel.m_Flags) );
		}

		BYTE NumLevelStatesByte = 0;
		GameStateLoader.Serialize( &NumLevelStatesByte, sizeof(NumLevelStatesByte) );
		NumLevelStates = NumLevelStatesByte;
		for( INT LevelIdx = 0; LevelIdx < NumLevelStates; LevelIdx++ )
		{
			FLevelState& LevelState = AddLevelState();
			m_StringDictionary.LoadFName( GameStateLoader, LevelState.m_LevelName );
			m_StringDictionary.LoadFName( GameStateLoader, LevelState.m_PackageName );
			BYTE SaveLoadLocation = 0;
			GameStateLoader.Serialize( &SaveLoadLocation, sizeof(SaveLoadLocation) );
			LevelState.m_Location = (ESaveLoadLocation)SaveLoadLocation;
			GameStateLoader.ByteOrderSerialize( &LevelState.m_NumObjects, sizeof(LevelState.m_NumObjects) );
			GameStateLoader.ByteOrderSerialize( &LevelState.m_NumSavedObjects, sizeof(LevelState.m_NumSavedObjects) );
			GameStateLoader.ByteOrderSerialize( &LevelState.m_NumSharedObjects, sizeof(LevelState.m_NumSharedObjects) );
			GameStateLoader.ByteOrderSerialize( &LevelState.m_NumSavedSharedObjects, sizeof(LevelState.m_NumSavedSharedObjects) );
			GameStateLoader.ByteOrderSerialize( &LevelState.m_SaveVersion, sizeof(LevelState.m_SaveVersion) );
			INT Reserved = 0;
			GameStateLoader.ByteOrderSerialize( &Reserved, sizeof(Reserved) );
			GameStateLoader.ByteOrderSerialize( &Reserved, sizeof(Reserved) );
		}
		GameStateLoader.ByteOrderSerialize( &m_GameStateData.m_nMissionIndex, sizeof(m_GameStateData.m_nMissionIndex) );
	}

	// the map to open is the last level state's package: retail copies that FName into m_RootLevelName here
	if( m_LevelStates.Num() > 0 )
	{
		m_GameStateData.m_RootLevelName = m_LevelStates(m_LevelStates.Num() - 1)->m_PackageName;
	}

	for( INT LevelIdx = 0; LevelIdx < NumLevelStates; LevelIdx++ )
	{
		FLevelState& LevelState = *m_LevelStates(LevelIdx);
		_rArchive << LevelState.m_CompressedObjectDictionary;
		_rArchive << LevelState.m_CompressedObjectData;
	}

	m_bCorrupt = FALSE;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x5ea500 (2012 0x6300c0) - the streaming level's own "not shared" bit
UBOOL IsSubLevelUnshared( const ULevel* _pLevel )
{
	if( _pLevel == NULL )
	{
		return FALSE;
	}
	// DISHONORED(bringup): retail reads ULevelStreaming::m_bConsiderForPartialSaves through the level's
	// streaming object (ULevelStreamingAlwaysLoaded::m_bConsiderForPartialSaves is the reflected bit agent S
	// added); the partial-save paths that need it are SLL_MEMORY_*, which only the object layer uses.
	return FALSE;
}

} // namespace DisSaveLoad

/*-----------------------------------------------------------------------------
	the level lookups of dishonoredutilities_saveload.cpp that the container needs
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7e6ce0 (2012 0x8245e0)
FName DisGetLevelName( ULevel* _pLevel )
{
	if( _pLevel == NULL || _pLevel->GetOutermost() == NULL )
	{
		return FName(NAME_None);
	}
	return _pLevel->GetOutermost()->GetFName();
}

// DISHONORED(port): 2013 rva 0x7ef1e0 (2012 0x82f2e0)
ULevel* DisFindLevelFromName( const FName& _rLevelName )
{
	if( GWorld == NULL || _rLevelName == NAME_None )
	{
		return NULL;
	}
	if( DisGetLevelName( GWorld->PersistentLevel ) == _rLevelName )
	{
		return GWorld->PersistentLevel;
	}
	for( INT StreamIdx = 0; StreamIdx < GWorld->GetWorldInfo()->StreamingLevels.Num(); StreamIdx++ )
	{
		ULevelStreaming* Streaming = GWorld->GetWorldInfo()->StreamingLevels(StreamIdx);
		if( Streaming && Streaming->LoadedLevel && DisGetLevelName( Streaming->LoadedLevel ) == _rLevelName )
		{
			return Streaming->LoadedLevel;
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x7ef0c0 (2012 0x82f1c0)
ULevel* DisGetCurrentLevel()
{
	return GWorld ? GWorld->CurrentLevel : NULL;
}
