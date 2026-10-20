/*=============================================================================
	FxArchive.cpp - FaceFX 1.7.3.1 archive (fxsdk\src\fxarchive.cpp, compiland FxArchive.obj).

	DISHONORED(port): agent FX1 (wave 26). See FxArchive.h for the format. Open (0xb180b0) on a loading
	archive accepts "FACE"/"FACB", an sdk version up to ours (or, from 0x6B8, file format version 0), and from
	sdk 0x640 reads the directory and reserves the object table for NumObjectPointers objects; anything else
	closes the store and leaves the archive invalid. Before 0x6A4 each of the two licensee strings is preceded
	by an FxUInt16 version. While loading the directory the old class name "FxDigitalAudio" is renamed to
	"FxAudio". The progress callback is driven by every operator<<; bool travels as an FxUInt32 (true == 1).
	Mounted archives (Mount/UnmountArchive) are the lazy-load registry: FxArray<FxArchive*> allocated by
	Startup.
	Named deviation: FxArchiveDirectory's constructor leaves NumObjectPointers uninitialised in retail
	(0xb1a050); it is 0 here.
	The progress update every operator<< inlines is one private member here (_updateProgress).
=============================================================================*/
#include "FxObject.h"
#include "FxSDK.h"
#include "FxRefObject.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxArchive) == 80, "FxArchive: 2012 PDB sizeof 80");
static_assert(sizeof(FxArchiveInternalData) == 0x638, "FxArchiveInternalData: FxArchive::Open allocates 0x638");
static_assert(sizeof(FxArchiveDirectory) == 1568, "FxArchiveDirectory: 2012 PDB sizeof 1568");
static_assert(sizeof(FxArchiveNameEntry) == 16, "FxArchiveNameEntry: 2012 PDB sizeof 16");
static_assert(sizeof(FxStringHash<FxUInt16, 7, FxArray>) == 1552, "FxStringHash<unsigned short,7,FxArray>: 2012 PDB sizeof 1552");
static_assert(sizeof(FxLazyLoader) == 16, "FxLazyLoader: 2012 PDB sizeof 16");

static FxArray<FxArchive*>* pMountedArchives = NULL;

/** 2012 0xb1b580. */
FxArchiveNameEntry::FxArchiveNameEntry()
	: NameStr(static_cast<FxSize>(0))
	, Name()
{
}

/** 2012 0xb19740. */
FxArchiveNameEntry::FxArchiveNameEntry(const FxString& name)
	: NameStr(name)
	, Name()
{
}

/** 2012 0xb1b610 (compiler generated). */
FxArchiveNameEntry::FxArchiveNameEntry(const FxArchiveNameEntry& other)
	: NameStr(other.NameStr)
	, Name(other.Name)
{
}

FxArchiveNameEntry& FxArchiveNameEntry::operator=(const FxArchiveNameEntry& other)
{
	NameStr = other.NameStr;
	Name = other.Name;
	return *this;
}

/** 2012 0xb19800 (compiler generated). */
FxArchiveNameEntry::~FxArchiveNameEntry()
{
}

/** 2012 0xb17ec0. */
FxArchive& operator<<(FxArchive& arc, FxArchiveNameEntry& name)
{
	if( arc.GetSDKVersion() < 0x6A4 && arc.IsLoading() )
	{
		FxUInt16 version = 0;
		arc << version;
	}
	arc << name.NameStr;
	return arc;
}

/** 2012 0xb1a050. */
FxArchiveDirectory::FxArchiveDirectory()
	: Names(0)
	, NumObjectPointers(0)
{
}

/** 2012 0xb1b720. */
FxArchiveDirectory::~FxArchiveDirectory()
{
}

