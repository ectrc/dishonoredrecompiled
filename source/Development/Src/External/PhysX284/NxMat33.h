/*=============================================================================
	NxMat33.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(layout): PDB NxMat33, sizeof 36, one member `Nx9Real data` @0. PDB Nx9Real is a union
	of `Nx9Real::S s` and `float m[3][3]`, and S's fields are _11 @0, _12 @4, _13 @8, _21 @12,
	_22 @16, _23 @20, _31 @24, _32 @28, _33 @32 - so the 36 bytes are ROW major: element (r,c) is
	data.m[r][c]. Engine's U2NMatrixCopy / N2UTransform (Engine/Src/UnNovodexSupport.cpp) go through
	setColumnMajor/getColumnMajor with the three Unreal basis rows in entries[0..2], [3..5], [6..8],
	i.e. an Unreal row is a PhysX column: PhysX is the column-vector convention.
=============================================================================*/

#ifndef NX_MAT33_H
#define NX_MAT33_H

#include "NxQuat.h"

// DISHONORED(layout): PDB enum NxMatrixType.
enum NxMatrixType
{
	NX_ZERO_MATRIX,
	NX_IDENTITY_MATRIX
};

// DISHONORED(layout): PDB Nx9Real (36 bytes, union of the nine named floats and float m[3][3]).
typedef struct
{
	union
	{
		struct { NxF32 _11, _12, _13, _21, _22, _23, _31, _32, _33; } s;
		NxF32 m[3][3];
	};
} Nx9Real;

class NxMat33
{
public:
	Nx9Real data;

	NX_INLINE NxMat33() {}
	NX_INLINE explicit NxMat33(NxMatrixType type)
	{
		switch (type)
		{
		case NX_ZERO_MATRIX:		zero(); break;
		case NX_IDENTITY_MATRIX:	id(); break;
		}
	}
	NX_INLINE NxMat33(const NxVec3& row0, const NxVec3& row1, const NxVec3& row2)
	{
		setRow(0, row0); setRow(1, row1); setRow(2, row2);
	}

	NX_INLINE NxReal& operator()(int row, int col) { return data.m[row][col]; }
	NX_INLINE NxReal operator()(int row, int col) const { return data.m[row][col]; }
	NX_INLINE NxReal& operator()(int i) { return ((NxReal*)&data)[i]; }
	NX_INLINE NxReal operator()(int i) const { return ((const NxReal*)&data)[i]; }

	NX_INLINE void zero() { memset(&data, 0, sizeof(data)); }
	NX_INLINE void id()
	{
		zero();
		data.m[0][0] = data.m[1][1] = data.m[2][2] = 1.0f;
	}
	NX_INLINE void setIdentity() { id(); }
	NX_INLINE bool isIdentity() const
	{
		return data.m[0][0] == 1.0f && data.m[1][1] == 1.0f && data.m[2][2] == 1.0f
			&& data.m[0][1] == 0.0f && data.m[0][2] == 0.0f && data.m[1][0] == 0.0f
			&& data.m[1][2] == 0.0f && data.m[2][0] == 0.0f && data.m[2][1] == 0.0f;
	}
	NX_INLINE bool isZero() const
	{
		for (int i = 0; i < 9; ++i) { if (((const NxReal*)&data)[i] != 0.0f) return false; }
		return true;
	}
	NX_INLINE bool isFinite() const
	{
		for (int i = 0; i < 9; ++i) { if (!NxMath::isFinite(((const NxReal*)&data)[i])) return false; }
		return true;
	}
	NX_INLINE void setNegative() { for (int i = 0; i < 9; ++i) { ((NxReal*)&data)[i] = -((NxReal*)&data)[i]; } }
	NX_INLINE void setDiagonal(const NxVec3& v) { zero(); data.m[0][0] = v.x; data.m[1][1] = v.y; data.m[2][2] = v.z; }
	NX_INLINE void getDiagonal(NxVec3& v) const { v.set(data.m[0][0], data.m[1][1], data.m[2][2]); }

	NX_INLINE void setRow(int row, const NxVec3& v) { data.m[row][0] = v.x; data.m[row][1] = v.y; data.m[row][2] = v.z; }
	NX_INLINE void setColumn(int col, const NxVec3& v) { data.m[0][col] = v.x; data.m[1][col] = v.y; data.m[2][col] = v.z; }
	NX_INLINE void getRow(int row, NxVec3& v) const { v.set(data.m[row][0], data.m[row][1], data.m[row][2]); }
	NX_INLINE void getColumn(int col, NxVec3& v) const { v.set(data.m[0][col], data.m[1][col], data.m[2][col]); }
	NX_INLINE NxVec3 getRow(int row) const { return NxVec3(data.m[row][0], data.m[row][1], data.m[row][2]); }
	NX_INLINE NxVec3 getColumn(int col) const { return NxVec3(data.m[0][col], data.m[1][col], data.m[2][col]); }

