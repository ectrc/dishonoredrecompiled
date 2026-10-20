/*=============================================================================
	FxArchiveStore.cpp - FaceFX 1.7.3.1 archive stores (fxsdk\src\fxarchivestore*.cpp, compilands
	FxArchiveStoreNull.obj, FxArchiveStoreMemory.obj, FxArchiveStoreMemoryNoCopy.obj, FxArchiveStoreFile.obj,
	FxArchiveStoreFileFast.obj).

	DISHONORED(port): agent FX1 (wave 26). As retail:
	  - the memory store copies the caller's bytes, starts writing with 1 KB and doubles; GetSize is the
	    high-water mark of what was written (_inUse), Length the capacity;
	  - the no-copy store reads the caller's bytes in place and refuses to write;
	  - the file store opens "rb" / "wb" (FM_Read|FM_Binary = 9, FM_Write|FM_Binary = 10) and reports every
	    Read/Write as successful whatever stdio returned;
	  - the fast file store reads the whole file into memory on OpenRead and serves it through a no-copy store.
	Tell / Seek / Length of the memory stores use FxSize positions behind the FxInt32 interface.
=============================================================================*/
#include "FxArchiveStore.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxArchiveStore) == 4, "FxArchiveStore: 2012 PDB sizeof 4");
static_assert(sizeof(FxArchiveStoreNull) == 8, "FxArchiveStoreNull: GetClassSize 8");
static_assert(sizeof(FxArchiveStoreMemory) == 20, "FxArchiveStoreMemory: GetClassSize 20");
static_assert(sizeof(FxArchiveStoreMemoryNoCopy) == 20, "FxArchiveStoreMemoryNoCopy: GetClassSize 20");
static_assert(sizeof(FxArchiveStoreFile) == 24, "FxArchiveStoreFile: GetClassSize 24");
static_assert(sizeof(FxArchiveStoreFileFast) == 36, "FxArchiveStoreFileFast: GetClassSize 36");

/** 2012 0xb22a90. */
FxArchiveStoreNull* FxArchiveStoreNull::Create()
{
	return new FxArchiveStoreNull();
}

/** 2012 0xb22b10. */
FxBool FxArchiveStoreNull::IsValid() const
{
	return FxTrue;
}

/** 2012 0xb22b20. */
FxBool FxArchiveStoreNull::OpenRead()
{
	return FxTrue;
}

/** 2012 0xb22b30. */
FxBool FxArchiveStoreNull::OpenWrite()
{
	return FxTrue;
}

/** 2012 0xb22b40. */
FxBool FxArchiveStoreNull::Close()
{
	return FxTrue;
}

/** 2012 0xb22b50. */
FxBool FxArchiveStoreNull::Read(FxByte*, FxSize)
{
	return FxTrue;
}

/** 2012 0xb22b60. */
FxBool FxArchiveStoreNull::Write(const FxByte*, FxSize size)
{
	_numBytes += size;
	return FxTrue;
}

/** 2012 0xb22b80. */
FxInt32 FxArchiveStoreNull::Tell() const
{
	return 0;
}

/** 2012 0xb22b90. */
void FxArchiveStoreNull::Seek(const FxInt32)
{
}

/** 2012 0xb22ba0. */
FxInt32 FxArchiveStoreNull::Length() const
{
	return 0;
}

/** 2012 0xb22bb0. */
FxSize FxArchiveStoreNull::GetSize() const
{
	return _numBytes;
}

/** 2012 0xb22bd0. */
void FxArchiveStoreNull::Destroy()
{
	this->~FxArchiveStoreNull();
}

/** 2012 0xb22bf0. */
FxSize FxArchiveStoreNull::GetClassSize()
{
	return sizeof(FxArchiveStoreNull);
}

/** 2012 0xb22fd0. */
FxArchiveStoreMemory* FxArchiveStoreMemory::Create(const FxByte* memory, FxSize size)
{
	return new FxArchiveStoreMemory(memory, size);
}

/** 2012 0xb23050. */
FxBool FxArchiveStoreMemory::IsValid() const
{
	return _memory && _size;
}

/** 2012 0xb23080. */
FxBool FxArchiveStoreMemory::OpenRead()
{
	return _memory && _size;
}

