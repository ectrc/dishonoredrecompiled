/*=============================================================================
	bink.h: reconstructed Bink 1.9p (binkw32.dll 1.9.16.0, RAD Game Tools, linked
	2009-09-03) public interface for the Engine/Bink glue.

	DISHONORED(retail): the real RAD header is not in the tree. This file covers the
	20 functions the retail 2013 exe imports from binkw32.dll (resources/docs/symbols/
	imports_2013.csv) plus the structures the reference glue touches. Layouts come
	from the 2012 Shipping PDB (resources/docs/types/types.json: BINK 928 bytes,
	BINKIO 324, BINKSND 384, BINKFRAMEBUFFERS 120, BINKREALTIME 56, BINKRECT 16,
	BINKPLANE 12, BUNDLEPOINTERS 36); flag values are from the public Bink 1.x header
	and cross-checked against the constants the retail exe pushes (see comments).
	Anything not backed by the PDB or the exe is marked UNVERIFIED.
	See resources/docs/middleware.md, section "Bink".
=============================================================================*/

#ifndef DISHONORED_BINK_H
#define DISHONORED_BINK_H

#include <stddef.h>

/*---------------------------------------------------------------------------
	RAD base types and linkage (rad.h subset)
---------------------------------------------------------------------------*/

#ifndef RADLINK
#define RADLINK __stdcall
#endif
#ifndef RADEXPLINK
#define RADEXPLINK __stdcall
#endif
#ifndef PTR4
#define PTR4
#endif

#ifdef __cplusplus
#define RADDEFFUNC extern "C"
#define RADDEFSTART extern "C" {
#define RADDEFEND }
#else
#define RADDEFFUNC
#define RADDEFSTART
#define RADDEFEND
#endif
#define RADEXPFUNC RADDEFFUNC

typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned __int64 U64;
typedef signed __int64 S64;
typedef float F32;
typedef double F64;
#ifdef _WIN64
typedef unsigned __int64 UINTa;
typedef signed __int64 SINTa;
#else
typedef unsigned int UINTa;
typedef signed int SINTa;
#endif

#ifndef RAD_NO_LOWERCASE_TYPES
typedef U8 u8;
typedef S8 s8;
typedef U16 u16;
typedef S16 s16;
typedef U32 u32;
typedef S32 s32;
typedef F32 f32;
typedef F64 f64;
#endif

RADDEFSTART

/*---------------------------------------------------------------------------
	Structures (2012 Shipping PDB layouts)
---------------------------------------------------------------------------*/

#define BINKMAXDIRTYRECTS 8
#define BINKMAXFRAMEBUFFERS 2

typedef struct BINK PTR4* HBINK;

typedef struct BINKRECT
{
	S32 Left;
	S32 Top;
	S32 Width;
	S32 Height;
} BINKRECT;

typedef struct BUNDLEPOINTERS
{
	void PTR4* typeptr;
	void PTR4* type16ptr;
	void PTR4* colorptr;
	void PTR4* bits2ptr;
	void PTR4* motionXptr;
	void PTR4* motionYptr;
	void PTR4* dctptr;
	void PTR4* mdctptr;
	void PTR4* patptr;
} BUNDLEPOINTERS;

