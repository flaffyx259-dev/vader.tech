#pragma once
#include "../SDK/sdk.hpp"
#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)

namespace Math
{
	// pi constants.
	constexpr float pi = 3.1415926535897932384f; // pi
	static constexpr long double M_RADPI = 57.295779513082f;
	static constexpr long double M_PIRAD = 0.01745329251f;
	constexpr float pi_2 = pi * 2.f;               // pi * 2

	bool IntersectSegmentToSegment(Vector s1, Vector s2, Vector k1, Vector k2, float radius);
	bool IntersectSegmentSphere(const Vector& vecRayOrigin, const Vector& vecRayDelta, const Vector& vecSphereCenter, float radius);
	bool IntersectSegmentCapsule(const Vector& start, const Vector& end, const Vector& min, const Vector& max, float radius);
	bool IntersectionBoundingBox(const Vector& start, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point = nullptr);

	void Rotate(std::array<Vector2D, 3>& points, float rotation);

	void angle_vectors(const QAngle& angles, Vector* forward, Vector* right = nullptr, Vector* up = nullptr);

	void AngleVectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up);

	void VectorAngles( const Vector & forward, QAngle & angles );

	void VectorAngles(const Vector& forward, Vector& angles);

	void SinCos(float a, float* s, float* c);

	void AngleVectors( const QAngle& angles, Vector* forward );

	void AngleVectors( const QAngle& angles, Vector& forward );

	float GetFov( const QAngle& viewAngle, const Vector& start, const Vector& end );

	float get_fov(const QAngle& angles, const QAngle& target);

	float VecGetFov(const Vector& src, const Vector& dst);

	float AngleNormalize(float angle);

	void NormalizeAngle(float& angle);

	float normalize_float(float angle);

	float ApproachAngle(float target, float value, float speed);

	void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);

	void VectorTransform2(Vector& in1, matrix3x4_t& in2, Vector& out);

	void SmoothAngle(QAngle src, QAngle& dst, float factor);

	void CalcAngle3(const Vector src, const Vector dst, QAngle& angles);

	QAngle CalcAngle(Vector src, Vector dst, bool bruh = false);
	Vector VecCalcAngle(const Vector& src, const Vector& dest);

	__forceinline float NormalizedAngle(float angle) {
		NormalizeAngle(angle);
		return angle;
	}

	__forceinline void normalize_angles(QAngle& angles)
	{
		while (angles.x > 89.0f)
			angles.x -= 180.0f;

		while (angles.x < -89.0f)
			angles.x += 180.0f;

		while (angles.y < -180.0f)
			angles.y += 360.0f;

		while (angles.y > 180.0f)
			angles.y -= 360.0f;

		angles.z = 0.0f;
	}

	__forceinline float normalize_angle(float angle)
	{
		//return remainderf(angle, 360.0f);
		float rtn = angle;

		for (; rtn > 180.0f; rtn = rtn - 360.0f)
			;
		for (; rtn < -180.0f; rtn = rtn + 360.0f)
			;

		return rtn;
	}

	__forceinline float normalize_pitch(float pitch)
	{
		while (pitch > 89.0f)
			pitch -= 180.0f;

		while (pitch < -89.0f)
			pitch += 180.0f;

		return pitch;
	}

	inline void Vecanglevectors(const Vector& angles, Vector* forward)
	{
		float sp, sy, cp, cy;

		SinCos(DEG2RAD(angles.x), &sp, &cp);
		SinCos(DEG2RAD(angles.y), &sy, &cy);

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}
	}

	Vector GetSmoothedVelocity(float min_delta, Vector a, Vector b);

	float AngleDiff(float src, float dst);

	void VectorVectors(const Vector& forward, Vector& right, Vector& up);

	void AngleMatrix(const QAngle& angles, matrix3x4_t& matrix);

	matrix3x4_t AngleMatrix(const QAngle angles);

	void MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out);

	void ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out);

	__forceinline static float Interpolate(const float from, const float to, const float percent) {
		return to * percent + from * (1.f - percent);
	}

	__forceinline constexpr float deg_to_rad(float val) {
		return val * (pi / 180.f);
	}

	__forceinline constexpr float rad_to_deg(float val) {
		return val * (180.f / pi);
	}

	__forceinline static Vector Interpolate(const Vector from, const Vector to, const float percent) {
		return to * percent + from * (1.f - percent);
	}

	__forceinline static void MatrixSetOrigin(Vector pos, matrix3x4_t& matrix) {
		matrix[0][3] = pos.x;
		matrix[1][3] = pos.y;
		matrix[2][3] = pos.z;
	}

	__forceinline static Vector MatrixGetOrigin(const matrix3x4_t& src) {
		return { src[0][3], src[1][3], src[2][3] };
	}

	__forceinline void VectorScale(const float* in, float scale, float* out) {
		out[0] = in[0] * scale;
		out[1] = in[1] * scale;
		out[2] = in[2] * scale;
	}

	struct CapsuleCollider {
		Vector min;
		Vector max;
		float radius;

		bool Intersect(const Vector& a, const Vector& b) const;
	};

	// mixed types involved.
	template < typename T >
	T Clamp(const T& val, const T& minVal, const T& maxVal) {
		if ((T)val < minVal)
			return minVal;
		else if ((T)val > maxVal)
			return maxVal;
		else
			return val;
	}

	template < typename t >
	__forceinline void clampSupremacy(t& n, const t& lower, const t& upper) {
		n = std::max(lower, std::min(n, upper));
	}

	template < typename T >
	T Hermite_Spline(
		T p1,
		T p2,
		T d1,
		T d2,
		float t) {
		float tSqr = t * t;
		float tCube = t * tSqr;

		float b1 = 2.0f * tCube - 3.0f * tSqr + 1.0f;
		float b2 = 1.0f - b1; // -2*tCube+3*tSqr;
		float b3 = tCube - 2 * tSqr + t;
		float b4 = tCube - tSqr;

		T output;
		output = p1 * b1;
		output += p2 * b2;
		output += d1 * b3;
		output += d2 * b4;

		return output;
	}

	template < typename T >
	T Hermite_Spline(T p0, T p1, T p2, float t) {
		return Hermite_Spline(p1, p2, p1 - p0, p2 - p1, t);
	}

	// wide -> multi-byte
	__forceinline std::string WideToMultiByte(const std::wstring& str) {
		std::string ret;
		int         str_len;

		// check if not empty str
		if (str.empty())
			return { };

		// count size
		str_len = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), 0, 0, 0, 0);

		// setup return value
		ret = std::string(str_len, 0);

		// final conversion
		WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &ret[0], str_len, 0, 0);

		return ret;
	}

	// multi-byte -> wide
	__forceinline std::wstring MultiByteToWide(const std::string& str) {
		std::wstring    ret;
		int		        str_len;

		// check if not empty str
		if (str.empty())
			return { };

		// count size
		str_len = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);

		// setup return value
		ret = std::wstring(str_len, 0);

		// final conversion
		MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &ret[0], str_len);

		return ret;
	}
}
