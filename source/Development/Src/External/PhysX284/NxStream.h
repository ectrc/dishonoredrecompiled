/*=============================================================================
	NxStream.h: the PhysX 2.8.4 stream callback, reconstructed for Dishonored.

	DISHONORED(layout): NxStream itself is declared in NxUserAllocator.h next to the other callbacks
	the engine implements (FNxMemoryBuffer, Engine/Src/UnNovodexSupport.h), with the vtable order the
	PDB gives: ~NxStream at vt[0], readByte..readBuffer (const) at vt[1..6], storeByte..storeBuffer at
	vt[7..12].
=============================================================================*/

#ifndef NX_STREAM_H
#define NX_STREAM_H

#include "NxUserAllocator.h"

#endif // NX_STREAM_H
