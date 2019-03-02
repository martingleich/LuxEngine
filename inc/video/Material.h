#ifndef INCLUDED_LUX_MATERIAL_H
#define INCLUDED_LUX_MATERIAL_H
#include "video/AbstractMaterial.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

/*
TODO: Make shader reconigurabable, i.e. Set all topological parameters(shader, techniques, requirements) This might require a reset of all parameters of something else.
But it should be doable. For ease of use we put it all into the normal Material class.
TODO: Add more references to abstract materials.
TODO: Allow a more flexible choice in selecting the used material class.
*/

class Material : public AbstractMaterial
{
public:
	struct SetData : public ShaderParamSetCallback::Data
	{
		explicit SetData(Material* _m) :
			m(_m)
		{
		}
		Material* m;
	};
	struct TechniqueData
	{
		TechniqueData() {}

		TechniqueData(const Pass& p, EMaterialTechnique t) :
			pass(p),
			tech(t)
		{
		}
		Pass pass;
		EMaterialTechnique tech;
	};

	struct Configuration
	{
		Configuration(const Pass& defaultPass, EMaterialReqFlag req) :
			pass(defaultPass),
			requirments(req)
		{
		}

		Configuration& Tech(EMaterialTechnique t, const Pass& p)
		{
			additionalTech.EmplaceBack(p, t);
			return *this;
		}

		video::Pass pass;
		EMaterialReqFlag requirments;
		core::Array<TechniqueData> additionalTech;
	};

private:
	class InternalTechnique : public AbstractMaterialTechnique
	{
	public:
		TechniqueData tech;

		// Map material params id to shader id, contains -1 if shaders doesn't contain argument.
		core::Array<int> paramIdToShaderId;

		InternalTechnique(const TechniqueData& t, const core::ParamPackage& materialParams) :
			tech(t)
		{
			paramIdToShaderId.Resize(materialParams.GetParamCount(), -1);
			auto& shaderParams = tech.pass.shader->GetParamPackage();
			for(int j = 0; j < materialParams.GetParamCount(); ++j)
				paramIdToShaderId[j] = shaderParams.GetParamIdByName(materialParams.GetParamName(j));
		}

		const Pass& GetPass() const override { return tech.pass; }
		void SendShaderSettings(ShaderParamSetCallback::Data* data) const override
		{
			LX_CHECK_NULL_ARG(data);
			const Material* mat = dynamic_cast<SetData&>(*data).m;
			LX_CHECK_NULL_ARG(mat);

			for(int i = 0; i < mat->m_Params.GetParamCount(); ++i) {
				auto shaderId = paramIdToShaderId[i];
				if(shaderId != -1) {
					auto param = mat->m_Params.FromID(i, true);
					tech.pass.shader->SetParam(shaderId, param.Pointer());
				}
			}
		}
	};

	class SharedMaterialData : public ReferenceCounted
	{
	public:
		SharedMaterialData(const Pass& defaultPass, EMaterialReqFlag req, const core::Array<TechniqueData>& additional)
		{
			auto& matParams = defaultPass.shader->GetParamPackage();
			m_Techniques.EmplaceBack(TechniqueData(defaultPass, EMaterialTechnique::Default), matParams);

			for(auto add : additional) {
				if(add.tech != EMaterialTechnique::Default)
					m_Techniques.EmplaceBack(add, matParams);
			}

			m_Requirements = req;

			m_DiffuseId = matParams.GetParamIdByName("diffuse");
			m_EmissiveId = matParams.GetParamIdByName("emissive");
			m_SpecularHardnessId = matParams.GetParamIdByName("specularHardness");
			m_SpecularIntensityId = matParams.GetParamIdByName("specularIntensity");
		}

		core::Array<InternalTechnique> m_Techniques;
		EMaterialReqFlag m_Requirements;

		// Material parameter id of diffuse/emissive/specular hardness/specular intensity or -1 if not available.
		int m_DiffuseId;
		int m_EmissiveId;
		int m_SpecularHardnessId;
		int m_SpecularIntensityId;
	};

public:
	explicit Material(const Configuration& config)
	{
		ReConfigure(config);
	}
	void ReConfigure(const Configuration& config)
	{
		m_Shared = LUX_NEW(SharedMaterialData)(config.pass, config.requirments, config.additionalTech);
		m_Params.Reset(&config.pass.shader->GetParamPackage());
	}

	core::Optional<AbstractMaterialTechnique*> GetTechnique(EMaterialTechnique technique = EMaterialTechnique::Default) const
	{
		for(int i = 0; i < m_Shared->m_Techniques.Size(); ++i) {
			if(m_Shared->m_Techniques[i].tech.tech == technique)
				return &m_Shared->m_Techniques[i];
		}

		return nullptr;
	}

