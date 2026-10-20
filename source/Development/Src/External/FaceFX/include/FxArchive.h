/*=============================================================================
	FxArchive.h - FaceFX 1.7.3.1 serialisation archive (fxarchive.h) and the Serialize bodies of the
	container templates (fxarray.h:149, fxstring.h:266, fxstringhash.h:174, fxarrayimpl.h:402).

	DISHONORED(port): agent FX1 (wave 26). Archive layout written by Open (save) and read by Open (load):
	  'F' 'A' 'C' ('E' little / 'B' big endian), FxSize sdkVersion (1731), FxSize fileFormatVersion (0, only
	  from sdk 0x6B8), licensee name and project name as {FxSize size incl. terminator, chars}, FxSize
	  licenseeVersion (1000), then from sdk 0x640 the FxArchiveDirectory: FxUInt16 version (1), the class
	  version hash (FxStringHash<FxUInt16,7,FxArray>), the archive name table (FxArray<FxArchiveNameEntry>) and
	  FxSize NumObjectPointers. Objects then refer to names and to earlier objects by index.
	Every FxArchive operator<< goes through SerializeRawData<T> (fxarchive.cpp:895) and the progress callback.
	Sizes (2012 PDB): FxArchive 80, FxArchiveInternalData 1592 (0x638, FxArchive::Open), FxArchiveDirectory
	1568, FxArchiveNameEntry 16, FxLazyLoader 16.
=============================================================================*/
#ifndef FxArchive_H__
#define FxArchive_H__

#include "FxArray.h"
#include "FxString.h"
#include "FxStringHash.h"
#include "FxName.h"
#include "FxArchiveStore.h"

namespace OC3Ent
{
namespace Face
{

class FxObject;
class FxArchive;

class FxLazyLoader
{
public:
	FxLazyLoader()
		: _archive(NULL)
		, _pos(0)
		, _isLoaded(FxFalse)
	{
	}

	virtual ~FxLazyLoader()
	{
	}

	virtual void Load() = 0;
	virtual void Unload() = 0;

	FxArchive* _archive;
	FxInt32 _pos;
	FxBool _isLoaded;
};

class FxArchiveNameEntry
{
public:
	FxArchiveNameEntry();
	FxArchiveNameEntry(const FxString& name);
	FxArchiveNameEntry(const FxArchiveNameEntry& other);
	FxArchiveNameEntry& operator=(const FxArchiveNameEntry& other);
	~FxArchiveNameEntry();

	FxString NameStr;
	FxName Name;
};

FxArchive& operator<<(FxArchive& arc, FxArchiveNameEntry& name);

class FxArchiveDirectory
{
public:
	FxArchiveDirectory();
	~FxArchiveDirectory();

	FxSize FindName(const FxString& name) const;
	void InitializeNames();
	void InsertClassVersion(const FxChar* classname);
	void InsertNonFxObjectDerivedClassVersion(const FxChar* classname, FxUInt16 classversion);
	FxUInt16 FindClassVersion(const FxChar* classname) const;

	FxArray<FxArchiveNameEntry> Names;
	FxSize NumObjectPointers;
	FxStringHash<FxUInt16, 7, FxArray> ClassVersions;
};

FxArchive& operator<<(FxArchive& arc, FxArchiveDirectory& directory);

class FxArchiveInternalData
{
public:
	FxArchiveInternalData();
	~FxArchiveInternalData();

	FxArray<FxObject*> ObjectTable;
	FxArray<FxLazyLoader*> LazyLoaders;
	FxArchiveDirectory Directory;
};

class FxArchive : public FxUseAllocator
{
public:
	enum FxArchiveMode
	{
		AM_LinearLoad = 0,
		AM_Load = AM_LinearLoad,
		AM_LazyLoad = 1,
		AM_CreateDirectory = 2,
		AM_ApproximateMemoryUsage = 3,
		AM_Save = 4
	};

	enum FxArchiveByteOrder
	{
		ABO_LittleEndian = 0,
		ABO_BigEndian = 1
	};

	FxArchive(FxArchiveStore* iStore, FxArchiveMode mode, FxArchiveByteOrder byteOrder = ABO_LittleEndian);
	~FxArchive();

	void Open();
	void Close();

	static void Startup();
	static void Shutdown();

	FxBool IsValid() const
	{
		return _isValid;
	}

