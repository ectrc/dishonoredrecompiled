/*=============================================================================
	NxVec3.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): PDB NxVec3, sizeof 12, x @0 y @4 z @8, all NxF32; PDB NxExtendedVec3,
	sizeof 24, x/y/z NxF64. The bodies are ordinary vector algebra and never cross the DLL boundary
	(the type is passed by value/reference and only its 12 bytes matter).
=============================================================================*/

#ifndef NX_VEC3_H
#define NX_VEC3_H

#include "NxMath.h"

class NxVec3
{
public:
	NxReal x, y, z;

	NX_INLINE NxVec3() {}
	NX_INLINE explicit NxVec3(NxReal a) : x(a), y(a), z(a) {}
	NX_INLINE NxVec3(NxReal nx, NxReal ny, NxReal nz) : x(nx), y(ny), z(nz) {}
	NX_INLINE NxVec3(const NxReal v[]) : x(v[0]), y(v[1]), z(v[2]) {}
	NX_INLINE NxVec3(const NxVec3& v) : x(v.x), y(v.y), z(v.z) {}

	NX_INLINE const NxReal* get() const { return &x; }
	NX_INLINE NxReal* get() { return &x; }
	NX_INLINE void get(NxF32* v) const { v[0] = (NxF32)x; v[1] = (NxF32)y; v[2] = (NxF32)z; }
	NX_INLINE void get(NxF64* v) const { v[0] = (NxF64)x; v[1] = (NxF64)y; v[2] = (NxF64)z; }
	NX_INLINE NxReal& operator[](int i) { return (&x)[i]; }
	NX_INLINE NxReal operator[](int i) const { return (&x)[i]; }

	NX_INLINE void setx(NxReal v) { x = v; }
	NX_INLINE void sety(NxReal v) { y = v; }
	NX_INLINE void setz(NxReal v) { z = v; }
	NX_INLINE void set(NxReal nx, NxReal ny, NxReal nz) { x = nx; y = ny; z = nz; }
	NX_INLINE void set(const NxVec3& v) { x = v.x; y = v.y; z = v.z; }
	NX_INLINE void set(const NxReal* v) { x = v[0]; y = v[1]; z = v[2]; }
	NX_INLINE void set(NxReal v) { x = y = z = v; }
	NX_INLINE void zero() { x = y = z = 0.0f; }
	NX_INLINE void setPlusInfinity() { x = y = z = NX_MAX_REAL; }
	NX_INLINE void setMinusInfinity() { x = y = z = NX_MIN_REAL; }
	NX_INLINE void setNegative() { x = -x; y = -y; z = -z; }
	NX_INLINE void setNegative(const NxVec3& v) { x = -v.x; y = -v.y; z = -v.z; }
	NX_INLINE void min(const NxVec3& v) { if (v.x < x) x = v.x; if (v.y < y) y = v.y; if (v.z < z) z = v.z; }
	NX_INLINE void max(const NxVec3& v) { if (v.x > x) x = v.x; if (v.y > y) y = v.y; if (v.z > z) z = v.z; }

	NX_INLINE void add(const NxVec3& a, const NxVec3& b) { x = a.x + b.x; y = a.y + b.y; z = a.z + b.z; }
	NX_INLINE void subtract(const NxVec3& a, const NxVec3& b) { x = a.x - b.x; y = a.y - b.y; z = a.z - b.z; }
	NX_INLINE void arrayMultiply(const NxVec3& a, const NxVec3& b) { x = a.x * b.x; y = a.y * b.y; z = a.z * b.z; }
	NX_INLINE void multiply(NxReal s, const NxVec3& a) { x = s * a.x; y = s * a.y; z = s * a.z; }
	NX_INLINE void multiplyAdd(NxReal s, const NxVec3& a, const NxVec3& b) { x = s * a.x + b.x; y = s * a.y + b.y; z = s * a.z + b.z; }