typedef struct BINKIO
{
	U32 (RADLINK PTR4* ReadHeader)(struct BINKIO PTR4* Bnkio, S32 Offset, void PTR4* Dest, U32 Size);
	U32 (RADLINK PTR4* ReadFrame)(struct BINKIO PTR4* Bnkio, U32 Framenum, S32 origofs, void PTR4* dest, U32 size);
	U32 (RADLINK PTR4* GetBufferSize)(struct BINKIO PTR4* Bnkio, U32 Size);
	void (RADLINK PTR4* SetInfo)(struct BINKIO PTR4* Bnkio, void PTR4* Buf, U32 Size, U32 FileSize, U32 simulate);
	U32 (RADLINK PTR4* Idle)(struct BINKIO PTR4* Bnkio);
	void (RADLINK PTR4* Close)(struct BINKIO PTR4* Bnkio);
	S32 (RADLINK PTR4* BGControl)(struct BINKIO PTR4* Bnkio, U32 Control);
	HBINK bink;
	volatile U32 ReadError;
	volatile U32 DoingARead;
	volatile U32 BytesRead;
	volatile U32 Working;
	volatile U32 TotalTime;
	volatile U32 ForegroundTime;
	volatile U32 IdleTime;
	volatile U32 ThreadTime;
	volatile U32 BufSize;
	volatile U32 BufHighUsed;
	volatile U32 CurBufSize;
	volatile U32 CurBufUsed;
	volatile U32 Suspended;
	volatile U8 iodata[160];
	void (RADLINK PTR4* suspend_callback)(struct BINKIO PTR4* Bnkio);
	S32 (RADLINK PTR4* try_suspend_callback)(struct BINKIO PTR4* Bnkio);
	void (RADLINK PTR4* resume_callback)(struct BINKIO PTR4* Bnkio);
	void (RADLINK PTR4* idle_on_callback)(struct BINKIO PTR4* Bnkio);
	volatile U32 callback_control[16];
} BINKIO;

typedef struct BINKSND
{
	U8 PTR4* sndwritepos;
	U32 audiodecompsize;
	U32 sndbufsize;
	U8 PTR4* sndbuf;
	U8 PTR4* sndend;
	U32 sndcomp;
	U8 PTR4* sndreadpos;
	U32 orig_freq;
	U32 freq;
	S32 bits;
	S32 chans;
	S32 BestSizeIn16;
	U32 BestSizeMask;
	S32 OnOff;
	U32 Latency;
	U32 VideoScale;
	U32 sndendframe;
	U32 sndpad;
	S32 sndprime;
	S32 NoThreadService;
	U32 SoundDroppedOut;
	U32 sndconvert8;
	U8 snddata[256];
	S32 (RADLINK PTR4* Ready)(struct BINKSND PTR4* BnkSnd);
	S32 (RADLINK PTR4* Lock)(struct BINKSND PTR4* BnkSnd, U8 PTR4* PTR4* addr, U32 PTR4* len);
	S32 (RADLINK PTR4* Unlock)(struct BINKSND PTR4* BnkSnd, U32 filled);
	void (RADLINK PTR4* Volume)(struct BINKSND PTR4* BnkSnd, S32 volume);
	void (RADLINK PTR4* Pan)(struct BINKSND PTR4* BnkSnd, S32 pan);
	S32 (RADLINK PTR4* Pause)(struct BINKSND PTR4* BnkSnd, S32 status);
	S32 (RADLINK PTR4* SetOnOff)(struct BINKSND PTR4* BnkSnd, S32 status);
	void (RADLINK PTR4* Close)(struct BINKSND PTR4* BnkSnd);
	void (RADLINK PTR4* MixBins)(struct BINKSND PTR4* BnkSnd, U32 PTR4* mix_bins, U32 total);
	void (RADLINK PTR4* MixBinVols)(struct BINKSND PTR4* BnkSnd, U32 PTR4* vol_mix_bins, S32 PTR4* volumes, U32 total);
} BINKSND;

typedef struct BINKPLANE
{
	S32 Allocate;
	void PTR4* Buffer;
	U32 BufferPitch;
} BINKPLANE;

typedef struct BINKFRAMEPLANESET
{
	BINKPLANE YPlane;
	BINKPLANE cRPlane;
	BINKPLANE cBPlane;
	BINKPLANE APlane;
} BINKFRAMEPLANESET;

typedef struct BINKFRAMEBUFFERS
{
	S32 TotalFrames;
	U32 YABufferWidth;
	U32 YABufferHeight;
	U32 cRcBBufferWidth;
	U32 cRcBBufferHeight;
	U32 FrameNum;
	BINKFRAMEPLANESET Frames[BINKMAXFRAMEBUFFERS];
} BINKFRAMEBUFFERS;

