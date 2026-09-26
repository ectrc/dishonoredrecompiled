/*=============================================================================
	NxUserAllocator.h, NxUserOutputStream, NxStream, NxUserEntityReport: the PhysX 2.8.4 callback
	interfaces the engine derives from, reconstructed for Dishonored.

	DISHONORED(layout): vtable slots from PhysXCore.pdb, in the order the DLL has them. These four are
	hand-written rather than generated because the engine implements them (FNxAllocator,
	FNxOutputStream, FNxMemoryBuffer in Engine/Src/UnNovodexSupport.h / UnPhysLevel.cpp), so the
	non-pure virtuals need the same forwarding bodies the SDK header has:

	  NxUserAllocator   vt[0] mallocDEBUG(NxU32,const char*,int,const char*,NxMemoryType) (not pure)
	                    vt[1] mallocDEBUG(NxU32,const char*,int) = 0
	                    vt[2] malloc(NxU32,NxMemoryType)  (not pure)
	                    vt[3] malloc(NxU32) = 0
	                    vt[4] realloc(void*,NxU32) = 0
	                    vt[5] free(void*) = 0
	                    vt[6] checkDEBUG() (not pure)
	                    vt[7] ~NxUserAllocator   <- the destructor is LAST, not first
	  NxUserOutputStream vt[0] reportError = 0, vt[1] reportAssertViolation = 0, vt[2] print = 0,
	                    vt[3] ~NxUserOutputStream
	  NxStream          vt[0] ~NxStream, vt[1..6] read* const = 0, vt[7..12] store* = 0
	  NxUserEntityReport<T> vt[0] onEvent(NxU32, T*) = 0 (no destructor at all)
=============================================================================*/

#ifndef NX_USER_ALLOCATOR_H
#define NX_USER_ALLOCATOR_H

#include "Nx.h"

// DISHONORED(layout): PDB enum NxMemoryType and NxErrorCode / NxAssertResponse are generated in
// NxGenerated.h, but these four interfaces are needed before it, so the two enums used in their
// signatures are declared here (identical values, see NxGenerated.h).
enum NxMemoryType
{
	NX_MEMORY_PERSISTENT,
	NX_MEMORY_TEMP,
	NX_MEMORY_TEMP_ARRAY,
	NX_MEMORY_LAST
};

enum NxErrorCode
{
	NXE_NO_ERROR = 0,
	NXE_INVALID_PARAMETER = 1,
	NXE_INVALID_OPERATION = 2,
	NXE_OUT_OF_MEMORY = 3,
	NXE_INTERNAL_ERROR = 4,
	NXE_ASSERTION = 107,
	NXE_DB_INFO = 205,
	NXE_DB_WARNING = 206,
	NXE_DB_PRINT = 208
};

enum NxAssertResponse
{
	NX_AR_CONTINUE,
	NX_AR_IGNORE,
	NX_AR_BREAKPOINT
};

// DISHONORED(layout): MSVC lays consecutive virtual OVERLOADS of the same name out in REVERSE
// declaration order (verified with cl /FAsc on build/agentAL_vt/vtprobe.cpp: `virtual int f(int)`
// declared before `virtual int f(int,int)` puts f(int,int) in slot 0). The shipped
// NxUserAllocatorDefault vtable in PhysXCore.dll (rva 0x3568e0) is
//   slot 0 mallocDEBUG (5 args)   slot 1 mallocDEBUG (3 args, pure in the base: `_purecall` at
//   slot 1 of ??_7NxUserAllocator@@6B@, rva 0x3568c0)   slot 2 malloc (2 args)   slot 3 malloc
//   (1 arg, pure)   slot 4 realloc   slot 5 free   slot 6 checkDEBUG   slot 7 ~NxUserAllocator
// so each overload pair below is declared with the LATER slot first.
class NxUserAllocator
{
public:
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line) = 0;					// vt[1]
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
	{																							// vt[0]
		NX_UNUSED(className); NX_UNUSED(type);
		return mallocDEBUG(size, fileName, line);
	}
	virtual void* malloc(size_t size) = 0;														// vt[3]
	virtual void* malloc(size_t size, NxMemoryType type) { NX_UNUSED(type); return malloc(size); }	// vt[2]
	virtual void* realloc(void* memory, size_t size) = 0;
	virtual void free(void* memory) = 0;
	virtual void checkDEBUG(void) {}
	virtual ~NxUserAllocator() {}
};

class NxUserOutputStream
{
public:
	virtual void reportError(NxErrorCode code, const char* message, const char* file, int line) = 0;
	virtual NxAssertResponse reportAssertViolation(const char* message, const char* file, int line) = 0;
	virtual void print(const char* message) = 0;
	virtual ~NxUserOutputStream() {}
};

class NxStream
{
public:
	virtual ~NxStream() {}
	virtual NxU8 readByte() const = 0;
	virtual NxU16 readWord() const = 0;
	virtual NxU32 readDword() const = 0;
	virtual NxF32 readFloat() const = 0;
	virtual NxF64 readDouble() const = 0;
	virtual void readBuffer(void* buffer, NxU32 size) const = 0;
	virtual NxStream& storeByte(NxU8 b) = 0;
	virtual NxStream& storeWord(NxU16 w) = 0;
	virtual NxStream& storeDword(NxU32 d) = 0;
	virtual NxStream& storeFloat(NxF32 f) = 0;
	virtual NxStream& storeDouble(NxF64 f) = 0;
	virtual NxStream& storeBuffer(const void* buffer, NxU32 size) = 0;

	NX_INLINE NxStream& storeByte(char b) { return storeByte((NxU8)b); }
	NX_INLINE NxStream& storeWord(NxI16 w) { return storeWord((NxU16)w); }
	NX_INLINE NxStream& storeDword(NxI32 d) { return storeDword((NxU32)d); }
};

template<class T>
class NxUserEntityReport
{
public:
	virtual bool onEvent(NxU32 nbEntities, T* entities) = 0;
};

#endif // NX_USER_ALLOCATOR_H
