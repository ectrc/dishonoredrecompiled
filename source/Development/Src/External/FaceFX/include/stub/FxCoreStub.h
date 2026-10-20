// FaceFX 1.7.3.1 core SHIM (agent FX2, wave 26). DELETE when FX1's core merges.
//
// FX2 ports the face graph while FX1 ports the core in parallel. This header stands in for FX1's public core so the
// graph compiles and CoreSmoke runs: the SDK names and signatures FX2's code calls (2012 PDB), the 2012 layouts where
// FX2's classes embed them (FxArrayBase 12 bytes, FxName 4 bytes, FxStringHash bins, FxClass fields) and the archive
// format FX2's archives are read with (FxArchive::Open 0xb180b0, the directory 0xb17f10, FxName 0xb05890,
// FxStringBase::Serialize 0xb062a0, FxArray::Serialize 0xb24c60, FxStringHash::Serialize 0xb1ab10, the object pointer
// operator<< 0xb21920). It is NOT a port: no refcounted name table (names are interned forever), no lazy loading,
// no mounted archives, no byte swapping, no file stores, memory through malloc. The replacement is FX1's
// External/FaceFX/include/Fx{Types,Memory,Util,String,Name,Array,StringHash,Object,Class,NamedObject,Archive,
// ArchiveStore*}.h: delete include/stub and Src/Stub, drop FX2_FACEFX_CORE_STUB, rebuild.
#ifndef FxCoreStub_H__
#define FxCoreStub_H__

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>

