#include "video/MaterialLibrary.h"

#include "video/Shader.h"
#include "video/VideoDriver.h"
#include "core/ResourceSystem.h"

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
		auto solid = LUX_NEW(Material);
		video::Pass pass;
		video::TextureStageSettings tss;
		pass.shader = GetFixedFunctionShader({"texture"}, {tss});
		solid->SetPass(pass);
		m_MaterialMap["solid"] = 0;
		m_MaterialList.PushBack(solid);
	}

	{
		video::Pass pass;
		pass.fogEnabled = false;
		pass.lighting = video::ELighting::Disabled;

		video::TextureStageSettings tss;
		tss.colorOperator = ETextureOperator::SelectArg1;
		tss.colorArg1 = ETextureArgument::Diffuse;
		pass.shader = GetFixedFunctionShader({"texture"}, {tss}, true);

		auto debug = LUX_NEW(Material);
		debug->SetPass(pass);
		m_MaterialMap["debugOverlay"] = 1;
		m_MaterialList.PushBack(debug);
	}

	{
		video::Pass pass;
		pass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
		pass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
		pass.alpha.blendOperator = video::EBlendOperator::Add;
		pass.zWriteEnabled = false;
		pass.fogEnabled = false;
		video::TextureStageSettings tss;
		pass.shader = GetFixedFunctionShader({"texture"}, {tss});

		auto transparent = LUX_NEW(Material);
		transparent->SetRequirements(video::EMaterialRequirement::Transparent);
		transparent->SetPass(pass);
		m_MaterialMap["transparent"] = 2;
		m_MaterialList.PushBack(transparent);
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
			illumCol = max(dot(normal, lightDir),0) * diffuse * lightDiffuse;
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
			illumCol = max(dot(normal, lightVector),0) * diffuse * lightDiffuse;
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

MaterialLibrary::~MaterialLibrary()
{
}

void MaterialLibrary::SetMaterial(
	const core::String& name, video::Material* material)
{
	auto& key = m_MaterialMap.At(name, m_MaterialList.Size());
	if(key == m_MaterialList.Size())
		m_MaterialList.PushBack(material);
	else
		m_MaterialList[key] = material;
}

StrongRef<video::Material> MaterialLibrary::GetMaterial(
	const core::String& name)
{
	auto it = m_MaterialMap.Find(name);
	if(it == m_MaterialMap.End())
		return nullptr;
	else
		return m_MaterialList[*it];
}

StrongRef<video::Material> MaterialLibrary::CloneMaterial(
	const core::String& name)
{
	auto m = GetMaterial(name);
	if(m)
		return m->Clone();
	else
		return nullptr;
}

void MaterialLibrary::SetMaterial(EKnownMaterial name, Material* material)
{
	m_MaterialList.At((u32)name) = material;
}

StrongRef<video::Material> MaterialLibrary::GetMaterial(EKnownMaterial name)
{
	return m_MaterialList.At((u32)name);
}

StrongRef<video::Material> MaterialLibrary::CloneMaterial(EKnownMaterial name)
{
	auto m = GetMaterial(name);
	if(m)
		return m->Clone();
	else
		return nullptr;
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromFile(
	video::EShaderLanguage language,
	const io::Path& VSPath, const core::String& VSEntryPoint,
	int VSMajor, int VSMinor,
	const io::Path& PSPath, const core::String& PSEntryPoint,
	int PSMajor, int PSMinor,
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

StrongRef<Shader> MaterialLibrary::GetFixedFunctionShader(
	const core::Array<core::String>& textures,
	const core::Array<TextureStageSettings>& stages,
	bool useVertexColors)
{
	return GetFixedFunctionShader(FixedFunctionParameters(textures, stages, useVertexColors));
}

StrongRef<Shader> MaterialLibrary::GetFixedFunctionShader(
	const FixedFunctionParameters& params)
{
	bool useCache = params.textures.Size() <= 1;
	if(useCache) {
		for(auto& p : m_FixedFunctionShaders) {
			if(p.params == params)
				return p.shader;
		}
	}

	auto shader = video::VideoDriver::Instance()->CreateFixedFunctionShader(params);
	if(shader)
		m_FixedFunctionShaders.EmplaceBack(params, shader);

	return shader;
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromMemory(
	EShaderLanguage language,
	const core::String& VSCode, const char* VSEntryPoint,
	int VSmajorVersion, int VSminorVersion,
	const core::String& PSCode, const char* PSEntryPoint,
	int PSmajorVersion, int PSminorVersion,
	core::Array<core::String>* errorList)
{
	return video::VideoDriver::Instance()->CreateShader(
		language,
		VSCode.Data_c(), VSEntryPoint, VSCode.Size(),
		VSmajorVersion, VSminorVersion,
		PSCode.Data_c(), PSEntryPoint, PSCode.Size(),
		PSmajorVersion, PSminorVersion,
		errorList);
}

bool MaterialLibrary::IsShaderSupported(
	EShaderLanguage lang,
	int vsMajor, int vsMinor,
	int psMajor, int psMinor)
{
	return video::VideoDriver::Instance()->IsShaderSupported(
		lang,
		vsMajor, vsMinor,
		psMajor, psMinor);
}

bool MaterialLibrary::GetShaderInclude(
	EShaderLanguage language, const core::String& name,
	const void*& outData, size_t& outBytes)
{
	ShaderInclude search(language, name);
	auto it = m_ShaderIncludes.Find(search);
	if(it == m_ShaderIncludes.End())
		return false;
	outData = it->PointerC();
	outBytes = it->GetSize();
	return true;
}

void MaterialLibrary::SetShaderInclude(
	EShaderLanguage language, const core::String& name,
	const void* data, size_t bytes)
{
	ShaderInclude search(language, name);
	auto& mem = m_ShaderIncludes.At(search);
	mem.SetMinSize(bytes);
	memcpy(mem, data, bytes);
}

} // namespace video
} // namespace lux