	NX_INLINE void setRowMajor(const NxF32* d) { memcpy(&data, d, 9 * sizeof(NxF32)); }
	NX_INLINE void getRowMajor(NxF32* d) const { memcpy(d, &data, 9 * sizeof(NxF32)); }
	NX_INLINE void setColumnMajor(const NxF32* d)
	{
		for (int c = 0; c < 3; ++c) { for (int r = 0; r < 3; ++r) { data.m[r][c] = d[c * 3 + r]; } }
	}
	NX_INLINE void getColumnMajor(NxF32* d) const
	{
		for (int c = 0; c < 3; ++c) { for (int r = 0; r < 3; ++r) { d[c * 3 + r] = data.m[r][c]; } }
	}
	NX_INLINE void setRowMajorStride4(const NxF32* d)
	{
		for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { data.m[r][c] = d[r * 4 + c]; } }
	}
	NX_INLINE void getRowMajorStride4(NxF32* d) const
	{
		for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { d[r * 4 + c] = data.m[r][c]; } }
	}
	NX_INLINE void setColumnMajorStride4(const NxF32* d)
	{
		for (int c = 0; c < 3; ++c) { for (int r = 0; r < 3; ++r) { data.m[r][c] = d[c * 4 + r]; } }
	}
	NX_INLINE void getColumnMajorStride4(NxF32* d) const
	{
		for (int c = 0; c < 3; ++c) { for (int r = 0; r < 3; ++r) { d[c * 4 + r] = data.m[r][c]; } }
	}

	// DISHONORED(written): rotX/rotY/rotZ set the matrix to a rotation of `angle` radians about that
	// axis in the column-vector convention (Engine/Src/NxForceFieldTornado.cpp, ForceFieldShape*.cpp
	// build shape poses with rotX(-NxPi/2) and friends).
	NX_INLINE void rotX(NxReal angle)
	{
		id();
		const NxReal c = NxMath::cos(angle), s = NxMath::sin(angle);
		data.m[1][1] = c; data.m[1][2] = -s;
		data.m[2][1] = s; data.m[2][2] = c;
	}
	NX_INLINE void rotY(NxReal angle)
	{
		id();
		const NxReal c = NxMath::cos(angle), s = NxMath::sin(angle);
		data.m[0][0] = c; data.m[0][2] = s;
		data.m[2][0] = -s; data.m[2][2] = c;
	}
	NX_INLINE void rotZ(NxReal angle)
	{
		id();
		const NxReal c = NxMath::cos(angle), s = NxMath::sin(angle);
		data.m[0][0] = c; data.m[0][1] = -s;
		data.m[1][0] = s; data.m[1][1] = c;
	}

	NX_INLINE void setTransposed(const NxMat33& other)
	{
		for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { data.m[r][c] = other.data.m[c][r]; } }
	}
	NX_INLINE void setTransposed()
	{
		NxMat33 t; t.setTransposed(*this); *this = t;
	}
	NX_INLINE NxReal determinant() const
	{
		return data.m[0][0] * (data.m[1][1] * data.m[2][2] - data.m[1][2] * data.m[2][1])
			 - data.m[0][1] * (data.m[1][0] * data.m[2][2] - data.m[1][2] * data.m[2][0])
			 + data.m[0][2] * (data.m[1][0] * data.m[2][1] - data.m[1][1] * data.m[2][0]);
	}
	NX_INLINE bool getInverse(NxMat33& out) const
	{
		const NxReal det = determinant();
		if (NxMath::abs(det) < 1e-20f) { return false; }
		const NxReal id = 1.0f / det;
		out.data.m[0][0] = id * (data.m[1][1] * data.m[2][2] - data.m[1][2] * data.m[2][1]);
		out.data.m[0][1] = id * (data.m[0][2] * data.m[2][1] - data.m[0][1] * data.m[2][2]);
		out.data.m[0][2] = id * (data.m[0][1] * data.m[1][2] - data.m[0][2] * data.m[1][1]);
		out.data.m[1][0] = id * (data.m[1][2] * data.m[2][0] - data.m[1][0] * data.m[2][2]);
		out.data.m[1][1] = id * (data.m[0][0] * data.m[2][2] - data.m[0][2] * data.m[2][0]);
		out.data.m[1][2] = id * (data.m[0][2] * data.m[1][0] - data.m[0][0] * data.m[1][2]);
		out.data.m[2][0] = id * (data.m[1][0] * data.m[2][1] - data.m[1][1] * data.m[2][0]);
		out.data.m[2][1] = id * (data.m[0][1] * data.m[2][0] - data.m[0][0] * data.m[2][1]);
		out.data.m[2][2] = id * (data.m[0][0] * data.m[1][1] - data.m[0][1] * data.m[1][0]);
		return true;
	}
	NX_INLINE bool setInverse(const NxMat33& other) { return other.getInverse(*this); }

	NX_INLINE void fromQuat(const NxQuat& q)
	{
		const NxReal x = q.x, y = q.y, z = q.z, w = q.w;
		data.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
		data.m[0][1] = 2.0f * (x * y - z * w);
		data.m[0][2] = 2.0f * (x * z + y * w);
		data.m[1][0] = 2.0f * (x * y + z * w);
		data.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
		data.m[1][2] = 2.0f * (y * z - x * w);
		data.m[2][0] = 2.0f * (x * z - y * w);
		data.m[2][1] = 2.0f * (y * z + x * w);
		data.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
	}
	NX_INLINE void toQuat(NxQuat& q) const
	{
		const NxReal tr = data.m[0][0] + data.m[1][1] + data.m[2][2];
		if (tr > 0.0f)
		{
			NxReal s = NxMath::sqrt(tr + 1.0f);
			q.w = s * 0.5f;
			s = 0.5f / s;
			q.x = (data.m[2][1] - data.m[1][2]) * s;
			q.y = (data.m[0][2] - data.m[2][0]) * s;
			q.z = (data.m[1][0] - data.m[0][1]) * s;
		}
		else
		{
			int i = 0;
			if (data.m[1][1] > data.m[0][0]) { i = 1; }
			if (data.m[2][2] > data.m[i][i]) { i = 2; }
			const int j = (i + 1) % 3, k = (j + 1) % 3;
			NxReal s = NxMath::sqrt(data.m[i][i] - data.m[j][j] - data.m[k][k] + 1.0f);
			NxReal* qv = &q.x;
			qv[i] = s * 0.5f;
			s = 0.5f / s;
			q.w = (data.m[k][j] - data.m[j][k]) * s;
			qv[j] = (data.m[j][i] + data.m[i][j]) * s;
			qv[k] = (data.m[k][i] + data.m[i][k]) * s;
		}
	}

	NX_INLINE void multiply(const NxVec3& src, NxVec3& dst) const
	{
		const NxReal x = data.m[0][0] * src.x + data.m[0][1] * src.y + data.m[0][2] * src.z;
		const NxReal y = data.m[1][0] * src.x + data.m[1][1] * src.y + data.m[1][2] * src.z;
		const NxReal z = data.m[2][0] * src.x + data.m[2][1] * src.y + data.m[2][2] * src.z;
		dst.set(x, y, z);
	}
	NX_INLINE void multiplyByTranspose(const NxVec3& src, NxVec3& dst) const
	{
		const NxReal x = data.m[0][0] * src.x + data.m[1][0] * src.y + data.m[2][0] * src.z;
		const NxReal y = data.m[0][1] * src.x + data.m[1][1] * src.y + data.m[2][1] * src.z;
		const NxReal z = data.m[0][2] * src.x + data.m[1][2] * src.y + data.m[2][2] * src.z;
		dst.set(x, y, z);
	}
	NX_INLINE void multiply(const NxMat33& a, const NxMat33& b)
	{
		NxMat33 r;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				r.data.m[i][j] = a.data.m[i][0] * b.data.m[0][j]
							   + a.data.m[i][1] * b.data.m[1][j]
							   + a.data.m[i][2] * b.data.m[2][j];
			}
		}
		*this = r;
	}
	NX_INLINE void add(const NxMat33& a, const NxMat33& b)
	{
		for (int i = 0; i < 9; ++i) { ((NxReal*)&data)[i] = ((const NxReal*)&a.data)[i] + ((const NxReal*)&b.data)[i]; }
	}
	NX_INLINE void subtract(const NxMat33& a, const NxMat33& b)
	{
		for (int i = 0; i < 9; ++i) { ((NxReal*)&data)[i] = ((const NxReal*)&a.data)[i] - ((const NxReal*)&b.data)[i]; }
	}
	NX_INLINE void multiply(NxReal s, const NxMat33& a)
	{
		for (int i = 0; i < 9; ++i) { ((NxReal*)&data)[i] = s * ((const NxReal*)&a.data)[i]; }
	}
	// Engine/Src/UnPhysActorForceField.cpp assigns a quaternion straight into NxMat34::M and
	// Engine/Src/NxForceFieldTornado.cpp multiplies a quaternion by a matrix.
	NX_INLINE NxMat33& operator=(const NxQuat& q) { fromQuat(q); return *this; }
	NX_INLINE void multiply(const NxQuat& q, const NxMat33& m) { NxMat33 r; r.fromQuat(q); multiply(r, m); }
	NX_INLINE void multiply(const NxMat33& m, const NxQuat& q) { NxMat33 r; r.fromQuat(q); multiply(m, r); }
	NX_INLINE NxVec3 operator*(const NxVec3& v) const { NxVec3 r; multiply(v, r); return r; }
	NX_INLINE NxMat33 operator*(const NxMat33& m) const { NxMat33 r; r.multiply(*this, m); return r; }
	NX_INLINE NxMat33 operator+(const NxMat33& m) const { NxMat33 r; r.add(*this, m); return r; }
	NX_INLINE NxMat33 operator-(const NxMat33& m) const { NxMat33 r; r.subtract(*this, m); return r; }
	NX_INLINE NxMat33 operator*(NxReal s) const { NxMat33 r; r.multiply(s, *this); return r; }
	NX_INLINE NxMat33& operator*=(NxReal s) { multiply(s, NxMat33(*this)); return *this; }
	NX_INLINE NxMat33& operator+=(const NxMat33& m) { add(NxMat33(*this), m); return *this; }
	NX_INLINE NxMat33& operator-=(const NxMat33& m) { subtract(NxMat33(*this), m); return *this; }
};

#endif // NX_MAT33_H
