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
	void SetPosition(const math::vector3f& p)
	{
		m_RelativeTrans.translation = p;
		SetDirty();
	}

	void SetPosition(float x, float y, float z)
	{
		SetPosition(math::vector3f(x, y, z));
	}

	//! Translate the node in relative coordinates
	/**
	\param p The translation
	*/
	void Translate(const math::vector3f& p)
	{
		m_RelativeTrans.translation += p;
		SetDirty();
	}

	void Translate(float x, float y, float z)
	{
		Translate(math::vector3f(x, y, z));
	}

#if 0
	//! Translate the node in global coordinates
	/**
	\param p The translation
	*/
	void TranslateGlobal(const math::vector3f& p)
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
	void SetOrientation(const math::quaternionf& o)
	{
		m_RelativeTrans.orientation = o;
		SetDirty();
	}

	//! Apply a rotation to the transformable
	/**
	\param o The orientation to apply.
	*/
	void Rotate(const math::quaternionf& o)
	{
		m_RelativeTrans.orientation *= o;
		SetDirty();
	}

	void Rotate(const math::vector3f& axis, math::anglef alpha)
	{
		Rotate(math::quaternionf(axis, alpha));
	}

	void RotateX(math::anglef alpha)
	{
		Rotate(math::vector3f::UNIT_X, alpha);
	}

	void RotateY(math::anglef alpha)
	{
		Rotate(math::vector3f::UNIT_Y, alpha);
	}

	void RotateZ(math::anglef alpha)
	{
		Rotate(math::vector3f::UNIT_Z, alpha);
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
	const math::vector3f& GetPosition() const
	{
		return m_RelativeTrans.translation;
	}

	//! Get the current relative orientation
	/**
	\return The current relative orientation
	*/
	const math::quaternionf& GetOrientation() const
	{
		return m_RelativeTrans.orientation;
	}

#if 0
	//! Get the current absolute position
	/**
	\return The current absolute position
	*/
	const math::vector3f& GetAbsolutePosition() const
	{
		return GetAbsoluteTransform().translation;
	}

	//! transform a relative position to another coordinate system
	/**
	\param point The relative position
	\param target The target coordinate system, use NULL to get absolute coordinates
	\return The transformed position
	*/
	math::vector3f FromRelativePos(const math::vector3f& point, const Transformable* target = nullptr)
	{
		math::vector3f out = GetAbsoluteTransform().TransformPoint(point);
		if(target) {
			out = target->GetAbsoluteTransform().TransformInvPoint(out);
		}

		return out;
	}

	//! Transforms a position to relative coordinates
	/**
	\param point The position to transform
	\param source The source coordinate system, use NULL for absolute coordinates
	\return The positon in relative coordinates
	*/
	math::vector3f ToRelativePos(const math::vector3f& point, const Transformable* source = nullptr)
	{
		math::vector3f out;
		if(source)
			out = source->GetAbsoluteTransform().TransformPoint(point);
		else
			out = point;

		return GetAbsoluteTransform().TransformInvPoint(out);
	}
	//! Transforms a relative direction to another coordinate system
	/**
	\param Dir The direction to transform
	\param target The target coordinate system, use NULL for absolute coordinates
	\return The transformed direction
	*/
	math::vector3f FromRelativeDir(const math::vector3f& Dir, const Transformable* target = nullptr)
	{
		math::vector3f out = GetAbsoluteTransform().TransformDir(Dir);
		if(target) {
			out = target->GetAbsoluteTransform().TransformInvDir(out);
		}

		return out;
	}

	//! Transforms a direction to relative coordinates
	/**
	\param dir The direction to transform
	\param source The source coordinate system, use NULL for absolute coordinates
	\return The direction in relative coordinates
	*/
	math::vector3f ToRelativeDir(const math::vector3f& dir, const Transformable* source = nullptr)
	{
		math::vector3f out;
		if(source)
			out = source->GetAbsoluteTransform().TransformDir(dir);
		else
			out = dir;

		return GetAbsoluteTransform().TransformInvDir(out);
	}
#endif

	//! Set the looking direction of the transfomable
	/**
	Rotates the transforable to move a local vector in direction of another vector
	\param dir The direction in which the local direction should point
	\param local The local vector which should used as pointer
	*/
	void SetDirection(const math::vector3f& dir, const math::vector3f& local = math::vector3f::UNIT_Z)
	{
		m_RelativeTrans.orientation = math::quaternionf::FromTo(local, dir);
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
	void SetDirectionUp(const math::vector3f& dir,
		const math::vector3f& up = math::vector3f::UNIT_Y,
		const math::vector3f& local_dir = math::vector3f::UNIT_Z,
		const math::vector3f& local_up = math::vector3f::UNIT_Y)
	{
		m_RelativeTrans.orientation = math::quaternionf::FromTo(
			local_dir, local_up,
			dir, up);
		SetDirty();
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

#endif // !INCLUDED_ITRANSFORMABLE_H
