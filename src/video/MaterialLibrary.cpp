#include "video/MaterialLibrary.h"

#include "video/Shader.h"
#include "video/VideoDriver.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "core/lxAlgorithm.h"
#include "core/Logger.h"

namespace lux
{
namespace video
{

static StrongRef<MaterialLibrary> g_MaterialLibrary;

void MaterialLibrary::Initialize(MaterialLibrary* matLib)
{
	if(!matLib)
		matLib = LUX_NEW(MaterialLibrary);

	if(!matLib)
		throw core::ErrorException("No material library available");
	g_MaterialLibrary = matLib;
}

MaterialLibrary::~MaterialLibrary()
{
	// At this point there shouldn't be any references to renderers
	for(auto& x : m_Renderers) {
		if(x->GetReferenceCount() != 1)
			log::Error("~MaterialLibrary still references to renderers");
	}
}

MaterialLibrary* MaterialLibrary::Instance()
{
	return g_MaterialLibrary;
}

void MaterialLibrary::Destroy()
{
	g_MaterialLibrary.Reset();
}

MaterialLibrary::MaterialLibrary()
{
	{
		AddMaterialRenderer("solidBase");
	}

	{
		auto transparentBase = AddMaterialRenderer("transparentBase");
		transparentBase->GetPass(0).requirements = EMaterialRequirement::Transparent;
	}

	{
		auto solid = AddMaterialRenderer("solid");
		solid->GetPass(0).AddTexture();
		solid->AddParam("diffMap", 0, EOptionId::Layer0);
	}

	{
		auto debug = AddMaterialRenderer("debugOverlay");
		auto& pass = debug->GetPass(0);
		pass.fogEnabled = false;
		pass.lighting = video::ELighting::Disabled;

		TextureStageSettings tss0;
		tss0.colorOperator = ETextureOperator::SelectArg1;
		tss0.colorArg1 = ETextureArgument::Diffuse;
		pass.layerSettings.PushBack(tss0);
		pass.useVertexColor = true;
	}

	{
		auto solidMix = AddMaterialRenderer("solidMix");
		auto& pass = solidMix->GetPass(0);
		pass.AddTexture(2);
		pass.useVertexColor = true;
		// Blend(Diff*Texture1, Texture2, DiffAlpha)
		TextureStageSettings tss0, tss1, tss2;
		tss0.colorOperator = ETextureOperator::SelectArg1;
		tss0.colorArg1 = ETextureArgument::Texture;

		tss1.colorOperator = ETextureOperator::Blend;
		tss1.colorArg1 = ETextureArgument::Current;
		tss1.colorArg2 = ETextureArgument::Texture;
		tss1.coordSource = 0;

		tss2.colorOperator = ETextureOperator::Modulate;
		tss2.colorArg1 = ETextureArgument::Current;
		tss2.colorArg2 = ETextureArgument::Diffuse;
		tss2.coordSource = 0;
		pass.layerSettings.PushBack(tss0);
		pass.layerSettings.PushBack(tss1);
		pass.layerSettings.PushBack(tss2);

		solidMix->AddParam("diffMap1", 0, EOptionId::Layer0);
		solidMix->AddParam("diffMap2", 0, EOptionId::Layer1);
	}

	{
		auto transparent = AddMaterialRenderer("transparent");
		auto& pass = transparent->GetPass(0);
		pass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
		pass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
		pass.alpha.blendOperator = video::EBlendOperator::Add;
		pass.zWriteEnabled = false;
		pass.fogEnabled = false;
		pass.requirements = video::EMaterialRequirement::Transparent;
		pass.AddTexture();

		transparent->AddParam("diffMap", 0, EOptionId::Layer0);
	}

	{
		const char* luxHLSLInclude =
R"(
// d is the distance from the geomtry to the camera.
// Returns 0 for minimal fog effect, 1 for maximal effect
float lxFog(float d, float4 fog1, float4 fog2) 
{
	float start = fog2.g;
	float end = fog2.b;
	float dens = fog2.a;
	float enabled = fog1.a;
	if(fog2.r == 1)
		return clamp((d-start) / (end - start), 0, 1) * enabled;
	else if(fog2.r == 2)
		return clamp(1-exp(-d*dens), 0, 1) * enabled;
	else if(fog2.r == 3)
		return clamp(1-exp(-pow(d*dens,2)), 0, 1) * enabled;
	else
		return 0;
}

float4 lxIlluminate(float3 camPos, float3 pos, float3 normal, float4 ambient, float4 emissive, float4 diffuse, float4 specular, float shininess, float lighting, float4x4 light)
{
	float4 color = float4(0,0,0,0);
	int iLighting = (int)lighting;

	if(iLighting % 2 != 0) { // AmbientEmissive
		color += ambient + emissive;
	}
	if((iLighting / 2) % 2 != 0) { // Diffuse Specular
		float4 lightDiffuse = float4(light._m00_m01_m02, 1);
		float3 lightDir = -light._m20_m21_m22;
		float3 lightPos = light._m10_m11_m12;
		float3 lightVector = lightDir;
		float illumFactor = 1;
		float4 illumCol;
		if(light._m03 == 1) { // Directonal
			illumCol = dot(normal, lightDir) * diffuse * lightDiffuse;
		} else { // Spot/Point light
			lightVector = lightPos - pos;
			float lightDistance = length(lightVector);
			lightVector /= lightDistance;
			float spotFactor = 1;
			if(light._m03 == 3) { // Spot
				float falloff = light._m30;
				float ci = light._m31;
				float co = light._m32;
				float rho = dot(lightDir, lightVector);
				if(rho <= co)
					spotFactor = 0;
				else if(rho <= ci)
					spotFactor = pow((rho - co) / (ci - co), falloff);
			}
			illumFactor = spotFactor / lightDistance;
			illumCol = dot(normal, lightVector) * diffuse * lightDiffuse;
		}
		if(shininess > 0) {
			float3 h = normalize(normalize(camPos - pos) + lightVector);
			float sf = pow(dot(h, normal), shininess);
			illumCol += lightDiffuse * specular * sf;
		}
		color += illumFactor * illumCol;
	}

	return color;
}
)";
		SetShaderInclude(EShaderLanguage::HLSL, "lux", luxHLSLInclude, strlen(luxHLSLInclude));
	}
}

StrongRef<Material> MaterialLibrary::CreateMaterial(const core::String& name)
{
	return CreateMaterial(GetMaterialRenderer(name));
}

StrongRef<Material> MaterialLibrary::CreateMaterial(MaterialRenderer* renderer)
{
	if(!renderer)
		return CreateMaterial("solid");
	return renderer->CreateMaterial();
}

StrongRef<MaterialRenderer> MaterialLibrary::AddMaterialRenderer(MaterialRenderer* renderer)
{
	LX_CHECK_NULL_ARG(renderer);

	if(renderer->GetName().IsEmpty())
		throw core::InvalidArgumentException("renderer", "Renderer must have an name");

	size_t id;
	if(FindRenderer(renderer->GetName(), id))
		throw core::InvalidArgumentException("renderer", "Name is already used");

	m_Renderers.PushBack(renderer);

	return renderer;
}

StrongRef<MaterialRenderer> MaterialLibrary::AddMaterialRenderer(const core::String& newName)
{
	StrongRef<MaterialRenderer> r = LUX_NEW(MaterialRenderer)(newName);
	return AddMaterialRenderer(r);
}

StrongRef<MaterialRenderer> MaterialLibrary::ReplaceMaterialRenderer(const core::String& name)
{
	size_t id;
	if(!FindRenderer(name, id))
		return AddMaterialRenderer(name);
	m_Renderers[id] = LUX_NEW(MaterialRenderer)(name);
	return m_Renderers[id];
}

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const core::String& name, const core::String& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const core::String& name, const MaterialRenderer* old)
{
	LX_CHECK_NULL_ARG(old);

	auto r = LUX_NEW(MaterialRenderer)(name, old);
	return AddMaterialRenderer(r);
}

