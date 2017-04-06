#ifndef INCLUDED_RENDERTYPE_H
#define INCLUDED_RENDERTYPE_H
#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

class VideoDriver;
class Shader;

class RenderType
{
public:
	RenderType(VideoDriver* driver, Shader* shader = nullptr, const core::ParamPackage* basePackage = nullptr);
	RenderType(const RenderType& other);

	virtual ~RenderType();

	void Enable(const core::PackagePuffer& puffer, bool fullReset);
	void Disable();

	core::ParamPackage& GetPackage()
	{
		return m_Package;
	}
	PipelineSettings& GetPipeline()
	{
		return m_Pipeline;
	}
	const core::ParamPackage& GetPackage() const
	{
		return m_Package;
	}
	const PipelineSettings& GetPipeline() const
	{
		return m_Pipeline;
	}
	Shader* GetShader() const
	{
		return m_Shader;
	}

protected:
	core::ParamPackage m_Package;
	PipelineSettings m_Pipeline;
	Shader* m_Shader;
	VideoDriver* m_Driver;
};

class RenderData
{
protected:
	core::PackagePuffer m_Puffer;
	RenderType*  m_Type;

public:
	RenderData() : m_Type(nullptr), m_Puffer(nullptr)
	{
	}
	RenderData(RenderType* type) : m_Type(type), m_Puffer(&type->GetPackage())
	{
	}

	virtual ~RenderData()
	{
	}
};

}
}

#endif // #ifndef INCLUDED_RENDERTYPE_H