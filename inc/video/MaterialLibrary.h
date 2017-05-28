#ifndef INCLUDED_MATERIALLIBRARY_H
#define INCLUDED_MATERIALLIBRARY_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "EShaderTypes.h"

namespace lux
{
namespace video
{
class Shader;
class Material;
class MaterialRenderer;

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

	//! Create a new material
	/**
	\param renderer The material renderer of the new material, if null is passed the solid renderer is used
	*/
	virtual StrongRef<Material> CreateMaterial(MaterialRenderer* renderer = nullptr) = 0;

	//! Create a new material
	/**
	\param rendererName The name of the material renderer
	*/
	virtual StrongRef<Material> CreateMaterial(const string& rendererName) = 0;

	//! Add a new material renderer
	/**
	\param renderer The material renderer
	\param name The name of the material renderer
	*/
	virtual StrongRef<MaterialRenderer> AddMaterialRenderer(MaterialRenderer* renderer) = 0;

	//! Clone an old material renderer
	/**
	\param newName The name of the new renderer
	\param baseName The name of the copied renderer
	\return The new material renderer
	*/
	virtual StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& newName, const string& baseName) = 0;

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
	\param baseMaterial The base rendersetting used for the shader, must not have a shader itself(use solid_base or transparent_base)
	\param name The name of the new material renderer
	\param [out] errorList If not null, here a list of all errors/warning while creating the shader is written.
	\return The new material renderer
	\throws ShaderCompileException
	\throws FileNotFoundException
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

	//! Removes a material renderer from the scene graph
	/**
	The user should only remove material renderer which he created by himself.
	All materials using this renderer will become invalid materials.
	*/
	virtual void RemoveMaterialRenderer(MaterialRenderer* renderer) = 0;

	//! Returns a material renderer by its index
	virtual StrongRef<MaterialRenderer> GetMaterialRenderer(size_t index) const = 0;

	//! Returns a material renderer by its name
	virtual StrongRef<MaterialRenderer> GetMaterialRenderer(const string& name) const = 0;

	//! Returns the total number of material renderers
	virtual size_t GetMaterialRendererCount() const = 0;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_MATERIALLIBRARY_H
