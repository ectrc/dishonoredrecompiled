/*=============================================================================
	NxVersionNumber.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(retail): the retail exe pushes 0x02080400 immediately before both NxCreatePhysicsSDK
	calls (2013 rva 0x3d57aa and 0x3d587b, 2012 rva 0x3f803a; resources/docs/middleware.md 2.2), and
	every shipped PhysX DLL is FileVersion 2.8.4.6.
=============================================================================*/

#ifndef NX_VERSION_NUMBER_H
#define NX_VERSION_NUMBER_H

#define NX_SDK_VERSION_MAJOR	2
#define NX_SDK_VERSION_MINOR	8
#define NX_SDK_VERSION_BUGFIX	4

#define NX_PHYSICS_SDK_VERSION \
	((NX_SDK_VERSION_MAJOR << 24) + (NX_SDK_VERSION_MINOR << 16) + (NX_SDK_VERSION_BUGFIX << 8) + 0)
#define NX_FOUNDATION_SDK_VERSION	NX_PHYSICS_SDK_VERSION

#endif // NX_VERSION_NUMBER_H