/** 2012 0xb197a0. */
FxSize FxArchiveDirectory::FindName(const FxString& name) const
{
	FxSize numNames = Names.Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		if( Names[i].NameStr == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb19680: every archive name becomes a live FxName (the table the name indices resolve through). */
void FxArchiveDirectory::InitializeNames()
{
	FxSize numNames = Names.Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		Names[i].Name = FxName(Names[i].NameStr.GetData());
	}
}

/** 2012 0xb19860. */
void FxArchiveDirectory::InsertClassVersion(const FxChar* classname)
{
	const FxClass* pClass = FxClass::FindClassDesc(FxName(classname));
	FxUInt16 classversion = pClass->GetCurrentVersion();
	ClassVersions.Insert(classname, classversion);
}

/** 2012 0xb198f0. */
void FxArchiveDirectory::InsertNonFxObjectDerivedClassVersion(const FxChar* classname, FxUInt16 classversion)
{
	ClassVersions.Insert(classname, classversion);
}

/** 2012 0xb19910. */
FxUInt16 FxArchiveDirectory::FindClassVersion(const FxChar* classname) const
{
	FxUInt16 classversion = 0;
	ClassVersions.Find(classname, classversion);
	return classversion;
}

/** 2012 0xb17f10. */
FxArchive& operator<<(FxArchive& arc, FxArchiveDirectory& directory)
{
	FxUInt16 version = 1;
	arc << version;
	if( version )
	{
		arc << directory.ClassVersions;
	}
	arc << directory.Names << directory.NumObjectPointers;
	if( arc.IsLoading() )
	{
		if( directory.ClassVersions.Find("FxDigitalAudio") )
		{
			FxUInt16 FxDigitalAudioVersion = 0;
			directory.ClassVersions.Remove("FxDigitalAudio", FxDigitalAudioVersion);
			directory.ClassVersions.Insert("FxAudio", FxDigitalAudioVersion);
		}
		directory.InitializeNames();
	}
	return arc;
}

/** 2012 0xb19fe0. */
FxArchiveInternalData::FxArchiveInternalData()
	: ObjectTable(0)
	, LazyLoaders(0)
{
}

/** 2012 0xb1b6b0. */
FxArchiveInternalData::~FxArchiveInternalData()
{
}

/** 2012 0xb17fd0. */
FxArchive::FxArchive(FxArchiveStore* iStore, FxArchiveMode mode, FxArchiveByteOrder byteOrder)
	: _store(iStore)
	, _mode(mode)
	, _byteOrder(byteOrder)
	, _isValid(FxFalse)
	, _fileFormatVersion(0)
	, _sdkVersion(0)
	, _sdkVersionString(NULL)
	, _sdkVersionStringSize(0)
	, _licenseeName(NULL)
	, _licenseeNameSize(0)
	, _licenseeProjectName(NULL)
	, _licenseeProjectNameSize(0)
	, _licenseeVersion(0)
	, _pInternalData(NULL)
	, _progressCallback(NULL)
	, _updateFrequency(0.0f)
	, _numBytesLoaded(0)
	, _numBytesTotal(0.0f)
	, _progress(0.0f)
	, _lastProgress(0.0f)
{
}

/** 2012 0xb18090. */
FxArchive::~FxArchive()
{
	Close();
}

/** 2012 0xb180b0. */
void FxArchive::Open()
{
	_pInternalData = static_cast<FxArchiveInternalData*>(FxAllocate(sizeof(FxArchiveInternalData)));
	FxDefaultConstruct(_pInternalData);
	if( !_store )
	{
		return;
	}
	if( IsSaving() )
	{
		_fileFormatVersion = 0;
		_sdkVersion = FxSDKGetVersion();
		FxTempCharBuffer<FxChar> licenseeNameTempBuffer(FxSDKGetLicenseeName().GetData());
		_licenseeNameSize = licenseeNameTempBuffer.GetBufferSize();
		_licenseeName = licenseeNameTempBuffer.Release();
		FxTempCharBuffer<FxChar> licenseeProjectNameTempBuffer(FxSDKGetLicenseeProjectName().GetData());
		_licenseeProjectNameSize = licenseeProjectNameTempBuffer.GetBufferSize();
		_licenseeProjectName = licenseeProjectNameTempBuffer.Release();
		_licenseeVersion = FxSDKGetLicenseeVersion();
		_store->OpenWrite();
		if( _store->IsValid() )
		{
			_isValid = FxTrue;
			FxChar archiveId[4] = { 'F', 'A', 'C', 'E' };
			if( _byteOrder != ABO_LittleEndian )
			{
				archiveId[3] = 'B';
				_byteOrder = ABO_BigEndian;
			}
			else
			{
				_byteOrder = ABO_LittleEndian;
			}
			*this << archiveId[0];
			*this << archiveId[1];
			*this << archiveId[2];
			*this << archiveId[3];
			*this << _sdkVersion;
			*this << _fileFormatVersion;
			*this << _licenseeNameSize;
			SerializePODArray(_licenseeName, _licenseeNameSize);
			*this << _licenseeProjectNameSize;
			SerializePODArray(_licenseeProjectName, _licenseeProjectNameSize);
			*this << _licenseeVersion;
			_numBytesTotal = 0.0f;
			_isValid = _store->IsValid();
		}
		return;
	}

	_store->OpenRead();
	if( _store->IsValid() )
	{
		_isValid = FxTrue;
	}
	_numBytesTotal = static_cast<FxReal>(_store->Length());
	if( _numBytesTotal <= 4.0f )
	{
		_store->Close();
		_numBytesTotal = 0.0f;
	}
	else
	{
		FxChar archiveId[4];
		*this << archiveId[0];
		*this << archiveId[1];
		*this << archiveId[2];
		*this << archiveId[3];
		if( archiveId[0] == 'F' && archiveId[1] == 'A' && archiveId[2] == 'C' && (archiveId[3] == 'E' || archiveId[3] == 'B') )
		{
			_byteOrder = archiveId[3] != 'E' ? ABO_BigEndian : ABO_LittleEndian;
			*this << _sdkVersion;
			FxBool canLoadData = FxFalse;
			if( _sdkVersion < 0x6B8 )
			{
				if( _sdkVersion <= FxSDKGetVersion() )
				{
					canLoadData = FxTrue;
				}
			}
			else
			{
				*this << _fileFormatVersion;
				if( !_fileFormatVersion )
				{
					canLoadData = FxTrue;
				}
			}
			if( !canLoadData )
			{
				_store->Close();
				_numBytesTotal = 0.0f;
			}
			else
			{
				if( _sdkVersion < 0x6A4 )
				{
					FxUInt16 version = 0;
					*this << version;
				}
				*this << _licenseeNameSize;
				_licenseeName = static_cast<FxChar*>(FxAllocate(_licenseeNameSize + 1));
				SerializePODArray(_licenseeName, _licenseeNameSize);
				_licenseeName[_licenseeNameSize++] = 0;
				if( _sdkVersion < 0x6A4 )
				{
					FxUInt16 version = 0;
					*this << version;
				}
				*this << _licenseeProjectNameSize;
				_licenseeProjectName = static_cast<FxChar*>(FxAllocate(_licenseeProjectNameSize + 1));
				SerializePODArray(_licenseeProjectName, _licenseeProjectNameSize);
				_licenseeProjectName[_licenseeProjectNameSize++] = 0;
				*this << _licenseeVersion;
				if( _sdkVersion >= 0x640 )
				{
					*this << _pInternalData->Directory;
					_pInternalData->ObjectTable.Reserve(_pInternalData->Directory.NumObjectPointers);
				}
			}
		}
		else
		{
			_store->Close();
			_numBytesTotal = 0.0f;
		}
	}
	_isValid = _store->IsValid();
}

/** 2012 0xb18670. */
void FxArchive::Close()
{
	if( _sdkVersionString )
	{
		FxFree(_sdkVersionString, _sdkVersionStringSize);
		_sdkVersionString = NULL;
		_sdkVersionStringSize = 0;
	}
	if( _licenseeName )
	{
		FxFree(_licenseeName, _licenseeNameSize);
		_licenseeName = NULL;
		_licenseeNameSize = 0;
	}
	if( _licenseeProjectName )
	{
		FxFree(_licenseeProjectName, _licenseeProjectNameSize);
		_licenseeProjectName = NULL;
		_licenseeProjectNameSize = 0;
	}
	FlushLazyLoaders(FxFalse);
	FxDelete(_pInternalData);
	if( _store )
	{
		FxSize size = _store->GetClassSize();
		_store->Close();
		_store->Destroy();
		FxFree(_store, size);
		_store = NULL;
	}
	_isValid = FxFalse;
}

/** 2012 0xb18790. */
void FxArchive::Startup()
{
	if( !pMountedArchives )
	{
		pMountedArchives = static_cast<FxArray<FxArchive*>*>(FxAllocate(sizeof(FxArray<FxArchive*>)));
		FxDefaultConstruct(pMountedArchives);
	}
}

/** 2012 0xb187c0. */
void FxArchive::Shutdown()
{
	UnmountAllArchives(FxFalse);
	FxDelete(pMountedArchives);
	pMountedArchives = NULL;
}

/** 2012 0xb187f0. */
const FxChar* FxArchive::GetSDKVersionString()
{
	if( _sdkVersionString )
	{
		FxFree(_sdkVersionString, _sdkVersionStringSize);
		_sdkVersionString = NULL;
		_sdkVersionStringSize = 0;
	}
	FxTempCharBuffer<FxChar> sdkVersionStringTempBuffer(FxSDKConvertVersionToString(_sdkVersion).GetData());
	_sdkVersionStringSize = sdkVersionStringTempBuffer.GetBufferSize();
	_sdkVersionString = sdkVersionStringTempBuffer.Release();
	return _sdkVersionString;
}

/** 2012 0xb188e0. */
void FxArchive::RegisterProgressCallback(void (*callbackFunction)(FxReal), FxReal updateFrequency)
{
	if( IsLoading() )
	{
		_progressCallback = callbackFunction;
		_updateFrequency = updateFrequency;
	}
}

/** 2012 0xb18910. */
FxObject* FxArchive::FindObject(FxSize index) const
{
	if( index == FxInvalidIndex || index >= _pInternalData->ObjectTable.Length() )
	{
		return NULL;
	}
	return _pInternalData->ObjectTable[index];
}

/** 2012 0xb18950. */
FxSize FxArchive::FindObject(FxObject* object) const
{
	return _pInternalData->ObjectTable.Find(object);
}

/** 2012 0xb18970: without an index the object is appended (or found); with one the table grows to it. */
FxSize FxArchive::AddObject(FxObject* object, FxSize index)
{
	if( !object )
	{
		return FxInvalidIndex;
	}
	if( index == FxInvalidIndex )
	{
		FxSize foundObjectIndex = FindObject(object);
		if( foundObjectIndex == FxInvalidIndex )
		{
			FxSize objectLocation = _pInternalData->ObjectTable.Length();
			_pInternalData->ObjectTable.PushBack(object);
			return objectLocation;
		}
		return foundObjectIndex;
	}
	if( index >= _pInternalData->ObjectTable.Length() )
	{
		FxSize numToAdd = index ? index - _pInternalData->ObjectTable.Length() + 1 : 1;
		for( FxSize i = 0; i < numToAdd; ++i )
		{
			_pInternalData->ObjectTable.PushBack(NULL);
		}
	}
	_pInternalData->ObjectTable[index] = object;
	return index;
}

/** 2012 0xb18a70. */
FxObject* FxArchive::ReplaceObject(FxObject* object, FxSize index)
{
	FxObject* cachedObject = NULL;
	if( index != FxInvalidIndex )
	{
		if( index >= _pInternalData->ObjectTable.Length() )
		{
			FxSize numToAdd = index ? index - _pInternalData->ObjectTable.Length() + 1 : 1;
			for( FxSize i = 0; i < numToAdd; ++i )
			{
				_pInternalData->ObjectTable.PushBack(NULL);
			}
		}
		cachedObject = _pInternalData->ObjectTable[index];
		_pInternalData->ObjectTable[index] = object;
	}
	return cachedObject;
}

/** 2012 0xb18b30. */
FxSize FxArchive::AddName(const FxString* name)
{
	FxSize index = FxInvalidIndex;
	if( name )
	{
		index = _pInternalData->Directory.FindName(*name);
		if( index == FxInvalidIndex )
		{
			_pInternalData->Directory.Names.PushBack(FxArchiveNameEntry(*name));
			return _pInternalData->Directory.Names.Length() - 1;
		}
	}
	return index;
}

/** 2012 0xb18bf0. */
FxRefString** FxArchive::GetName(FxSize index)
{
	return _pInternalData->Directory.Names[index].Name._ppRefString;
}

/** 2012 0xb18c20: before sdk 0x6A4 every class wrote its own FxUInt16 version; since, the directory holds it. */
FxUInt16 FxArchive::SerializeClassVersion(const FxChar* classname, FxBool bIsNonFxObjectDerived, FxUInt16 classversion)
{
	FxUInt16 version = 0;
	if( _sdkVersion < 0x6A4 )
	{
		*this << version;
		return version;
	}
	if( IsSaving() )
	{
		if( bIsNonFxObjectDerived )
		{
			_pInternalData->Directory.InsertNonFxObjectDerivedClassVersion(classname, classversion);
		}
		else
		{
			_pInternalData->Directory.InsertClassVersion(classname);
		}
	}
	return _pInternalData->Directory.FindClassVersion(classname);
}

/** 2012 0xb18cb0. */
FxInt32 FxArchive::Tell() const
{
	if( _store )
	{
		return _store->Tell();
	}
	return 0;
}

/** 2012 0xb18ce0. */
void FxArchive::Seek(FxInt32 pos)
{
	if( _store )
	{
		_store->Seek(pos);
	}
}

/** 2012 0xb18d10. */
void FxArchive::MountArchive(FxArchive* arc)
{
	if( pMountedArchives )
	{
		pMountedArchives->PushBack(arc);
	}
}

/** 2012 0xb18d30. */
void FxArchive::UnmountArchive(FxSize index, FxBool load)
{
	if( pMountedArchives )
	{
		FxArchive* pArchive = (*pMountedArchives)[index];
		if( pArchive )
		{
			pArchive->FlushLazyLoaders(load);
			FxDelete(pArchive);
			(*pMountedArchives)[index] = NULL;
		}
	}
}

/** 2012 0xb18d90. */
FxSize FxArchive::GetNumMountedArchives()
{
	if( pMountedArchives )
	{
		return pMountedArchives->Length();
	}
	return 0;
}

/** 2012 0xb18db0. */
void FxArchive::UnmountAllArchives(FxBool load)
{
	if( pMountedArchives )
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			UnmountArchive(i, load);
		}
		pMountedArchives->Clear();
	}
}

