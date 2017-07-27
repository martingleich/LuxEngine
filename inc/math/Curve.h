#ifndef INCLUDED_CURVE_H
#define INCLUDED_CURVE_H
#include "lxMath.h"
#include "core/lxArray.h"
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

//! Parametric curve through vector values.
template <typename T>
class Curve
{
private:
	//! A single defined value.
	struct Sample
	{
		float x; //! The parameter of the value.
		T value; //! The value of the value.

		Sample() {}
		Sample(float _x, const T& _value) :
			x(_x),
			value(_value)
		{
		}

		bool operator<(const Sample& other) const
		{
			return x < other.x;
		}

		bool operator==(const Sample& other) const
		{
			return x == other.x;
		}
	};

public:
	//! Creates a default curve.
	/*
	The default curve has no points, and is interpolated smooth, and clamped on the edges.
	*/
	Curve() :
		m_EdgeHandling(EEdgeHandling::Clamp),
		m_Interpolation(EInterpolation::Smooth)
	{
	}

	//! Add a single sample to the curve.
	/**
	If there already is a sample for the given parameter, the old data is overwritten.
	Remark: For best speed add samples in order of growing parameter.
	\param x The parameter of the value.
	\param value The value of the sample.
	*/
	void AddSample(float x, const T& value)
	{
		if(m_Samples.IsEmpty() || m_Samples.Last()->x <= x) {
			if(!m_Samples.IsEmpty() && m_Samples.Last()->x == x)
				m_Samples.Last()->value = value;
			else
				m_Samples.PushBack(Sample(x, value));
		} else {
			typename core::Array<Sample>::Iterator n, i;
			Sample dummy;
			dummy.x = x;
			n = core::BinarySearch(dummy, m_Samples.First(), m_Samples.End(), &i);
			if(n == m_Samples.End())
				m_Samples.Insert(Sample(x, value), i);
			else
				i->value = value;
		}
	}

	//! Add multiple samples to the curve.
	/**
	If there already is a sample for the given parameter, the old data is overwritten.
	Remark: For best speed add samples in order of growing parameter.
	\param x The parameters of the samples
	\param values The values of the sample.
	\param count The number of values to add.
	*/
	void AddSampleBatch(const float* x, const T* values, size_t count)
	{
		m_Samples.Reserve(m_Samples.Size() + count);
		// IDEA: Optimize this.
		for(size_t i = 0; i < count; ++i)
			AddSample(x[i], values[i]);
	}

	//! Get the value at any parameter.
	/**
	Not given parameters are interpolated with the given technique.
	If there are no samples, the default-constructed value is returned.
	\param x The parameter.
	\return The interpolated value.
	*/
	T GetValue(float x) const
	{
		if(m_Samples.Size() == 0)
			return T();
		if(m_Samples.Size() == 1)
			return m_Samples[0].value;

		x = MapToValidRange(x); // x in [lower, upper]
		u32 upper, lower;
		GetBounds(x, lower, upper);
		const float xl = m_Samples[lower].x;
		const T vl = m_Samples[lower].value;
		const float xu = m_Samples[upper].x;
		const T vu = m_Samples[upper].value;
		const float t = (x - xl) / (xu - xl);

		if(m_Interpolation == EInterpolation::Const) {
			return t < 0.5f ? vl : vu;
		} else if(m_Interpolation == EInterpolation::Linear) {
			return math::Lerp(vl, vu, t);
		} else if(m_Interpolation == EInterpolation::Smooth) {
			T tu = GetTangent(upper);
			T tl = GetTangent(lower);

			return InterpolateHermite(
				vl, tl,
				vu, tu,
				t);
		} else {
			return T();
		}
	}

	//! Get the bounds of a given parameter.
	/**
	\param The x value to query.
	\param [out] lower The biggest sample which is smaller or equal to the x value.
	\param [out] upper The smaller sample which is bigger than the x value.
	*/
	void GetBounds(float x, u32& lower, u32& upper) const
	{
		const u32 t = GetUpperBound(x);

		if(t != 0) {
			upper = t;
			lower = t - 1;
		} else {
			upper = 1;
			lower = 0;
		}
	}

	//! The value at a given index.
	const T& GetSampleValue(u32 i) const
	{
		return m_Samples[i].value;
	}

	//! The parameter at a given index.
	float GetSampleX(u32 i) const
	{
		return m_Samples[i].x;
	}

	//! Set the value at a index.
	void SetSample(u32 i, const T& value)
	{
		m_Samples[i] = value;
	}

	//! Remove a sample by an index.
	void RemoveSample(u32 i)
	{
		m_Samples.Erase(core::AdvanceIterator(m_Samples.First(), i), true);
	}

	//! The total number of samples.
	u32 GetSampleCount() const
	{
		return m_Samples.Size();
	}

	//! Remove all samples.
	void Clear()
	{
		m_Samples.Clear();
	}