	FxBool IsSaving() const
	{
		return _mode == AM_Save || _mode == AM_CreateDirectory || _mode == AM_ApproximateMemoryUsage;
	}

	FxBool IsLoading() const
	{
		return static_cast<FxSize>(_mode) < static_cast<FxSize>(AM_CreateDirectory);
	}

	FxArchiveMode GetMode() const
	{
		return _mode;
	}

	FxArchiveByteOrder GetByteOrder() const
	{
		return _byteOrder;
	}

	FxArchiveStore* GetStore() const
	{
		return _store;
	}

	FxSize GetSDKVersion() const
	{
		return _sdkVersion;
	}

	FxSize GetFileFormatVersion() const
	{
		return _fileFormatVersion;
	}

	const FxChar* GetLicenseeName() const
	{
		return _licenseeName;
	}

	const FxChar* GetLicenseeProjectName() const
	{
		return _licenseeProjectName;
	}

	FxSize GetLicenseeVersion() const
	{
		return _licenseeVersion;
	}

	const FxChar* GetSDKVersionString();

	void RegisterProgressCallback(void (*callbackFunction)(FxReal), FxReal updateFrequency);

	FxObject* FindObject(FxSize index) const;
	FxSize FindObject(FxObject* object) const;
	FxSize AddObject(FxObject* object, FxSize index = FxInvalidIndex);
	FxObject* ReplaceObject(FxObject* object, FxSize index);

	FxSize AddName(const FxString* name);
	FxRefString** GetName(FxSize index);

	FxUInt16 SerializeClassVersion(const FxChar* classname, FxBool bIsNonFxObjectDerived = FxFalse, FxUInt16 classversion = 0);

	FxInt32 Tell() const;
	void Seek(FxInt32 pos);

	static void MountArchive(FxArchive* arc);
	static void UnmountArchive(FxSize index, FxBool load = FxFalse);
	static FxSize GetNumMountedArchives();
	static void UnmountAllArchives(FxBool load = FxFalse);
	static void ForceLoadAllMountedArchives();
	static void ForceUnloadAllMountedArchives();

	void AddLazyLoader(FxLazyLoader* lazyLoader);
	void RemoveLazyLoader(FxLazyLoader* lazyLoader);
	void FlushLazyLoaders(FxBool load = FxFalse);

	FxArchive& operator<<(FxChar& elem);
	FxArchive& operator<<(FxUChar& elem);
	FxArchive& operator<<(FxInt16& elem);
	FxArchive& operator<<(FxUInt16& elem);
	FxArchive& operator<<(FxInt32& elem);
	FxArchive& operator<<(FxUInt32& elem);
	FxArchive& operator<<(FxReal& elem);
	FxArchive& operator<<(FxDReal& elem);
	FxArchive& operator<<(FxBool& elem);

	const FxArchiveInternalData* GetInternalData() const;
	void SetInternalDataState(const FxArchiveInternalData* pData);

	template<typename T> void SerializePODArray(T* array, FxSize length)
	{
		if( IsValid() )
		{
			if( IsSaving() )
			{
				if( _byteOrder == ABO_BigEndian )
				{
					_byteSwapPODArray(array, length);
				}
				_store->Write(reinterpret_cast<const FxByte*>(array), sizeof(T) * length);
				if( _byteOrder == ABO_BigEndian )
				{
					_byteSwapPODArray(array, length);
				}
			}
			else
			{
				_store->Read(reinterpret_cast<FxByte*>(array), sizeof(T) * length);
				if( _byteOrder == ABO_BigEndian )
				{
					_byteSwapPODArray(array, length);
				}
			}
		}
	}

private:
	FxArchive(const FxArchive&);
	FxArchive& operator=(const FxArchive&);

	template<typename T> void _byteSwapPODArray(T* array, FxSize length)
	{
		for( FxSize i = 0; i < length; ++i )
		{
			FxByteSwap(reinterpret_cast<FxByte*>(array + i), sizeof(T));
		}
	}

	void _updateProgress(FxInt32 numBytes);

