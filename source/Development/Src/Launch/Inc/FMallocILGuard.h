/*=============================================================================
	FMallocILGuard.h: agent IL's guard-page heap (DISHONORED(bringup), diagnostic only, -ilguardheap).

	Wraps GMalloc. From process start every block carries a 16-byte header (magic, size) and an 8-byte tail canary.
	Once armed (GMalloc->Exec("ILGUARD ARM"), raised by the first matinee controller add):
	- a block of up to one page is placed at the END of its own committed page, followed by a reserved no-access page;
	  up to 16 pages likewise in a medium arena. Its free decommits the pages and the slot goes to the back of a FIFO,
	  so a write past the end or after the free faults on the writer's instruction (the vectored handler logs the slot:
	  size, state, the alloc and free trails);
	- a header block freed after arming is filled with 0xDD and held in a FIFO quarantine; every tick (and at eviction)
	  the pattern is verified, a changed byte is reported with the block's free trail;
	- double frees, foreign frees and broken canaries are reported.
	Trails are stack scans (values inside the exe's .text above ESP), raw addresses resolved offline against the .map.
	Report: DishonoredGame/Logs/ILGuard.log (written with WriteFile, never through GMalloc).
=============================================================================*/

#ifndef __FMALLOCILGUARD_H__
#define __FMALLOCILGUARD_H__

class FMallocILGuard : public FMalloc
{
	enum { PAGE = 4096, HEADER = 16, TAIL = 8, TRAIL = 12 };
	enum { MAGIC_LIVE = 0x31474C49, MAGIC_FREE = 0x46474C49, TAIL_WORD = 0xA5C3A5C3 };
	enum { SLOT_FREE = 0, SLOT_LIVE = 1 };
	enum { MAX_REPORTS = 400 };

	struct FSlotRecord
	{
		DWORD Size;
		DWORD State;
		DWORD Serial;
		DWORD AllocTrail[TRAIL];
		DWORD FreeTrail[TRAIL];
	};

	struct FArena
	{
		BYTE* Base;
		DWORD SlotBytes;
		DWORD DataPages;
		DWORD NumSlots;
		DWORD* Queue;
		DWORD QueueHead;
		DWORD QueueCount;
		FSlotRecord* Records;
	};

	struct FQuarantined
	{
		BYTE* Data;
		DWORD Size;
		DWORD Serial;
		DWORD FreeTrail[TRAIL];
	};

	FMalloc* Inner;
	CRITICAL_SECTION Lock;
	volatile LONG bArmed;
	HANDLE ReportFile;
	DWORD TextStart;
	DWORD TextEnd;
	FArena Small;
	FArena Medium;
	FQuarantined* Quarantine;
	DWORD QuarantineCapacity;
	DWORD QuarantineHead;
	DWORD QuarantineCount;
	DWORD QuarantineBytes;
	DWORD QuarantineMaxBytes;
	DWORD Serial;
	DWORD Reports;
	DWORD GuardedAllocs;
	DWORD FallbackAllocs;
	DWORD TickCount;
	static FMallocILGuard* Instance;

	void Report( const char* Format, ... )
	{
		if( ReportFile == INVALID_HANDLE_VALUE || Reports >= MAX_REPORTS )
		{
			return;
		}
		char Buffer[2048];
		va_list Args;
		va_start( Args, Format );
		const int Length = _vsnprintf( Buffer, sizeof(Buffer) - 2, Format, Args );
		va_end( Args );
		const DWORD Count = Length < 0 ? sizeof(Buffer) - 2 : (DWORD)Length;
		Buffer[Count] = '\n';
		DWORD Written = 0;
		WriteFile( ReportFile, Buffer, Count + 1, &Written, NULL );
		FlushFileBuffers( ReportFile );
	}

	void ReportTrail( const char* Label, const DWORD* Trail )
	{
		Report( "  %s %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X", Label, Trail[0], Trail[1], Trail[2], Trail[3], Trail[4], Trail[5],
			Trail[6], Trail[7], Trail[8], Trail[9], Trail[10], Trail[11] );
	}

