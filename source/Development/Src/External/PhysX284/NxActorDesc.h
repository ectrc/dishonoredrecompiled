/*=============================================================================
	NxActorDesc.h: NxActorDescBase / NxActorDesc, reconstructed for Dishonored.

	DISHONORED(layout|retail): PDB NxActorDescBase, sizeof 92: globalPose @0, body @48, density @52,
	flags @56, group @60, dominanceGroup @62, contactReportFlags @64, forceFieldMaterial @68,
	userData @72, name @76, compartment @80, type @84, `NxShapeDesc*** shapesStart` @88; PDB
	NxActorDesc, sizeof 104, adds `NxArray<NxShapeDesc*> shapes` @92.

	What shapesStart is, from the shipped NxCharacter.dll's own NxActorDesc::NxActorDesc()
	(rva 0x19d0, disassembled): memset(this, 0, 0x68); globalPose.id(); body=0; density=0; flags=0;
	group=0; dominanceGroup=0; contactReportFlags=0; forceFieldMaterial=0; userData=0; name=0;
	compartment=0; then `lea edi,[esi+5Ch]` (the NxArray at +92), `mov [edi+4],[edi]` (last = first,
	an empty array), `mov [esi+58h],edi` -> shapesStart is the address of the array's `first`
	pointer, and `mov [esi+54h],1` -> type = NX_ADT_DEFAULT (1 in the PDB's NxActorDescType).
	So the SDK reads shapesStart[0] as the first shape pointer and shapesStart[1] as one past the
	last, which is exactly the NxArray {first,last,memEnd} layout (NxArray.h).
=============================================================================*/

#ifndef NX_ACTOR_DESC_H
#define NX_ACTOR_DESC_H

#include "NxGenerated.h"

class NxActorDescBase
{
public:
	NxMat34 globalPose;					// @0
	const NxBodyDesc* body;				// @48
	NxReal density;						// @52
	NxU32 flags;						// @56
	NxActorGroup group;					// @60
	NxDominanceGroup dominanceGroup;	// @62
	NxU32 contactReportFlags;			// @64
	NxForceFieldMaterial forceFieldMaterial;	// @68
	void* userData;						// @72
	const char* name;					// @76
	NxCompartment* compartment;			// @80
	NxActorDescType type;				// @84
	NxShapeDesc*** shapesStart;			// @88

	NX_INLINE NxU32 getNbShapes() const
	{
		return shapesStart ? (NxU32)(shapesStart[1] - shapesStart[0]) : 0;
	}
	NX_INLINE NxShapeDesc* const* getShapes() const
	{
		return shapesStart ? (NxShapeDesc* const*)shapesStart[0] : 0;
	}
	NX_INLINE bool isValid() const { return density >= 0.0f; }

protected:
	NX_INLINE NxActorDescBase() : shapesStart(0) { setToDefaultBase(); }
	NX_INLINE void setToDefaultBase()
	{
		globalPose.id();
		body = 0;
		density = 0.0f;
		flags = 0;
		group = 0;
		dominanceGroup = 0;
		contactReportFlags = 0;
		forceFieldMaterial = 0;
		userData = 0;
		name = 0;
		compartment = 0;
		type = NX_ADT_DEFAULT;
	}
};

class NxActorDesc : public NxActorDescBase
{
public:
	NxArray<NxShapeDesc*> shapes;		// @92

	NX_INLINE NxActorDesc() { setToDefault(); }
	NX_INLINE NxActorDesc(const NxActorDesc& other) : NxActorDescBase(), shapes(other.shapes)
	{
		copyFrom(other);
	}
	NX_INLINE NxActorDesc& operator=(const NxActorDesc& other)
	{
		if (this != &other)
		{
			shapes = other.shapes;
			copyFrom(other);
		}
		return *this;
	}
	NX_INLINE void setToDefault()
	{
		setToDefaultBase();
		shapes.clear();
		shapesStart = (NxShapeDesc***)&shapes;
	}
	NX_INLINE bool isValid() const { return NxActorDescBase::isValid(); }

private:
	NX_INLINE void copyFrom(const NxActorDesc& other)
	{
		globalPose = other.globalPose;
		body = other.body;
		density = other.density;
		flags = other.flags;
		group = other.group;
		dominanceGroup = other.dominanceGroup;
		contactReportFlags = other.contactReportFlags;
		forceFieldMaterial = other.forceFieldMaterial;
		userData = other.userData;
		name = other.name;
		compartment = other.compartment;
		type = other.type;
		// never copied: it has to point at this object's own array
		shapesStart = (NxShapeDesc***)&shapes;
	}
};

#endif // NX_ACTOR_DESC_H