/** 2012 0xb18e10. */
void FxArchive::ForceLoadAllMountedArchives()
{
	if( pMountedArchives )
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			FxArchive* pArchive = (*pMountedArchives)[i];
			if( pArchive )
			{
				FxSize numLazyLoaders = pArchive->_pInternalData->LazyLoaders.Length();
				for( FxSize j = 0; j < numLazyLoaders; ++j )
				{
					FxLazyLoader* lazyLoader = pArchive->_pInternalData->LazyLoaders[j];
					if( lazyLoader )
					{
						lazyLoader->Load();
					}
				}
			}
		}
	}
}

/** 2012 0xb18ed0. */
void FxArchive::ForceUnloadAllMountedArchives()
{
	if( pMountedArchives )
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			FxArchive* pArchive = (*pMountedArchives)[i];
			if( pArchive )
			{
				FxSize numLazyLoaders = pArchive->_pInternalData->LazyLoaders.Length();
				for( FxSize j = 0; j < numLazyLoaders; ++j )
				{
					FxLazyLoader* lazyLoader = pArchive->_pInternalData->LazyLoaders[j];
					if( lazyLoader )
					{
						lazyLoader->Unload();
					}
				}
			}
		}
	}
}

/** 2012 0xb18f90. */
void FxArchive::AddLazyLoader(FxLazyLoader* lazyLoader)
{
	if( lazyLoader )
	{
		if( !lazyLoader->_archive )
		{
			lazyLoader->_archive = this;
			lazyLoader->_pos = _store->Tell();
		}
		_pInternalData->LazyLoaders.PushBack(lazyLoader);
	}
}

