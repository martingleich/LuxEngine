#ifndef INCLUDED_LUX_SCENE_ANIMATION_CURVE_H
#define INCLUDED_LUX_SCENE_ANIMATION_CURVE_H
#include "core/ReferenceCounted.h"
#include "math/CurveInterpolation.h"
#include "math/Vector3.h"
#include "core/VariableAccess.h"

namespace lux
{
namespace scene
{

class Curve : public ReferenceCounted
{
public:
	//! Get value of the curve at some time.
	/**
	\param time The time to query, must be between start and end.
	\param T Must be a class with the type of the curve.
	\return The value of the curve.
	*/
	template <typename T>
	T Evaluate(float time, u32* token = nullptr) const
	{
		T var;
		Evaluate(time, core::VariableAccess(core::TemplType<T>::Get(), &var), token);
		return var;
	}

	//! Get value of the curve at some time.
	/**
	\param time The time to query, must be between start and end.
	\param access The variable where the data is written, must be of the correct type.
	*/
	virtual void Evaluate(float time, const core::VariableAccess& access, u32* token = nullptr) const = 0;

	//! The first valid time of the curve.
	/**
	Might be infinite.
	*/
	virtual float GetStart() const = 0;
	//! The last valid time of the curve.
	/**
	Might be infinite.
	*/
	virtual float GetEnd() const = 0;

	//! The type of the elements of the curve.
	virtual core::Type GetType() const = 0;
};

class KeyFrameCurve : public Curve
{
public:
	void SetInterpolation(math::EInterpolation interpolation)
	{
		m_Interpolation = interpolation;
	}
	math::EInterpolation GetInterpolation() const
	{
		return m_Interpolation;
	}
	void SetEdgeHandling(math::EEdgeHandling handling)
	{
		m_EdgeHandling = handling;
	}
	math::EEdgeHandling GetEdgeHandling() const
	{
		return m_EdgeHandling;
	}

	template <typename T>
	void AddSample(float x, T value)
	{
		Samples<T>().PushBack(math::Sample<T>(x, value));
	}

	template <typename T>
	core::Array<math::Sample<T>>& Samples()
	{
		if(core::TemplType<T>::Get() == m_Type)
			return *((core::Array<math::Sample<T>>*)SamplesPointer());
		throw core::TypeException("Wrong type queried", core::TemplType<T>::Get(), m_Type);
	}

	template <typename T>
	const core::Array<math::Sample<T>>& Samples() const
	{
		if(core::TemplType<T>::Get() == m_Type)
			return *((const core::Array<math::Sample<T>>*)SamplesPointer());
		throw core::TypeException("Wrong type queried", core::TemplType<T>::Get(), m_Type);
	}

	core::Type GetType() const
	{
		return m_Type;
	}

protected:
	virtual void* SamplesPointer() = 0;
	virtual const void* SamplesPointer() const = 0;

protected:
	core::Type m_Type;
	math::EInterpolation m_Interpolation = math::EInterpolation::Smooth;
	math::EEdgeHandling m_EdgeHandling = math::EEdgeHandling::Clamp;
};

template <typename T>
class KeyFrameCurveImpl : public KeyFrameCurve
{
public:
	KeyFrameCurveImpl()
	{
		m_Type = core::TemplType<T>::Get();
	}

	void Evaluate(float time, const core::VariableAccess& access, u32* token=nullptr) const
	{
		access = math::CurveInterpolation(
			m_Samples.Data(), m_Samples.Size(),
			time, m_EdgeHandling, m_Interpolation, token);
	}

	float GetStart() const
	{
		if(m_Samples.IsEmpty())
			return 0;
		else
			return m_Samples.Front().x;
	}

	float GetEnd() const
	{
		if(m_Samples.IsEmpty())
			return 0;
		else
			return m_Samples.Back().x;
	}

protected:
	void* SamplesPointer()
	{
		return &m_Samples;
	}
	const void* SamplesPointer() const
	{
		return &m_Samples;
	}

private:
	core::Array<math::Sample<T>> m_Samples;
};

inline StrongRef<KeyFrameCurve> MakeKeyFrameCurve(core::Type type)
{
	if(type == core::Types::Float())
		return LUX_NEW(KeyFrameCurveImpl<float>());
	else if(type == core::Types::Vector2F())
		return LUX_NEW(KeyFrameCurveImpl<math::Vector2F>());
	else if(type == core::Types::Vector3F())
		return LUX_NEW(KeyFrameCurveImpl<math::Vector3F>());
	else if(type == core::Types::ColorF())
		return LUX_NEW(KeyFrameCurveImpl<video::ColorF>());
	else {
		lxAssertNeverReach("Type does not support interpolation.");
		return nullptr;
	}
}

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_ANIMATION_CURVE_H