void MaterialLibrary::RemoveMaterialRenderer(MaterialRenderer* renderer)
{
	if(!renderer)
		return;

	for(auto it = m_Renderers.First(); it != m_Renderers.End(); ++it) {
		if(*it == renderer) {
			m_Renderers.Erase(it);
			return;
		}
	}
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromFile(
	video::EShaderLanguage language,
	const io::Path& VSPath, const core::String& VSEntryPoint, int VSMajor, int VSMinor,
	const io::Path& PSPath, const core::String& PSEntryPoint, int PSMajor, int PSMinor,
	core::Array<core::String>* errorList)
{
	StrongRef<io::File> PSFile;
	StrongRef<io::File> VSFile;
	if(VSPath == PSPath) {
		PSFile = VSFile = io::FileSystem::Instance()->OpenFile(VSPath);
	} else {
		PSFile = io::FileSystem::Instance()->OpenFile(PSPath);
		VSFile = io::FileSystem::Instance()->OpenFile(VSPath);
	}

	core::RawMemory vsCodeBuffer;
	char* vsCode = (char*)VSFile->GetBuffer();
	if(!vsCode) {
		vsCodeBuffer.SetSize(VSFile->GetSize() + 1);
		vsCode = vsCodeBuffer;
		VSFile->ReadBinary(VSFile->GetSize(), vsCode);
		vsCode[VSFile->GetSize()] = 0;
	}

	core::RawMemory psCodeBuffer;
	char* psCode = (char*)PSFile->GetBuffer();
	if(!psCode) {
		if(PSFile == VSFile) {
			psCode = vsCode;
		} else {
			psCodeBuffer.SetSize(PSFile->GetSize() + 1);
			psCode = psCodeBuffer;
			PSFile->ReadBinary(PSFile->GetSize(), psCode);
			psCode[PSFile->GetSize()] = 0;
		}
	}

	return video::VideoDriver::Instance()->CreateShader(
		language,
		vsCode, VSEntryPoint.Data(), VSFile->GetSize(), VSMajor, VSMinor,
		psCode, PSEntryPoint.Data(), PSFile->GetSize(), PSMajor, PSMinor,
		errorList);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(size_t index) const
{
	return m_Renderers.At(index);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(const core::String& name) const
{
	size_t id;
	if(!FindRenderer(name, id))
		throw core::ObjectNotFoundException(name.Data());

	return m_Renderers[id];
}

bool MaterialLibrary::ExistsMaterialRenderer(const core::String& name, MaterialRenderer** outRenderer) const
{
	size_t id;
	if(!FindRenderer(name, id))
		return false;

	if(outRenderer)
		*outRenderer = m_Renderers[id];
	return true;
}

size_t MaterialLibrary::GetMaterialRendererCount() const
{
	return m_Renderers.Size();
}

bool MaterialLibrary::FindRenderer(const core::String& name, size_t& id) const
{
	for(id = 0; id < m_Renderers.Size(); ++id) {
		if(m_Renderers[id]->GetName() == name)
			return true;
	}

	return false;
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromMemory(
	EShaderLanguage language,
	const core::String& VSCode, const char* VSEntryPoint, int VSmajorVersion, int VSminorVersion,
	const core::String& PSCode, const char* PSEntryPoint, int PSmajorVersion, int PSminorVersion,
	core::Array<core::String>* errorList)
{
	return video::VideoDriver::Instance()->CreateShader(
		language,
		VSCode.Data_c(), VSEntryPoint, VSCode.Size(), VSmajorVersion, VSminorVersion,
		PSCode.Data_c(), PSEntryPoint, PSCode.Size(), PSmajorVersion, PSminorVersion,
		errorList);
}

bool MaterialLibrary::IsShaderSupported(EShaderLanguage lang, int vsMajor, int vsMinor, int psMajor, int psMinor)
{
	return video::VideoDriver::Instance()->IsShaderSupported(lang, vsMajor, vsMinor, psMajor, psMinor);
}

bool MaterialLibrary::GetShaderInclude(EShaderLanguage language, const core::String& name, const void*& outData, size_t& outBytes)
{
	ShaderInclude search(language, name);
	auto it = m_ShaderIncludes.Find(search);
	if(it == m_ShaderIncludes.End())
		return false;
	outData = it->PointerC();
	outBytes = it->GetSize();
	return true;
}

void MaterialLibrary::SetShaderInclude(EShaderLanguage language, const core::String& name, const void* data, size_t bytes)
{
	ShaderInclude search(language, name);
	auto& mem = m_ShaderIncludes.At(search);
	mem.SetMinSize(bytes);
	memcpy(mem, data, bytes);
}

} // namespace video
} // namespace lux