/** 2012 0xb18fe0. */
void FxArchive::RemoveLazyLoader(FxLazyLoader* lazyLoader)
{
	FxSize index = _pInternalData->LazyLoaders.Find(lazyLoader);
	if( index != FxInvalidIndex )
	{
		_pInternalData->LazyLoaders.Remove(index);
		lazyLoader->_archive = NULL;
		lazyLoader->_pos = 0;
	}
}

/** 2012 0xb19040. */
void FxArchive::FlushLazyLoaders(FxBool load)
{
	FxSize numLazyLoaders = _pInternalData->LazyLoaders.Length();
	for( FxSize i = 0; i < numLazyLoaders; ++i )
	{
		FxLazyLoader* lazyLoader = _pInternalData->LazyLoaders[i];
		if( load )
		{
			lazyLoader->Load();
		}
		lazyLoader->_archive = NULL;
		lazyLoader->_pos = 0;
	}
	_pInternalData->LazyLoaders.Clear();
}

/** 2012 0xb1a390 / 0xb1a490 / 0xb1a5a0 / 0xb1a6b0 / 0xb1a7c0 (char, short, int, float, double): the store
    always sees little or big endian as the archive asks, the caller's value is never swapped in place on save. */
template<typename T> static FxArchive& SerializeRawData(FxArchive& arc, T& data)
{
	if( arc.IsValid() )
	{
		if( arc.IsSaving() )
		{
			if( arc.GetByteOrder() == FxArchive::ABO_BigEndian )
			{
				T temp = data;
				FxByteSwap(reinterpret_cast<FxByte*>(&temp), sizeof(T));
				arc.GetStore()->Write(reinterpret_cast<const FxByte*>(&temp), sizeof(T));
			}
			else
			{
				arc.GetStore()->Write(reinterpret_cast<const FxByte*>(&data), sizeof(T));
			}
		}
		else
		{
			arc.GetStore()->Read(reinterpret_cast<FxByte*>(&data), sizeof(T));
			if( arc.GetByteOrder() == FxArchive::ABO_BigEndian )
			{
				FxByteSwap(reinterpret_cast<FxByte*>(&data), sizeof(T));
			}
		}
	}
	return arc;
}