namespace OC3Ent
{
namespace Face
{

typedef char FxChar;
typedef unsigned char FxUInt8;
typedef unsigned char FxByte;
typedef short FxInt16;
typedef unsigned short FxUInt16;
typedef int FxInt32;
typedef unsigned int FxUInt32;
typedef unsigned int FxSize;
typedef float FxReal;
typedef double FxDReal;
typedef bool FxBool;

#define FxTrue true
#define FxFalse false
static const FxSize FxInvalidIndex = 0xFFFFFFFF;
#define FX_REAL_MAX 3.4028235e38f
#define FX_REAL_MIN 1.1754944e-38f
#define FX_REAL_EPSILON 1.1920929e-07f
#define FX_INT32_MIN (-2147483647 - 1)

inline void FxSDKStartup() {}
inline void* FxAllocate( FxSize numBytes ) { return malloc(numBytes ? numBytes : 1); }
inline void FxFree( void* ptr, FxSize ) { free(ptr); }
inline void* FxMemcpy( void* dest, const void* src, FxSize n ) { return memcpy(dest, src, n); }
inline void* FxMemmove( void* dest, const void* src, FxSize n ) { return memmove(dest, src, n); }
inline void* FxMemset( void* dest, FxInt32 c, FxSize n ) { return memset(dest, c, n); }

class FxUseAllocator
{
public:
	static void* operator new( FxSize numBytes ) { return FxAllocate(numBytes); }
	static void operator delete( void* ptr, FxSize numBytes ) { FxFree(ptr, numBytes); }
	static void* operator new( FxSize, void* where ) { return where; }
	static void operator delete( void*, void* ) {}
};

template<typename T> inline T FxMin( const T& a, const T& b ) { return b < a ? b : a; }
template<typename T> inline T FxMax( const T& a, const T& b ) { return b > a ? b : a; }
template<typename T> inline T FxClamp( const T& valMin, const T& val, const T& valMax ) { T a = FxMin(val, valMax); return FxMax(a, valMin); }
template<typename T> inline void FxSwap( T& first, T& second ) { T temp = first; first = second; second = temp; }
template<typename T> inline T FxSquare( const T& a ) { return a * a; }
inline FxReal FxAbs( FxReal x ) { return x < 0.0f ? -x : x; }
inline FxReal FxSqrt( FxReal x ) { return static_cast<FxReal>(sqrt(x)); }
inline FxBool FxRealEquality( FxReal first, FxReal second ) { return (first - second) < FX_REAL_EPSILON && (first - second) > -FX_REAL_EPSILON; }
inline FxBool FxRealEqualityRelaxed( FxReal first, FxReal second ) { FxReal difference = first - second; return difference < 0.001f && difference > -0.001f; }

class FxArchive;
template<typename T> FxArchive& FxSerializeObjectPointer( FxArchive& arc, T*& obj );

template<typename T> inline void FxDefaultConstruct( T* p ) { ::new(static_cast<void*>(p)) T(); }
template<typename T> inline void FxConstruct( T* p, const T& value ) { ::new(static_cast<void*>(p)) T(value); }
template<typename T> inline void FxDestroy( T* p ) { p->~T(); }

template<typename T> class FxArrayBase
{
public:
	explicit FxArrayBase( FxSize count = 0 ) : _v(count ? static_cast<T*>(FxAllocate(count * sizeof(T))) : 0), _allocatedCount(count), _usedCount(0) {}
	~FxArrayBase()
	{
		for( FxSize i = 0; i < _usedCount; ++i ) FxDestroy(_v + i);
		if( _v ) FxFree(_v, _allocatedCount * sizeof(T));
	}
	void Swap( FxArrayBase& other ) { FxSwap(_v, other._v); FxSwap(_allocatedCount, other._allocatedCount); FxSwap(_usedCount, other._usedCount); }

	T* _v;
	FxSize _allocatedCount;
	FxSize _usedCount;

private:
	FxArrayBase( const FxArrayBase& );
	FxArrayBase& operator=( const FxArrayBase& );
};

template<typename T> class FxArray
{
public:
	typedef T* Iterator;
	typedef const T* ConstIterator;

	explicit FxArray( FxSize count = 0 ) : _container(count) {}
	FxArray( const FxArray& other ) : _container(other.Length()) { for( FxSize i = 0; i < other.Length(); ++i ) PushBack(other[i]); }
	FxArray& operator=( const FxArray& other )
	{
		if( this != &other ) { FxArray temp(other); _container.Swap(temp._container); }
		return *this;
	}

	FxSize Length() const { return _container._usedCount; }
	FxSize Allocated() const { return _container._allocatedCount; }
	FxBool IsEmpty() const { return _container._usedCount == 0; }
	T& operator[]( FxSize index ) { return _container._v[index]; }
	const T& operator[]( FxSize index ) const { return _container._v[index]; }
	T& Back() { return _container._v[_container._usedCount - 1]; }
	const T& Back() const { return _container._v[_container._usedCount - 1]; }
	Iterator Begin() { return _container._v; }
	Iterator End() { return _container._v + _container._usedCount; }
	ConstIterator Begin() const { return _container._v; }
	ConstIterator End() const { return _container._v + _container._usedCount; }
	void Prefetch() const {}

	void Reserve( FxSize count )
	{
		if( count <= _container._allocatedCount ) return;
		FxArrayBase<T> temp(count);
		for( FxSize i = 0; i < _container._usedCount; ++i ) FxConstruct(temp._v + i, _container._v[i]);
		temp._usedCount = _container._usedCount;
		_container.Swap(temp);
	}
	void PushBack( const T& element )
	{
		if( _container._usedCount == _container._allocatedCount )
		{
			T copy(element);
			Reserve(_container._allocatedCount ? _container._allocatedCount * 2 : 1);
			FxConstruct(_container._v + _container._usedCount++, copy);
			return;
		}
		FxConstruct(_container._v + _container._usedCount++, element);
	}
	void Insert( const T& element, FxSize index )
	{
		if( index >= _container._usedCount ) { PushBack(element); return; }
		T copy(element);
		PushBack(Back());
		for( FxSize i = _container._usedCount - 2; i > index; --i ) _container._v[i] = _container._v[i - 1];
		_container._v[index] = copy;
	}
	void Remove( FxSize index )
	{
		for( FxSize i = index; i + 1 < _container._usedCount; ++i ) _container._v[i] = _container._v[i + 1];
		FxDestroy(_container._v + --_container._usedCount);
	}
	void RemoveIterator( Iterator iter ) { Remove(static_cast<FxSize>(iter - _container._v)); }
	void Clear()
	{
		for( FxSize i = 0; i < _container._usedCount; ++i ) FxDestroy(_container._v + i);
		_container._usedCount = 0;
	}
	FxSize Find( const T& element, FxSize startIndex = 0 ) const
	{
		for( FxSize i = startIndex; i < _container._usedCount; ++i ) if( _container._v[i] == element ) return i;
		return FxInvalidIndex;
	}
	void Swap( FxArray& other ) { _container.Swap(other._container); }
	FxArchive& Serialize( FxArchive& arc );

	FxArrayBase<T> _container;
};

template<typename T> inline FxArchive& operator<<( FxArchive& arc, FxArray<T>& array ) { return array.Serialize(arc); }

template<typename T> class FxStringBase
{
public:
	FxStringBase() : _container(1) { _container._v[0] = 0; }
	FxStringBase( const T* str, FxSize length = FxInvalidIndex ) : _container(1) { _container._v[0] = 0; Assign(str, length); }
	FxStringBase( const FxStringBase& other ) : _container(1) { _container._v[0] = 0; Assign(other.GetData(), other.Length()); }
	FxStringBase& operator=( const FxStringBase& other ) { if( this != &other ) Assign(other.GetData(), other.Length()); return *this; }

	FxSize Length() const { return _container._usedCount; }
	FxBool IsEmpty() const { return _container._usedCount == 0; }
	const T* GetData() const { return _container._v; }
	T* GetData() { return _container._v; }
	const T* GetCstr() const { return _container._v; }
	T operator[]( FxSize index ) const { return _container._v[index]; }
	FxArchive& Serialize( FxArchive& arc );

	void Assign( const T* str, FxSize length )
	{
		if( !str ) length = 0;
		else if( length == FxInvalidIndex ) length = static_cast<FxSize>(strlen(str));
		FxArrayBase<T> temp(length + 1);
		if( length ) FxMemcpy(temp._v, str, length * sizeof(T));
		temp._v[length] = 0;
		temp._usedCount = length;
		_container.Swap(temp);
		temp._usedCount = 0;
	}

	FxArrayBase<T> _container;
};

typedef FxStringBase<FxChar> FxString;

template<typename T> inline FxBool operator==( const FxStringBase<T>& first, const FxStringBase<T>& second ) { return first.Length() == second.Length() && memcmp(first.GetData(), second.GetData(), first.Length() * sizeof(T)) == 0; }
template<typename T> inline FxBool operator==( const FxStringBase<T>& first, const T* second ) { return strcmp(first.GetData(), second ? second : "") == 0; }
template<typename T> inline FxBool operator!=( const FxStringBase<T>& first, const FxStringBase<T>& second ) { return !(first == second); }
template<typename T> inline FxArchive& operator<<( FxArchive& arc, FxStringBase<T>& str ) { return str.Serialize(arc); }

class FxName
{
public:
	FxName() : _ppRefString(NullSlot()) {}
	FxName( const FxChar* name ) : _ppRefString(Intern(name)) {}
	FxName( const FxName& other ) : _ppRefString(other._ppRefString) {}
	FxName& operator=( const FxName& other ) { _ppRefString = other._ppRefString; return *this; }
	FxBool operator==( const FxName& other ) const { return _ppRefString == other._ppRefString; }
	FxBool operator!=( const FxName& other ) const { return _ppRefString != other._ppRefString; }
	const FxString& GetAsString() const { return **_ppRefString; }
	const FxChar* GetAsCstr() const { return (*_ppRefString)->GetData(); }

