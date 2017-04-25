#include "video/RenderType.h"
#include "video/VideoDriver.h"
#include "video/Shader.h"

namespace lux
{
namespace video
{

void RenderType::Enable(const core::PackagePuffer& puffer, bool fullReset, const RenderData* renderData)
{
	if(m_Shader) {
		m_Shader->Enable();
		m_Shader->LoadParams(puffer, renderData);
	}

	m_Driver->EnablePipeline(m_Pipeline, fullReset);
}

void RenderType::Disable()
{
	if(m_Shader)
		m_Shader->Disable();
}

RenderType::RenderType(VideoDriver* driver, Shader* shader, const core::ParamPackage* basePackage) :
	m_Driver(driver),
	m_Shader(shader)
{
	if(basePackage)
		m_Package = *basePackage;
	if(m_Shader)
		m_Shader->Grab();
}

RenderType::RenderType(const RenderType& other) :
	m_Package(other.m_Package),
	m_Pipeline(other.m_Pipeline),
	m_Shader(other.m_Shader),
	m_Driver(other.m_Driver)
{
}

RenderType::~RenderType()
{
	if(m_Shader)
		m_Shader->Drop();
}

}
}