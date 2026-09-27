#pragma once
// GFxUI/inc/gfxuifile.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x5bff20  public: virtual bool __thiscall FGFxFile::IsValid(void)
//   0x5bff30  public: virtual __int64 __thiscall FGFxFile::LTell(void)
//   0x5bff40  public: virtual __int64 __thiscall FGFxFile::LGetLength(void)

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the GFx file seam. GFxFileOpener (4 slots) is what the runtime asks for a
// GFile (19 slots) whenever it needs bytes - an imported movie, a fontlib, an external image.
// Layouts from the 2012 PDB; 2013 rvas from match_2012_2013.csv:
//   FGFxFileOpener::OpenFile           0x58d780  (2012 0x5dda60)
//   FGFxFileOpener::GetFileModifyTime  0x58d9a0  (2012 0x5ddc80)
//   FGFxFile::FGFxFile                 0x587340  (2012 0x5caeb0)
//   FGFxFile::Read                     0x572210  (2012 0x5b6d20)
//   FGFxFile::Seek                     0x5722a0  (2012 0x5b6db0)
//
// FGFxFile is a memory file: PDB sizeof 88 with "BYTE* Buffer" @8, "INT Length" @12,
// "INT Position" @16, "char Filename[64]" @20 and "INT ErrorCode" @84 - so the retail opener does
// not stream from disk at all, it hands the runtime a pointer into a package's bytes. That is
// exactly the shape SwfMovie::RawData has (agentBB.md), and it is why the whole 19-slot GFile
// implementation is real code rather than a stub: reading from a buffer needs no engine.
// gfxuirenderer.h rather than GFx3.h directly, for the seam census the bringup bodies record into.
#include "gfxuirenderer.h"

class FGFxFile : public GFile
{
public:
    unsigned char* Buffer;        // @8
    int            Length;        // @12
    int            Position;      // @16
    char           Filename[64];  // @20
    int            ErrorCode;     // @84

    FGFxFile(const char* InFilename, unsigned char* InBuffer, int InLength);   // 2013 0x587340

    virtual ~FGFxFile();                                               // vt[0]
    virtual const char* GetFilePath();                                 // vt[1]
    virtual bool IsValid();                                            // vt[2]
    virtual bool IsWritable();                                         // vt[3]
    virtual int  Tell();                                               // vt[4]
    virtual __int64 LTell();                                           // vt[5]
    virtual int  GetLength();                                          // vt[6]
    virtual __int64 LGetLength();                                      // vt[7]
    virtual int  GetErrorCode();                                       // vt[8]
    virtual int  Write(const unsigned char* Data, int Count);          // vt[9]
    virtual int  Read(unsigned char* Data, int Count);                 // vt[10] 2013 0x572210
    virtual int  SkipBytes(int Count);                                 // vt[11]
    virtual int  BytesAvailable();                                     // vt[12]
    virtual bool Flush();                                              // vt[13]
    virtual int  Seek(int Offset, int Origin);                          // vt[14] 2013 0x5722a0
    virtual __int64 LSeek(__int64 Offset, int Origin);                  // vt[15]
    virtual bool ChangeSize(int NewSize);                              // vt[16]
    virtual int  CopyFromStream(GFile* Other, int ByteSize);            // vt[17]
    virtual bool Close();                                              // vt[18]
};

class FGFxFileOpener : public GFxFileOpener
{
public:
    virtual ~FGFxFileOpener();                                                    // vt[0]
    virtual GFile* OpenFile(const char* Url, int Flags, int Mode);                 // vt[1] 2013 0x58d780
    virtual __int64 GetFileModifyTime(const char* Url);                            // vt[2] 2013 0x58d9a0
    virtual GFile* OpenFileEx(const char* Url, GFxLog* Log, int Flags, int Mode);   // vt[3]
};
