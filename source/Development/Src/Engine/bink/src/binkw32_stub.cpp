// DISHONORED(retail): source of the Bink import library (cmake/Bink.cmake, DISHONORED_WITH_BINK).
//
// The retail binkw32.dll (Bink 1.9p, 1.9.16.0, linked 2009-09-03, 72 exports) exports the
// *decorated* stdcall names, e.g. "_BinkOpen@8", and the retail 2013 exe imports them by that
// name (resources/docs/symbols/imports_2013.csv). lib.exe /def cannot express that on x86: a .def
// entry "BinkOpen@8" imports "BinkOpen@8" and "_BinkOpen@8" produces the symbol "__BinkOpen@8"
// (verified, see resources/docs/agents/agentU.md). RAD's own import library comes from
// __declspec(dllexport) on the stdcall functions, which is what this stub reproduces: it is
// linked into build/<dir>/bink/stub/binkw32.dll (never copied next to the exe) only for its
// import library, which makes our exe import "binkw32.dll!_BinkOpen@8" exactly like retail.
//
// The 20 entry points below are the ones the retail exe imports; the DLL's full export table
// (dumpbin /exports) with ordinals, for reference:
//   1 BinkBufferBlit@12            2 BinkBufferCheckWinPos@12     3 BinkBufferClear@8
//   4 BinkBufferClose@4            5 BinkBufferGetDescription@4   6 BinkBufferGetError@0
//   7 BinkBufferLock@4             8 BinkBufferOpen@16            9 BinkBufferSetDirectDraw@8
//  10 BinkBufferSetHWND@8         11 BinkBufferSetOffset@12      12 BinkBufferSetResolution@12
//  13 BinkBufferSetScale@12       14 BinkBufferUnlock@4          15 BinkCheckCursor@20
//  16 BinkClose@4                 17 BinkCloseTrack@4            18 BinkControlBackgroundIO@8
//  19 BinkControlPlatformFeatures@8 20 BinkCopyToBuffer@28       21 BinkCopyToBufferRect@44
//  22 BinkDDSurfaceType@4         23 BinkDX8SurfaceType@4        24 BinkDX9SurfaceType@4
//  25 BinkDoFrame@4               26 BinkDoFrameAsync@12         27 BinkDoFrameAsyncWait@8
//  28 BinkDoFramePlane@8          29 BinkGetError@0              30 BinkGetFrameBuffersInfo@8
//  31 BinkGetKeyFrame@12          32 BinkGetPalette@4            33 BinkGetRealtime@12
//  34 BinkGetRects@8              35 BinkGetSummary@8            36 BinkGetTrackData@8
//  37 BinkGetTrackID@8            38 BinkGetTrackMaxSize@8       39 BinkGetTrackType@8
//  40 BinkGoto@12                 41 BinkIsSoftwareCursor@8      42 BinkLogoAddress@0
//  43 BinkNextFrame@4             44 BinkOpen@8                  45 BinkOpenDirectSound@4
//  46 BinkOpenMiles@4             47 BinkOpenTrack@8             48 BinkOpenWaveOut@4
//  49 BinkPause@8                 50 BinkRegisterFrameBuffers@8  51 BinkRequestStopAsyncThread@4
//  52 BinkRestoreCursor@4         53 BinkService@4               54 BinkSetError@4
//  55 BinkSetFrameRate@8          56 BinkSetIO@4                 57 BinkSetIOSize@4
//  58 BinkSetMemory@8             59 BinkSetMixBinVolumes@20     60 BinkSetMixBins@16
//  61 BinkSetPan@12               62 BinkSetSimulate@4           63 BinkSetSoundOnOff@8
//  64 BinkSetSoundSystem@8        65 BinkSetSoundTrack@8         66 BinkSetVideoOnOff@8
//  67 BinkSetVolume@12            68 BinkShouldSkip@4            69 BinkStartAsyncThread@8
//  70 BinkWait@4                  71 BinkWaitStopAsyncThread@4   72 RADTimerRead@0

#define BINK_STUB extern "C" __declspec(dllexport)

BINK_STUB void* __stdcall BinkOpen(const char*, unsigned int) { return 0; }
BINK_STUB void __stdcall BinkClose(void*) {}
BINK_STUB int __stdcall BinkDoFrame(void*) { return 0; }
BINK_STUB void __stdcall BinkNextFrame(void*) {}
BINK_STUB int __stdcall BinkWait(void*) { return 0; }
BINK_STUB int __stdcall BinkShouldSkip(void*) { return 0; }
BINK_STUB int __stdcall BinkPause(void*, int) { return 0; }
BINK_STUB void __stdcall BinkGoto(void*, unsigned int, int) {}
BINK_STUB unsigned int __stdcall BinkGetKeyFrame(void*, unsigned int, int) { return 0; }
BINK_STUB int __stdcall BinkGetRects(void*, unsigned int) { return 0; }
BINK_STUB void __stdcall BinkGetRealtime(void*, void*, unsigned int) {}
BINK_STUB void __stdcall BinkGetFrameBuffersInfo(void*, void*) {}
BINK_STUB void __stdcall BinkRegisterFrameBuffers(void*, void*) {}
BINK_STUB char* __stdcall BinkGetError(void) { return 0; }
BINK_STUB void __stdcall BinkSetIOSize(unsigned int) {}
BINK_STUB int __stdcall BinkSetSoundSystem(void*, unsigned int) { return 0; }
BINK_STUB void* __stdcall BinkOpenDirectSound(unsigned int) { return 0; }
BINK_STUB int __stdcall BinkSetSoundTrack(unsigned int, unsigned int*) { return 0; }
BINK_STUB void __stdcall BinkSetVolume(void*, unsigned int, int) {}
BINK_STUB void __stdcall BinkSetPan(void*, unsigned int, int) {}
