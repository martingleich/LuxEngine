#ifndef INCLUDED_LUX_MATERIALLIBRARY_H
#define INCLUDED_LUX_MATERIALLIBRARY_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "core/lxArray.h"
#include "core/lxOrderedMap.h"

#include "video/Material.h"
#include "video/Shader.h"
#include "video/FixedFunctionShader.h"
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
	enum EKnownMaterial
	{
		Solid,
		DebugOverlay,
		Transparent,
	};

	LUX_API MaterialLibrary();
	LUX_API virtual ~MaterialLibrary();

	LUX_API static void Initialize(MaterialLibrary* library = nullptr);
	LUX_API static MaterialLibrary* Instance();
	LUX_API static void Destroy();

	LUX_API void SetMaterial(core::StringView name, Material* material);
	LUX_API StrongRef<video::Material> GetMaterial(core::StringView name);
	LUX_API StrongRef<video::Material> TryGetMaterial(core::StringView name);
	LUX_API StrongRef<video::Material> CloneMaterial(core::StringView name);
	LUX_API StrongRef<video::Material> CreateMaterial(video::Pass pass, EMaterialReqFlag reqs = EMaterialReqFlag::None);
	LUX_API StrongRef<video::Material> CreateSolidMaterial(video::Shader* shader);
	LUX_API StrongRef<video::Material> CreateTransparentMaterial(video::Shader* shader);

	LUX_API void SetMaterial(EKnownMaterial name, Material* material);
	LUX_API StrongRef<video::Material> GetMaterial(EKnownMaterial name);
	LUX_API StrongRef<video::Material> CloneMaterial(EKnownMaterial name);

	struct ShaderCompileInfo
	{
		bool failed;
		core::Array<core::String> messages;
	};
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
	\throws UnhandledShaderCompileErrorException only if the error list if null
	\throws FileNotFoundException
	*/
	LUX_API StrongRef<Shader> CreateShaderFromFile(
		video::EShaderLanguage language,
		const io::Path& VSPath, core::StringView VSEntryPoint,
		int VSMajor, int VSMinor,
		const io::Path& PSPath, core::StringView PSEntryPoint,
		int PSMajor, int PSMinor,
		ShaderCompileInfo* compileInfo = nullptr);

	//! Creates a new shader from code
	/**
	\throws UnhandledShaderCompileErrorException only if the error list is null
	*/
	LUX_API StrongRef<Shader> CreateShaderFromMemory(
		EShaderLanguage language,
		core::StringView VSCode, core::StringView VSEntryPoint,
		int VSmajorVersion, int VSminorVersion,
		core::StringView PSCode, core::StringView PSEntryPoint,
		int PSmajorVersion, int PSminorVersion,
		ShaderCompileInfo* compileInfo = nullptr);

	//! Retrieves a shader matching a fixed function description.
	LUX_API StrongRef<Shader> GetFixedFunctionShader(
		const core::Array<core::String>& textures,
		const core::Array<TextureStageSettings>& stages,
		bool useVertexColors = false);

	//! Retrieves a shader matching a fixed function description.
	LUX_API StrongRef<Shader> GetFixedFunctionShader(const FixedFunctionParameters& params);

	//! Checks if some shader language and version is supported
	LUX_API bool IsShaderSupported(
		EShaderLanguage lang,
		int vsMajor, int vsMinor,
		int psMajor, int psMinor);

	LUX_API bool GetShaderInclude(
		EShaderLanguage language, core::StringView name,
		core::StringView& outData);
	LUX_API void SetShaderInclude(
		EShaderLanguage language, core::StringView name,
		core::StringView data);

private:
	struct ShaderInclude
	{
		EShaderLanguage language;
		core::String name;

		ShaderInclude(EShaderLanguage& lang, const core::String& n) :
			language(lang),
			name(n)
		{
		}

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
	struct FixedFunctionEntry
	{
		FixedFunctionEntry(
			const FixedFunctionParameters& _params,
			Shader* _shader) :
			params(_params),
			shader(_shader)
		{}
		FixedFunctionParameters params;
		StrongRef<Shader> shader;
	};

	core::OrderedMap<ShaderInclude, core::RawMemory> m_ShaderIncludes;
	core::HashMap<core::String, int> m_MaterialMap;
	core::Array<StrongRef<Material>> m_MaterialList;

	core::Array<FixedFunctionEntry> m_FixedFunctionShaders;

	VideoDriver* m_Driver;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_LUX_MATERIALLIBRARY_H