typedef struct BINK
{
	U32 Width;
	U32 Height;
	U32 Frames;
	U32 FrameNum;
	U32 LastFrameNum;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 ReadError;
	U32 OpenFlags;
	U32 BinkType;
	U32 Size;
	U32 FrameSize;
	U32 SndSize;
	U32 FrameChangePercent;
	BINKRECT FrameRects[BINKMAXDIRTYRECTS];
	S32 NumRects;
	BINKFRAMEBUFFERS PTR4* FrameBuffers;
	void PTR4* MaskPlane;
	U32 MaskPitch;
	U32 MaskLength;
	void PTR4* AsyncMaskPlane;
	void PTR4* InUseMaskPlane;
	void PTR4* LastMaskPlane;
	U32 LargestFrameSize;
	U32 InternalFrames;
	S32 NumTracks;
	U32 Highest1SecRate;
	U32 Highest1SecFrame;
	S32 Paused;
	S32 async_in_progress[2];
	U32 soundon;
	U32 videoon;
	void PTR4* compframe;
	U32 compframesize;
	U32 compframeoffset;
	U32 compframekey;
	U32 skippedlastblit;
	U32 playingtracks;
	BINKSND PTR4* bsnd;
	S32 PTR4* trackindexes;
	BUNDLEPOINTERS bunp;
	U32 changepercent;
	void PTR4* alloccompframe;
	void PTR4* preloadptr;
	U32 PTR4* frameoffsets;
	BINKIO bio;
	U8 PTR4* ioptr;
	U32 iosize;
	U32 decompwidth;
	U32 decompheight;
	U32 PTR4* tracksizes;
	U32 PTR4* tracktypes;
	S32 PTR4* trackIDs;
	U32 numrects;
	U32 playedframes;
	U32 firstframetime;
	U32 startblittime;
	U32 startsynctime;
	U32 startsyncframe;
	U32 twoframestime;
	U32 slowestframetime;
	U32 slowestframe;
	U32 slowest2frametime;
	U32 slowest2frame;
	U32 totalmem;
	U32 timevdecomp;
	U32 timeadecomp;
	U32 timeblit;
	U32 timeopen;
	U32 fileframerate;
	U32 fileframeratediv;
	U32 runtimeframes;
	S32 rtindex;
	U32 PTR4* rtframetimes;
	U32 PTR4* rtadecomptimes;
	U32 PTR4* rtvdecomptimes;
	U32 PTR4* rtblittimes;
	U32 PTR4* rtreadtimes;
	U32 PTR4* rtidlereadtimes;
	U32 PTR4* rtthreadreadtimes;
	U32 lastblitflags;
	U32 lastdecompframe;
	U32 lastfinisheddoframe;
	U32 lastresynctime;
	U32 doresync;
	U32 soundskips;
	U32 skipped_status_this_frame;
	U32 very_delayed;
	U32 skippedblits;
	U32 skipped_in_a_row;
	U32 paused_sync_diff;
	U32 last_time_almost_empty;
	U32 last_read_count;
	U32 last_sound_count;
	U32 snd_callback_buffer[16];
	S32 allkeys;
	BINKFRAMEBUFFERS PTR4* allocatedframebuffers;
} BINK;

typedef struct BINKREALTIME
{
	U32 FrameNum;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 Frames;
	U32 FramesTime;
	U32 FramesVideoDecompTime;
	U32 FramesAudioDecompTime;
	U32 FramesReadTime;
	U32 FramesIdleReadTime;
	U32 FramesThreadReadTime;
	U32 FramesBlitTime;
	U32 ReadBufferSize;
	U32 ReadBufferUsed;
	U32 FramesDataRate;
} BINKREALTIME;

