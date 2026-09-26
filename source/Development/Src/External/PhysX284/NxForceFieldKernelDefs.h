/*=============================================================================
	NxForceFieldKernelDefs.h: the PhysX 2.8.4 force-field kernel DSL, reconstructed for Dishonored.

	DISHONORED(written): Engine/Inc/ForceFunction{Radial,Tornado,TornadoAngular,Sample}.h are written
	in the SDK's little kernel language - NX_START_FORCEFIELD(Name) / NxFConst / NxBConst /
	NX_START_FUNCTION / NxFailIf / NxFinishIf / NxSelect / NxFloat / NxBool / NxVector /
	NX_END_FUNCTION / NX_END_FORCEFIELD - and the SDK expands it into a class deriving from
	NxForceFieldKernel with one setter per constant (NxForceFieldRadial.cpp calls
	`Kernel->setRadialStrength(...)` on a `new NxForceFieldKernelRadial`).

	The base class's vtable comes from PhysXCore.pdb (NxGenerated.h): ~NxForceFieldKernel vt[0],
	parse() const vt[1], evaluate(NxVec3& force, NxVec3& torque, const NxVec3& pos, const NxVec3& vel)
	const vt[2], getType() const vt[3], clone() const vt[4], update() const vt[5], setEpsilon vt[6],
	`void* userData` @4; NX_FFK_CUSTOM_KERNEL is 1 in the PDB's NxForcFieldKernelType.

	The expansion below evaluates the kernel body directly in C++ floats, which is what the SDK's own
	host expansion does; the alternative PPU/bytecode expansion of the real header is irrelevant here
	because the shipped DLLs are the SDK and we only have to hand them a kernel object whose vtable
	matches. NxFloat/NxBool/NxVector are the DSL's expression types, kept as thin value wrappers so
	the bodies compile unchanged (they use `&`/`|` on conditions and `.recip()` on scalars).
=============================================================================*/

#ifndef NX_FORCE_FIELD_KERNEL_DEFS_H
#define NX_FORCE_FIELD_KERNEL_DEFS_H

#include "NxPhysics.h"

class NxBool
{
public:
	bool b;

	NX_INLINE NxBool() : b(false) {}
	NX_INLINE NxBool(bool v) : b(v) {}
	NX_INLINE operator bool() const { return b; }
	NX_INLINE NxBool operator&(const NxBool& o) const { return NxBool(b && o.b); }
	NX_INLINE NxBool operator|(const NxBool& o) const { return NxBool(b || o.b); }
	NX_INLINE NxBool operator!() const { return NxBool(!b); }
};

class NxFloat
{
public:
	NxReal f;

	NX_INLINE NxFloat() : f(0.0f) {}
	NX_INLINE NxFloat(NxReal v) : f(v) {}
	NX_INLINE NxFloat(int v) : f((NxReal)v) {}
	NX_INLINE NxFloat(double v) : f((NxReal)v) {}
	// deliberately no implicit conversion back to NxReal: with the converting constructors above it
	// makes every `1 - x` in the kernel bodies ambiguous between the built-in and the operators below.
	NX_INLINE NxReal get() const { return f; }

	NX_INLINE NxFloat recip() const { return NxFloat(f != 0.0f ? 1.0f / f : 0.0f); }
	NX_INLINE NxFloat sqrtf() const { return NxFloat(NxMath::sqrt(f)); }
	NX_INLINE NxFloat abs() const { return NxFloat(NxMath::abs(f)); }
	NX_INLINE NxFloat operator-() const { return NxFloat(-f); }
	NX_INLINE NxFloat operator+(const NxFloat& o) const { return NxFloat(f + o.f); }
	NX_INLINE NxFloat operator-(const NxFloat& o) const { return NxFloat(f - o.f); }
	NX_INLINE NxFloat operator*(const NxFloat& o) const { return NxFloat(f * o.f); }
	NX_INLINE NxFloat operator/(const NxFloat& o) const { return NxFloat(o.f != 0.0f ? f / o.f : 0.0f); }
	NX_INLINE NxFloat& operator+=(const NxFloat& o) { f += o.f; return *this; }
	NX_INLINE NxFloat& operator-=(const NxFloat& o) { f -= o.f; return *this; }
	NX_INLINE NxFloat& operator*=(const NxFloat& o) { f *= o.f; return *this; }
	NX_INLINE NxBool operator<(const NxFloat& o) const { return NxBool(f < o.f); }
	NX_INLINE NxBool operator>(const NxFloat& o) const { return NxBool(f > o.f); }
	NX_INLINE NxBool operator<=(const NxFloat& o) const { return NxBool(f <= o.f); }
	NX_INLINE NxBool operator>=(const NxFloat& o) const { return NxBool(f >= o.f); }
	NX_INLINE NxBool operator==(const NxFloat& o) const { return NxBool(f == o.f); }
	NX_INLINE NxBool operator!=(const NxFloat& o) const { return NxBool(f != o.f); }
};

class NxVector
{
public:
	NxVec3 v;

