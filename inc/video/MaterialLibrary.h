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
	virtual ~MaterialLibrary()
	{
	}

	//! Add a new material renderer
	/**
	\param renderer The material renderer
	\param name The name of the material renderer
	\return The index of the new renderer.
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
	\param [out] errorList If not null, here a list of all errors/warning while creating the shader is written.
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> AddShaderMaterialRenderer(
		video::EShaderLanguage language,
		const io::path& VSPath, const string& VSEntryPoint, int VSMajor, int VSMinor,
		const io::path& PSPath, const string& PSEntryPoint, int PSMajor, int PSMinor,
		const MaterialRenderer* baseMaterial, const string& name,
		core::array<string>* errorList=nullptr) = 0;

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