/** 2012 0xb230b0. */
FxBool FxArchiveStoreMemory::OpenWrite()
{
	if( _memory || _size )
	{
		return FxFalse;
	}
	_size = 1024;
	_memory = static_cast<FxByte*>(FxAllocate(_size));
	return FxTrue;
}

/** 2012 0xb23100. */
FxBool FxArchiveStoreMemory::Close()
{
	if( _memory )
	{
		FxFree(_memory, _size);
		_memory = NULL;
	}
	return FxTrue;
}

/** 2012 0xb23140. */
FxBool FxArchiveStoreMemory::Read(FxByte* data, FxSize size)
{
	if( !_memory || size > _size - _currentPos )
	{
		return FxFalse;
	}
	FxMemcpy(data, _memory + _currentPos, size);
	_currentPos += size;
	return FxTrue;
}

/** 2012 0xb231a0. */
FxBool FxArchiveStoreMemory::Write(const FxByte* data, FxSize size)
{
	if( !_memory )
	{
		return FxFalse;
	}
	if( size > _size - _currentPos )
	{
		FxSize oldSize = _size;
		while( _size - _currentPos < size )
		{
			_size *= 2;
		}
		FxByte* newMemory = static_cast<FxByte*>(FxAllocate(_size));
		FxMemcpy(newMemory, _memory, oldSize);
		FxFree(_memory, oldSize);
		_memory = newMemory;
	}
	FxMemcpy(_memory + _currentPos, data, size);
	_currentPos += size;
	_inUse = FxMax(_inUse, _currentPos);
	return FxTrue;
}

/** 2012 0xb23290. */
FxInt32 FxArchiveStoreMemory::Tell() const
{
	return static_cast<FxInt32>(_currentPos);
}

/** 2012 0xb232b0. */
void FxArchiveStoreMemory::Seek(const FxInt32 pos)
{
	_currentPos = static_cast<FxSize>(pos);
}

/** 2012 0xb232d0. */
FxInt32 FxArchiveStoreMemory::Length() const
{
	return static_cast<FxInt32>(_size);
}

/** 2012 0xb232f0. */
const FxByte* FxArchiveStoreMemory::GetMemory()
{
	return _memory;
}

/** 2012 0xb23310. */
FxSize FxArchiveStoreMemory::GetSize()
{
	return _inUse;
}

/** 2012 0xb23330. */
FxArchiveStoreMemory::FxArchiveStoreMemory(const FxByte* memory, FxSize size)
	: _size(size)
	, _inUse(size)
	, _memory(NULL)
	, _currentPos(0)
{
	if( memory && size )
	{
		_memory = static_cast<FxByte*>(FxAllocate(_size));
		FxMemcpy(_memory, memory, _size);
	}
	else
	{
		_size = 0;
		_inUse = 0;
	}
}

/** 2012 0xb23400. */
FxArchiveStoreMemory::~FxArchiveStoreMemory()
{
	Close();
}

/** 2012 0xb23460. */
void FxArchiveStoreMemory::Destroy()
{
	this->~FxArchiveStoreMemory();
}

/** 2012 0xb23480. */
FxSize FxArchiveStoreMemory::GetClassSize()
{
	return sizeof(FxArchiveStoreMemory);
}

/** 2012 0xb22c80. */
FxArchiveStoreMemoryNoCopy* FxArchiveStoreMemoryNoCopy::Create(const FxByte* memory, FxSize size)
{
	return new FxArchiveStoreMemoryNoCopy(memory, size);
}

/** 2012 0xb22d00. */
FxBool FxArchiveStoreMemoryNoCopy::IsValid() const
{
	return _memory && _size;
}

/** 2012 0xb22d30. */
FxBool FxArchiveStoreMemoryNoCopy::OpenRead()
{
	return _memory && _size;
}

/** 2012 0xb22d60. */
FxBool FxArchiveStoreMemoryNoCopy::OpenWrite()
{
	return FxFalse;
}

/** 2012 0xb22d70. */
FxBool FxArchiveStoreMemoryNoCopy::Close()
{
	return FxTrue;
}

