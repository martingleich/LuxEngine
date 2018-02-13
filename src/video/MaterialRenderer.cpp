#include "video/MaterialRenderer.h"
#include "video/RenderSettings.h"
#include "video/MaterialImpl.h"

namespace lux
{
namespace video
{

MaterialRenderer::MaterialRenderer(const core::String& name, const MaterialRenderer* old) :
	m_ParamCallback(nullptr),
	m_Name(name)
{
	if(old)
		m_Passes = old->m_Passes;
	else
		AddPass();
}

MaterialRenderer::~MaterialRenderer()
{
}

void MaterialRenderer::SetParamSetCallback(ParamSetCallback* callback)
{
	m_ParamCallback = callback;
}

ParamSetCallback* MaterialRenderer::GetParamSetCallback() const
{
	return m_ParamCallback;
}

Pass MaterialRenderer::GeneratePassData(size_t passId, const Material* material) const
{
	auto& pass = m_Passes.At(passId);

	Pass out(pass);
	out.ambient = material->GetAmbient();
	out.diffuse = material->GetDiffuse();
	out.emissive = material->GetEmissive();
	out.specular = material->GetSpecular()* material->GetPower();
	out.shininess = material->GetShininess();

	for(auto& x : m_Options) {
		if(x.pass == passId && !x.isShader) {
			auto param = material->Param(x.id);
			out.SetOption(x.mappingId, param.Pointer());
		}
	}

	return out;
}

void MaterialRenderer::SendShaderSettings(size_t passId, const Pass& pass, const Material* material) const
{
	if(passId >= m_Passes.Size())
		throw core::InvalidArgumentException("passId");

	if(!pass.shader)
		return;

	for(const auto& option : m_Options) {
		if(option.isShader) {
			auto param = material->Param(option.id);
			pass.shader->SetParam(param.Pointer(), option.mappingId);
		}
	}

	for(const auto& v : m_ShaderValues) {
		if(v.pass == passId)
			pass.shader->SetParam(v.obj.Data(), v.id);
	}

	pass.shader->LoadSceneParams(pass);

	if(m_ParamCallback)
		m_ParamCallback->SendShaderSettings(passId, pass, material);
}

const core::ParamPackage& MaterialRenderer::GetParams() const
{
	return m_Params;
}

StrongRef<Material> MaterialRenderer::CreateMaterial()
{
	return LUX_NEW(MaterialImpl)(this);
}

const core::String& MaterialRenderer::GetName() const
{
	return m_Name;
}

EMaterialRequirement MaterialRenderer::GetRequirements() const
{
	EMaterialRequirement r = EMaterialRequirement::None;
	for(auto& pass : m_Passes)
		r |= pass.requirements;

	return r;
}

size_t MaterialRenderer::AddPass(const Pass& pass)
{
	m_Passes.PushBack(pass);
	return m_Passes.Size() - 1;
}

void MaterialRenderer::SetPass(size_t passId, const Pass& pass)
{
	m_Passes.At(passId) = pass;
}

const Pass& MaterialRenderer::GetPass(size_t passId) const
{
	return m_Passes.At(passId);
}

Pass& MaterialRenderer::GetPass(size_t passId)
{
	return m_Passes.At(passId);
}

size_t MaterialRenderer::GetPassCount()
{
	return m_Passes.Size();
}

core::VariableAccess MaterialRenderer::AddParam(const core::String& paramName, const core::Type& type)
{
	u32 id = m_Params.AddParam(type, paramName);
	return m_Params.DefaultValue(id);
}

core::VariableAccess MaterialRenderer::SetShaderValue(u32 passId, const core::String& name)
{
	auto& pass = m_Passes.At(passId);
	auto shader = pass.shader;
	if(!shader)
		throw core::InvalidArgumentException("passId", "Pass contains no shader");

	u32 id = shader->GetParamPackage().GetParamId(name);
	auto desc = shader->GetParamPackage().GetParamDesc(id);

	for(auto& v : m_ShaderValues) {
		if(v.pass == passId && v.id == id)
			return core::VariableAccess(desc.type, v.obj.Data());
	}

	m_ShaderValues.PushBack(ShaderValue(passId, id, desc.type));
	return core::VariableAccess(desc.type, m_ShaderValues.Back().obj.Data());
}

core::VariableAccess MaterialRenderer::AddShaderParam(const core::String& paramName, u32 passId, const core::String& name)
{
	auto& pass = m_Passes.At(passId);
	if(!pass.shader)
		throw core::InvalidArgumentException("name", "Is not a valid shader parameter");

	u32 id = pass.shader->GetParamPackage().GetParamId(name);
	return AddShaderParam(paramName, passId, id);
}

core::VariableAccess MaterialRenderer::AddShaderParam(const core::String& paramName, u32 passId, u32 paramId)
{
	auto& pass = m_Passes.At(passId);

	if(!pass.shader || paramId >= pass.shader->GetParamPackage().GetParamCount())
		throw core::InvalidArgumentException("paramId", "Is not a valid shader parameter");

	core::ParamDesc desc = pass.shader->GetParamPackage().GetParamDesc(paramId);
	const core::String& name = paramName.IsEmpty() ? desc.name : paramName;
	return AddParamMapping(desc.type, name, passId, paramId, true);
}

core::VariableAccess MaterialRenderer::AddParam(const core::String& paramName, u32 passId, EOptionId optionId)
{
	auto& pass = m_Passes.At(passId);
	auto optionType = pass.GetOptionType((u32)optionId);
	const core::String& name = paramName.IsEmpty() ? pass.GetOptionName((u32)optionId) : paramName;
	return AddParamMapping(optionType, name, passId, (u32)optionId, false);
}

core::VariableAccess MaterialRenderer::AddParamMapping(const core::Type& type, const core::String& paramName, u32 passId, u32 mappingId, bool isShader)
{
	u32 id = m_Params.AddParam(type, paramName);

	ParamMapping mapping;
	mapping.id = id;
	mapping.pass = passId;
	mapping.isShader = isShader;
	mapping.mappingId = mappingId;
	m_Options.PushBack(mapping);

	return m_Params.DefaultValue(id);
}

} // namespace video 
} // namespace lux
