#ifndef INCLUDED_MATERIALLIBRARY_H
#define INCLUDED_MATERIALLIBRARY_H
#include "video/Material.h"
#include "EShaderTypes.h"
#include "video/Shader.h"

namespace lux
{
namespace video
{

//! The material library
/**
Caches materialsrenders and is used to created new ones.
*/
class MaterialLibrary : public ReferenceCounted
{
public:
	static const size_t INVALID_ID = 0xFFFFFFFF;

public:
	virtual ~MaterialLibrary()
	{
	}

	//! Add a new material renderer
	/**
	\param renderer The material renderer
	\param name The name of the material renderer
	\return The index of the new renderer, or INVALID_ID if an error occured
	*/
	virtual size_t AddMaterialRenderer(MaterialRenderer* renderer, const string& name) = 0;

	//! Clone an old material renderer
	/**
	\param name The name of the new renderer
	\param oldName The name of the copied renderer
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const string& oldName) = 0;

	//! Clone an old material renderer
	/**
	\param name The name of the new renderer
	\param old The copied material renderer
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const MaterialRenderer* old) = 0;

	//! Add a shader material renderer
	/**
	\param VSPath The path of the vertex shader
	\param VSEntryPoint The name of the vertex shader entry point, if empty defaults to mainVS
	\param VSType The type of the vertex shader
	\param PSPath The path of the pixel shader
	\param PSEntryPoint The name of the pixel shader entry point, if empty defaults to mainPS
	\param PSType The type of the pixel shader
	\param baseMaterial The base rendersetting used for the shader
	\param name The name of the new material renderer
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> AddShaderMaterialRenderer(
		const io::path& VSPath, const string& VSEntryPoint, video::EVertexShaderType VSType,
		const io::path& PSPath, const string& PSEntryPoint, video::EPixelShaderType PSType,
		const MaterialRenderer* baseMaterial, const string& name) = 0;

	//! Add a shader material renderer
	/**
	\param shader The shader to use for the material renderer.
	\param baseMaterial The base rendersetting used for the shader
	\param name The name of the new material renderer
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> AddShaderMaterialRenderer(
		Shader* shader,
		const MaterialRenderer* baseMaterial, const string& name) = 0;

	//! Returns a material renderer by its index
	virtual StrongRef<MaterialRenderer> GetMaterialRenderer(size_t index) const = 0;

	//! Returns a material renderer by its name
	virtual StrongRef<MaterialRenderer> GetMaterialRenderer(const string& name) const = 0;

	//! Returns the id of a material renderer
	virtual size_t GetRendererID(MaterialRenderer* renderer) const = 0;

	//! Returns the name of a material renderer
	virtual const string& GetRendererName(MaterialRenderer* renderer) const = 0;

	//! Returns the total number of material renderers
	virtual size_t GetMaterialRendererCount() const = 0;
};

}
}

#endif // INCLUDED_MATERIALLIBRARY_H