	NX_INLINE NxVector() { v.zero(); }
	NX_INLINE NxVector(const NxVec3& src) : v(src) {}
	NX_INLINE NxVector(const NxFloat& x, const NxFloat& y, const NxFloat& z) : v(x.f, y.f, z.f) {}

	NX_INLINE void zero() { v.zero(); }
	NX_INLINE NxFloat getX() const { return NxFloat(v.x); }
	NX_INLINE NxFloat getY() const { return NxFloat(v.y); }
	NX_INLINE NxFloat getZ() const { return NxFloat(v.z); }
	NX_INLINE void setX(const NxFloat& s) { v.x = s.f; }
	NX_INLINE void setY(const NxFloat& s) { v.y = s.f; }
	NX_INLINE void setZ(const NxFloat& s) { v.z = s.f; }
	NX_INLINE NxFloat dot(const NxVector& o) const { return NxFloat(v.dot(o.v)); }
	NX_INLINE NxFloat magnitude() const { return NxFloat(v.magnitude()); }
	NX_INLINE NxFloat magnitudeSquared() const { return NxFloat(v.magnitudeSquared()); }
	NX_INLINE NxVector operator+(const NxVector& o) const { return NxVector(v + o.v); }
	NX_INLINE NxVector operator-(const NxVector& o) const { return NxVector(v - o.v); }
	NX_INLINE NxVector operator*(const NxFloat& s) const { return NxVector(v * s.f); }
	NX_INLINE NxVector cross(const NxVector& o) const { return NxVector(v.cross(o.v)); }
};

NX_INLINE NxFloat operator+(NxReal a, const NxFloat& b) { return NxFloat(a + b.f); }
NX_INLINE NxFloat operator-(NxReal a, const NxFloat& b) { return NxFloat(a - b.f); }
NX_INLINE NxFloat operator*(NxReal a, const NxFloat& b) { return NxFloat(a * b.f); }
NX_INLINE NxFloat operator/(NxReal a, const NxFloat& b) { return NxFloat(b.f != 0.0f ? a / b.f : 0.0f); }
NX_INLINE NxFloat NxSelect(const NxBool& cond, const NxFloat& whenTrue, const NxFloat& whenFalse)
{
	return cond.b ? whenTrue : whenFalse;
}
NX_INLINE NxVector NxSelect(const NxBool& cond, const NxVector& whenTrue, const NxVector& whenFalse)
{
	return cond.b ? whenTrue : whenFalse;
}

#define NxFConst(name)								\
	NxFloat name;									\
	NX_INLINE void set##name(NxReal value_) { name = NxFloat(value_); }	\
	NX_INLINE NxReal get##name() const { return name.f; }

#define NxBConst(name)								\
	NxBool name;									\
	NX_INLINE void set##name(bool value_) { name = NxBool(value_); }	\
	NX_INLINE bool get##name() const { return name.b; }

#define NxVConst(name)								\
	NxVector name;									\
	NX_INLINE void set##name(const NxVec3& value_) { name = NxVector(value_); }	\
	NX_INLINE NxVec3 get##name() const { return name.v; }

#define NX_START_FORCEFIELD(fieldName)				\
	class NxForceFieldKernel##fieldName : public NxForceFieldKernel	\
	{												\
	public:											\
		NX_INLINE NxForceFieldKernel##fieldName() { userData = 0; }

#define NX_START_FUNCTION																			\
		virtual bool evaluate(NxVec3& outForce, NxVec3& outTorque,									\
			const NxVec3& inPosition, const NxVec3& inVelocity) const								\
		{																							\
			const NxVector Position(inPosition);													\
			const NxVector Velocity(inVelocity);													\
			NxVector force, torque;																	\
			NX_UNUSED(Position); NX_UNUSED(Velocity);												\
			{

#define NX_END_FUNCTION																				\
			}																						\
			outForce = force.v;																		\
			outTorque = torque.v;																	\
			return true;																			\
		}

// the kernel body bails out of the whole evaluation (no force at all) ...
#define NxFailIf(condition)																			\
		if ((bool)(condition)) { outForce.zero(); outTorque.zero(); return false; }
// ... or keeps whatever it has written so far and stops.
#define NxFinishIf(condition)																		\
		if ((bool)(condition)) { outForce = force.v; outTorque = torque.v; return true; }

#define NX_END_FORCEFIELD(fieldName)																\
		virtual void parse() const {}																\
		virtual NxU32 getType() const { return NX_FFK_CUSTOM_KERNEL; }								\
		virtual NxForceFieldKernel* clone() const { return new NxForceFieldKernel##fieldName(*this); }	\
		virtual void update(NxForceFieldKernel& other) const										\
		{																							\
			NxForceFieldKernel##fieldName* typed = static_cast<NxForceFieldKernel##fieldName*>(&other);	\
			*typed = *this;																			\
		}																							\
		virtual void setEpsilon(NxF32 epsilon) { NX_UNUSED(epsilon); }								\
	};

#endif // NX_FORCE_FIELD_KERNEL_DEFS_H
