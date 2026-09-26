/*=============================================================================
	NxBounds3.h, NxPlane, NxRay, NxSegment, NxSphere, NxBox, NxCapsule, NxTriangle: the PhysX 2.8.4
	simple geometry value types, reconstructed for Dishonored.

	DISHONORED(layout): PDB sizes and member offsets - NxBounds3 24 (min @0, max @12), NxPlane 16
	(normal @0, d @12), NxRay 24 (orig @0, dir @12), NxSegment 24 (p0 @0, p1 @12), NxSphere 16
	(center @0, radius @12), NxBox 60 (center @0, extents @12, rot @24), NxCapsule 28 (NxSegment base
	+ radius @24), NxTriangle 36 (NxVec3 mVerts[3]), NxTriangle32 12 (NxU32 v[3]),
	NxGroupsMask 16 (bits0..bits3).
=============================================================================*/

#ifndef NX_BOUNDS3_H
#define NX_BOUNDS3_H

#include "NxMat34.h"

class NxBounds3
{
public:
	NxVec3 min;
	NxVec3 max;

	NX_INLINE NxBounds3() {}

	NX_INLINE void set(const NxVec3& lo, const NxVec3& hi) { min = lo; max = hi; }
	NX_INLINE void set(NxReal minx, NxReal miny, NxReal minz, NxReal maxx, NxReal maxy, NxReal maxz)
	{
		min.set(minx, miny, minz);
		max.set(maxx, maxy, maxz);
	}
	NX_INLINE void setEmpty() { min.setPlusInfinity(); max.setMinusInfinity(); }
	NX_INLINE void setInfinite() { min.setMinusInfinity(); max.setPlusInfinity(); }
	NX_INLINE bool isEmpty() const { return min.x > max.x; }
	NX_INLINE void include(const NxVec3& v) { min.min(v); max.max(v); }
	NX_INLINE void include(const NxBounds3& b) { min.min(b.min); max.max(b.max); }
	NX_INLINE void combine(const NxBounds3& b) { include(b); }
	NX_INLINE bool intersects(const NxBounds3& b) const
	{
		if (b.min.x > max.x || min.x > b.max.x) return false;
		if (b.min.y > max.y || min.y > b.max.y) return false;
		if (b.min.z > max.z || min.z > b.max.z) return false;
		return true;
	}
	NX_INLINE bool intersects2D(const NxBounds3& b, NxU32 upAxis) const
	{
		const NxU32 a0 = (upAxis + 1) % 3, a1 = (upAxis + 2) % 3;
		if (b.min[a0] > max[a0] || min[a0] > b.max[a0]) return false;
		if (b.min[a1] > max[a1] || min[a1] > b.max[a1]) return false;
		return true;
	}
	NX_INLINE bool contain(const NxVec3& v) const
	{
		return v.x >= min.x && v.x <= max.x && v.y >= min.y && v.y <= max.y && v.z >= min.z && v.z <= max.z;
	}
	NX_INLINE void getCenter(NxVec3& c) const { c.add(min, max); c *= 0.5f; }
	NX_INLINE NxVec3 getCenter() const { NxVec3 c; getCenter(c); return c; }
	NX_INLINE void getExtents(NxVec3& e) const { e.subtract(max, min); e *= 0.5f; }
	NX_INLINE NxVec3 getExtents() const { NxVec3 e; getExtents(e); return e; }
	NX_INLINE void getDimensions(NxVec3& d) const { d.subtract(max, min); }
	NX_INLINE void setCenterExtents(const NxVec3& c, const NxVec3& e) { min.subtract(c, e); max.add(c, e); }
	NX_INLINE void scale(NxReal s)
	{
		NxVec3 c = getCenter(), e = getExtents();
		e *= s;
		setCenterExtents(c, e);
	}
	NX_INLINE void fatten(NxReal d) { min.x -= d; min.y -= d; min.z -= d; max.x += d; max.y += d; max.z += d; }
	NX_INLINE void boundsOfPoints(const NxVec3& p0, const NxVec3& p1) { min = p0; max = p0; include(p1); }
	NX_INLINE void transform(const NxMat34& m)
	{
		const NxVec3 c = getCenter();
		const NxVec3 e = getExtents();
		NxVec3 nc;
		m.multiply(c, nc);
		NxVec3 ne;
		for (int r = 0; r < 3; ++r)
		{
			ne[r] = NxMath::abs(m.M(r, 0)) * e.x + NxMath::abs(m.M(r, 1)) * e.y + NxMath::abs(m.M(r, 2)) * e.z;
		}
		setCenterExtents(nc, ne);
	}
};

class NxPlane
{
public:
	NxVec3 normal;
	NxReal d;

	NX_INLINE NxPlane() {}
	NX_INLINE NxPlane(NxReal nx, NxReal ny, NxReal nz, NxReal distance) : normal(nx, ny, nz), d(distance) {}
	NX_INLINE NxPlane(const NxVec3& n, NxReal distance) : normal(n), d(distance) {}
	NX_INLINE NxPlane(const NxVec3& p, const NxVec3& n) : normal(n), d(-p.dot(n)) {}

