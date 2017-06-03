#ifndef INCLUDED_MATERIALRENDERER_H
#define INCLUDED_MATERIALRENDERER_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace core
{
class ParamPackage;
}
namespace video
{
class VideoDriver;
class Material;
class RenderSettings;
class Shader;
class PipelineSettings;
class DeviceState;

//! A Materialrenderer
/**
Material rendered communicate the data inside a material to the driver.
*/
class MaterialRenderer : public ReferenceCounted
{
public:
	//! Requierments of a material renderer(flag class)
	enum class ERequirement
	{
		None = 0,
		Transparent = 1,
	};

public:
	virtual ~MaterialRenderer() {}

	// Wird von der Video-Schnittstelle aufgerufen und lässt den renderer seine Renderstates setzen
	// Wird während VideoDriver::SetMaterial aufgerufen
	//! Enable a material
	/**
	\param material The material to enable
	*/
	virtual void Begin(const RenderSettings& settings, DeviceState& state) = 0;

	// Wird aufgerufen, wenn das material nicht mehr gebraucht wird
	// Wird in VideoDriver::SetMaterial aufgerufen wenn ein material ersetzt wird
	//! Will be called when a material is disabled
	/**
	You only should reset renderstates which no other MaterialRenders uses.
	The other will be set in OnSetMaterial
	*/
	virtual void End(DeviceState& state) = 0;

	//! Get the requirements of this renderer
	virtual ERequirement GetRequirements() const = 0;

	//! Clones the material renderer
	virtual StrongRef<MaterialRenderer> Clone(const string& newName, Shader* shader = nullptr, const core::ParamPackage* basePackage = nullptr) const = 0;

	virtual StrongRef<Material> CreateMaterial() = 0;

	virtual const PipelineSettings& GetPipeline() const = 0;
	virtual void SetPipeline(const PipelineSettings& set) = 0;

	virtual const core::ParamPackage& GetPackage() const = 0;
	virtual core::ParamPackage& GetPackage() = 0;
	virtual StrongRef<Shader> GetShader() const = 0;

	virtual const string& GetName() const = 0;
};

} // namespace video

DECLARE_FLAG_CLASS(video::MaterialRenderer::ERequirement);

} // namespace lux

#endif