	static const FxName NullName;

	static FxString** Intern( const FxChar* name )
	{
		if( !name || !name[0] ) return NullSlot();
		FxArray<FxString**>& table = Table();
		for( FxSize i = 0; i < table.Length(); ++i ) if( **table[i] == name ) return table[i];
		FxString** slot = new FxString*(new FxString(name));
		table.PushBack(slot);
		return slot;
	}

	FxString** _ppRefString;

private:
	static FxString** NullSlot() { static FxString* empty = new FxString(""); return &empty; }
	static FxArray<FxString**>& Table() { static FxArray<FxString**> table; return table; }
};

__declspec(selectany) const FxName FxName::NullName;

FxArchive& operator<<( FxArchive& arc, FxName& name );

template<typename T, FxSize hashBits, template<typename> class Container> class FxStringHash
{
public:
	struct FxHashBinEntry
	{
		FxHashBinEntry() : object() {}
		FxHashBinEntry( const FxString& iKey, const T& iObject ) : key(iKey), object(iObject) {}
		FxString key;
		T object;
	};

	FxStringHash() : numInsertions(0), numRemovals(0), numCollisions(0), numProbes(0) {}

	FxBool Insert( const FxChar* key, const T& object )
	{
		Container<FxHashBinEntry>& theBin = hashBins[_hash(key)];
		T found;
		if( _findKeyInBin(key, theBin, found) ) return FxFalse;
		if( !theBin.IsEmpty() ) ++numCollisions;
		++numInsertions;
		theBin.PushBack(FxHashBinEntry(FxString(key), object));
		return FxTrue;
	}
	FxBool Remove( const FxChar* key )
	{
		Container<FxHashBinEntry>& theBin = hashBins[_hash(key)];
		for( FxSize i = 0; i < theBin.Length(); ++i ) if( theBin[i].key == key ) { ++numRemovals; theBin.Remove(i); return FxTrue; }
		return FxFalse;
	}
	FxBool Find( const FxChar* key ) const { T found; return _findKeyInBin(key, hashBins[_hash(key)], found); }
	FxBool Find( const FxChar* key, T& object ) const { return _findKeyInBin(key, hashBins[_hash(key)], object); }
	void Clear() { for( FxSize i = 0; i < GetNumBins(); ++i ) hashBins[i].Clear(); }
	FxSize GetNumBins() const { return 1 << hashBits; }
	FxSize GetNumBinsInUse() const { FxSize n = 0; for( FxSize i = 0; i < GetNumBins(); ++i ) if( !hashBins[i].IsEmpty() ) ++n; return n; }
	FxArchive& Serialize( FxArchive& arc );

	FxSize numInsertions;
	FxSize numRemovals;
	FxSize numCollisions;
	mutable FxSize numProbes;
	Container<FxHashBinEntry> hashBins[1 << hashBits];

private:
	FxSize _hash( const FxChar* key ) const
	{
		FxInt16 binIndex = 5381;
		FxSize keyLength = static_cast<FxSize>(strlen(key));
		for( FxSize i = 0; i < keyLength; ++i ) binIndex = static_cast<FxInt16>(key[i] + 33 * binIndex);
		return binIndex & ((1 << hashBits) - 1);
	}
	FxBool _findKeyInBin( const FxChar* key, const Container<FxHashBinEntry>& theBin, T& object ) const
	{
		for( FxSize i = 0; i < theBin.Length(); ++i ) if( theBin[i].key == key ) { ++numProbes; object = theBin[i].object; return FxTrue; }
		return FxFalse;
	}
};

template<typename T, FxSize hashBits, template<typename> class Container>
inline FxArchive& operator<<( FxArchive& arc, FxStringHash<T, hashBits, Container>& hash ) { return hash.Serialize(arc); }

class FxObject;

class FxClass : public FxUseAllocator
{
public:
	FxClass( const FxChar* className, FxUInt16 currentVersion, const FxClass* pBaseClassDesc, FxSize classSize, FxObject* (*constructObjectFn)() )
		: _pBaseClassDesc(pBaseClassDesc), _constructObjectFn(constructObjectFn), _name(className), _numChildren(0), _size(classSize), _currentVersion(currentVersion)
	{
		Registry().PushBack(this);
	}
	const FxName& GetName() const { return _name; }
	FxUInt16 GetCurrentVersion() const { return _currentVersion; }
	FxSize GetSize() const { return _size; }
	const FxClass* GetBaseClassDesc() const { return _pBaseClassDesc; }
	FxObject* ConstructObject() const { return _constructObjectFn ? _constructObjectFn() : 0; }
	FxBool IsExactKindOf( const FxClass* classDesc ) const { return classDesc && _name == classDesc->_name; }
	FxBool IsKindOf( const FxClass* classDesc ) const
	{
		for( const FxClass* pClass = this; pClass; pClass = pClass->_pBaseClassDesc ) if( pClass->IsExactKindOf(classDesc) ) return FxTrue;
		return FxFalse;
	}
	static const FxClass* FindClassDesc( const FxName& className )
	{
		FxArray<FxClass*>& registry = Registry();
		for( FxSize i = 0; i < registry.Length(); ++i ) if( registry[i]->_name == className ) return registry[i];
		return 0;
	}

