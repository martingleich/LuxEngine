#ifndef INCLUDED_LUX_MATERIAL_H
#define INCLUDED_LUX_MATERIAL_H
#include "video/AbstractMaterial.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

class Material : public AbstractMaterial
{
public:
	struct MaterialSetData : public ShaderParamSetCallback::Data
	{
		Material* m;
	};
	struct MaterialTechnique
	{
		const Pass& pass;
		EMaterialTechnique tech;
	};
	class SharedMaterialData : public ReferenceCounted
	{
	public:
		SharedMaterialData(const Pass& defaultPass, EMaterialReqFlag req,
			const MaterialTechnique* additional, int additionalCount)
		{
			Technique def;
			def.pass = defaultPass;
			def.technique = EMaterialTechnique::Default;
			auto& defaultParams = defaultPass.shader->GetParamPackage();
			for(int i = 0; i < defaultParams.GetParamCount(); ++i)
				def.paramIdToShaderId.PushBack(i);
			m_Techniques.PushBack(def);
			for(int i = 0; i < additionalCount; ++i) {
				Technique tech;
				tech.pass = additional[i].pass;
				tech.technique = additional[i].tech;
				tech.paramIdToShaderId.Resize(defaultParams.GetParamCount(), -1);
				auto& shaderParams = tech.pass.shader->GetParamPackage();
				for(int j = 0; j < defaultParams.GetParamCount(); ++j)
					tech.paramIdToShaderId[j] = shaderParams.GetParamIdByName(defaultParams.GetParamName(j));
			}

			m_Requirements = req;

			m_DiffuseId = defaultParams.GetParamIdByName("diffuse");
			m_EmissiveId = defaultParams.GetParamIdByName("emissive");
			m_SpecularHardnessId = defaultParams.GetParamIdByName("specularHardness");
			m_SpecularIntensityId = defaultParams.GetParamIdByName("specularIntensity");
		}

		class Technique : public AbstractMaterialTechnique
		{
		public:
			Pass pass;
			EMaterialTechnique technique;
			core::Array<int> paramIdToShaderId;

			const Pass& GetPass() const { return pass; }
			void SendShaderSettings(ShaderParamSetCallback::Data* data) const override
			{
				LX_CHECK_NULL_ARG(data);
				const Material* mat = dynamic_cast<MaterialSetData&>(*data).m;
				LX_CHECK_NULL_ARG(mat);

				for(int i = 0; i < mat->m_Params.GetParamCount(); ++i) {
					auto shaderId = paramIdToShaderId[i];
					if(shaderId != -1) {
						auto param = mat->m_Params.FromID(i, true);
						pass.shader->SetParam(shaderId, param.Pointer());
					}
				}
			}
		};

		core::Array<Technique> m_Techniques;
		EMaterialReqFlag m_Requirements;

		int m_DiffuseId;
		int m_EmissiveId;
		int m_SpecularHardnessId;
		int m_SpecularIntensityId;
	};

public:
	Material(const Pass& pass, EMaterialReqFlag req) :
		m_Shared(LUX_NEW(SharedMaterialData(pass, req, nullptr, 0))),
		m_Params(&pass.shader->GetParamPackage())
	{
	}

	AbstractMaterialTechnique* GetTechnique(EMaterialTechnique technique = EMaterialTechnique::Default) const
	{
		for(int i = 0; i < m_Shared->m_Techniques.Size(); ++i) {
			if(m_Shared->m_Techniques[i].technique == technique)
				return &m_Shared->m_Techniques[i];
		}
		return &m_Shared->m_Techniques[0];
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