	EMaterialReqFlag GetRequirements() const override { return m_Shared->m_Requirements; }

	StrongRef<Material> Clone() const
	{
		return LUX_NEW(Material)(*this);
	}

	///////////////////////////////////

	core::VariableAccess Param(core::StringView name) const
	{
		return m_Params.FromName(name, true);
	}
	core::VariableAccess Param(core::StringView name)
	{
		return m_Params.FromName(name, false);
	}
	core::VariableAccess Param(int id) const
	{
		return m_Params.FromID(id, true);
	}
	core::VariableAccess Param(int id)
	{
		return m_Params.FromID(id, false);
	}

	void SetTexture(int layer, video::BaseTexture* texture)
	{
		auto type = m_Params.GetType();
		int l = 0;
		int i = 0;
		for(; i < type->GetParamCount(); ++i) {
			if(type->GetParamType(i) == core::Types::Texture()) {
				if(l == layer)
					break;
				++l;
			}
		}
		if(i != type->GetParamCount())
			m_Params.FromID(i, false).Set(video::TextureLayer(texture));
	}
	void SetTexture(core::StringView str, video::BaseTexture* texture)
	{
		m_Params.FromName(str, false).Set(video::TextureLayer(texture));
	}
	video::TextureLayer GetTexture(core::StringView str) const
	{
		return m_Params.FromName(str, true).Get<video::TextureLayer>();
	}
	video::TextureLayer GetTexture(int id) const
	{
		return m_Params.FromID(id, true).Get<video::TextureLayer>();
	}

	void SetColor(core::StringView str, const video::ColorF& color)
	{
		m_Params.FromName(str, false).Set(color);
	}
	void SetColor(int id, const video::ColorF& color)
	{
		m_Params.FromID(id, false).Set(color);
	}
	video::ColorF GetColor(core::StringView str) const
	{
		return m_Params.FromName(str, true).Get<video::ColorF>();
	}
	video::ColorF GetColor(int id) const
	{
		return m_Params.FromID(id, true).Get<video::ColorF>();
	}

	void SetFloat(core::StringView str, float f)
	{
		m_Params.FromName(str, false).Set(f);
	}
	void SetFloat(int id, float f)
	{
		m_Params.FromID(id, false).Set(f);
	}
	float GetFloat(core::StringView str) const
	{
		return m_Params.FromName(str, true).Get<float>();
	}
	float GetFloat(int id) const
	{
		return m_Params.FromID(id, true).Get<float>();
	}

	bool HasDiffuse() const { return m_Shared->m_DiffuseId >= 0; }
	bool HasEmissive() const { return m_Shared->m_EmissiveId >= 0; }
	bool HasSpecularHardness() const { return m_Shared->m_SpecularHardnessId >= 0; }
	bool HasSpecularIntensity() const { return m_Shared->m_SpecularIntensityId >= 0; }

	void SetDiffuse(const video::ColorF& color)
	{
		if(HasDiffuse())
			SetColor(m_Shared->m_DiffuseId, color);
	}
	void SetEmissive(float emissive) { if(HasEmissive()) SetFloat(m_Shared->m_EmissiveId, emissive); }
	void SetSpecularHardness(float hardness) { if(HasSpecularHardness()) SetFloat(m_Shared->m_SpecularHardnessId, hardness); }
	void SetSpecularIntensity(float intensity) { if(HasSpecularIntensity()) SetFloat(m_Shared->m_SpecularIntensityId, intensity); }
	void SetAlpha(float alpha) { if(HasDiffuse()) { auto c = GetDiffuse(); c.SetAlpha(alpha); SetDiffuse(c); } }

	video::ColorF GetDiffuse() const
	{
		if(HasDiffuse())
			return GetColor(m_Shared->m_DiffuseId);
		return video::ColorF();
	}
	float GetEmissive() const
	{
		if(HasEmissive())
			return GetFloat(m_Shared->m_EmissiveId);
		return 0;
	}
	float GetSpecularHardness() const
	{
		if(HasSpecularHardness())
			return GetFloat(m_Shared->m_SpecularHardnessId);
		return 0;
	}
	float GetSpecularIntensity() const
	{
		if(HasSpecularIntensity())
			return GetFloat(m_Shared->m_SpecularIntensityId);
		return 0;
	}
	float GetAlpha() const
	{
		if(HasDiffuse())
			return GetDiffuse().GetAlpha();
		return 0;
	}

private:
	StrongRef<SharedMaterialData> m_Shared;
	core::PackagePuffer m_Params;
};

} // namespace video
} // namespace lux

#endif