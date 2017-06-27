#ifndef INCLUDED_MATERIAL_IMPL_H
#define INCLUDED_MATERIAL_IMPL_H
#include "video/Material.h"
#include "video/MaterialRenderer.h"

namespace lux
{
namespace video
{
class MaterialRenderer;

class MaterialImpl : public Material
{
public:
	MaterialImpl(MaterialRenderer* renderer);
	~MaterialImpl();

	////////////////////////////////////////////////////////////////////

	void SetRenderer(MaterialRenderer* renderer);
	MaterialRenderer* GetRenderer() const;

	////////////////////////////////////////////////////////////////////

	void CopyFrom(const Material* _other);
	bool Equal(const Material* _other) const;

	////////////////////////////////////////////////////////////////////

	void SetAmbient(float ambient);
	float GetAmbient() const;
	void SetDiffuse(const Colorf& diffuse);
	const Colorf& GetDiffuse() const;
	void SetAlpha(float alpha);
	float GetAlpha() const;
	void SetEmissive(const Colorf& emissive);
	const Colorf& GetEmissive() const;
	void SetSpecular(const Colorf& specular);
	const Colorf& GetSpecular() const;
	void SetShininess(float shininess);
	float GetShininess() const;
	void SetPower(float power);
	float GetPower() const;

	////////////////////////////////////////////////////////////////////

	core::PackageParam Param(const string_type& name);
	core::PackageParam Param(const string_type& name) const;
	core::PackageParam Param(u32 id);
	core::PackageParam Param(u32 id) const;
	core::PackageParam Layer(u32 layer);
	core::PackageParam Layer(u32 layer) const;

	////////////////////////////////////////////////////////////////////

	u32 GetTextureCount() const;
	u32 GetParamCount() const;

	////////////////////////////////////////////////////////////////////

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

	////////////////////////////////////////////////////////////////////


private:
	struct RenderData : public WeakRefBase
	{
		MaterialRenderer* renderer;
		core::PackagePuffer puffer;

		RenderData(MaterialRenderer* _renderer) :
			renderer(_renderer),
			puffer(_renderer ? &_renderer->GetPackage() : nullptr)
		{
			AddTo(renderer);
		}

		~RenderData()
		{
			RemoveFrom(renderer);
		}


		RenderData& operator=(const RenderData& other)
		{
			puffer = other.puffer;
			AssignTo(renderer, other.renderer);
			renderer = other.renderer;
			return *this;
		}

		void Destroy()
		{
			puffer.SetType(nullptr);
			renderer = nullptr;
		}

		void Set(MaterialRenderer* r)
		{
			AssignTo(renderer, r);

			renderer = r;
			if(renderer)
				puffer.SetType(&renderer->GetPackage());
			else
				puffer.SetType(nullptr);
		}
	};

	float m_Ambient;
	Colorf m_Diffuse;
	Colorf m_Emissive;
	Colorf m_Specular;
	float m_Shininess;
	float m_Power;

	RenderData m_RenderData;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_MATERIAL_IMPL_H