/** 2012 0xb22d80. */
FxBool FxArchiveStoreMemoryNoCopy::Read(FxByte* data, FxSize size)
{
	if( !_memory || size > _size - _currentPos )
	{
		return FxFalse;
	}
	FxMemcpy(data, _memory + _currentPos, size);
	_currentPos += size;
	return FxTrue;
}

/** 2012 0xb22de0. */
FxBool FxArchiveStoreMemoryNoCopy::Write(const FxByte*, FxSize)
{
	return FxFalse;
}

/** 2012 0xb22df0. */
FxInt32 FxArchiveStoreMemoryNoCopy::Tell() const
{
	return static_cast<FxInt32>(_currentPos);
}

/** 2012 0xb22e10. */
void FxArchiveStoreMemoryNoCopy::Seek(const FxInt32 pos)
{
	_currentPos = static_cast<FxSize>(pos);
}

/** 2012 0xb22e30. */
FxInt32 FxArchiveStoreMemoryNoCopy::Length() const
{
	return static_cast<FxInt32>(_size);
}

/** 2012 0xb22e50. */
const FxByte* FxArchiveStoreMemoryNoCopy::GetMemory()
{
	return _memory;
}

/** 2012 0xb22e70. */
FxSize FxArchiveStoreMemoryNoCopy::GetSize()
{
	return _inUse;
}

/** 2012 0xb22e90. */
FxArchiveStoreMemoryNoCopy::FxArchiveStoreMemoryNoCopy(const FxByte* memory, FxSize size)
	: _size(size)
	, _inUse(size)
	, _memory(NULL)
	, _currentPos(0)
{
	if( memory && size )
	{
		_memory = memory;
	}
	else
	{
		_size = 0;
		_inUse = 0;
	}
}

/** 2012 0xb22f10. */
FxArchiveStoreMemoryNoCopy::~FxArchiveStoreMemoryNoCopy()
{
	Close();
}

/** 2012 0xb22f70. */
void FxArchiveStoreMemoryNoCopy::Destroy()
{
	this->~FxArchiveStoreMemoryNoCopy();
}

/** 2012 0xb22f90. */
FxSize FxArchiveStoreMemoryNoCopy::GetClassSize()
{
	return sizeof(FxArchiveStoreMemoryNoCopy);
}

/** 2012 0xb222f0. */
FxArchiveStoreFile* FxArchiveStoreFile::Create(const FxChar* filename)
{
	return new FxArchiveStoreFile(filename);
}

/** 2012 0xb22370. */
FxBool FxArchiveStoreFile::IsValid() const
{
	return _file.IsValid();
}

/** 2012 0xb22390. */
FxBool FxArchiveStoreFile::OpenRead()
{
	_file.Open(_filename.GetData(), FxFile::FM_Read | FxFile::FM_Binary);
	return _file.IsValid();
}

/** 2012 0xb223c0. */
FxBool FxArchiveStoreFile::OpenWrite()
{
	_file.Open(_filename.GetData(), FxFile::FM_Write | FxFile::FM_Binary);
	return _file.IsValid();
}

/** 2012 0xb223f0. */
FxBool FxArchiveStoreFile::Close()
{
	_file.Close();
	return FxTrue;
}

/** 2012 0xb22410. */
FxBool FxArchiveStoreFile::Read(FxByte* data, FxSize size)
{
	_file.Read(data, size);
	return FxTrue;
}

/** 2012 0xb22440. */
FxBool FxArchiveStoreFile::Write(const FxByte* data, FxSize size)
{
	_file.Write(data, size);
	return FxTrue;
}

/** 2012 0xb22470. */
FxInt32 FxArchiveStoreFile::Tell() const
{
	return _file.Tell();
}

/** 2012 0xb22490. */
void FxArchiveStoreFile::Seek(const FxInt32 pos)
{
	_file.Seek(pos);
}

/** 2012 0xb224b0. */
FxInt32 FxArchiveStoreFile::Length() const
{
	return _file.Length();
}

/** 2012 0xb224d0. */
FxArchiveStoreFile::FxArchiveStoreFile(const FxChar* filename)
	: _filename(filename)
	, _file()
{
}