void FxArchive::_updateProgress(FxInt32 numBytes)
{
	if( _progressCallback )
	{
		_numBytesLoaded += numBytes;
		_progress = static_cast<FxReal>(_numBytesLoaded) / _numBytesTotal;
		if( _updateFrequency <= _progress - _lastProgress )
		{
			_progressCallback(_progress);
			_lastProgress = _progress;
		}
	}
}

/** 2012 0xb190d0. */
FxArchive& FxArchive::operator<<(FxChar& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(1);
	return *this;
}

/** 2012 0xb19160. */
FxArchive& FxArchive::operator<<(FxUChar& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(1);
	return *this;
}

/** 2012 0xb191f0. */
FxArchive& FxArchive::operator<<(FxInt16& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(2);
	return *this;
}

/** 2012 0xb19280. */
FxArchive& FxArchive::operator<<(FxUInt16& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(2);
	return *this;
}

/** 2012 0xb19310. */
FxArchive& FxArchive::operator<<(FxInt32& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(4);
	return *this;
}

/** 2012 0xb193a0. */
FxArchive& FxArchive::operator<<(FxUInt32& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(4);
	return *this;
}

/** 2012 0xb19430. */
FxArchive& FxArchive::operator<<(FxReal& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(4);
	return *this;
}

/** 2012 0xb194c0. */
FxArchive& FxArchive::operator<<(FxDReal& elem)
{
	SerializeRawData(*this, elem);
	_updateProgress(8);
	return *this;
}

/** 2012 0xb19550: a bool is stored as a 32-bit value and only 1 reads back as true; it counts one byte. */
FxArchive& FxArchive::operator<<(FxBool& elem)
{
	FxUInt32 temp = elem;
	SerializeRawData(*this, temp);
	elem = temp == 1;
	_updateProgress(1);
	return *this;
}

/** 2012 0xb19600. */
const FxArchiveInternalData* FxArchive::GetInternalData() const
{
	return _pInternalData;
}

/** 2012 0xb19620: a save archive takes the directory a CreateDirectory pass built, and writes it. */
void FxArchive::SetInternalDataState(const FxArchiveInternalData* pData)
{
	_pInternalData->Directory.ClassVersions = pData->Directory.ClassVersions;
	_pInternalData->Directory.Names = pData->Directory.Names;
	_pInternalData->Directory.NumObjectPointers = pData->ObjectTable.Length();
	*this << _pInternalData->Directory;
}

} // namespace Face
} // namespace OC3Ent
