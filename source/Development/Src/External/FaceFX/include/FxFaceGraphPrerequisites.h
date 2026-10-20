// The one include every FX2 face-graph header starts from: FX1's core. (FX2's stand-in for the core, include/stub
// behind FX2_FACEFX_CORE_STUB, was deleted by FX3.)
#ifndef FxFaceGraphPrerequisites_H__
#define FxFaceGraphPrerequisites_H__

#include "FxPlatform.h"
#include "FxSDK.h"
#include "FxUtil.h"
#include "FxMemory.h"
#include "FxString.h"
#include "FxName.h"
#include "FxArray.h"
#include "FxStringHash.h"
#include "FxObject.h"
#include "FxNamedObject.h"
#include "FxArchive.h"
#include "FxArchiveStoreMemory.h"
#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxArchiveStoreNull.h"

#ifndef FX_REAL_MAX
#define FX_REAL_MAX 3.4028235e38f
#endif
#ifndef FX_REAL_MIN
#define FX_REAL_MIN 1.1754944e-38f
#endif
#ifndef FX_INT32_MIN
#define FX_INT32_MIN (-2147483647 - 1)
#endif
#ifndef FX_INT32_MAX
#define FX_INT32_MAX 2147483647
#endif

#endif
