#ifndef INCLUDED_TRANSFORMABLE_H
#define INCLUDED_TRANSFORMABLE_H
#include "math/Transformation.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace scene
{

//! Any transformable object
class Transformable
{
public:
	Transformable() :
		m_IsDirty(true)
	{}

	//! Set a new uniform scalation
	/**
	\param s The the scale
	*/
	void SetScale(float s)
	{
		m_RelativeTrans.scale = s;
		SetDirty();
	}

	//! Change the scale
	/**
	\param s The change of the scale
	*/
	void Scale(float s)
	{
		m_RelativeTrans.scale *= s;
		SetDirty();
	}

	//! Set a new position
	/**
	\param p The new position
	*/
	void SetPosition(const math::Vector3F& p)
	{
		m_RelativeTrans.translation = p;
		SetDirty();
	}

	void SetPosition(float x, float y, float z)
	{
		SetPosition(math::Vector3F(x, y, z));
	}

	//! Translate the node in relative coordinates
	/**
	\param p The translation
	*/
	void Translate(const math::Vector3F& p)
	{
		m_RelativeTrans.translation += p;
		SetDirty();
	}

	void Translate(float x, float y, float z)
	{
		Translate(math::Vector3F(x, y, z));
	}

#if 0
	//! Translate the node in global coordinates
	/**
	\param p The translation
	*/
	void TranslateGlobal(const math::Vector3F& p)
	{
		math::Transformation t = GetRelativeTransform();
		t.translation += t.TransformDir(GetAbsoluteTransform().TransformInvDir(p));
		SetRelativeTransform(t);
	}
#endif

	//! Set a new orientation
	/**
	\param o The new orientation
	*/
	void SetOrientation(const math::QuaternionF& o)
	{
		m_RelativeTrans.orientation = o;
		SetDirty();
	}

	//! Apply a rotation to the transformable
	/**
	\param o The orientation to apply.
	*/
	void Rotate(const math::QuaternionF& o)
	{
		m_RelativeTrans.orientation *= o;
		SetDirty();
	}

	void Rotate(const math::Vector3F& axis, math::AngleF alpha)
	{
		Rotate(math::QuaternionF(axis, alpha));
	}

	void RotateX(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_X, alpha);
	}

	void RotateY(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_Y, alpha);
	}

	void RotateZ(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_Z, alpha);
	}

	//! Get the current relative scale
	/**
	\return The current relative scale
	*/
	float GetScale() const
	{
		return m_RelativeTrans.scale;
	}

	//! Get the current relative position
	/**
	\return The current relative position
	*/
	const math::Vector3F& GetPosition() const
	{
		return m_RelativeTrans.translation;
	}

	//! Get the current relative orientation
	/**
	\return The current relative orientation
	*/
	const math::QuaternionF& GetOrientation() const
	{
		return m_RelativeTrans.orientation;
	}

	//! Set the looking direction of the transfomable
	/**
	Rotates the transforable to move a local vector in direction of another vector
	\param dir The direction in which the local direction should point
	\param local The local vector which should used as pointer
	*/
	void SetDirection(const math::Vector3F& dir, const math::Vector3F& local = math::Vector3F::UNIT_Z)
	{
		m_RelativeTrans.orientation = math::QuaternionF::FromTo(local, dir);
		SetDirty();
	}

	//! Set the looking directon of the transformable
	/**
	local_dir and local_up must be orthogonal.
	\param dir The new direction of the local dir axis
	\param up The new direction of the local up axis
	\param local_dir The local direction vector
	\param local_up The local up vector
	*/
	void SetDirectionUp(const math::Vector3F& dir,
		const math::Vector3F& up = math::Vector3F::UNIT_Y,
		const math::Vector3F& local_dir = math::Vector3F::UNIT_Z,
		const math::Vector3F& local_up = math::Vector3F::UNIT_Y)
	{
		m_RelativeTrans.orientation = math::QuaternionF::FromTo(
			local_dir, local_up,
			dir, up);
		SetDirty();
	}

	void LookAt(const math::Vector3F& pos,
		const math::Vector3F& up = math::Vector3F::UNIT_Y,
		const math::Vector3F& local_dir = math::Vector3F::UNIT_Z,
		const math::Vector3F& local_up = math::Vector3F::UNIT_Y)
	{
		SetDirectionUp(pos-GetPosition(), up, local_dir, local_up);
	}

protected:
	bool IsDirty() const
	{
		return m_IsDirty;
	}

	virtual void SetDirty() const
	{
		m_IsDirty = true;
	}

	void ClearDirty() const
	{
		m_IsDirty = false;
	}

protected:
	math::Transformation m_RelativeTrans;
	mutable bool m_IsDirty;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_TRANSFORMABLE_H
