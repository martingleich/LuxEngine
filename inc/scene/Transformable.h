#ifndef INCLUDED_TRANSFORMABLE_H
#define INCLUDED_TRANSFORMABLE_H
#include "math/Transformation.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace scene
{

//! Any transformable object, placed in a hierachy.
class Transformable : public virtual ReferenceCounted
{
public:
	//! Called when a new relative transformation must be set
	/**
	\param t The new relative transform
	*/
	virtual void SetRelativeTransform(const math::Transformation& t) = 0;
	//! Called to retrieve the relative transform
	/**
	\return The current relative transform
	*/
	virtual const math::Transformation& GetRelativeTransform() const = 0;
	
	//! Called to retrieve the absolute transform
	/**
	\return The current absolute transform
	*/
	virtual const math::Transformation& GetAbsoluteTransform() const = 0;

	//! Set a new uniform scalation
	/**
	\param s The the scale
	*/
	void SetScale(float s)
	{
		math::Transformation t = GetRelativeTransform();
		t.scale = s;
		SetRelativeTransform(t);
	}

	//! Change the scale
	/** 
	\param s The change of the scale
	*/
	void Scale(float s)
	{
		math::Transformation t = GetRelativeTransform();
		t.scale *= s;
		SetRelativeTransform(t);
	}

	//! Set a new position
	/**
	\param p The new position
	*/
	void SetPosition(const math::vector3f& p)
	{
		math::Transformation t = GetRelativeTransform();
		t.translation = p;
		SetRelativeTransform(t);
	}

	//! Translate the node in relative coordinates
	/**
	\param p The translation
	*/
	void Translate(const math::vector3f& p)
	{
		math::Transformation t = GetRelativeTransform();
		t.translation += p;
		SetRelativeTransform(t);
	}

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

	//! Set a new orientation
	/**
	\param o The new orientation
	*/
	void SetOrientation(const math::quaternionf& o)
	{
		math::Transformation t = GetRelativeTransform();
		t.orientation = o;
		SetRelativeTransform(t);
	}

	//! Apply a rotation to the transformable
	/**
	\param o The orientation to apply.
	*/
	void Rotate(const math::quaternionf& o)
	{
		math::Transformation t = GetRelativeTransform();
		t.orientation *= o;
		SetRelativeTransform(t);
	}

	//! Get the current relative scale
	/**
	\return The current relative scale
	*/
	float GetScale() const
	{
		return GetRelativeTransform().scale;
	}

	//! Get the current relative position
	/**
	\return The current relative position
	*/
	const math::vector3f& GetPosition() const
	{
		return GetRelativeTransform().translation;
	}

	//! Get the current relative orientation
	/**
	\return The current relative orientation
	*/
	const math::quaternionf& GetOrientation() const
	{
		return GetRelativeTransform().orientation;
	}

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

	//! Set the looking direction of the transfomable
	/**
	Rotates the transforable to move a local vector in direction of another vector
	\param dir The direction in which the local direction should point
	\param local The local vector which should used as pointer
	*/
	void SetDirection(const math::vector3f& dir, const math::vector3f& local = math::vector3f::UNIT_Z)
	{
		math::Transformation t = GetRelativeTransform();
		t.orientation = math::quaternionf::FromTo(local, dir);
		SetRelativeTransform(t);
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
		math::Transformation t = GetRelativeTransform();
		t.orientation = math::quaternionf::FromTo(
			local_dir, local_up,
			dir, up);
		SetRelativeTransform(t);
	}
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_ITRANSFORMABLE_H