	void CaptureTrailFrom( const DWORD* Stack, DWORD* Out )
	{
		const DWORD* StackBase = (const DWORD*)( (NT_TIB*)NtCurrentTeb() )->StackBase;
		INT Found = 0;
		for( const DWORD* Cursor = Stack; Cursor < StackBase && Cursor < Stack + 1024 && Found < TRAIL; Cursor++ )
		{
			if( *Cursor >= TextStart && *Cursor < TextEnd )
			{
				Out[Found++] = *Cursor;
			}
		}
		while( Found < TRAIL )
		{
			Out[Found++] = 0;
		}
	}

	void CaptureTrail( DWORD* Out )
	{
		DWORD Anchor = 0;
		CaptureTrailFrom( &Anchor, Out );
	}

	void InitArena( FArena& Arena, DWORD DataPages, DWORD MaxBytes )
	{
		appMemzero( &Arena, sizeof(Arena) );
		Arena.DataPages = DataPages;
		Arena.SlotBytes = ( DataPages + 1 ) * PAGE;
		for( DWORD Bytes = MaxBytes; Bytes >= 16 * 1024 * 1024 && !Arena.Base; Bytes /= 2 )
		{
			Arena.Base = (BYTE*)VirtualAlloc( NULL, Bytes, MEM_RESERVE, PAGE_NOACCESS );
			if( Arena.Base )
			{
				Arena.NumSlots = Bytes / Arena.SlotBytes;
			}
		}
		if( !Arena.Base )
		{
			return;
		}
		Arena.Queue = (DWORD*)VirtualAlloc( NULL, Arena.NumSlots * sizeof(DWORD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
		Arena.Records = (FSlotRecord*)VirtualAlloc( NULL, Arena.NumSlots * sizeof(FSlotRecord), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
		for( DWORD Index = 0; Index < Arena.NumSlots; Index++ )
		{
			Arena.Queue[Index] = Index;
		}
		Arena.QueueCount = Arena.NumSlots;
	}

	static UBOOL InArena( const FArena& Arena, const void* Ptr )
	{
		return Arena.Base && (const BYTE*)Ptr >= Arena.Base && (const BYTE*)Ptr < Arena.Base + Arena.NumSlots * Arena.SlotBytes;
	}

	static DWORD AlignOf( DWORD Size, DWORD Alignment )
	{
		if( Alignment == DEFAULT_ALIGNMENT )
		{
			return Size >= 16 ? 16 : 8;
		}
		return Alignment;
	}

	void* ArenaMalloc( FArena& Arena, DWORD Size, DWORD Alignment )
	{
		if( !Arena.Base || Arena.QueueCount == 0 )
		{
			return NULL;
		}
		const DWORD Slot = Arena.Queue[Arena.QueueHead];
		Arena.QueueHead = ( Arena.QueueHead + 1 ) % Arena.NumSlots;
		Arena.QueueCount--;
		BYTE* SlotBase = Arena.Base + Slot * Arena.SlotBytes;
		if( !VirtualAlloc( SlotBase, Arena.DataPages * PAGE, MEM_COMMIT, PAGE_READWRITE ) )
		{
			return NULL;
		}
		const DWORD Align = AlignOf( Size, Alignment );
		const DWORD Rounded = ( Size + Align - 1 ) & ~( Align - 1 );
		BYTE* Data = SlotBase + Arena.DataPages * PAGE - Rounded;
		FSlotRecord& Record = Arena.Records[Slot];
		Record.Size = Size;
		Record.State = SLOT_LIVE;
		Record.Serial = ++Serial;
		CaptureTrail( Record.AllocTrail );
		GuardedAllocs++;
		return Data;
	}

	void ArenaFree( FArena& Arena, void* Ptr )
	{
		const DWORD Slot = (DWORD)( (BYTE*)Ptr - Arena.Base ) / Arena.SlotBytes;
		FSlotRecord& Record = Arena.Records[Slot];
		if( Record.State != SLOT_LIVE )
		{
			DWORD Trail[TRAIL];
			CaptureTrail( Trail );
			Report( "DOUBLE FREE guarded %p size %u serial %u", Ptr, Record.Size, Record.Serial );
			ReportTrail( "alloc", Record.AllocTrail );
			ReportTrail( "free1", Record.FreeTrail );
			ReportTrail( "free2", Trail );
			return;
		}
		Record.State = SLOT_FREE;
		CaptureTrail( Record.FreeTrail );
		VirtualFree( Arena.Base + Slot * Arena.SlotBytes, Arena.DataPages * PAGE, MEM_DECOMMIT );
		Arena.Queue[( Arena.QueueHead + Arena.QueueCount ) % Arena.NumSlots] = Slot;
		Arena.QueueCount++;
	}

	UBOOL ArenaSize( const FArena& Arena, void* Ptr, DWORD& Size ) const
	{
		if( !InArena( Arena, Ptr ) )
		{
			return FALSE;
		}
		Size = Arena.Records[(DWORD)( (BYTE*)Ptr - Arena.Base ) / Arena.SlotBytes].Size;
		return TRUE;
	}

	static DWORD* HeaderOf( void* Ptr )
	{
		return (DWORD*)( (BYTE*)Ptr - HEADER );
	}

	void* HeaderMalloc( DWORD Size, DWORD Alignment )
	{
		const DWORD Align = Max<DWORD>( 16, Alignment );
		BYTE* Base = (BYTE*)Inner->Malloc( Size + HEADER + Align + TAIL, DEFAULT_ALIGNMENT );
		if( !Base )
		{
			return NULL;
		}
		BYTE* Data = (BYTE*)( ( (PTRINT)Base + HEADER + Align - 1 ) & ~(PTRINT)( Align - 1 ) );
		const DWORD Offset = (DWORD)( Data - Base );
		DWORD* Header = HeaderOf( Data );
		Header[0] = MAGIC_LIVE;
		Header[1] = Size;
		Header[2] = Offset;
		Header[3] = ++Serial;
		appMemcpy( Data + Size, "\xC3\xA5\xC3\xA5\xC3\xA5\xC3\xA5", TAIL );
		return Data;
	}

	UBOOL CheckTail( void* Ptr, DWORD Size )
	{
		static const BYTE Expected[TAIL] = { 0xC3, 0xA5, 0xC3, 0xA5, 0xC3, 0xA5, 0xC3, 0xA5 };
		if( appMemcmp( (BYTE*)Ptr + Size, Expected, TAIL ) == 0 )
		{
			return TRUE;
		}
		const BYTE* Tail = (BYTE*)Ptr + Size;
		DWORD Trail[TRAIL];
		CaptureTrail( Trail );
		Report( "OVERFLOW header block %p size %u serial %u tail %02X %02X %02X %02X %02X %02X %02X %02X", Ptr, Size, HeaderOf( Ptr )[3],
			Tail[0], Tail[1], Tail[2], Tail[3], Tail[4], Tail[5], Tail[6], Tail[7] );
		ReportTrail( "free", Trail );
		return FALSE;
	}

	UBOOL VerifyQuarantined( const FQuarantined& Entry, const char* When )
	{
		const BYTE* Data = Entry.Data;
		for( DWORD Offset = 0; Offset < Entry.Size; Offset++ )
		{
			if( Data[Offset] != 0xDD )
			{
				DWORD Last = Offset;
				for( DWORD Scan = Offset; Scan < Entry.Size; Scan++ )
				{
					if( Data[Scan] != 0xDD )
					{
						Last = Scan;
					}
				}
				const DWORD Dump = Min<DWORD>( Entry.Size - Offset, 32 );
				char Hex[32 * 3 + 1];
				for( DWORD Byte = 0; Byte < Dump; Byte++ )
				{
					_snprintf( Hex + Byte * 3, 4, "%02X ", Data[Offset + Byte] );
				}
				Hex[Dump * 3] = 0;
				Report( "WRITE AFTER FREE (%s) header block %p size %u serial %u bytes %u..%u changed: %s", When, Data, Entry.Size, Entry.Serial, Offset, Last, Hex );
				ReportTrail( "free", Entry.FreeTrail );
				return FALSE;
			}
		}
		return TRUE;
	}

	void EvictQuarantine()
	{
		FQuarantined& Entry = Quarantine[QuarantineHead];
		VerifyQuarantined( Entry, "evict" );
		DWORD* Header = HeaderOf( Entry.Data );
		QuarantineBytes -= Entry.Size;
		QuarantineHead = ( QuarantineHead + 1 ) % QuarantineCapacity;
		QuarantineCount--;
		Inner->Free( Entry.Data - Header[2] );
	}

	void HeaderFree( void* Ptr )
	{
		DWORD* Header = HeaderOf( Ptr );
		if( Header[0] == MAGIC_FREE )
		{
			DWORD Trail[TRAIL];
			CaptureTrail( Trail );
			Report( "DOUBLE FREE header block %p size %u serial %u", Ptr, Header[1], Header[3] );
			ReportTrail( "free2", Trail );
			return;
		}
		if( Header[0] != MAGIC_LIVE )
		{
			DWORD Trail[TRAIL];
			CaptureTrail( Trail );
			Report( "FOREIGN FREE %p header %08X %08X %08X %08X", Ptr, Header[0], Header[1], Header[2], Header[3] );
			ReportTrail( "free", Trail );
			Inner->Free( Ptr );
			return;
		}
		const DWORD Size = Header[1];
		CheckTail( Ptr, Size );
		if( !bArmed || !Quarantine || Size > QuarantineMaxBytes / 4 )
		{
			Header[0] = MAGIC_FREE;
			Inner->Free( (BYTE*)Ptr - Header[2] );
			return;
		}
		while( QuarantineCount > 0 && ( QuarantineCount == QuarantineCapacity || QuarantineBytes + Size > QuarantineMaxBytes ) )
		{
			EvictQuarantine();
		}
		Header[0] = MAGIC_FREE;
		appMemset( Ptr, 0xDD, Size );
		FQuarantined& Entry = Quarantine[( QuarantineHead + QuarantineCount ) % QuarantineCapacity];
		Entry.Data = (BYTE*)Ptr;
		Entry.Size = Size;
		Entry.Serial = Header[3];
		CaptureTrail( Entry.FreeTrail );
		QuarantineCount++;
		QuarantineBytes += Size;
	}

	UBOOL SizeOf( void* Ptr, DWORD& Size )
	{
		if( ArenaSize( Small, Ptr, Size ) || ArenaSize( Medium, Ptr, Size ) )
		{
			return TRUE;
		}
		DWORD* Header = HeaderOf( Ptr );
		if( Header[0] == MAGIC_LIVE )
		{
			Size = Header[1];
			return TRUE;
		}
		return FALSE;
	}

	void* LockedMalloc( DWORD Size, DWORD Alignment )
	{
		if( bArmed && Alignment <= PAGE )
		{
			void* Guarded = NULL;
			if( Size <= PAGE )
			{
				Guarded = ArenaMalloc( Small, Size, Alignment );
			}
			else if( Size <= Medium.DataPages * PAGE )
			{
				Guarded = ArenaMalloc( Medium, Size, Alignment );
			}
			if( Guarded )
			{
				return Guarded;
			}
			FallbackAllocs++;
		}
		return HeaderMalloc( Size, Alignment );
	}

	void LockedFree( void* Ptr )
	{
		if( InArena( Small, Ptr ) )
		{
			ArenaFree( Small, Ptr );
		}
		else if( InArena( Medium, Ptr ) )
		{
			ArenaFree( Medium, Ptr );
		}
		else
		{
			HeaderFree( Ptr );
		}
	}

	void DescribeFault( const FArena& Arena, const char* Name, BYTE* Address, EXCEPTION_POINTERS* Info )
	{
		const DWORD Slot = (DWORD)( Address - Arena.Base ) / Arena.SlotBytes;
		const DWORD PageInSlot = (DWORD)( Address - Arena.Base - Slot * Arena.SlotBytes ) / PAGE;
		const FSlotRecord& Record = Arena.Records[Slot];
		BYTE* DataEnd = Arena.Base + Slot * Arena.SlotBytes + Arena.DataPages * PAGE;
		const DWORD Align = AlignOf( Record.Size, DEFAULT_ALIGNMENT );
		BYTE* Data = DataEnd - ( ( Record.Size + Align - 1 ) & ~( Align - 1 ) );
		const char* Kind = PageInSlot >= Arena.DataPages ? "OVERFLOW past end" : ( Record.State == SLOT_FREE ? "USE AFTER FREE" : "fault in live slot" );
		Report( "FAULT %s (%s arena) %s address %p eip %08X thread %u: slot %u size %u state %u serial %u data %p offset %d",
			Kind, Name, Info->ExceptionRecord->ExceptionInformation[0] ? "write" : "read", Address, Info->ContextRecord->Eip, GetCurrentThreadId(),
			Slot, Record.Size, Record.State, Record.Serial, Data, (INT)( Address - Data ) );
		ReportTrail( "alloc", Record.AllocTrail );
		ReportTrail( "free", Record.FreeTrail );
		DWORD Trail[TRAIL];
		CaptureTrailFrom( (const DWORD*)Info->ContextRecord->Esp, Trail );
		ReportTrail( "fault", Trail );
	}

	static LONG CALLBACK VectoredHandler( EXCEPTION_POINTERS* Info )
	{
		if( Instance && Info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && Info->ExceptionRecord->NumberParameters >= 2 )
		{
			BYTE* Address = (BYTE*)Info->ExceptionRecord->ExceptionInformation[1];
			if( InArena( Instance->Small, Address ) )
			{
				Instance->DescribeFault( Instance->Small, "small", Address, Info );
			}
			else if( InArena( Instance->Medium, Address ) )
			{
				Instance->DescribeFault( Instance->Medium, "medium", Address, Info );
			}
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

public:
	FMallocILGuard( FMalloc* InInner )
	:	Inner( InInner )
	,	bArmed( 0 )
	,	Quarantine( NULL )
	,	QuarantineCapacity( 0 )
	,	QuarantineHead( 0 )
	,	QuarantineCount( 0 )
	,	QuarantineBytes( 0 )
	,	QuarantineMaxBytes( 48 * 1024 * 1024 )
	,	Serial( 0 )
	,	Reports( 0 )
	,	GuardedAllocs( 0 )
	,	FallbackAllocs( 0 )
	,	TickCount( 0 )
	{
		InitializeCriticalSection( &Lock );
		Instance = this;
		CreateDirectoryW( L"..\\..\\DishonoredGame\\Logs", NULL );
		ReportFile = CreateFileW( L"..\\..\\DishonoredGame\\Logs\\ILGuard.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		BYTE* Image = (BYTE*)GetModuleHandleW( NULL );
		IMAGE_NT_HEADERS* Nt = (IMAGE_NT_HEADERS*)( Image + ( (IMAGE_DOS_HEADER*)Image )->e_lfanew );
		IMAGE_SECTION_HEADER* Section = IMAGE_FIRST_SECTION( Nt );
		TextStart = (DWORD)Image + Section->VirtualAddress;
		TextEnd = TextStart + Section->Misc.VirtualSize;
		InitArena( Small, 1, 320 * 1024 * 1024 );
		InitArena( Medium, 16, 64 * 1024 * 1024 );
		QuarantineCapacity = 256 * 1024;
		Quarantine = (FQuarantined*)VirtualAlloc( NULL, QuarantineCapacity * sizeof(FQuarantined), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
		AddVectoredExceptionHandler( 1, VectoredHandler );
		Report( "ILGuard: image %p text %08X..%08X (preferred base 00400000), small arena %p %u slots, medium arena %p %u slots, quarantine %u entries %u MB",
			Image, TextStart, TextEnd, Small.Base, Small.NumSlots, Medium.Base, Medium.NumSlots, Quarantine ? QuarantineCapacity : 0, QuarantineMaxBytes >> 20 );
	}

	virtual DWORD QuantizeSize( DWORD Size, DWORD Alignment )
	{
		return Size;
	}

	virtual void* Malloc( DWORD Size, DWORD Alignment )
	{
		EnterCriticalSection( &Lock );
		void* Result = LockedMalloc( Size, Alignment );
		LeaveCriticalSection( &Lock );
		return Result;
	}

	virtual void* Realloc( void* Ptr, DWORD NewSize, DWORD Alignment )
	{
		if( !Ptr )
		{
			return NewSize ? Malloc( NewSize, Alignment ) : NULL;
		}
		EnterCriticalSection( &Lock );
		void* Result = NULL;
		DWORD OldSize = 0;
		if( NewSize == 0 )
		{
			LockedFree( Ptr );
		}
		else if( SizeOf( Ptr, OldSize ) )
		{
			Result = LockedMalloc( NewSize, Alignment );
			if( Result )
			{
				appMemcpy( Result, Ptr, Min( OldSize, NewSize ) );
			}
			LockedFree( Ptr );
		}
		else
		{
			DWORD Trail[TRAIL];
			CaptureTrail( Trail );
			Report( "FOREIGN REALLOC %p to %u", Ptr, NewSize );
			ReportTrail( "realloc", Trail );
			Result = Inner->Realloc( Ptr, NewSize, Alignment );
		}
		LeaveCriticalSection( &Lock );
		return Result;
	}

	virtual void Free( void* Ptr )
	{
		if( !Ptr )
		{
			return;
		}
		EnterCriticalSection( &Lock );
		LockedFree( Ptr );
		LeaveCriticalSection( &Lock );
	}

	virtual void* PhysicalAlloc( DWORD Count, ECacheBehaviour CacheBehaviour )
	{
		return Inner->PhysicalAlloc( Count, CacheBehaviour );
	}

	virtual void PhysicalFree( void* Original )
	{
		Inner->PhysicalFree( Original );
	}

	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		if( ParseCommand( &Cmd, TEXT("ILGUARD") ) )
		{
			if( ParseCommand( &Cmd, TEXT("ARM") ) && InterlockedExchange( &bArmed, 1 ) == 0 )
			{
				Report( "ILGuard: armed at serial %u", Serial );
				Ar.Logf( TEXT("DISHONORED(bringup): ilguard armed (guard pages on every block up to %u KB, write-after-free pattern on the rest)"), Medium.DataPages * 4 );
			}
			return TRUE;
		}
		return Inner->Exec( Cmd, Ar );
	}

	virtual void Tick( FLOAT DeltaTime )
	{
		if( bArmed )
		{
			EnterCriticalSection( &Lock );
			for( DWORD Index = 0; Index < QuarantineCount; Index++ )
			{
				FQuarantined& Entry = Quarantine[( QuarantineHead + Index ) % QuarantineCapacity];
				if( !VerifyQuarantined( Entry, "tick" ) )
				{
					appMemset( Entry.Data, 0xDD, Entry.Size );
				}
			}
			if( ( ++TickCount % 60 ) == 0 )
			{
				Report( "ILGuard: tick %u guarded %u fallback %u small free %u/%u medium free %u/%u quarantine %u blocks %u KB", TickCount, GuardedAllocs, FallbackAllocs,
					Small.QueueCount, Small.NumSlots, Medium.QueueCount, Medium.NumSlots, QuarantineCount, QuarantineBytes >> 10 );
			}
			LeaveCriticalSection( &Lock );
		}
		Inner->Tick( DeltaTime );
	}

	virtual UBOOL IsInternallyThreadSafe() const
	{
		return TRUE;
	}

	virtual void GetAllocationInfo( FMemoryAllocationStats& MemStats )
	{
		Inner->GetAllocationInfo( MemStats );
	}

	virtual void DumpAllocations( class FOutputDevice& Ar )
	{
		Inner->DumpAllocations( Ar );
	}

	virtual UBOOL ValidateHeap()
	{
		return Inner->ValidateHeap();
	}

	virtual UBOOL TrimMemory( SIZE_T ReservePad, UBOOL bShowStats )
	{
		return Inner->TrimMemory( ReservePad, bShowStats );
	}

	virtual UBOOL GetAllocationSize( void* Original, DWORD& SizeOut )
	{
		EnterCriticalSection( &Lock );
		const UBOOL bKnown = SizeOf( Original, SizeOut );
		LeaveCriticalSection( &Lock );
		return bKnown;
	}
};

FMallocILGuard* FMallocILGuard::Instance = NULL;

#endif