	NX_INLINE NxPlane& set(const NxVec3& p, const NxVec3& n) { normal = n; d = -p.dot(n); return *this; }
	NX_INLINE NxPlane& zero() { normal.zero(); d = 0.0f; return *this; }
	NX_INLINE NxReal distance(const NxVec3& p) const { return p.dot(normal) + d; }
	NX_INLINE bool belongs(const NxVec3& p) const { return NxMath::abs(distance(p)) < 1.0e-7f; }
	NX_INLINE NxVec3 pointInPlane() const { return normal * -d; }
	NX_INLINE void project(const NxVec3& p, NxVec3& out) const { out = p - normal * distance(p); }
	NX_INLINE NxPlane& normalize() { const NxReal m = normal.magnitude(); if (m != 0.0f) { const NxReal r = 1.0f / m; normal *= r; d *= r; } return *this; }
	NX_INLINE NxPlane& inverseTransform(const NxMat34& m)
	{
		NxVec3 p = pointInPlane();
		NxVec3 np;
		m.multiplyByInverseRT(p, np);
		NxVec3 nn;
		m.M.multiplyByTranspose(normal, nn);
		return set(np, nn);
	}
};

class NxRay
{
public:
	NxVec3 orig;
	NxVec3 dir;

	NX_INLINE NxRay() {}
	NX_INLINE NxRay(const NxVec3& o, const NxVec3& d) : orig(o), dir(d) {}
};

class NxSegment
{
public:
	NxVec3 p0;
	NxVec3 p1;

	NX_INLINE NxSegment() {}
	NX_INLINE NxSegment(const NxVec3& a, const NxVec3& b) : p0(a), p1(b) {}

	NX_INLINE const NxVec3& getOrigin() const { return p0; }
	NX_INLINE NxVec3 computeDirection() const { return p1 - p0; }
	NX_INLINE void computeDirection(NxVec3& out) const { out.subtract(p1, p0); }
	NX_INLINE NxReal computeLength() const { return (p1 - p0).magnitude(); }
	NX_INLINE NxReal computeSquareLength() const { return (p1 - p0).magnitudeSquared(); }
	NX_INLINE void computePoint(NxVec3& out, NxReal t) const { out = p0 + (p1 - p0) * t; }
};

class NxSphere
{
public:
	NxVec3 center;
	NxReal radius;

	NX_INLINE NxSphere() {}
	NX_INLINE NxSphere(const NxVec3& c, NxReal r) : center(c), radius(r) {}

	NX_INLINE bool contains(const NxVec3& p) const { return center.distanceSquared(p) <= radius * radius; }
};

class NxBox
{
public:
	NxVec3 center;
	NxVec3 extents;
	NxMat33 rot;

	NX_INLINE NxBox() {}
	NX_INLINE NxBox(const NxVec3& c, const NxVec3& e, const NxMat33& r) : center(c), extents(e), rot(r) {}

	NX_INLINE void setEmpty() { center.zero(); extents.set(-NX_MAX_REAL, -NX_MAX_REAL, -NX_MAX_REAL); rot.id(); }
	NX_INLINE bool isValid() const { return !(extents.x < 0.0f || extents.y < 0.0f || extents.z < 0.0f); }
	NX_INLINE void rotate(const NxMat34& m, NxBox& out) const
	{
		m.multiply(center, out.center);
		out.extents = extents;
		out.rot.multiply(m.M, rot);
	}
};

class NxCapsule : public NxSegment
{
public:
	NxReal radius;

	NX_INLINE NxCapsule() {}
	NX_INLINE NxCapsule(const NxSegment& s, NxReal r) : NxSegment(s), radius(r) {}
};

class NxTriangle
{
public:
	NxVec3 verts[3];

	NX_INLINE NxTriangle() {}
	NX_INLINE NxTriangle(const NxVec3& a, const NxVec3& b, const NxVec3& c) { verts[0] = a; verts[1] = b; verts[2] = c; }

	NX_INLINE void normal(NxVec3& out) const
	{
		out.cross(verts[1] - verts[0], verts[2] - verts[0]);
		out.normalize();
	}
	NX_INLINE void denormalizedNormal(NxVec3& out) const { out.cross(verts[1] - verts[0], verts[2] - verts[0]); }
	NX_INLINE NxReal area() const
	{
		NxVec3 n;
		denormalizedNormal(n);
		return n.magnitude() * 0.5f;
	}
};

class NxTriangle32
{
public:
	NxU32 v[3];

	NX_INLINE NxTriangle32() {}
	NX_INLINE NxTriangle32(NxU32 a, NxU32 b, NxU32 c) { v[0] = a; v[1] = b; v[2] = c; }
};

class NxGroupsMask
{
public:
	NxU32 bits0, bits1, bits2, bits3;

	NX_INLINE NxGroupsMask() : bits0(0), bits1(0), bits2(0), bits3(0) {}
	NX_INLINE void setToDefault() { bits0 = bits1 = bits2 = bits3 = 0; }
};

#endif // NX_BOUNDS3_H
