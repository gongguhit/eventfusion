// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_GEOMETRYTYPES_DV_H_
#define FLATBUFFERS_GENERATED_GEOMETRYTYPES_DV_H_

#include "../external/flatbuffers/flatbuffers.h"

namespace dv {

struct Point3f;

struct Point2f;

struct Vec3f;

struct Vec2f;

struct Quaternion;

bool operator==(const Point3f &lhs, const Point3f &rhs);
bool operator==(const Point2f &lhs, const Point2f &rhs);
bool operator==(const Vec3f &lhs, const Vec3f &rhs);
bool operator==(const Vec2f &lhs, const Vec2f &rhs);
bool operator==(const Quaternion &lhs, const Quaternion &rhs);

inline const flatbuffers::TypeTable *Point3fTypeTable();

inline const flatbuffers::TypeTable *Point2fTypeTable();

inline const flatbuffers::TypeTable *Vec3fTypeTable();

inline const flatbuffers::TypeTable *Vec2fTypeTable();

inline const flatbuffers::TypeTable *QuaternionTypeTable();

/// Structure representing absolute position of a 3D point
FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Point3f FLATBUFFERS_FINAL_CLASS {
	typedef Point3f NativeTableType;
	typedef Point3f TableType;

private:
	float x_;
	float y_;
	float z_;

public:
	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.Point3f";
	}
	Point3f() {
		memset(static_cast<void *>(this), 0, sizeof(Point3f));
	}
	Point3f(float _x, float _y, float _z) :
		x_(flatbuffers::EndianScalar(_x)),
		y_(flatbuffers::EndianScalar(_y)),
		z_(flatbuffers::EndianScalar(_z)) {
	}
	float x() const {
		return flatbuffers::EndianScalar(x_);
	}
	float y() const {
		return flatbuffers::EndianScalar(y_);
	}
	float z() const {
		return flatbuffers::EndianScalar(z_);
	}
};

FLATBUFFERS_STRUCT_END(Point3f, 12);

inline bool operator==(const Point3f &lhs, const Point3f &rhs) {
	return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) && (lhs.z() == rhs.z());
}

/// Structure representing absolute position of a 2D point
FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Point2f FLATBUFFERS_FINAL_CLASS {
	typedef Point2f NativeTableType;
	typedef Point2f TableType;

private:
	float x_;
	float y_;

public:
	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.Point2f";
	}
	Point2f() {
		memset(static_cast<void *>(this), 0, sizeof(Point2f));
	}
	Point2f(float _x, float _y) : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)) {
	}
	float x() const {
		return flatbuffers::EndianScalar(x_);
	}
	float y() const {
		return flatbuffers::EndianScalar(y_);
	}
};

FLATBUFFERS_STRUCT_END(Point2f, 8);

inline bool operator==(const Point2f &lhs, const Point2f &rhs) {
	return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y());
}

/// Structure representing a 3D vector
FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vec3f FLATBUFFERS_FINAL_CLASS {
	typedef Vec3f NativeTableType;
	typedef Vec3f TableType;

private:
	float x_;
	float y_;
	float z_;

public:
	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.Vec3f";
	}
	Vec3f() {
		memset(static_cast<void *>(this), 0, sizeof(Vec3f));
	}
	Vec3f(float _x, float _y, float _z) :
		x_(flatbuffers::EndianScalar(_x)),
		y_(flatbuffers::EndianScalar(_y)),
		z_(flatbuffers::EndianScalar(_z)) {
	}
	float x() const {
		return flatbuffers::EndianScalar(x_);
	}
	float y() const {
		return flatbuffers::EndianScalar(y_);
	}
	float z() const {
		return flatbuffers::EndianScalar(z_);
	}
};

FLATBUFFERS_STRUCT_END(Vec3f, 12);

inline bool operator==(const Vec3f &lhs, const Vec3f &rhs) {
	return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) && (lhs.z() == rhs.z());
}

/// Structure representing a 2D vector
FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vec2f FLATBUFFERS_FINAL_CLASS {
	typedef Vec2f NativeTableType;
	typedef Vec2f TableType;

private:
	float x_;
	float y_;

public:
	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.Vec2f";
	}
	Vec2f() {
		memset(static_cast<void *>(this), 0, sizeof(Vec2f));
	}
	Vec2f(float _x, float _y) : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)) {
	}
	float x() const {
		return flatbuffers::EndianScalar(x_);
	}
	float y() const {
		return flatbuffers::EndianScalar(y_);
	}
};

FLATBUFFERS_STRUCT_END(Vec2f, 8);

inline bool operator==(const Vec2f &lhs, const Vec2f &rhs) {
	return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y());
}

/// Quaternion with Eigen compatible memory layout, should follow the Hamilton convention
FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Quaternion FLATBUFFERS_FINAL_CLASS {
	typedef Quaternion NativeTableType;
	typedef Quaternion TableType;

private:
	float w_;
	float x_;
	float y_;
	float z_;

public:
	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.Quaternion";
	}
	Quaternion() {
		memset(static_cast<void *>(this), 0, sizeof(Quaternion));
	}
	Quaternion(float _w, float _x, float _y, float _z) :
		w_(flatbuffers::EndianScalar(_w)),
		x_(flatbuffers::EndianScalar(_x)),
		y_(flatbuffers::EndianScalar(_y)),
		z_(flatbuffers::EndianScalar(_z)) {
	}
	float w() const {
		return flatbuffers::EndianScalar(w_);
	}
	float x() const {
		return flatbuffers::EndianScalar(x_);
	}
	float y() const {
		return flatbuffers::EndianScalar(y_);
	}
	float z() const {
		return flatbuffers::EndianScalar(z_);
	}
};

FLATBUFFERS_STRUCT_END(Quaternion, 16);

inline bool operator==(const Quaternion &lhs, const Quaternion &rhs) {
	return (lhs.w() == rhs.w()) && (lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) && (lhs.z() == rhs.z());
}

inline const flatbuffers::TypeTable *Point3fTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1}
    };
	static const int64_t values[]          = {0, 4, 8, 12};
	static const char *const names[]       = {"x", "y", "z"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_STRUCT, 3, type_codes, nullptr, values, names};
	return &tt;
}

inline const flatbuffers::TypeTable *Point2fTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1}
    };
	static const int64_t values[]          = {0, 4, 8};
	static const char *const names[]       = {"x", "y"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_STRUCT, 2, type_codes, nullptr, values, names};
	return &tt;
}

inline const flatbuffers::TypeTable *Vec3fTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1}
    };
	static const int64_t values[]          = {0, 4, 8, 12};
	static const char *const names[]       = {"x", "y", "z"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_STRUCT, 3, type_codes, nullptr, values, names};
	return &tt;
}

inline const flatbuffers::TypeTable *Vec2fTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1}
    };
	static const int64_t values[]          = {0, 4, 8};
	static const char *const names[]       = {"x", "y"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_STRUCT, 2, type_codes, nullptr, values, names};
	return &tt;
}

inline const flatbuffers::TypeTable *QuaternionTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1},
        {flatbuffers::ET_FLOAT, 0, -1},
		{flatbuffers::ET_FLOAT, 0, -1}
    };
	static const int64_t values[]          = {0, 4, 8, 12, 16};
	static const char *const names[]       = {"w", "x", "y", "z"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_STRUCT, 4, type_codes, nullptr, values, names};
	return &tt;
}

} // namespace dv

#endif // FLATBUFFERS_GENERATED_GEOMETRYTYPES_DV_H_
