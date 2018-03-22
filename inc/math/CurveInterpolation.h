#ifndef INCLUDED_CURVE_INTERPOLATION_H
#define INCLUDED_CURVE_INTERPOLATION_H
#include "lxMath.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace math
{

//! Handling of value beyond the defined value.
enum class EEdgeHandling
{
	Clamp, //! Use the value of the next border.
	Loop, //! Repeat the defined range.
	Mirror, //! Mirror the defined range.
};

//! How are values interpolated
enum class EInterpolation
{
	Const, //! Use the next value.
	Linear, //! Interpolate linear between the next two value.
	Smooth, //! Interpolate smooth between the points.
};

template <typename T>
struct Sample
{
	float x;
	T value;

	Sample() {}
	Sample(float _x, const T& _value) :
		x(_x),
		value(_value)
	{}
	bool operator<(const Sample& other) const
	{
		if(x == other.x)
			return value < other.value;
		else
			return x < other.x;
	}
	bool operator==(const Sample& other) const
	{
		return x == other.x && value == other.value;
	}

	LX_DEFINE_COMPARE_FUNCTIONS_BY_SMALLER_AND_EQUAL(Sample)
};

namespace CurveHelper
{
//! Map a given x value to a the valid x range(i.e. [smallestX, biggerX])
inline float MapToValidRange(float x, float minX, float maxX, EEdgeHandling edgeHandling)
{
	const float width = maxX - minX;

	float newX = x;

	if(x < minX || x > maxX) {
		if(edgeHandling == EEdgeHandling::Clamp) {
			newX = math::Clamp(x, minX, maxX);
		} else if(edgeHandling == EEdgeHandling::Mirror) {
			newX = fabsf(minX - x);

			newX = fmodf(newX, 2.0f*width);

			if(newX >= width)
				newX = 2.0f*width - newX;

			newX += minX;
		} else if(edgeHandling == EEdgeHandling::Loop) {
			newX = fmodf(x - minX, width);
			if(newX < 0)
				newX += width;
			newX += minX;
		}
	}

	lxAssert(newX >= minX && newX <= maxX);
	return newX;
}

//! The index of the next element >= x, to the right.
template <typename T>
int GetUpperBound(const Sample<T>* samples, int count, float x)
{
	const Sample<T>* n;
	const Sample<T>* i;
	auto range = core::MakeRange(samples, samples + count);
	Sample<T> dummy;
	dummy.x = x;
	n = core::BinarySearch(dummy, range, &i);
	if(n == range.End())
		return core::IteratorDistance(range.First(), i);
	else
		return core::IteratorDistance(range.First(), n);
}

template <typename T>
void GetBounds(const Sample<T>* samples, int count, float x, int& lower, int& upper)
{
	const int t = GetUpperBound(samples, count, x);

	if(t != 0) {
		upper = t;
		lower = t - 1;
	} else {
		upper = 1;
		lower = 0;
	}
}

//! Return the tangent at a given index.
template <typename T>
T GetTangent(const Sample<T>* samples, int count, int idx, EEdgeHandling edgeHandling)
{
	T a, b;
	float da;
	float db;
	if(edgeHandling == EEdgeHandling::Clamp) {
		if(idx == 0) {
			a = samples[0].value;
			b = samples[1].value;
			db = samples[1].x - samples[0].x;

			da = db;
		} else if(idx == count - 1) {
			a = samples[idx - 1].value;
			b = samples[idx].value;

			da = samples[idx].x - samples[idx - 1].x;
			db = da;
		} else {
			a = samples[idx - 1].value;
			b = samples[idx + 1].value;

			da = samples[idx].x - samples[idx - 1].x;
			db = samples[idx + 1].x - samples[idx].x;
		}
	} else if(edgeHandling == EEdgeHandling::Loop) {
		if(idx == 0) {
			a = samples[count - 2].value;
			b = samples[idx + 1].value;

			da = samples[count - 1].x - samples[count - 2].x;
			db = samples[idx + 1].x - samples[idx].x;
		} else if(idx == count - 1) {
			a = samples[idx - 1].value;
			b = samples[1].value;

			da = samples[idx].x - samples[idx - 1].x;
			db = samples[1].x - samples[0].x;
		} else {
			a = samples[idx - 1].value;
			b = samples[idx + 1].value;

			da = samples[idx].x - samples[idx - 1].x;
			db = samples[idx + 1].x - samples[idx].x;
		}
	} else if(edgeHandling == EEdgeHandling::Mirror) {
		if(idx == 0) {
			return (samples[1].value - samples[1].value);
		} else if(idx == count - 1) {
			return (samples[1].value - samples[1].value);
		} else {
			a = samples[idx - 1].value;
			b = samples[idx + 1].value;

			da = samples[idx].x - samples[idx - 1].x;
			db = samples[idx + 1].x - samples[idx].x;
		}
	} else {
		lxAssertNeverReach("Invalid edge handling type.");
		a = b = samples[idx].value;
		da = db = 1.0f;
	}

	T p = samples[idx].value;
	float invda = 0.5f*((da+db) / da);
	float invdb = 0.5f*((da+db) / db);
	return (p - a) * invda + (b - p) * invdb;
}
}

//! Interpolate a curve for an array of samples
/**
Requierments for T:
Default and copy constructable.
Assignment operator.
Addition, Subtraction, Postmultiplication with float.

\param samples The sample points, must be sorted by x value.
\param count The number of samples.
\param x The x value to interpolate.
\param edgeHandling How are x values outside of the valid range handled(extrapolation).
\param interpolate The used interpolation technique
*/
template <typename T>
T CurveInterpolation(
	const Sample<T>* samples, int count,
	float x,
	EEdgeHandling edgeHandling = EEdgeHandling::Clamp,
	EInterpolation interpolate = EInterpolation::Smooth)
{
	if(count == 0)
		return T();
	if(count == 1)
		return samples[0].value;

	x = CurveHelper::MapToValidRange(x, samples[0].x, samples[count - 1].x, edgeHandling); // x in [lower, upper]
	int upper, lower;
	CurveHelper::GetBounds(samples, count, x, lower, upper);
	const float xl = samples[lower].x;
	const T vl = samples[lower].value;
	const float xu = samples[upper].x;
	const T vu = samples[upper].value;
	const float t = (x - xl) / (xu-xl);

	if(interpolate == EInterpolation::Const) {
		return t < 0.5f ? vl : vu;
	} else if(interpolate == EInterpolation::Linear) {
		return math::Lerp(vl, vu, t);
	} else if(interpolate == EInterpolation::Smooth) {
		T tu = CurveHelper::GetTangent(samples, count, upper, edgeHandling);
		T tl = CurveHelper::GetTangent(samples, count, lower, edgeHandling);

		return InterpolateHermite(
			vl, tl,
			vu, tu,
			t);
	} else {
		lxAssertNeverReach("Unknown interpolation type");
		return T();
	}
}

}
}

#endif // #ifndef INCLUDED_CURVE_H
