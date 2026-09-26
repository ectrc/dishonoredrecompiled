/*=============================================================================
	NxArray.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): every NxArray<T,Alloc> instantiation in PhysXCore.pdb is 12 bytes with
	`T* first` @0, `T* last` @4, `T* memEnd` @8 and the allocator as an empty base, so size() is
	last-first and capacity() is memEnd-first. NxAllocatorDefault is a 1-byte empty type; the retail
	exe imports NxGetPhysicsSDKAllocator from PhysXLoader.dll (imports_2013.csv) precisely because the
	default allocator routes through it, which is what allocate()/deallocate() do below.

	This matters at the ABI: NxActorDesc::shapes is such an array and the SDK reads it through
	NxActorDescBase::shapesStart (see NxActorDesc.h).
=============================================================================*/

#ifndef NX_ARRAY_H
#define NX_ARRAY_H

#include "Nx.h"

class NxUserAllocator;
NX_C_EXPORT NXPHYSXLOADER_API NxUserAllocator* NX_CALL_CONV NxGetPhysicsSDKAllocator();

void* NxAllocatorDefaultMalloc(size_t size);
void NxAllocatorDefaultFree(void* mem);

class NxAllocatorDefault
{
public:
	NX_INLINE void* allocate(size_t size) { return NxAllocatorDefaultMalloc(size); }
	NX_INLINE void deallocate(void* mem) { NxAllocatorDefaultFree(mem); }
};

template<class T, class Alloc = NxAllocatorDefault>
class NxArray : public Alloc
{
public:
	T* first;
	T* last;
	T* memEnd;

	NX_INLINE NxArray() : first(0), last(0), memEnd(0) {}
	NX_INLINE NxArray(const NxArray& other) : first(0), last(0), memEnd(0) { *this = other; }
	NX_INLINE ~NxArray() { if (first) { this->deallocate(first); } first = last = memEnd = 0; }

	NX_INLINE NxArray& operator=(const NxArray& other)
	{
		if (this != &other)
		{
			clear();
			reserve(other.size());
			for (const T* p = other.first; p != other.last; ++p) { *last++ = *p; }
		}
		return *this;
	}

	NX_INLINE NxU32 size() const { return (NxU32)(last - first); }
	NX_INLINE NxU32 capacity() const { return (NxU32)(memEnd - first); }
	NX_INLINE bool empty() const { return first == last; }
	NX_INLINE T* begin() { return first; }
	NX_INLINE const T* begin() const { return first; }
	NX_INLINE T* end() { return last; }
	NX_INLINE const T* end() const { return last; }
	NX_INLINE T& operator[](NxU32 i) { return first[i]; }
	NX_INLINE const T& operator[](NxU32 i) const { return first[i]; }
	NX_INLINE T& back() { return last[-1]; }
	NX_INLINE const T& back() const { return last[-1]; }
	NX_INLINE void clear() { last = first; }
	NX_INLINE void reset() { clear(); }

	NX_INLINE void reserve(NxU32 n)
	{
		if (n <= capacity()) { return; }
		const NxU32 used = size();
		T* mem = (T*)this->allocate(n * sizeof(T));
		if (first)
		{
			memcpy(mem, first, used * sizeof(T));
			this->deallocate(first);
		}
		first = mem;
		last = mem + used;
		memEnd = mem + n;
	}
	NX_INLINE T& pushBack(const T& v)
	{
		if (last == memEnd) { reserve(capacity() ? capacity() * 2 : 4); }
		*last = v;
		return *last++;
	}
	NX_INLINE T popBack() { return *--last; }
	// the engine spells it push_back (UnPhysActor.cpp, UnPhysAsset.cpp, UnSVehicle.cpp)
	NX_INLINE T& push_back(const T& v) { return pushBack(v); }
	NX_INLINE T pop_back() { return popBack(); }
	NX_INLINE void resize(NxU32 n)
	{
		reserve(n);
		while (size() < n) { *last++ = T(); }
		last = first + n;
	}
	NX_INLINE void replaceWithLast(NxU32 i) { first[i] = *--last; }
};

#endif // NX_ARRAY_H
