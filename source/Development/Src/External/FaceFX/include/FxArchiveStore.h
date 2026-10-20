/*=============================================================================
	FxArchiveStore.h - FaceFX 1.7.3.1 archive backing stores (fxarchivestore.h and the five store headers).

	DISHONORED(port): agent FX1 (wave 26). The vtable is the PDB's FxArchiveStore_vtbl, in this order:
	IsValid, OpenRead, OpenWrite, Close, Read, Write, Tell, Seek, Length, ~dtor, Destroy, GetClassSize.
	Stores are created with Create() on the SDK allocator and owned by the FxArchive that receives them:
	FxArchive::Close calls Close, Destroy (the non-deleting destructor) and FxFree(store, GetClassSize()).
	Header inlines: FxArchiveStore ctor 0xb22600 / dtor 0xb22620, FxArchiveStoreNull ctor 0xb22c00 / dtor 0xb22c60.
	Sizes (2012 PDB): FxArchiveStore 4, Null 8, Memory 20, MemoryNoCopy 20, File 24, FileFast 36.
=============================================================================*/
#ifndef FxArchiveStore_H__
#define FxArchiveStore_H__

#include "FxString.h"
#include "FxFile.h"

namespace OC3Ent
{
namespace Face
{

class FxArchiveStore : public FxUseAllocator
{
public:
	virtual FxBool IsValid() const = 0;
	virtual FxBool OpenRead() = 0;
	virtual FxBool OpenWrite() = 0;
	virtual FxBool Close() = 0;
	virtual FxBool Read(FxByte* data, FxSize size) = 0;
	virtual FxBool Write(const FxByte* data, FxSize size) = 0;
	virtual FxInt32 Tell() const = 0;
	virtual void Seek(const FxInt32 pos) = 0;
	virtual FxInt32 Length() const = 0;

protected:
	friend class FxArchive;
	friend class FxArchiveStoreFileFast;

	FxArchiveStore()
	{
	}

	virtual ~FxArchiveStore()
	{
	}

	virtual void Destroy() = 0;
	virtual FxSize GetClassSize() = 0;
};

class FxArchiveStoreNull : public FxArchiveStore
{
public:
	static FxArchiveStoreNull* Create();

	virtual FxBool IsValid() const;
	virtual FxBool OpenRead();
	virtual FxBool OpenWrite();
	virtual FxBool Close();
	virtual FxBool Read(FxByte* data, FxSize size);
	virtual FxBool Write(const FxByte* data, FxSize size);
	virtual FxInt32 Tell() const;
	virtual void Seek(const FxInt32 pos);
	virtual FxInt32 Length() const;

	FxSize GetSize() const;

protected:
	FxArchiveStoreNull()
		: _numBytes(0)
	{
	}

	virtual ~FxArchiveStoreNull()
	{
	}

	virtual void Destroy();
	virtual FxSize GetClassSize();

	FxSize _numBytes;
};

class FxArchiveStoreMemory : public FxArchiveStore
{
public:
	static FxArchiveStoreMemory* Create(const FxByte* memory, FxSize size);

	virtual FxBool IsValid() const;
	virtual FxBool OpenRead();
	virtual FxBool OpenWrite();
	virtual FxBool Close();
	virtual FxBool Read(FxByte* data, FxSize size);
	virtual FxBool Write(const FxByte* data, FxSize size);
	virtual FxInt32 Tell() const;
	virtual void Seek(const FxInt32 pos);
	virtual FxInt32 Length() const;

	const FxByte* GetMemory();
	FxSize GetSize();

protected:
	FxArchiveStoreMemory(const FxByte* memory, FxSize size);
	virtual ~FxArchiveStoreMemory();
	virtual void Destroy();
	virtual FxSize GetClassSize();

	FxSize _size;
	FxSize _inUse;
	FxByte* _memory;
	FxSize _currentPos;
};

class FxArchiveStoreMemoryNoCopy : public FxArchiveStore
{
public:
	static FxArchiveStoreMemoryNoCopy* Create(const FxByte* memory, FxSize size);

	virtual FxBool IsValid() const;
	virtual FxBool OpenRead();
	virtual FxBool OpenWrite();
	virtual FxBool Close();
	virtual FxBool Read(FxByte* data, FxSize size);
	virtual FxBool Write(const FxByte* data, FxSize size);
	virtual FxInt32 Tell() const;
	virtual void Seek(const FxInt32 pos);
	virtual FxInt32 Length() const;

	const FxByte* GetMemory();
	FxSize GetSize();

protected:
	FxArchiveStoreMemoryNoCopy(const FxByte* memory, FxSize size);
	virtual ~FxArchiveStoreMemoryNoCopy();
	virtual void Destroy();
	virtual FxSize GetClassSize();

	FxSize _size;
	FxSize _inUse;
	const FxByte* _memory;
	FxSize _currentPos;
};

class FxArchiveStoreFile : public FxArchiveStore
{
public:
	static FxArchiveStoreFile* Create(const FxChar* filename);

	virtual FxBool IsValid() const;
	virtual FxBool OpenRead();
	virtual FxBool OpenWrite();
	virtual FxBool Close();
	virtual FxBool Read(FxByte* data, FxSize size);
	virtual FxBool Write(const FxByte* data, FxSize size);
	virtual FxInt32 Tell() const;
	virtual void Seek(const FxInt32 pos);
	virtual FxInt32 Length() const;

protected:
	FxArchiveStoreFile(const FxChar* filename);
	virtual ~FxArchiveStoreFile();
	virtual void Destroy();
	virtual FxSize GetClassSize();

	FxString _filename;
	FxFile _file;
};

class FxArchiveStoreFileFast : public FxArchiveStoreFile
{
public:
	static FxArchiveStoreFileFast* Create(const FxChar* filename);

	virtual FxBool IsValid() const;
	virtual FxBool OpenRead();
	virtual FxBool OpenWrite();
	virtual FxBool Close();
	virtual FxBool Read(FxByte* data, FxSize size);
	virtual FxBool Write(const FxByte* data, FxSize size);
	virtual FxInt32 Tell() const;
	virtual void Seek(const FxInt32 pos);
	virtual FxInt32 Length() const;

protected:
	FxArchiveStoreFileFast(const FxChar* filename);
	virtual ~FxArchiveStoreFileFast();
	virtual void Destroy();
	virtual FxSize GetClassSize();

	FxByte* _pRawFileMemory;
	FxSize _rawFileSize;
	FxArchiveStoreMemoryNoCopy* _pMemoryStore;
};

} // namespace Face
} // namespace OC3Ent

#endif
