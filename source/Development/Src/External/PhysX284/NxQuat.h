/*=============================================================================
	NxQuat.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): PDB NxQuat, sizeof 16, x @0 y @4 z @8 w @12 (the imaginary part first, the
	real part last). Engine's U2NQuaternion builds it as NxQuat(NxVec3(X,Y,Z), W)
	(Engine/Src/UnNovodexSupport.cpp), which is the (vector, scalar) constructor below.
=============================================================================*/

#ifndef NX_QUAT_H
#define NX_QUAT_H

#include "NxVec3.h"

class NxQuat
{
public:
	NxReal x, y, z, w;

	NX_INLINE NxQuat() {}
	NX_INLINE NxQuat(const NxVec3& v, NxReal nw) : x(v.x), y(v.y), z(v.z), w(nw) {}
	NX_INLINE explicit NxQuat(NxReal nw) : x(0.0f), y(0.0f), z(0.0f), w(nw) {}
	NX_INLINE NxQuat(const NxQuat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}

	NX_INLINE void id() { x = y = z = 0.0f; w = 1.0f; }
	NX_INLINE void setIdentity() { id(); }
	NX_INLINE void zero() { x = y = z = 0.0f; w = 0.0f; }
	NX_INLINE void setWXYZ(NxReal nw, NxReal nx, NxReal ny, NxReal nz) { w = nw; x = nx; y = ny; z = nz; }
	NX_INLINE void setXYZW(NxReal nx, NxReal ny, NxReal nz, NxReal nw) { x = nx; y = ny; z = nz; w = nw; }
	NX_INLINE void setXYZW(const NxReal* v) { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }
	NX_INLINE void getWXYZ(NxF32* v) const { v[0] = w; v[1] = x; v[2] = y; v[3] = z; }
	NX_INLINE void getXYZW(NxF32* v) const { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	NX_INLINE void setx(NxReal v) { x = v; }
	NX_INLINE void sety(NxReal v) { y = v; }
	NX_INLINE void setz(NxReal v) { z = v; }
	NX_INLINE void setw(NxReal v) { w = v; }
	NX_INLINE NxReal getx() const { return x; }
	NX_INLINE NxReal gety() const { return y; }
	NX_INLINE NxReal getz() const { return z; }
	NX_INLINE NxReal getw() const { return w; }

	NX_INLINE void fromAngleAxis(NxReal angleDeg, const NxVec3& axis)
	{
		NxVec3 a(axis); a.normalize();
		const NxReal half = 0.5f * NxMath::degToRad(angleDeg);
		const NxReal s = NxMath::sin(half);
		w = NxMath::cos(half); x = a.x * s; y = a.y * s; z = a.z * s;
	}
	NX_INLINE void fromAngleAxisFast(NxReal angleRad, const NxVec3& axis)
	{
		const NxReal half = 0.5f * angleRad;
		const NxReal s = NxMath::sin(half);
		w = NxMath::cos(half); x = axis.x * s; y = axis.y * s; z = axis.z * s;
	}
	NX_INLINE NxReal getAngle() const { return 2.0f * NxMath::acos(w); }
	NX_INLINE NxReal getAngle(const NxQuat& q) const { return 2.0f * NxMath::acos(dot(q)); }
	NX_INLINE void getAxis(NxVec3& axis) const
	{
		axis.set(x, y, z);
		const NxReal m = axis.magnitude();
		if (m > NX_EPS_REAL) { axis *= 1.0f / m; } else { axis.set(1.0f, 0.0f, 0.0f); }
	}
	NX_INLINE void getAngleAxis(NxReal& angleDeg, NxVec3& axis) const
	{
		getAxis(axis);
		angleDeg = NxMath::radToDeg(getAngle());
	}
	NX_INLINE NxReal dot(const NxQuat& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
	NX_INLINE NxReal magnitude() const { return NxMath::sqrt(x * x + y * y + z * z + w * w); }
	NX_INLINE NxReal magnitudeSquared() const { return x * x + y * y + z * z + w * w; }
	NX_INLINE NxReal normalize()
	{
		const NxReal m = magnitude();
		if (m > NX_EPS_REAL) { const NxReal r = 1.0f / m; x *= r; y *= r; z *= r; w *= r; }
		return m;
	}
	NX_INLINE bool isFinite() const
	{
		return NxMath::isFinite(x) && NxMath::isFinite(y) && NxMath::isFinite(z) && NxMath::isFinite(w);
	}
	NX_INLINE void conjugate() { x = -x; y = -y; z = -z; }
	NX_INLINE void invert() { conjugate(); }
	NX_INLINE void negate() { x = -x; y = -y; z = -z; w = -w; }
	NX_INLINE void multiply(const NxQuat& a, const NxQuat& b)
	{
		const NxReal nw = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
		const NxReal nx = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
		const NxReal ny = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
		const NxReal nz = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
		w = nw; x = nx; y = ny; z = nz;
	}
	NX_INLINE void rotate(NxVec3& v) const
	{
		const NxVec3 qv(x, y, z);
		const NxVec3 t = qv.cross(v) * 2.0f;
		v = v + t * w + qv.cross(t);
	}
	NX_INLINE void rotate(const NxVec3& in, NxVec3& out) const { out = in; rotate(out); }
	NX_INLINE void inverseRotate(NxVec3& v) const { NxQuat q(*this); q.conjugate(); q.rotate(v); }
	NX_INLINE void inverseRotate(const NxVec3& in, NxVec3& out) const { out = in; inverseRotate(out); }
	NX_INLINE void slerp(const NxReal t, const NxQuat& left, const NxQuat& right);

	NX_INLINE bool operator==(const NxQuat& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
	NX_INLINE NxQuat& operator=(const NxQuat& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
	NX_INLINE NxQuat& operator*=(const NxQuat& q) { multiply(NxQuat(*this), q); return *this; }
	NX_INLINE NxQuat& operator+=(const NxQuat& q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }
	NX_INLINE NxQuat& operator-=(const NxQuat& q) { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }
	NX_INLINE NxQuat& operator*=(NxReal s) { x *= s; y *= s; z *= s; w *= s; return *this; }
	NX_INLINE NxQuat operator-() const { return NxQuat(NxVec3(x, y, z), -w); }
	NX_INLINE NxVec3 operator*(const NxVec3& v) const { NxVec3 r; rotate(v, r); return r; }
	NX_INLINE NxQuat operator*(const NxQuat& q) const { NxQuat r; r.multiply(*this, q); return r; }
};

NX_INLINE void NxQuat::slerp(const NxReal t, const NxQuat& left, const NxQuat& right)
{
	const NxReal eps = 0.00001f;
	NxReal cosine = left.dot(right);
	NxReal sgn = 1.0f;
	if (cosine < 0.0f) { cosine = -cosine; sgn = -1.0f; }
	NxReal s0, s1;
	if (cosine > 1.0f - eps)
	{
		s0 = 1.0f - t;
		s1 = t;
	}
	else
	{
		const NxReal theta = NxMath::acos(cosine);
		const NxReal invSin = 1.0f / NxMath::sin(theta);
		s0 = NxMath::sin((1.0f - t) * theta) * invSin;
		s1 = NxMath::sin(t * theta) * invSin;
	}
	s1 *= sgn;
	x = s0 * left.x + s1 * right.x;
	y = s0 * left.y + s1 * right.y;
	z = s0 * left.z + s1 * right.z;
	w = s0 * left.w + s1 * right.w;
}

#endif // NX_QUAT_H
