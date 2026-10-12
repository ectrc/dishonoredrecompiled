// AkAudioNative.h - hand-written native types of the AkAudio module that the generated class bodies (Inc/CppText/*.h)
// refer to. The generator (resources/tools/symbols/gen_classes_header.py --sdk) includes <Module>Native.h from <Module>.h
// and from the first generated class header, so this file is what carries agent AN's hand edit of AkAudioClasses.h
// (the Wwise 2012.1 headers and the two forward declarations) across a regeneration (agent FX, wave 22).
#ifndef _INC_AKAUDIONATIVE
#define _INC_AKAUDIONATIVE

#include "AkAudioWwise.h"

// UAkBank / UAkEvent are Engine classes (Engine/Inc/EngineArkaneClasses.h); the CppText bodies name them before the
// generated class headers that use them by pointer.
class UAkBank;
class UAkEvent;

#endif // _INC_AKAUDIONATIVE
