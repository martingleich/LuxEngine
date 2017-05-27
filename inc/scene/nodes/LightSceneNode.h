#ifndef INCLUDED_LIGHTSCENENODE_H
#define INCLUDED_LIGHTSCENENODE_H
#include "scene/SceneNode.h"
#include "video/LightData.h"

namespace lux
{
namespace scene
{

//! Represent a light in the scenegraph
class LightSceneNode : public SceneNode
{
public:
	//! Set the light parameters
	/**
	\param light The new light settings
	*/
	virtual void SetLightData(const video::LightData& light) = 0;

	//! Get the current light parameters
	/**
	\return The current light data
	*/
	virtual const video::LightData& GetLightData() const = 0;

	//! Get the current light parameters
	/**
	\return The current light data
	*/
	virtual video::LightData& GetLightData() = 0;

	//! Set the range of the light
	/**
	See \ref video::LightData::range
	\param range The range of the light
	*/
	virtual void SetRange(float range) = 0;

	//! Get the range of the light
	/**
	See \ref video::LightData::range
	\return The range of the light
	*/
	virtual float GetRange() const = 0;

	//! Set the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\param color The new diffuse color of the light
	*/
	virtual void SetColor(const video::Colorf& color) = 0;

	//! Get the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\return The diffuse color of the light
	*/
	virtual const video::Colorf& GetColor() const = 0;

	//! Set the type of the light
	/**
	See \ref video::LightData::type
	\param type The type of the light
	*/
	virtual void SetLightType(video::LightData::EType type) = 0;

	//! Get the type of the light
	/**
	See \ref video::LightData::type
	\return The type of the light
	*/
	virtual video::LightData::EType GetLightType() const = 0;

	//! Get the orientation of the light
	/**
	Only meaningfull for directional or spotlights.\n
	To change the orientation use the scenenode method.
	\return The orientation of the light
	*/
	virtual const math::vector3f& GetDirection() = 0;
};

} // namespace scene
} // namespace lux

#endif