// UNVERIFIED: not in the 2012 PDB (the glue only references BinkGetSummary in a comment);
// field order from the public Bink 1.x header.
typedef struct BINKSUMMARY
{
	U32 Width;
	U32 Height;
	U32 TotalTime;
	U32 FileFrameRate;
	U32 FileFrameRateDiv;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 TotalOpenTime;
	U32 TotalFrames;
	U32 TotalPlayedFrames;
	U32 SkippedFrames;
	U32 SkippedBlits;
	U32 SoundSkips;
	U32 TotalBlitTime;
	U32 TotalReadTime;
	U32 TotalVideoDecompTime;
	U32 TotalAudioDecompTime;
	U32 TotalIdleReadTime;
	U32 TotalBackReadTime;
	U32 TotalReadSpeed;
	U32 SlowestFrameTime;
	U32 Slowest2FrameTime;
	U32 SlowestFrameNum;
	U32 Slowest2FrameNum;
	U32 AverageDataRate;
	U32 AverageFrameSize;
	U32 HighestMemAmount;
	U32 TotalIOMemory;
	U32 HighestIOUsed;
	U32 Highest1SecRate;
	U32 Highest1SecFrame;
} BINKSUMMARY;

/*---------------------------------------------------------------------------
	Flags. Values verified against the retail 2013 exe where noted:
	  BinkOpen(mem, 0x04004400) at rva 0xfc2a1  = BINKFROMMEMORY|BINKSNDTRACK|BINKNOFRAMEBUFFERS
	  BinkOpen(mem, 0x04100400) at rva 0x1beb45 = BINKFROMMEMORY|BINKALPHA|BINKNOFRAMEBUFFERS
	  BinkGetRects(bink, 0) at rva 0x1e71e4     = BINKSURFACEFAST
	  BinkGetKeyFrame(.., 1) / BinkGoto(.., 1) at rva 0xfc5ae = BINKGETKEYNEXT / BINKGOTOQUICK
	The remaining values are the public Bink 1.x header values (UNVERIFIED in the exe).
---------------------------------------------------------------------------*/

#define BINKMARKDATA          0x00000004L
#define BINKALPHA             0x00100000L
#define BINKGRAYSCALE         0x00020000L
#define BINKNOFRAMEBUFFERS    0x00000400L
#define BINKNOSKIP            0x00080000L
#define BINKPRELOADALL        0x00002000L
#define BINKSNDTRACK          0x00004000L
#define BINKOLDFRAMEFORMAT    0x00008000L
#define BINKIOSIZE            0x01000000L
#define BINKIOPROCESSOR       0x02000000L
#define BINKFROMMEMORY        0x04000000L
#define BINKNOTHREADEDIO      0x08000000L
#define BINKNOFILLIOBUF       0x00200000L
#define BINKSIMULATE          0x00400000L
#define BINKFILEHANDLE        0x00800000L
#define BINKCOPYALL           0x80000000L

#define BINKSURFACEFAST       0x00000000L
#define BINKSURFACESLOW       0x08000000L
#define BINKSURFACEDIRECT     0x04000000L
#define BINKCOPYNOSCALING     0x70000000L
#define BINKCOPY2XH           0x10000000L
#define BINKCOPY2XHI          0x20000000L
#define BINKCOPY2XW           0x30000000L
#define BINKCOPY2XWH          0x40000000L
#define BINKCOPY2XWHI         0x50000000L
#define BINKCOPY1XI           0x60000000L

#define BINKGOTOQUICK         1
#define BINKGOTOQUICKSOUND    2

#define BINKGETKEYPREVIOUS    0
#define BINKGETKEYNEXT        1
#define BINKGETKEYCLOSEST     2

#define BINKBGIOSUSPEND       1
#define BINKBGIORESUME        2
#define BINKBGIOWAIT          4

#define BINKSNDTRACKMAX       16

/*---------------------------------------------------------------------------
	Sound system plug-in prototypes (BinkSetSoundSystem / BinkOpenDirectSound)
---------------------------------------------------------------------------*/

typedef S32 (RADLINK PTR4* BINKSNDOPEN)(struct BINKSND PTR4* BnkSnd, U32 freq, S32 bits, S32 chans, U32 flags, HBINK bink);
typedef BINKSNDOPEN (RADLINK PTR4* BINKSNDSYSOPEN)(UINTa param);

/*---------------------------------------------------------------------------
	The 20 entry points the retail 2013 exe imports (imports_2013.csv), stdcall
	stack sizes as decorated in binkw32.dll's export table.
---------------------------------------------------------------------------*/

