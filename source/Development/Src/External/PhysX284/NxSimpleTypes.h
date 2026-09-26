/*=============================================================================
	NxSimpleTypes.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): the widths the shipped PhysXCore.pdb / PhysXCooking.pdb report for every Nx
	member (read with resources/tools/pdb/dia_types.py; the DLLs in Dishonored_Debug2012 are
	byte-identical to the retail 2013 ones, resources/docs/binaries.md). Nothing here comes from an
	NVIDIA SDK.
=============================================================================*/

#ifndef NX_SIMPLE_TYPES_H
#define NX_SIMPLE_TYPES_H

typedef signed char			NxI8;
typedef signed short		NxI16;
typedef signed int			NxI32;
typedef __int64				NxI64;
typedef unsigned char		NxU8;
typedef unsigned short		NxU16;
typedef unsigned int		NxU32;
typedef unsigned __int64	NxU64;
typedef float				NxF32;
typedef double				NxF64;
typedef NxF32				NxReal;

// DISHONORED(layout): the NxU16 handle typedefs; every one of them is an `unsigned short` member in
// the PDB (NxActorDescBase::group @60, ::dominanceGroup @62, ::forceFieldMaterial @68,
// NxShapeDesc::group @60, ::materialIndex @62).
typedef NxU16				NxActorGroup;
typedef NxU16				NxCollisionGroup;
typedef NxU16				NxMaterialIndex;
typedef NxU16				NxDominanceGroup;
typedef NxU16				NxForceFieldMaterial;
typedef NxU16				NxForceFieldVariety;
typedef NxU16				NxCompartmentIndex;
typedef NxU32				NxTriangleID;
typedef NxU32				NxSubmeshIndex;

#define NX_MAX_I8			127
#define NX_MIN_I8			(-128)
#define NX_MAX_U8			255
#define NX_MAX_I16			32767
#define NX_MIN_I16			(-32768)
#define NX_MAX_U16			65535
#define NX_MAX_I32			2147483647
#define NX_MIN_I32			(-2147483647 - 1)
#define NX_MAX_U32			0xffffffffu
#define NX_MAX_F32			3.4028234663852885981170418348452e+38f
#define NX_MIN_F32			(-NX_MAX_F32)
#define NX_MAX_REAL			NX_MAX_F32
#define NX_MIN_REAL			NX_MIN_F32
#define NX_EPS_F32			1.192092896e-07f
#define NX_EPS_REAL			NX_EPS_F32
#define NX_PI				3.141592653589793f
#define NX_HALF_PI			1.57079632679489661923f

#endif // NX_SIMPLE_TYPES_H
