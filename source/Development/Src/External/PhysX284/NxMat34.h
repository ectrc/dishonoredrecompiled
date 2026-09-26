/*=============================================================================
	NxMat34.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): PDB NxMat34, sizeof 48, `NxMat33 M` @0 and `NxVec3 t` @36. The identity form
	is the one NxActorDesc::NxActorDesc() writes in the shipped NxCharacter.dll (rva 0x19d0: nine
	stores of 1/0/0, 0/1/0, 0/0/1 at +0..+32 and three zeroes at +36..+44 before it sets body, density
	and the rest), so id() is M=identity, t=0.
=============================================================================*/

#ifndef NX_MAT34_H
#define NX_MAT34_H

#include "NxMat33.h"

class NxMat34
{
public:
	NxMat33 M;
	NxVec3 t;

	NX_INLINE NxMat34() {}
	NX_INLINE explicit NxMat34(bool identity) { if (identity) { id(); } else { zero(); } }
	NX_INLINE NxMat34(const NxMat33& rot, const NxVec3& trans) : M(rot), t(trans) {}

	NX_INLINE void zero() { M.zero(); t.zero(); }
	NX_INLINE void id() { M.id(); t.zero(); }
	NX_INLINE void setIdentity() { id(); }
	NX_INLINE bool isIdentity() const { return M.isIdentity() && t.isZero(); }
	NX_INLINE bool isFinite() const { return M.isFinite() && t.isFinite(); }

	NX_INLINE void setRowMajor44(const NxF32* d) { M.setRowMajorStride4(d); t.set(d[12], d[13], d[14]); }
	NX_INLINE void getRowMajor44(NxF32* d) const
	{
		M.getRowMajorStride4(d);
		d[3] = d[7] = d[11] = 0.0f;
		d[12] = t.x; d[13] = t.y; d[14] = t.z; d[15] = 1.0f;
	}
	NX_INLINE void setColumnMajor44(const NxF32* d) { M.setColumnMajorStride4(d); t.set(d[12], d[13], d[14]); }
	NX_INLINE void getColumnMajor44(NxF32* d) const
	{
		M.getColumnMajorStride4(d);
		d[3] = d[7] = d[11] = 0.0f;
		d[12] = t.x; d[13] = t.y; d[14] = t.z; d[15] = 1.0f;
	}

	NX_INLINE void multiply(const NxVec3& src, NxVec3& dst) const
	{
		M.multiply(src, dst);
		dst += t;
	}
	NX_INLINE void multiplyByInverseRT(const NxVec3& src, NxVec3& dst) const
	{
		NxVec3 d;
		d.subtract(src, t);
		M.multiplyByTranspose(d, dst);
	}
	NX_INLINE void multiply(const NxMat34& left, const NxMat34& right)
	{
		NxVec3 nt;
		left.multiply(right.t, nt);
		NxMat33 nm;
		nm.multiply(left.M, right.M);
		M = nm;
		t = nt;
	}
	NX_INLINE void multiplyInverseRTLeft(const NxMat34& left, const NxMat34& right)
	{
		NxVec3 nt;
		left.multiplyByInverseRT(right.t, nt);
		NxMat33 lt;
		lt.setTransposed(left.M);
		NxMat33 nm;
		nm.multiply(lt, right.M);
		M = nm;
		t = nt;
	}
	NX_INLINE void setInverseRT(const NxMat34& other)
	{
		M.setTransposed(other.M);
		M.multiply(other.t, t);
		t.setNegative();
	}
	NX_INLINE bool getInverse(NxMat34& out) const
	{
		if (!M.getInverse(out.M)) { return false; }
		out.M.multiply(t, out.t);
		out.t.setNegative();
		return true;
	}
	NX_INLINE bool getInverseRT(NxMat34& out) const { out.setInverseRT(*this); return true; }
	NX_INLINE NxVec3 operator*(const NxVec3& v) const { NxVec3 r; multiply(v, r); return r; }
	// the SDK spells the inverse rigid transform `%`; Engine/Src/UnPhysComponent.cpp:5836 and
	// UnSVehicle.cpp:610 use it to take a world point into a shape's local frame.
	NX_INLINE NxVec3 operator%(const NxVec3& v) const { NxVec3 r; multiplyByInverseRT(v, r); return r; }
	NX_INLINE NxMat34 operator*(const NxMat34& m) const { NxMat34 r; r.multiply(*this, m); return r; }
};

#endif // NX_MAT34_H