	//! Set the edge handling.
	void SetEdgeHandling(EEdgeHandling handling)
	{
		m_EdgeHandling = handling;
	}

	//! Returns the current edge handling.
	EEdgeHandling GetEdgeHandling() const
	{
		return m_EdgeHandling;
	}

	//! Set the interpolation technique.
	void SetInterpolation(EInterpolation interpolation)
	{
		m_Interpolation = interpolation;
	}

	//! Returns the current interpolation techinque.
	EInterpolation GetInterpolation() const
	{
		return m_Interpolation;
	}

private:
	//! Map a given x value to a the valid x range(i.e. [smallestX, biggerX])
	float MapToValidRange(float x) const
	{
		const float lowerRange = m_Samples.First()->x;
		const float upperRange = m_Samples.Last()->x;
		const float width = upperRange - lowerRange;

		float newX = x;

		if(x < lowerRange || x > upperRange) {
			if(m_EdgeHandling == EEdgeHandling::Clamp) {
				newX = math::Clamp(x, lowerRange, upperRange);
			} else if(m_EdgeHandling == EEdgeHandling::Mirror) {
				newX = fabsf(lowerRange - x);

				newX = fmodf(newX, 2.0f*width);

				if(newX >= width)
					newX = 2.0f*width - newX;

				newX += lowerRange;
			} else if(m_EdgeHandling == EEdgeHandling::Loop) {
				newX = fmodf(x - lowerRange, width);
				if(newX < 0)
					newX += width;
				newX += lowerRange;
			}
		}

		lxAssert(newX >= lowerRange && newX <= upperRange);
		return newX;
	}

	//! Return the tangent at a given index.
	T GetTangent(u32 idx) const
	{
		T a, b;
		float da;
		float db;
		if(m_EdgeHandling == EEdgeHandling::Clamp) {
			if(idx == 0) {
				a = m_Samples[0].value;
				b = m_Samples[1].value;
				db = m_Samples[1].x - m_Samples[0].x;

				da = db;
			} else if(idx == m_Samples.Size() - 1) {
				a = m_Samples[idx - 1].value;
				b = m_Samples[idx].value;

				da = m_Samples[idx].x - m_Samples[idx - 1].x;
				db = da;
			} else {
				a = m_Samples[idx - 1].value;
				b = m_Samples[idx + 1].value;

				da = m_Samples[idx].x - m_Samples[idx - 1].x;
				db = m_Samples[idx + 1].x - m_Samples[idx].x;
			}
		} else if(m_EdgeHandling == EEdgeHandling::Loop) {
			if(idx == 0) {
				a = m_Samples[m_Samples.Size() - 2].value;
				b = m_Samples[idx + 1].value;

				da = m_Samples[m_Samples.Size() - 1].x - m_Samples[m_Samples.Size() - 2].x;
				db = m_Samples[idx + 1].x - m_Samples[idx].x;
			} else if(idx == m_Samples.Size() - 1) {
				a = m_Samples[idx - 1].value;
				b = m_Samples[1].value;

				da = m_Samples[idx].x - m_Samples[idx - 1].x;
				db = m_Samples[1].x - m_Samples[0].x;
			} else {
				a = m_Samples[idx - 1].value;
				b = m_Samples[idx + 1].value;

				da = m_Samples[idx].x - m_Samples[idx - 1].x;
				db = m_Samples[idx + 1].x - m_Samples[idx].x;
			}
		} else if(m_EdgeHandling == EEdgeHandling::Mirror) {
			if(idx == 0) {
				return (m_Samples[1].value - m_Samples[1].value);
			} else if(idx == m_Samples.Size() - 1) {
				return (m_Samples[1].value - m_Samples[1].value);
			} else {
				a = m_Samples[idx - 1].value;
				b = m_Samples[idx + 1].value;

				da = m_Samples[idx].x - m_Samples[idx - 1].x;
				db = m_Samples[idx + 1].x - m_Samples[idx].x;
			}
		} else {
			lxAssertNeverReach("Invalid edge handling type.");
			a = b = m_Samples[idx].value;
			da = db = 1.0f;
		}

		T p = m_Samples[idx].value;
		float invda = 1.0f / da;
		float invdb = 1.0f / db;
		return 0.5f * ((p - a) * invda + (b - p) * invdb);
	}

	//! The index of the next element >= x, to the right.
	u32 GetUpperBound(float x) const
	{
		typename core::Array<Sample>::ConstIterator n, i;
		Sample dummy;
		dummy.x = x;
		n = core::BinarySearch(dummy, m_Samples, &i);
		if(n == m_Samples.End())
			return core::IteratorDistance(m_Samples.First(), i);
		else
			return core::IteratorDistance(m_Samples.First(), n);
	}

private:
	core::Array<Sample> m_Samples;
	EEdgeHandling m_EdgeHandling;
	EInterpolation m_Interpolation;
};

}
}

#endif // #ifndef INCLUDED_CURVE_H