	const FxClass* _pBaseClassDesc;
	FxObject* (*_constructObjectFn)();
	FxName _name;
	FxSize _numChildren;
	FxSize _size;
	FxUInt16 _currentVersion;

private:
	static FxArray<FxClass*>& Registry() { static FxArray<FxClass*> registry; return registry; }
};

#define FX_DECLARE_CLASS_COMMON(klass) \
public: \
	static FxClass* AllocateStaticClass##klass(); \
	static const FxClass* StaticClass() { if( !_pClassDesc ) _pClassDesc = AllocateStaticClass##klass(); return _pClassDesc; } \
	virtual const FxClass* GetClassDesc() const { return StaticClass(); } \
	virtual FxUInt16 GetCurrentVersion() const { return StaticClass()->GetCurrentVersion(); } \
	friend FxArchive& operator<<( FxArchive& arc, klass& obj ) { obj.Serialize(arc); return arc; } \
	friend FxArchive& operator<<( FxArchive& arc, klass*& obj ) { return FxSerializeObjectPointer(arc, obj); } \
private: \
	static const FxClass* _pClassDesc;

#define FX_DECLARE_CLASS(klass, base) \
	FX_DECLARE_CLASS_COMMON(klass) \
public: \
	static FxObject* ConstructObject() { return new klass(); } \
private:

#define FX_DECLARE_ABSTRACT_CLASS(klass, base) \
	FX_DECLARE_CLASS_COMMON(klass) \
public: \
	static FxObject* ConstructObject() { return 0; } \
private:

#define FX_IMPLEMENT_CLASS(klass, version, base) \
	const FxClass* klass::_pClassDesc = 0; \
	FxClass* klass::AllocateStaticClass##klass() { return new FxClass(#klass, version, base::StaticClass(), sizeof(klass), klass::ConstructObject); }

#define FX_IMPLEMENT_BASE_CLASS(klass, version) \
	const FxClass* klass::_pClassDesc = 0; \
	FxClass* klass::AllocateStaticClass##klass() { return new FxClass(#klass, version, 0, sizeof(klass), klass::ConstructObject); }

class FxObject : public FxUseAllocator
{
	FX_DECLARE_CLASS(FxObject, FxObject)
public:
	FxObject() {}
	virtual ~FxObject() {}
	virtual void Serialize( FxArchive& arc );
	FxBool IsKindOf( const FxClass* classDesc ) const { const FxClass* pClass = GetClassDesc(); return pClass && pClass->IsKindOf(classDesc); }
};

template<typename T> inline T* FxCast( FxObject* pObj ) { return (pObj && pObj->IsKindOf(T::StaticClass())) ? static_cast<T*>(pObj) : 0; }

class FxDataContainer
{
public:
	FxDataContainer() : _userData(0) {}
	void* GetUserData() { return _userData; }
	void SetUserData( void* userData ) { _userData = userData; }

	void* _userData;
};

class FxNamedObject : public FxObject
{
	FX_DECLARE_CLASS(FxNamedObject, FxObject)
public:
	FxNamedObject() : _name(FxName::NullName) {}
	FxNamedObject( const FxName& name ) : _name(name) {}
	FxNamedObject( const FxNamedObject& other ) : FxObject(), _name(other._name) {}
	FxNamedObject& operator=( const FxNamedObject& other ) { if( this != &other ) _name = other._name; return *this; }
	virtual ~FxNamedObject() {}
	void SetName( const FxName& name ) { _name = name; }
	const FxName& GetName() const { return _name; }
	const FxString& GetNameAsString() const { return _name.GetAsString(); }
	const FxChar* GetNameAsCstr() const { return _name.GetAsCstr(); }
	virtual void Serialize( FxArchive& arc );

protected:
	FxName _name;
};

class FxArchiveStore : public FxUseAllocator
{
public:
	virtual ~FxArchiveStore() {}
	virtual FxBool Read( FxByte* data, FxSize numBytes ) = 0;
	virtual FxBool Write( const FxByte* data, FxSize numBytes ) = 0;
	virtual FxInt32 Length() const = 0;
};

class FxArchiveStoreNull : public FxArchiveStore
{
public:
	static FxArchiveStoreNull* Create() { return new FxArchiveStoreNull(); }
	virtual FxBool Read( FxByte*, FxSize ) { return FxTrue; }
	virtual FxBool Write( const FxByte*, FxSize ) { return FxTrue; }
	virtual FxInt32 Length() const { return 0; }
};

class FxArchiveStoreMemory : public FxArchiveStore
{
public:
	static FxArchiveStoreMemory* Create( const FxByte*, FxSize ) { return new FxArchiveStoreMemory(); }
	virtual FxBool Read( FxByte*, FxSize ) { return FxFalse; }
	virtual FxBool Write( const FxByte* data, FxSize numBytes ) { for( FxSize i = 0; i < numBytes; ++i ) _bytes.PushBack(data[i]); return FxTrue; }
	virtual FxInt32 Length() const { return static_cast<FxInt32>(_bytes.Length()); }
	const FxByte* GetMemory() const { return _bytes.Length() ? &_bytes[0] : 0; }
	FxSize GetSize() const { return _bytes.Length(); }

private:
	FxArray<FxByte> _bytes;
};

class FxArchiveStoreMemoryNoCopy : public FxArchiveStore
{
public:
	static FxArchiveStoreMemoryNoCopy* Create( const FxByte* memory, FxSize size ) { return new FxArchiveStoreMemoryNoCopy(memory, size); }
	FxArchiveStoreMemoryNoCopy( const FxByte* memory, FxSize size ) : _memory(memory), _size(size), _pos(0) {}
	virtual FxBool Read( FxByte* data, FxSize numBytes )
	{
		if( _pos + numBytes > _size ) { FxMemset(data, 0, numBytes); _pos = _size; _overrun = FxTrue; return FxFalse; }
		FxMemcpy(data, _memory + _pos, numBytes);
		_pos += numBytes;
		return FxTrue;
	}
	virtual FxBool Write( const FxByte*, FxSize ) { return FxFalse; }
	virtual FxInt32 Length() const { return static_cast<FxInt32>(_size); }
	FxSize Tell() const { return _pos; }

	static FxBool _overrun;

private:
	const FxByte* _memory;
	FxSize _size;
	FxSize _pos;
};

__declspec(selectany) FxBool FxArchiveStoreMemoryNoCopy::_overrun = FxFalse;

struct FxArchiveDirectory
{
	FxArray<FxString> Names;
	FxSize NumObjectPointers;
	FxStringHash<FxUInt16, 7, FxArray> ClassVersions;

	FxArchiveDirectory() : NumObjectPointers(0) {}
	FxUInt16 FindClassVersion( const FxChar* className ) const { FxUInt16 version = 0; ClassVersions.Find(className, version); return version; }
	FxSize FindName( const FxString& name ) const { for( FxSize i = 0; i < Names.Length(); ++i ) if( Names[i] == name ) return i; return FxInvalidIndex; }
};

class FxArchive : public FxUseAllocator
{
public:
	enum FxArchiveMode { AM_LinearLoad = 0, AM_Load = 0, AM_LazyLoad = 1, AM_CreateDirectory = 2, AM_ApproximateMemoryUsage = 3, AM_Save = 4 };
	enum FxArchiveByteOrder { ABO_LittleEndian = 0, ABO_BigEndian = 1 };

	FxArchive( FxArchiveStore* pStore, FxArchiveMode mode, FxArchiveByteOrder byteOrder = ABO_LittleEndian )
		: _store(pStore), _mode(mode), _byteOrder(byteOrder), _isValid(FxFalse), _sdkVersion(0), _ownDirectory(FxTrue), _directory(new FxArchiveDirectory()) {}
	~FxArchive()
	{
		delete _store;
		if( _ownDirectory ) delete _directory;
	}

	FxBool IsLoading() const { return _mode == AM_Load || _mode == AM_LazyLoad; }
	FxBool IsSaving() const { return _mode == AM_Save || _mode == AM_CreateDirectory || _mode == AM_ApproximateMemoryUsage; }
	FxBool IsValid() const { return _isValid; }
	FxSize GetSDKVersion() const { return _sdkVersion; }
	const FxArchiveDirectory* GetInternalData() const { return _directory; }
	void SetInternalDataState( const FxArchiveDirectory* pDirectory )
	{
		_directory->Names = pDirectory->Names;
		_directory->NumObjectPointers = pDirectory->NumObjectPointers;
		for( FxSize i = 0; i < pDirectory->ClassVersions.GetNumBins(); ++i ) _directory->ClassVersions.hashBins[i] = pDirectory->ClassVersions.hashBins[i];
		if( _mode == AM_Save ) _serializeDirectory();
	}
	void RegisterProgressCallback( void (*)(FxReal), FxReal ) {}

	void Open()
	{
		_isValid = FxTrue;
		FxChar magic[4] = { 'F', 'A', 'C', 'E' };
		if( IsSaving() )
		{
			_sdkVersion = 1731;
			FxSize fileFormatVersion = 0;
			FxString licensee("Unreal Engine 3 Licensee");
			FxString project("Unreal Engine 3 Project");
			FxSize licenseeSize = licensee.Length() + 1;
			FxSize projectSize = project.Length() + 1;
			FxSize licenseeVersion = 1000;
			SerializePODArray(magic, 4);
			*this << _sdkVersion << fileFormatVersion << licenseeSize;
			SerializePODArray(licensee.GetData(), licenseeSize);
			*this << projectSize;
			SerializePODArray(project.GetData(), projectSize);
			*this << licenseeVersion;
			return;
		}
		SerializePODArray(magic, 4);
		if( magic[0] != 'F' || magic[1] != 'A' || magic[2] != 'C' || magic[3] != 'E' ) { _isValid = FxFalse; return; }
		*this << _sdkVersion;
		FxSize fileFormatVersion = 0;
		if( _sdkVersion >= 0x6B8 ) *this << fileFormatVersion;
		FxSize size = 0;
		*this << size;
		FxArray<FxChar> skip(size + 1);
		SerializePODArray(skip._container._v, size);
		*this << size;
		FxArray<FxChar> skipProject(size + 1);
		SerializePODArray(skipProject._container._v, size);
		*this << size;
		_serializeDirectory();
		_objects.Reserve(_directory->NumObjectPointers);
		_isValid = !FxArchiveStoreMemoryNoCopy::_overrun;
	}

	FxUInt16 SerializeClassVersion( const FxChar* className, FxBool = FxFalse, FxUInt16 classVersion = 0, const FxClass* pClass = 0 )
	{
		if( IsSaving() )
		{
			if( !pClass ) pClass = FxClass::FindClassDesc(FxName(className));
			FxUInt16 version = pClass ? pClass->GetCurrentVersion() : classVersion;
			_directory->ClassVersions.Insert(className, version);
		}
		return _directory->FindClassVersion(className);
	}

	template<typename T> void SerializePODArray( T* array, FxSize length )
	{
		if( IsSaving() ) _store->Write(reinterpret_cast<const FxByte*>(array), length * sizeof(T));
		else _store->Read(reinterpret_cast<FxByte*>(array), length * sizeof(T));
	}
	template<typename T> FxArchive& SerializeRaw( T& elem ) { SerializePODArray(&elem, 1); return *this; }

	FxArchive& operator<<( FxChar& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxUInt8& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxInt16& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxUInt16& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxInt32& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxUInt32& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxReal& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxDReal& elem ) { return SerializeRaw(elem); }
	FxArchive& operator<<( FxBool& elem ) { FxUInt32 temp = elem; SerializeRaw(temp); elem = temp == 1; return *this; }

	FxSize AddName( const FxString& name )
	{
		FxSize index = _directory->FindName(name);
		if( index != FxInvalidIndex ) return index;
		_directory->Names.PushBack(name);
		return _directory->Names.Length() - 1;
	}
	const FxString* GetName( FxSize index ) const { return index < _directory->Names.Length() ? &_directory->Names[index] : 0; }
	FxObject* FindObject( FxSize index ) const { return index < _objects.Length() ? _objects[index] : 0; }
	FxSize FindObject( FxObject* object ) const { return _objects.Find(object); }
	FxSize AddObject( FxObject* object, FxSize index = FxInvalidIndex )
	{
		if( !object ) return FxInvalidIndex;
		if( index == FxInvalidIndex )
		{
			FxSize found = FindObject(object);
			if( found != FxInvalidIndex ) return found;
			_objects.PushBack(object);
			return _objects.Length() - 1;
		}
		while( _objects.Length() <= index ) _objects.PushBack(0);
		_objects[index] = object;
		return index;
	}

private:
	void _serializeDirectory()
	{
		FxUInt16 version = 1;
		*this << version;
		if( version ) *this << _directory->ClassVersions;
		FxSize numNames = _directory->Names.Length();
		*this << numNames;
		if( IsLoading() )
		{
			_directory->Names.Clear();
			for( FxSize i = 0; i < numNames; ++i ) { FxString name; *this << name; _directory->Names.PushBack(name); }
		}
		else
		{
			for( FxSize i = 0; i < numNames; ++i ) *this << _directory->Names[i];
		}
		*this << _directory->NumObjectPointers;
	}

	FxArchiveStore* _store;
	FxArchiveMode _mode;
	FxArchiveByteOrder _byteOrder;
	FxBool _isValid;
	FxSize _sdkVersion;
	FxBool _ownDirectory;
	FxArchiveDirectory* _directory;
	FxArray<FxObject*> _objects;
};

template<typename T> inline FxArchive& FxStringBase<T>::Serialize( FxArchive& arc )
{
	FxSize length = Length();
	arc << length;
	if( arc.IsSaving() ) { arc.SerializePODArray(_container._v, length); return arc; }
	FxArrayBase<T> temp(length + 1);
	arc.SerializePODArray(temp._v, length);
	temp._v[length] = 0;
	temp._usedCount = length;
	_container.Swap(temp);
	temp._usedCount = 0;
	return arc;
}

template<typename T> struct FxIsPOD { enum { value = 0 }; };
template<> struct FxIsPOD<FxReal> { enum { value = 1 }; };
template<> struct FxIsPOD<FxSize> { enum { value = 1 }; };
template<> struct FxIsPOD<FxInt32> { enum { value = 1 }; };
template<> struct FxIsPOD<FxUInt16> { enum { value = 1 }; };
template<> struct FxIsPOD<FxChar> { enum { value = 1 }; };
template<> struct FxIsPOD<FxByte> { enum { value = 1 }; };

template<typename T, int isPOD> struct FxArraySerializer
{
	static void Run( FxArray<T>& array, FxArchive& arc, FxSize length )
	{
		if( arc.IsSaving() ) { for( FxSize i = 0; i < length; ++i ) arc << array[i]; return; }
		FxArrayBase<T> temp(length);
		for( FxSize j = 0; j < length; ++j ) { FxDefaultConstruct(temp._v + j); arc << temp._v[j]; }
		temp._usedCount = length;
		array._container.Swap(temp);
	}
};

template<typename T> struct FxArraySerializer<T, 1>
{
	static void Run( FxArray<T>& array, FxArchive& arc, FxSize length )
	{
		if( arc.IsSaving() ) { arc.SerializePODArray(array._container._v, length); return; }
		FxArrayBase<T> temp(length);
		arc.SerializePODArray(temp._v, length);
		temp._usedCount = length;
		array._container.Swap(temp);
	}
};

template<typename T> inline FxArchive& FxArray<T>::Serialize( FxArchive& arc )
{
	FxSize length = Length();
	arc << length;
	FxArraySerializer<T, FxIsPOD<T>::value>::Run(*this, arc, length);
	return arc;
}

template<typename T, FxSize hashBits, template<typename> class Container>
inline FxArchive& FxStringHash<T, hashBits, Container>::Serialize( FxArchive& arc )
{
	if( arc.IsLoading() ) Clear();
	FxSize numBinsInUse = arc.IsSaving() ? GetNumBinsInUse() : 0;
	arc << numBinsInUse;
	if( arc.IsSaving() )
	{
		for( FxSize binIndex = 0; binIndex < GetNumBins(); ++binIndex )
		{
			if( hashBins[binIndex].IsEmpty() ) continue;
			FxSize index = binIndex;
			FxSize len = hashBins[binIndex].Length();
			arc << index << len;
			for( FxSize i = 0; i < len; ++i ) arc << hashBins[binIndex][i].key << hashBins[binIndex][i].object;
		}
		return arc;
	}
	for( FxSize j = 0; j < numBinsInUse; ++j )
	{
		FxSize serializedBinIndex = 0;
		FxSize serializedBinLen = 0;
		arc << serializedBinIndex << serializedBinLen;
		for( FxSize i = 0; i < serializedBinLen; ++i )
		{
			FxHashBinEntry serializedEntry;
			arc << serializedEntry.key << serializedEntry.object;
			if( serializedBinIndex < GetNumBins() ) hashBins[serializedBinIndex].PushBack(serializedEntry);
		}
	}
	return arc;
}

inline FxArchive& operator<<( FxArchive& arc, FxName& name )
{
	FxUInt16 version = arc.SerializeClassVersion("FxName", FxTrue, 1);
	if( arc.IsSaving() )
	{
		FxSize nameIndex = arc.AddName(name.GetAsString());
		arc << nameIndex;
		return arc;
	}
	if( version )
	{
		FxSize index = FxInvalidIndex;
		arc << index;
		const FxString* pName = arc.GetName(index);
		name = pName ? FxName(pName->GetData()) : FxName::NullName;
		return arc;
	}
	FxString str;
	arc << str;
	name = FxName(str.GetData());
	return arc;
}

template<typename T> inline FxArchive& FxSerializeObjectPointer( FxArchive& arc, T*& obj )
{
	if( arc.IsLoading() )
	{
		FxSize index = FxInvalidIndex;
		arc << index;
		if( index == FxInvalidIndex ) { obj = 0; return arc; }
		FxObject* foundObject = arc.FindObject(index);
		if( foundObject ) { obj = static_cast<T*>(foundObject); return arc; }
		FxName className;
		arc << className;
		FxSize classSize = 0;
		arc << classSize;
		const FxClass* objectClass = FxClass::FindClassDesc(className);
		if( !objectClass ) { obj = 0; return arc; }
		FxObject* newObject = objectClass->ConstructObject();
		if( !newObject ) { obj = 0; return arc; }
		newObject->Serialize(arc);
		obj = static_cast<T*>(newObject);
		arc.AddObject(obj, index);
		return arc;
	}
	if( !obj )
	{
		FxSize nullIndex = FxInvalidIndex;
		arc << nullIndex;
		return arc;
	}
	FxBool needToSaveObject = arc.FindObject(obj) == FxInvalidIndex;
	FxSize objIndex = arc.AddObject(obj);
	arc << objIndex;
	if( needToSaveObject )
	{
		FxName name(obj->GetClassDesc()->GetName());
		FxSize size = obj->GetClassDesc()->GetSize();
		arc << name << size;
		obj->Serialize(arc);
	}
	return arc;
}

inline void FxObject::Serialize( FxArchive& arc ) { arc.SerializeClassVersion("FxObject"); }
inline void FxNamedObject::Serialize( FxArchive& arc ) { FxObject::Serialize(arc); arc.SerializeClassVersion("FxNamedObject"); arc << _name; }

__declspec(selectany) const FxClass* FxObject::_pClassDesc = 0;
inline FxClass* FxObject::AllocateStaticClassFxObject() { return new FxClass("FxObject", 0, 0, sizeof(FxObject), FxObject::ConstructObject); }
__declspec(selectany) const FxClass* FxNamedObject::_pClassDesc = 0;
inline FxClass* FxNamedObject::AllocateStaticClassFxNamedObject() { return new FxClass("FxNamedObject", 0, FxObject::StaticClass(), sizeof(FxNamedObject), FxNamedObject::ConstructObject); }

} // namespace Face
} // namespace OC3Ent

#endif