RADEXPFUNC HBINK RADEXPLINK BinkOpen(char const PTR4* name, U32 flags);                    // _BinkOpen@8
RADEXPFUNC void RADEXPLINK BinkClose(HBINK bnk);                                            // _BinkClose@4
RADEXPFUNC S32 RADEXPLINK BinkDoFrame(HBINK bnk);                                           // _BinkDoFrame@4
RADEXPFUNC void RADEXPLINK BinkNextFrame(HBINK bnk);                                        // _BinkNextFrame@4
RADEXPFUNC S32 RADEXPLINK BinkWait(HBINK bnk);                                              // _BinkWait@4
RADEXPFUNC S32 RADEXPLINK BinkShouldSkip(HBINK bnk);                                        // _BinkShouldSkip@4
RADEXPFUNC S32 RADEXPLINK BinkPause(HBINK bnk, S32 pause);                                  // _BinkPause@8
RADEXPFUNC void RADEXPLINK BinkGoto(HBINK bnk, U32 frame, S32 flags);                       // _BinkGoto@12
RADEXPFUNC U32 RADEXPLINK BinkGetKeyFrame(HBINK bnk, U32 frame, S32 flags);                 // _BinkGetKeyFrame@12
RADEXPFUNC S32 RADEXPLINK BinkGetRects(HBINK bnk, U32 flags);                               // _BinkGetRects@8
RADEXPFUNC void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME PTR4* run, U32 frames); // _BinkGetRealtime@12
RADEXPFUNC void RADEXPLINK BinkGetFrameBuffersInfo(HBINK bink, BINKFRAMEBUFFERS PTR4* fbset); // _BinkGetFrameBuffersInfo@8
RADEXPFUNC void RADEXPLINK BinkRegisterFrameBuffers(HBINK bink, BINKFRAMEBUFFERS PTR4* fbset); // _BinkRegisterFrameBuffers@8
RADEXPFUNC char PTR4* RADEXPLINK BinkGetError(void);                                        // _BinkGetError@0
RADEXPFUNC void RADEXPLINK BinkSetIOSize(U32 iosize);                                       // _BinkSetIOSize@4
RADEXPFUNC S32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, UINTa param);             // _BinkSetSoundSystem@8
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(UINTa param);                         // _BinkOpenDirectSound@4
RADEXPFUNC S32 RADEXPLINK BinkSetSoundTrack(U32 total_tracks, U32 PTR4* tracks);            // _BinkSetSoundTrack@8
RADEXPFUNC void RADEXPLINK BinkSetVolume(HBINK bnk, U32 trackid, S32 volume);               // _BinkSetVolume@12
RADEXPFUNC void RADEXPLINK BinkSetPan(HBINK bnk, U32 trackid, S32 pan);                     // _BinkSetPan@12

RADDEFEND

#ifdef __cplusplus
static_assert(sizeof(BINKRECT) == 16, "BINKRECT: 2012 PDB size 16");
static_assert(sizeof(BINKPLANE) == 12, "BINKPLANE: 2012 PDB size 12");
static_assert(sizeof(BINKFRAMEPLANESET) == 48, "BINKFRAMEPLANESET: 2012 PDB size 48");
static_assert(sizeof(BINKFRAMEBUFFERS) == 120, "BINKFRAMEBUFFERS: 2012 PDB size 120");
static_assert(sizeof(BINKIO) == 324, "BINKIO: 2012 PDB size 324");
static_assert(sizeof(BINKSND) == 384, "BINKSND: 2012 PDB size 384");
static_assert(sizeof(BINKREALTIME) == 56, "BINKREALTIME: 2012 PDB size 56");
static_assert(sizeof(BINK) == 928, "BINK: 2012 PDB size 928");
static_assert(offsetof(BINK, bio) == 340, "BINK::bio: 2012 PDB offset 340");
static_assert(offsetof(BINK, allocatedframebuffers) == 924, "BINK::allocatedframebuffers: 2012 PDB offset 924");
#endif

#endif // DISHONORED_BINK_H
