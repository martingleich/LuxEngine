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

class VideoDriver;

class ShaderFactory : public ReferenceCounted
{
public:
	struct ShaderCompileInfo
	{
		bool failed;
		core::Array<ShaderCompileMessage> messages;
	};

private:
	ShaderFactory();
public:
	LUX_API ~ShaderFactory();

	LUX_API static void Initialize();
	LUX_API static ShaderFactory* Instance();
	LUX_API static void Destroy();

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
		const io::Path& vsPath, core::StringView vsProfile,
		const io::Path& psPath, core::StringView psProfile,
		ShaderCompileInfo* compileInfo = nullptr);

	//! Creates a new shader from code
	/**
	\throws UnhandledShaderCompileErrorException only if the error list is null
	*/
	LUX_API StrongRef<Shader> CreateShaderFromMemory(
		EShaderLanguage language,
		core::StringView vsCode, core::StringView vsProfile,
		core::StringView psCode, core::StringView psProfile,
		ShaderCompileInfo* compileInfo = nullptr);

	//! Retrieves a shader matching a fixed function description.
	LUX_API StrongRef<Shader> GetFixedFunctionShader(const FixedFunctionParameters& params);

	//! Checks if some shader language and version is supported
	LUX_API bool IsShaderSupported(EShaderLanguage lang, core::StringView vsProfile, core::StringView psProfile);

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

		unsigned int GetHash() const
		{
			core::SequenceHasher seq;
			seq.Add(core::HashType<EShaderLanguage>()(language));
			seq.Add(core::HashType<core::String>()(name));
			return seq.GetHash();
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

	core::HashMap<ShaderInclude, core::RawMemory, core::MemberFuncHashType<ShaderInclude>> m_ShaderIncludes;
	core::Array<FixedFunctionEntry> m_FixedFunctionShaders;

	VideoDriver* m_Driver;
};

class MaterialLibrary : public ReferenceCounted
{
public:
	static LUX_API core::StringView SolidName;
	static LUX_API core::StringView DebugOverlayName;
	static LUX_API core::StringView TransparentName;

private:
	MaterialLibrary();
public:
	LUX_API ~MaterialLibrary();

	LUX_API static void Initialize();
	LUX_API static MaterialLibrary* Instance();
	LUX_API static void Destroy();

	LUX_API void SetMaterial(core::StringView name, Material* material);
	LUX_API StrongRef<video::Material> GetMaterial(core::StringView name);
	LUX_API StrongRef<video::Material> TryGetMaterial(core::StringView name);
	LUX_API StrongRef<video::Material> CloneMaterial(core::StringView name);

	LUX_API StrongRef<video::Material> CreateMaterial(const video::Pass& pass, EMaterialReqFlag reqs);
	LUX_API StrongRef<video::Material> CreateMaterial(const Material::Configuration& config);
	
	LUX_API StrongRef<video::Material> CreateSolidMaterial(video::Shader* shader);
	LUX_API StrongRef<video::Material> CreateTransparentMaterial(video::Shader* shader);

private:
	core::HashMap<core::String, int> m_MaterialMap;
	core::Array<StrongRef<Material>> m_MaterialList;

	VideoDriver* m_Driver;
	ShaderFactory* m_ShaderFactory;

	StrongRef<Shader> m_ShadowCasterShader;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_LUX_MATERIALLIBRARY_H
