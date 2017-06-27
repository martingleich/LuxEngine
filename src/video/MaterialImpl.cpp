#include "MaterialImpl.h"
#include "video/MaterialRenderer.h"

namespace lux
{
namespace video
{

MaterialImpl::MaterialImpl(MaterialRenderer* renderer) :
	m_Ambient(1.0f),
	m_Diffuse(Color::LightGray),
	m_Emissive(Color::Black),
	m_Specular(Color::White),
	m_Shininess(0.0f),
	m_Power(1.0f),
	m_RenderData(renderer)
{
}

MaterialImpl::~MaterialImpl()
{
}

////////////////////////////////////////////////////////////////////

void MaterialImpl::SetRenderer(MaterialRenderer* renderer)
{
	m_RenderData.Set(renderer);
}

MaterialRenderer* MaterialImpl::GetRenderer() const
{
	return m_RenderData.renderer;
}

////////////////////////////////////////////////////////////////////

void MaterialImpl::CopyFrom(const Material* _other)
{
	if(this == _other)
		return;

	const MaterialImpl* other = dynamic_cast<const MaterialImpl*>(_other);
	m_RenderData = other->m_RenderData;

	m_Ambient = other->m_Ambient;
	m_Diffuse = other->m_Diffuse;
	m_Emissive = other->m_Emissive;
	m_Specular = other->m_Specular;
	m_Shininess = other->m_Shininess;
	m_Power = other->m_Power;
}

bool MaterialImpl::Equal(const Material* _other) const
{
	if(this == _other)
		return true;

	const MaterialImpl* other = dynamic_cast<const MaterialImpl*>(_other);
	return m_RenderData.puffer == other->m_RenderData.puffer &&
		m_Ambient == other->m_Ambient &&
		m_Diffuse == other->m_Diffuse &&
		m_Emissive == other->m_Emissive &&
		m_Specular == other->m_Specular &&
		m_Shininess == other->m_Shininess;
}

////////////////////////////////////////////////////////////////////

void MaterialImpl::SetAmbient(float ambient)
{
	m_Ambient = ambient;
}

float MaterialImpl::GetAmbient() const
{
	return m_Ambient;
}

void MaterialImpl::SetDiffuse(const Colorf& diffuse)
{
	m_Diffuse = diffuse;
}

const Colorf& MaterialImpl::GetDiffuse() const
{
	return m_Diffuse;
}

void MaterialImpl::SetAlpha(float alpha)
{
	m_Diffuse.SetAlpha(alpha);
}

float MaterialImpl::GetAlpha() const
{
	return m_Diffuse.GetAlpha();
}

void MaterialImpl::SetEmissive(const Colorf& emissive)
{
	m_Emissive = emissive;
}

const Colorf& MaterialImpl::GetEmissive() const
{
	return m_Emissive;
}

void MaterialImpl::SetSpecular(const Colorf& specular)
{
	m_Specular = specular;
}
const Colorf& MaterialImpl::GetSpecular() const
{
	return m_Specular;
}

void MaterialImpl::SetShininess(float shininess)
{
	m_Shininess = shininess;
}

float MaterialImpl::GetShininess() const
{
	return m_Shininess;
}

void MaterialImpl::SetPower(float power)
{
	m_Power=power;
}

float MaterialImpl::GetPower() const
{
	return m_Power;
}

////////////////////////////////////////////////////////////////////

core::PackageParam MaterialImpl::Param(const string_type& name)
{
	return m_RenderData.puffer.FromName(name, false);
}

core::PackageParam MaterialImpl::Param(const string_type& name) const
{
	return m_RenderData.puffer.FromName(name, true);
}

core::PackageParam MaterialImpl::Param(u32 id)
{
	return m_RenderData.puffer.FromID(id, false);
}
core::PackageParam MaterialImpl::Param(u32 id) const
{
	return m_RenderData.puffer.FromID(id, true);
}

core::PackageParam MaterialImpl::Layer(u32 layer)
{
	return m_RenderData.puffer.FromType(core::Type::Texture, layer, false);
}
core::PackageParam MaterialImpl::Layer(u32 layer) const
{
	return m_RenderData.puffer.FromType(core::Type::Texture, layer, true);
}

////////////////////////////////////////////////////////////////////

u32 MaterialImpl::GetTextureCount() const
{
	return m_RenderData.puffer.GetTextureCount();
}

u32 MaterialImpl::GetParamCount() const
{
	return m_RenderData.puffer.GetParamCount();
}

////////////////////////////////////////////////////////////////////

core::Name MaterialImpl::GetReferableSubType() const
{
	return core::ResourceType::Material;
}

StrongRef<Referable> MaterialImpl::Clone() const
{
	return LUX_NEW(MaterialImpl)(*this);
}

}
}