	FxArchiveStore* _store;
	FxArchiveMode _mode;
	FxArchiveByteOrder _byteOrder;
	FxBool _isValid;
	FxSize _fileFormatVersion;
	FxSize _sdkVersion;
	FxChar* _sdkVersionString;
	FxSize _sdkVersionStringSize;
	FxChar* _licenseeName;
	FxSize _licenseeNameSize;
	FxChar* _licenseeProjectName;
	FxSize _licenseeProjectNameSize;
	FxSize _licenseeVersion;
	FxArchiveInternalData* _pInternalData;
	void (*_progressCallback)(FxReal);
	FxReal _updateFrequency;
	FxInt32 _numBytesLoaded;
	FxReal _numBytesTotal;
	FxReal _progress;
	FxReal _lastProgress;
};

template<typename T> FxArchive& FxArray<T>::Serialize(FxArchive& arc)
{
	if( arc.GetSDKVersion() < 0x6A4 && arc.IsLoading() )
	{
		FxUInt16 version = 0;
		arc << version;
	}
	FxSize length = Length();
	arc << length;
	if( arc.IsSaving() )
	{
		if( FxTypeTraits::is_arithmetic<T>::value )
		{
			arc.SerializePODArray(_container._v, length);
		}
		else
		{
			for( FxSize i = 0; i < length; ++i )
			{
				arc << _container._v[i];
			}
		}
	}
	else
	{
		FxArrayBase<T> temp(length);
		if( FxTypeTraits::is_arithmetic<T>::value )
		{
			arc.SerializePODArray(temp._v, length);
		}
		else
		{
			for( FxSize j = 0; j < length; ++j )
			{
				FxDefaultConstruct(temp._v + j);
				arc << temp._v[j];
			}
		}
		temp._usedCount = length;
		_container.Swap(temp);
	}
	return arc;
}

template<typename T> inline FxArchive& operator<<(FxArchive& arc, FxArray<T>& array)
{
	return array.Serialize(arc);
}

template<typename T> FxArchive& FxStringBase<T>::Serialize(FxArchive& arc)
{
	FxUInt16 version = 1;
	if( arc.GetSDKVersion() < 0x6A4 && arc.IsLoading() )
	{
		arc << version;
	}
	FxSize length = Length();
	arc << length;
	if( arc.IsSaving() )
	{
		arc.SerializePODArray(_container._v, length);
	}
	else
	{
		FxArrayBase<T> temp(length + 1);
		arc.SerializePODArray(temp._v, length);
		temp._usedCount += length;
		temp.Swap(_container);
		_container._v[_container._usedCount] = 0;
	}
	return arc;
}

template<typename T> inline FxArchive& operator<<(FxArchive& arc, FxStringBase<T>& str)
{
	return str.Serialize(arc);
}

template<typename T, FxSize HashBits, template<typename> class Container>
FxArchive& FxStringHash<T, HashBits, Container>::Serialize(FxArchive& arc)
{
	if( arc.IsLoading() )
	{
		Clear();
	}
	FxSize numBinsInUse = 0;
	if( arc.IsSaving() )
	{
		numBinsInUse = GetNumBinsInUse();
	}
	arc << numBinsInUse;
	FxSize numTotalBins = GetNumBins();
	if( arc.IsSaving() )
	{
		for( FxSize binIndex = 0; binIndex < numTotalBins; ++binIndex )
		{
			if( !hashBins[binIndex].IsEmpty() )
			{
				arc << binIndex;
				FxHashBin& theBin = hashBins[binIndex];
				FxSize len = theBin.Length();
				arc << len;
				for( typename FxHashBin::Iterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
				{
					arc << (*curr).key << (*curr).object;
				}
			}
		}
	}
	else if( arc.IsLoading() )
	{
		for( FxSize j = 0; j < numBinsInUse; ++j )
		{
			FxSize serializedBinIndex = 0;
			arc << serializedBinIndex;
			FxSize serializedBinLen = 0;
			arc << serializedBinLen;
			for( FxSize i = 0; i < serializedBinLen; ++i )
			{
				FxHashBinEntry serializedEntry;
				arc << serializedEntry.key << serializedEntry.object;
				hashBins[serializedBinIndex].PushBack(serializedEntry);
			}
		}
	}
	return arc;
}

template<typename T, FxSize HashBits, template<typename> class Container>
inline FxArchive& operator<<(FxArchive& arc, FxStringHash<T, HashBits, Container>& hash)
{
	return hash.Serialize(arc);
}

} // namespace Face
} // namespace OC3Ent

#endif
