#ifndef INCLUDED_MATERIALRENDERER_H
#define INCLUDED_MATERIALRENDERER_H
#include "core/ReferenceCounted.h"
#include "video/VertexTypes.h"
#include "video/PipelineSettings.h"
#include "video/RenderType.h"

namespace lux
{
namespace video
{

class VideoDriver;
class Material;

//! A Materialrenderer
/**
Material rendered communicate the data inside a material to the driver.
*/
class MaterialRenderer : public ReferenceCounted, public RenderType
{
public:
	MaterialRenderer(video::VideoDriver* driver, Shader* shader, const core::ParamPackage* basePackage) : RenderType(driver, shader, basePackage)
	{
	}

	MaterialRenderer(const MaterialRenderer& other) : RenderType(other)
	{
	}

	virtual ~MaterialRenderer() {}

	// Wird von der Video-Schnittstelle aufgerufen und lässt den renderer seine Renderstates setzen
	// Wird während VideoDriver::SetMaterial aufgerufen
	//! Enable a material
	/**
	\param material The material to enable
	\param lastMaterial The material which is currently active
	\param resetAll Should all pipeline changes be done, regardless if the lastMaterial has already set them
	*/
	virtual void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false) = 0;

	//! Will be called each time after geometry is rendered
	/**
	\param pass The next renderpass
	\return Return false to abort the rendering
	*/
	virtual bool OnRender(u32 pass)
	{
		if(pass == 0)
			return true;
		else
			return false;
	}

	// Wird aufgerufen, wenn das material nicht mehr gebraucht wird
	// Wird in VideoDriver::SetMaterial aufgerufen wenn ein material ersetzt wird
	//! Will be called when a material is disabled
	/**
	You only should reset renderstates which no other MaterialRenders uses.
	The other will be set in OnSetMaterial
	*/
	virtual void OnUnsetMaterial()
	{
		this->Disable();
	}

	//! Are objects rendered with this method transparent
	virtual bool IsTransparent() const
	{
		return false;
	}

	//! Clones the material renderer
	virtual MaterialRenderer* Clone(Shader* shader = nullptr, const core::ParamPackage* basePackage = nullptr) const
	{
		LUX_UNUSED(shader);
		LUX_UNUSED(basePackage);
		return nullptr;
	}
};

}    

}    


#endif