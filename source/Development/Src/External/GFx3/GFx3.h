// Scaleform GFx 3.3.89 - the umbrella header of the reconstructed API.
//
// Include this and nothing else. What is in the directory:
//   GTypes.h      hand-written kernel: scalars, GColor/GPoint/GRect/GMatrix2D/GMatrix3D/GViewport,
//                 the GRefCount* chain, GPtr, GList, GArray, GString, GLock, the stat tag classes
//   GFx3Enums.h   GENERATED file-scope enums
//   GFx3Gen.h     GENERATED classes: every member at its PDB offset, every virtual at its PDB slot
//   GFxValue.h    hand-written GFxValue + GFxValue::ObjectInterface + DisplayInfo (81 % of the
//                 engine's traffic into libgfx goes through these)
//   GFxGfxFile.h  the container parser: the cooked assets are gfxexport GFX, see agentBB.md
//   GFx3Layout.cpp  the layout assertions against the PDB - the library's translation unit
//
// Evidence, once: GFx cannot be imported. It is 5,635 functions and 1.13 MiB inside the retail
// exe's own .text with no symbol in any shipped DLL, and 5,235 of them (92.9 %) are byte-identical
// between the 2012 QA build (which has a PDB) and the retail 2013 build. So the 2012 PDB describes
// the retail runtime exactly and every layout here is read out of it with
// resources/tools/pdb/dia_types.py. resources/docs/gfx_decision.md, resources/docs/agents/agentBB.md.
#ifndef INC_GFX3_H
#define INC_GFX3_H

#include "GTypes.h"
#include "GFx3Enums.h"
#include "GFx3Gen.h"
#include "GFxValue.h"
#include "GFxGfxFile.h"

#endif // INC_GFX3_H
