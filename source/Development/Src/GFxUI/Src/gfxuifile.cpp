// GFxUI/src/gfxuifile.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x5b6cd0  public: virtual int __thiscall FGFxFile::Write(unsigned char const *, int)
//   0x5b6d20  public: virtual int __thiscall FGFxFile::Read(unsigned char *, int)
//   0x5b6d70  public: virtual int __thiscall FGFxFile::SkipBytes(int)
//   0x5b6da0  public: virtual int __thiscall FGFxFile::BytesAvailable(void)
//   0x5b6db0  public: virtual int __thiscall FGFxFile::Seek(int, int)
//   0x5b6e20  public: virtual __int64 __thiscall FGFxFile::LSeek(__int64, int)
//   0x5b6e40  public: virtual bool __thiscall FGFxFile::ChangeSize(int)
//   0x5caeb0  public: __thiscall FGFxFile::FGFxFile(char const * const, unsigned char * const, int)

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the file seam. Declarations and 2013 rvas in gfxuifile.h. FGFxFile is a memory
// file over a buffer the caller owns, which is what the PDB layout says retail does, so all 19
// GFile slots are real code here rather than bringup stubs.
#include "gfxuifile.h"

#include <string.h>

FGFxFile::FGFxFile(const char* InFilename, unsigned char* InBuffer, int InLength)
    : Buffer(InBuffer), Length(InLength), Position(0), ErrorCode(0)
{
    // 2013 0x587340. The PDB's 64-byte inline name buffer, truncated the same way.
    Filename[0] = 0;
    if (InFilename)
    {
        strncpy(Filename, InFilename, sizeof(Filename) - 1);
        Filename[sizeof(Filename) - 1] = 0;
    }
    if (Buffer == 0 || Length < 0)
    {
        Length = 0;
        ErrorCode = GFileConstants::Error_FileNotFound;
    }
}

FGFxFile::~FGFxFile() {}

const char* FGFxFile::GetFilePath() { return Filename; }
bool FGFxFile::IsValid() { return Buffer != 0 && ErrorCode == 0; }   // 2012 0x5bff20
bool FGFxFile::IsWritable() { return false; }
int FGFxFile::Tell() { return Position; }
__int64 FGFxFile::LTell() { return (__int64)Position; }              // 2012 0x5bff30
int FGFxFile::GetLength() { return Length; }
__int64 FGFxFile::LGetLength() { return (__int64)Length; }           // 2012 0x5bff40
int FGFxFile::GetErrorCode() { return ErrorCode; }

int FGFxFile::Write(const unsigned char* Data, int Count)
{
    (void)Data; (void)Count;
    ErrorCode = GFileConstants::Error_Access;
    return -1;
}

int FGFxFile::Read(unsigned char* Data, int Count)
{
    // 2013 0x572210.
    if (Data == 0 || Count <= 0 || Buffer == 0) return 0;
    int avail = Length - Position;
    if (avail <= 0) return 0;
    int n = Count < avail ? Count : avail;
    memcpy(Data, Buffer + Position, (size_t)n);
    Position += n;
    return n;
}

int FGFxFile::SkipBytes(int Count)
{
    int avail = Length - Position;
    int n = Count < avail ? Count : avail;
    if (n < 0) n = 0;
    Position += n;
    return n;
}

int FGFxFile::BytesAvailable() { return Length - Position; }
bool FGFxFile::Flush() { return true; }

int FGFxFile::Seek(int Offset, int Origin)
{
    // 2013 0x5722a0.
    int target = Offset;
    if (Origin == GFileConstants::Seek_Cur) target = Position + Offset;
    else if (Origin == GFileConstants::Seek_End) target = Length + Offset;
    if (target < 0 || target > Length) return -1;
    Position = target;
    return Position;
}

__int64 FGFxFile::LSeek(__int64 Offset, int Origin)
{
    return (__int64)Seek((int)Offset, Origin);
}

bool FGFxFile::ChangeSize(int NewSize)
{
    (void)NewSize;
    ErrorCode = GFileConstants::Error_Access;
    return false;
}

int FGFxFile::CopyFromStream(GFile* Other, int ByteSize)
{
    (void)Other; (void)ByteSize;
    ErrorCode = GFileConstants::Error_Access;
    return -1;
}

bool FGFxFile::Close()
{
    Buffer = 0;
    Length = 0;
    Position = 0;
    return true;
}

// ---------------------------------------------------------------------------------------------
// FGFxFileOpener. In retail the opener resolves the url against the package the movie came from and
// hands back an FGFxFile over that package's bytes; the resolution needs UObject and belongs with
// package BE's natives, so this is DISHONORED(bringup): it records the request and returns nothing,
// which makes the runtime report a missing import rather than crash.
FGFxFileOpener::~FGFxFileOpener() {}

GFile* FGFxFileOpener::OpenFile(const char* Url, int Flags, int Mode)
{
    // 2013 0x58d780.
    (void)Url; (void)Flags; (void)Mode;
    GFXUI_SEAM_TRACE("FGFxFileOpener::OpenFile");
    return 0;
}

__int64 FGFxFileOpener::GetFileModifyTime(const char* Url)
{
    // 2013 0x58d9a0. Cooked content never changes under us, so retail's answer of 0 stands.
    (void)Url;
    GFXUI_SEAM_TRACE("FGFxFileOpener::GetFileModifyTime");
    return 0;
}

GFile* FGFxFileOpener::OpenFileEx(const char* Url, GFxLog* Log, int Flags, int Mode)
{
    (void)Log;
    return OpenFile(Url, Flags, Mode);
}
