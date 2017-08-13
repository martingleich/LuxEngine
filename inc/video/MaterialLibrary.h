#ifndef INCLUDED_MATERIALLIBRARY_H
#define INCLUDED_MATERIALLIBRARY_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "core/lxOrderedMap.h"

#include "video/MaterialRenderer.h"
#include "video/Shader.h"
#include "video/VideoEnums.h"

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
	LUX_API MaterialLibrary();
	LUX_API virtual ~MaterialLibrary();

	LUX_API static void Initialize(MaterialLibrary* library = nullptr);
	LUX_API static MaterialLibrary* Instance();
	LUX_API static void Destroy();

	//! Create a new material
	/**
	\param renderer The material renderer of the new material, if null is passed the solid renderer is used
	*/
	LUX_API StrongRef<Material> CreateMaterial(MaterialRenderer* renderer = nullptr);

	//! Create a new material
	/**
	\param rendererName The name of the material renderer
	*/
	LUX_API StrongRef<Material> CreateMaterial(const String& rendererName);

	//! Add a new material renderer
	/**
	\param renderer The material renderer
	\param name The name of the material renderer
	*/
	LUX_API StrongRef<MaterialRenderer> AddMaterialRenderer(MaterialRenderer* renderer);
	LUX_API StrongRef<MaterialRenderer> ReplaceMaterialRenderer(const String& newName);

	//! Add a new material renderer
	/**
	\param name The name of the material renderer
	*/
	LUX_API StrongRef<MaterialRenderer> AddMaterialRenderer(const String& newName);

	//! Clone an old material renderer
	/**
	\param newName The name of the new renderer
	\param baseName The name of the copied renderer
	\return The new material renderer
	*/
	LUX_API StrongRef<MaterialRenderer> CloneMaterialRenderer(const String& newName, const String& baseName);

	//! Clone an old material renderer
	/**
	\param name The name of the new renderer
	\param old The copied material renderer
	\return The new material renderer
	*/
	LUX_API StrongRef<MaterialRenderer> CloneMaterialRenderer(const String& name, const MaterialRenderer* old);

	//! Removes a material renderer from the scene graph
	/**
	The user should only remove material renderer which he created by himself.
	All materials using this renderer will become invalid materials.
	*/
	LUX_API void RemoveMaterialRenderer(MaterialRenderer* renderer);

	//! Returns a material renderer by its index
	LUX_API StrongRef<MaterialRenderer> GetMaterialRenderer(size_t index) const;

	//! Returns a material renderer by its name
	LUX_API StrongRef<MaterialRenderer> GetMaterialRenderer(const String& name) const;

	//! Check if a material renderer exists
	LUX_API bool ExistsMaterialRenderer(const String& name, MaterialRenderer** outRenderer=nullptr) const;

	//! Returns the total number of material renderers
	LUX_API size_t GetMaterialRendererCount() const;

	//! Create a shader from file
	/**
	\param VSPath The path of the vertex shader
	\param VSEntryPoint The name of the vertex shader entry point, if empty defaults to mainVS
	\param VSType The type of the vertex shader
	\param PSPath The path of the pixel shader
	\param PSEntryPoint The name of the pixel shader entry point, if empty defaults to mainPS
	\param PSType The type of the pixel shader
	\param [out] errorList If not null, here a list of all errors/warning while creating the shader is written.
	\return The new shader
	\throws ShaderCompileException
	\throws FileNotFoundException
	*/
	LUX_API StrongRef<Shader> CreateShaderFromFile(
		video::EShaderLanguage language,
		const io::Path& VSPath, const String& VSEntryPoint, int VSMajor, int VSMinor,
		const io::Path& PSPath, const String& PSEntryPoint, int PSMajor, int PSMinor,
		core::Array<String>* errorList = nullptr);

	//! Creates a new shader from code
	/**
	\throws ShaderCompileException
	*/
	LUX_API StrongRef<Shader> CreateShaderFromMemory(
		EShaderLanguage language,
		const String& VSCode, const char* VSEntryPoint, int VSmajorVersion, int VSminorVersion,
		const String& PSCode, const char* PSEntryPoint, int PSmajorVersion, int PSminorVersion,
		core::Array<String>* errorList=nullptr);

	LUX_API bool GetShaderInclude(EShaderLanguage language, const String& name, const void*& outData, size_t& outBytes);
	LUX_API void SetShaderInclude(EShaderLanguage language, const String& name, const void* data, size_t bytes);

private:
	bool FindRenderer(const String& name, size_t& id) const;

private:
	core::Array<StrongRef<MaterialRenderer>> m_Renderers;

	struct ShaderInclude
	{
		EShaderLanguage language;
		String name;

		ShaderInclude(EShaderLanguage& lang, const String& n) :
			language(lang),
			name(n)
		{}

		bool operator<(const ShaderInclude& other) const
		{
			if(language != other.language)
				return language < other.language;
			else
				return name < other.name;
		}

		bool operator==(const ShaderInclude& other) const
		{
			return language == other.language && name == other.name;
		}
	};

	core::OrderedMap<ShaderInclude, core::RawMemory> m_ShaderIncludes;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_MATERIALLIBRARY_H