	NX_INLINE NxReal dot(const NxVec3& v) const { return x * v.x + y * v.y + z * v.z; }
	NX_INLINE bool sameDirection(const NxVec3& v) const { return dot(v) >= 0.0f; }
	NX_INLINE NxReal magnitude() const { return NxMath::sqrt(x * x + y * y + z * z); }
	NX_INLINE NxReal magnitudeSquared() const { return x * x + y * y + z * z; }
	NX_INLINE NxReal distance(const NxVec3& v) const { NxVec3 d; d.subtract(*this, v); return d.magnitude(); }
	NX_INLINE NxReal distanceSquared(const NxVec3& v) const { NxVec3 d; d.subtract(*this, v); return d.magnitudeSquared(); }
	NX_INLINE void cross(const NxVec3& a, const NxVec3& b)
	{
		const NxReal cx = a.y * b.z - a.z * b.y;
		const NxReal cy = a.z * b.x - a.x * b.z;
		const NxReal cz = a.x * b.y - a.y * b.x;
		x = cx; y = cy; z = cz;
	}
	NX_INLINE NxVec3 cross(const NxVec3& v) const { NxVec3 r; r.cross(*this, v); return r; }
	NX_INLINE bool equals(const NxVec3& v, NxReal eps) const
	{
		return NxMath::equals(x, v.x, eps) && NxMath::equals(y, v.y, eps) && NxMath::equals(z, v.z, eps);
	}
	NX_INLINE bool isZero() const { return x == 0.0f && y == 0.0f && z == 0.0f; }
	NX_INLINE bool isFinite() const { return NxMath::isFinite(x) && NxMath::isFinite(y) && NxMath::isFinite(z); }
	NX_INLINE NxReal normalize()
	{
		const NxReal m = magnitude();
		if (m != 0.0f) { const NxReal r = 1.0f / m; x *= r; y *= r; z *= r; }
		return m;
	}
	NX_INLINE bool isNormalized() const { return NxMath::equals(magnitude(), 1.0f, 0.0001f); }
	NX_INLINE NxReal normalizeSafe()
	{
		const NxReal m = magnitude();
		if (m < NX_EPS_REAL) { x = 1.0f; y = 0.0f; z = 0.0f; return 0.0f; }
		const NxReal r = 1.0f / m; x *= r; y *= r; z *= r;
		return m;
	}
	NX_INLINE void setMagnitude(NxReal l)
	{
		const NxReal m = magnitude();
		if (m != 0.0f) { const NxReal r = l / m; x *= r; y *= r; z *= r; }
	}
	NX_INLINE NxU32 closestAxis() const
	{
		const NxReal ax = NxMath::abs(x), ay = NxMath::abs(y), az = NxMath::abs(z);
		if (ay > ax) return az > ay ? 2u : 1u;
		return az > ax ? 2u : 0u;
	}

	NX_INLINE bool operator==(const NxVec3& v) const { return x == v.x && y == v.y && z == v.z; }
	NX_INLINE bool operator!=(const NxVec3& v) const { return x != v.x || y != v.y || z != v.z; }
	NX_INLINE const NxVec3& operator=(const NxVec3& v) { x = v.x; y = v.y; z = v.z; return *this; }
	NX_INLINE NxVec3& operator+=(const NxVec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	NX_INLINE NxVec3& operator-=(const NxVec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	NX_INLINE NxVec3& operator*=(NxReal s) { x *= s; y *= s; z *= s; return *this; }
	NX_INLINE NxVec3& operator/=(NxReal s) { const NxReal r = 1.0f / s; x *= r; y *= r; z *= r; return *this; }
	NX_INLINE NxVec3 operator-() const { return NxVec3(-x, -y, -z); }
	NX_INLINE NxVec3 operator+(const NxVec3& v) const { return NxVec3(x + v.x, y + v.y, z + v.z); }
	NX_INLINE NxVec3 operator-(const NxVec3& v) const { return NxVec3(x - v.x, y - v.y, z - v.z); }
	NX_INLINE NxVec3 operator*(NxReal s) const { return NxVec3(x * s, y * s, z * s); }
	NX_INLINE NxVec3 operator/(NxReal s) const { const NxReal r = 1.0f / s; return NxVec3(x * r, y * r, z * r); }
	NX_INLINE NxVec3 operator^(const NxVec3& v) const { return cross(v); }
	NX_INLINE NxReal operator|(const NxVec3& v) const { return dot(v); }
};

NX_INLINE NxVec3 operator*(NxReal s, const NxVec3& v) { return NxVec3(v.x * s, v.y * s, v.z * s); }

class NxExtendedVec3
{
public:
	NxF64 x, y, z;

	NX_INLINE NxExtendedVec3() {}
	NX_INLINE NxExtendedVec3(NxF64 nx, NxF64 ny, NxF64 nz) : x(nx), y(ny), z(nz) {}
	NX_INLINE void set(NxF64 nx, NxF64 ny, NxF64 nz) { x = nx; y = ny; z = nz; }
	NX_INLINE void zero() { x = y = z = 0.0; }
	NX_INLINE NxF64& operator[](int i) { return (&x)[i]; }
	NX_INLINE NxF64 operator[](int i) const { return (&x)[i]; }
	NX_INLINE bool isZero() const { return x == 0.0 && y == 0.0 && z == 0.0; }
};

typedef NxExtendedVec3 NxExtended;

#endif // NX_VEC3_H