/** 2012 0xb22550. */
FxArchiveStoreFile::~FxArchiveStoreFile()
{
	FxArchiveStoreFile::Close();
}

/** 2012 0xb225d0. */
void FxArchiveStoreFile::Destroy()
{
	this->~FxArchiveStoreFile();
}

/** 2012 0xb225f0. */
FxSize FxArchiveStoreFile::GetClassSize()
{
	return sizeof(FxArchiveStoreFile);
}

/** 2012 0xb226a0. */
FxArchiveStoreFileFast* FxArchiveStoreFileFast::Create(const FxChar* filename)
{
	return new FxArchiveStoreFileFast(filename);
}

/** 2012 0xb22720. */
FxBool FxArchiveStoreFileFast::IsValid() const
{
	if( !_pMemoryStore )
	{
		return FxFalse;
	}
	return _pMemoryStore->IsValid() && FxArchiveStoreFile::IsValid();
}

/** 2012 0xb22780. */
FxBool FxArchiveStoreFileFast::OpenRead()
{
	FxBool returnValue = FxFalse;
	if( FxArchiveStoreFile::OpenRead() )
	{
		_rawFileSize = FxArchiveStoreFile::Length();
		_pRawFileMemory = static_cast<FxByte*>(FxAllocate(_rawFileSize));
		if( FxArchiveStoreFile::Read(_pRawFileMemory, _rawFileSize) )
		{
			_pMemoryStore = FxArchiveStoreMemoryNoCopy::Create(_pRawFileMemory, _rawFileSize);
			return FxTrue;
		}
	}
	return returnValue;
}

/** 2012 0xb22810. */
FxBool FxArchiveStoreFileFast::OpenWrite()
{
	return FxFalse;
}

/** 2012 0xb22820. */
FxBool FxArchiveStoreFileFast::Close()
{
	if( _pMemoryStore )
	{
		static_cast<FxArchiveStore*>(_pMemoryStore)->Destroy();
		FxFree(_pMemoryStore, sizeof(FxArchiveStoreMemoryNoCopy));
		_pMemoryStore = NULL;
	}
	if( _pRawFileMemory )
	{
		FxFree(_pRawFileMemory, _rawFileSize);
		_pRawFileMemory = NULL;
		_rawFileSize = 0;
	}
	return FxArchiveStoreFile::Close();
}

/** 2012 0xb228a0. */
FxBool FxArchiveStoreFileFast::Read(FxByte* data, FxSize size)
{
	return _pMemoryStore && _pMemoryStore->Read(data, size);
}

/** 2012 0xb228e0. */
FxBool FxArchiveStoreFileFast::Write(const FxByte*, FxSize)
{
	return FxFalse;
}

/** 2012 0xb228f0. */
FxInt32 FxArchiveStoreFileFast::Tell() const
{
	if( _pMemoryStore )
	{
		return _pMemoryStore->Tell();
	}
	return 0;
}

/** 2012 0xb22920. */
void FxArchiveStoreFileFast::Seek(const FxInt32 pos)
{
	if( _pMemoryStore )
	{
		_pMemoryStore->Seek(pos);
	}
}

/** 2012 0xb22950. */
FxInt32 FxArchiveStoreFileFast::Length() const
{
	if( _pMemoryStore )
	{
		return _pMemoryStore->Length();
	}
	return 0;
}

/** 2012 0xb22980. */
FxArchiveStoreFileFast::FxArchiveStoreFileFast(const FxChar* filename)
	: FxArchiveStoreFile(filename)
	, _pRawFileMemory(NULL)
	, _rawFileSize(0)
	, _pMemoryStore(NULL)
{
}

/** 2012 0xb229d0. */
FxArchiveStoreFileFast::~FxArchiveStoreFileFast()
{
	FxArchiveStoreFileFast::Close();
}

/** 2012 0xb22a30. */
void FxArchiveStoreFileFast::Destroy()
{
	this->~FxArchiveStoreFileFast();
}

/** 2012 0xb22a50. */
FxSize FxArchiveStoreFileFast::GetClassSize()
{
	return sizeof(FxArchiveStoreFileFast);
}

} // namespace Face
} // namespace OC3Ent
