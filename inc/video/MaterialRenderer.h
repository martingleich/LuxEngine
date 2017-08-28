#ifndef INCLUDED_MATERIALRENDERER_H
#define INCLUDED_MATERIALRENDERER_H
#include "core/ReferenceCounted.h"
#include "core/lxString.h"
#include "video/Pass.h"
#include "video/Material.h"

namespace lux
{
namespace core
{
class ParamPackage;
}
namespace video
{
class VideoDriver;
class RenderSettings;
class Shader;

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
	LUX_API MaterialRenderer(const String& name, const MaterialRenderer* old = nullptr);
	MaterialRenderer(const MaterialRenderer&) = delete;
	MaterialRenderer& operator=(const MaterialRenderer&) = delete;
	LUX_API ~MaterialRenderer();

	LUX_API void SetParamSetCallback(ParamSetCallback* callback);
	LUX_API ParamSetCallback* GetParamSetCallback() const;
	
	LUX_API Pass GeneratePassData(size_t passId, const RenderSettings& settings) const;
	LUX_API void SendShaderSettings(size_t passId, const Pass& pass, const RenderSettings& settings) const;

	LUX_API const core::ParamPackage& GetParams() const;

	LUX_API StrongRef<Material> CreateMaterial();

	LUX_API const String& GetName() const;

	LUX_API ERequirement GetRequirements() const;
	LUX_API size_t AddPass(const Pass& pass = Pass());
	LUX_API void SetPass(size_t passId, const Pass& pass);
	LUX_API const Pass& GetPass(size_t passId) const;
	LUX_API Pass& GetPass(size_t passId);
	LUX_API size_t GetPassCount();

	LUX_API core::VariableAccess AddParam(const String& paramName, const core::Type& type);

	LUX_API core::VariableAccess SetShaderValue(u32 passId, const String& name);
	LUX_API core::VariableAccess AddShaderParam(const String& paramName, u32 passId, const String& name);
	LUX_API core::VariableAccess AddShaderParam(const String& paramName, u32 passId, u32 paramId);

	LUX_API core::VariableAccess AddParam(const String& paramName, u32 passId, EOptionId optionId);

private:
	struct ParamMapping
	{
		u32 id; // The id of the param in the package

		u32 pass; // The corresponding pass-id
		bool isShader; // Corresponds the mapping to a shader

		u32 mappingId; // The correspoding option or shaderparam id
	};

	struct ShaderValue
	{
		u32 pass;
		u32 id;
		core::AnyObject obj;

		ShaderValue(u32 p, u32 i, core::Type t) :
			pass(p),
			id(i),
			obj(t)
		{}
	};

	core::VariableAccess AddParamMapping(const core::Type& type, const String& paramName, u32 passId, u32 mappingId, bool isShader);

private:
	core::ParamPackage m_Params;
	core::Array<Pass> m_Passes;

	core::Array<ShaderValue> m_ShaderValues;

	core::Array<ParamMapping> m_Options;
	ParamSetCallback* m_ParamCallback;

	String m_Name;
};

} // namespace video

DECLARE_FLAG_CLASS(video::MaterialRenderer::ERequirement);

} // namespace lux

#endif