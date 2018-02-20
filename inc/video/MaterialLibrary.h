#ifndef INCLUDED_MATERIALLIBRARY_H
#define INCLUDED_MATERIALLIBRARY_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "core/lxOrderedMap.h"

#include "video/Material.h"
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

	LUX_API void SetMaterial(const core::String& name, Material* material);
	LUX_API StrongRef<video::Material> GetMaterial(const core::String& name);
	LUX_API StrongRef<video::Material> CloneMaterial(const core::String& name);

	//! Create a shader from file
	/**
	\param VSPath The path of the vertex shader
	\param VSEntryPoint The name of the vertex shader entry point, if empty defaults to mainVS
	\param VSType The type of the vertex shader
	\param PSPath The path of the pixel shader
	\param PSEntryPoint The name of the pixel shader entry point, if empty defaults to mainPS
	\param PSType The type of the pixel shader
	\param [out] errorList If not null, here a list of all errors/warning
		while creating the shader is written.
	\return The new shader
	\throws ShaderCompileException
	\throws FileNotFoundException
	*/
	LUX_API StrongRef<Shader> CreateShaderFromFile(
		video::EShaderLanguage language,
		const io::Path& VSPath, const core::String& VSEntryPoint,
		int VSMajor, int VSMinor,
		const io::Path& PSPath, const core::String& PSEntryPoint,
		int PSMajor, int PSMinor,
		core::Array<core::String>* errorList = nullptr);

	//! Creates a new shader from code
	/**
	\throws ShaderCompileException
	*/
	LUX_API StrongRef<Shader> CreateShaderFromMemory(
		EShaderLanguage language,
		const core::String& VSCode, const char* VSEntryPoint,
		int VSmajorVersion, int VSminorVersion,
		const core::String& PSCode, const char* PSEntryPoint,
		int PSmajorVersion, int PSminorVersion,
		core::Array<core::String>* errorList=nullptr);

	//! Checks if some shader language and version is supported
	LUX_API bool IsShaderSupported(
		EShaderLanguage lang,
		int vsMajor, int vsMinor,
		int psMajor, int psMinor);

	LUX_API bool GetShaderInclude(
		EShaderLanguage language, const core::String& name,
		const void*& outData, size_t& outBytes);
	LUX_API void SetShaderInclude(
		EShaderLanguage language, const core::String& name,
		const void* data, size_t bytes);

private:
	struct ShaderInclude
	{
		EShaderLanguage language;
		core::String name;

		ShaderInclude(EShaderLanguage& lang, const core::String& n) :
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
	core::HashMap<core::String, StrongRef<Material>> m_BaseMaterials;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_MATERIALLIBRARY_H
