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
	LUX_API void SetLightData(const video::LightData& light);

	//! Get the current light parameters
	/**
	\return The current light data
	*/
	LUX_API const video::LightData& GetLightData() const;

	//! Get the current light parameters
	/**
	\return The current light data
	*/
	LUX_API video::LightData& GetLightData();

	//! Set the range of the light
	/**
	See \ref video::LightData::range
	\param range The range of the light
	*/
	LUX_API void SetRange(float range);

	//! Get the range of the light
	/**
	See \ref video::LightData::range
	\return The range of the light
	*/
	LUX_API float GetRange() const;

	//! Set the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\param color The new diffuse color of the light
	*/
	LUX_API void SetColor(const video::Colorf& color);

	//! Get the diffuse color of the light
	/**
	See \ref video::LightData::diffuse
	\return The diffuse color of the light
	*/
	LUX_API const video::Colorf& GetColor() const;

	//! Set the type of the light
	/**
	See \ref video::LightData::type
	\param type The type of the light
	*/
	LUX_API void SetLightType(video::LightData::EType type);

	//! Get the type of the light
	/**
	See \ref video::LightData::type
	\return The type of the light
	*/
	LUX_API video::LightData::EType GetLightType() const;

	//! Get the orientation of the light
	/**
	Only meaningfull for directional or spotlights.\n
	To change the orientation use the scenenode method.
	\return The orientation of the light
	*/
	LUX_API const math::vector3f& GetDirection();

	void OnRegisterSceneNode();
	void Render();

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	video::LightData m_LightData;
};

} // namespace scene
} // namespace lux

#endif