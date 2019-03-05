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

static StrongRef<ShaderFactory> g_ShaderFactory;

void ShaderFactory::Initialize()
{
	if(g_ShaderFactory)
		throw core::InvalidOperationException("Shader factory already initialized.");
	g_ShaderFactory = LUX_NEW(ShaderFactory);
}

ShaderFactory* ShaderFactory::Instance()
{
	return g_ShaderFactory;
}

void ShaderFactory::Destroy()
{
	g_ShaderFactory.Reset();
}

ShaderFactory::ShaderFactory() :
	m_Driver(VideoDriver::Instance())
{
	LX_CHECK_NULL_ARG(m_Driver);

	// TODO: Move into dependency injection.
	core::StringView luxHLSLInclude(
		R"(
// d is the distance from the geomtry to the camera.
// Returns 0 for minimal fog effect, 1 for maximal effect
float lxFog(float d, float4 fogA, float4 fogB) 
{
	float start = fogB.g;
	float end = fogB.b;
	float dens = fogB.a;
	if(fogB.r == 1)
		return clamp((d-start) / (end - start), 0, 1);
	else if(fogB.r == 2)
		return clamp(1-exp(-d*dens), 0, 1);
	else if(fogB.r == 3)
		return clamp(1-exp(-pow(d*dens,2)), 0, 1);
	else
		return 0;
}

float4 lxIlluminate(float3 camPos, float3 pos, float3 normal, float4 ambient, float emissive, float4 diffuse, float specular, float specularHardness, float lighting, float4x4 light)
{
	float4 color = float4(0,0,0,0);
	int iLighting = (int)lighting;

	if(iLighting % 2 != 0) // Ambient Emissive
		color += ambient + emissive*diffuse;
	
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
		if(specularHardness > 0) {
			float3 h = normalize(normalize(camPos - pos) + lightVector);
			float sf = pow(dot(h, normal), specularHardness);
			illumCol += lightDiffuse * specular * sf;
		}
		color += illumFactor * illumCol;
	}

	return color;
}
)");
	SetShaderInclude(EShaderLanguage::HLSL, "lux", luxHLSLInclude);// No terminating null
}

ShaderFactory::~ShaderFactory()
{
}


StrongRef<Shader> ShaderFactory::CreateShaderFromFile(
	EShaderLanguage language,
	const io::Path& vsPath, core::StringView vsProfile,
	const io::Path& psPath, core::StringView psProfile,
	ShaderCompileInfo* outInfo)
{
	StrongRef<io::File> PSFile;
	StrongRef<io::File> VSFile;
	if(vsPath == vsPath) {
		PSFile = VSFile = io::FileSystem::Instance()->OpenFile(vsPath);
	} else {
		PSFile = io::FileSystem::Instance()->OpenFile(psPath);
		VSFile = io::FileSystem::Instance()->OpenFile(vsPath);
	}

	core::RawMemory vsCodeBuffer;
	char* vsCodePtr = (char*)VSFile->GetBuffer();
	int vsCodeSize = core::SafeCast<int>(VSFile->GetSize());
	if(!vsCodePtr) {
		vsCodeBuffer.SetSize((size_t)vsCodeSize);
		vsCodePtr = vsCodeBuffer;
		VSFile->ReadBinary(VSFile->GetSize(), vsCodePtr);
	}
	core::StringView vsCode(vsCodePtr, vsCodeSize);

	core::RawMemory psCodeBuffer;
	char* psCodePtr;
	int psCodeSize = 0;
	if(PSFile == VSFile) {
		psCodePtr = vsCodePtr;
		psCodeSize = vsCodeSize;
	} else {
		psCodePtr = (char*)PSFile->GetBuffer();
		if(!psCodePtr) {
			psCodeSize = core::SafeCast<int>(VSFile->GetSize());
			psCodeBuffer.SetSize((size_t)psCodeSize);
			psCodePtr = psCodeBuffer;
			PSFile->ReadBinary(PSFile->GetSize(), psCodePtr);
		}
	}
	core::StringView psCode(psCodePtr, psCodeSize);

	return CreateShaderFromMemory(
		language,
		vsCode, vsProfile,
		psCode, psProfile,
		outInfo);
}

StrongRef<Shader> ShaderFactory::GetFixedFunctionShader(
	const FixedFunctionParameters& params)
{
	// Why only use the cache sometimes.
	bool useCache = params.textures.Size() <= 1;
	if(useCache) {
		for(auto& p : m_FixedFunctionShaders) {
			if(p.params == params)
				return p.shader;
		}
	}

	auto shader = m_Driver->CreateFixedFunctionShader(params);
	m_FixedFunctionShaders.EmplaceBack(params, shader);
	return shader;
}

StrongRef<Shader> ShaderFactory::CreateShaderFromMemory(
	EShaderLanguage language,
	core::StringView vsCode, core::StringView vsProfile,
	core::StringView psCode, core::StringView psProfile,
	ShaderCompileInfo* outInfo)
{
	core::Array<ShaderCompileMessage> messages;
	auto shader = m_Driver->CreateShader(
		language,
		vsCode, vsProfile,
		psCode, psProfile,
		messages);

	if(outInfo) {
		outInfo->failed = shader == nullptr;
		outInfo->messages = messages;
	} else if(!shader) {
		throw UnhandledShaderCompileErrorException();
	}

	return shader;
}

bool ShaderFactory::IsShaderSupported(EShaderLanguage lang, core::StringView vsProfile, core::StringView psProfile)
{
	return m_Driver->IsShaderSupported(lang, vsProfile, psProfile);
}

bool ShaderFactory::GetShaderInclude(
	EShaderLanguage language, core::StringView name,
	core::StringView& outData)
{
	ShaderInclude search(language, name);
	auto it = m_ShaderIncludes.Find(search);
	if(it == m_ShaderIncludes.End())
		return false;
	outData = core::StringView((const char*)it->Pointer(), (int)it->GetSize());
	return true;
}

void ShaderFactory::SetShaderInclude(
	EShaderLanguage language, core::StringView name,
	core::StringView data)
{
	ShaderInclude search(language, name);
	m_ShaderIncludes.At(search).Set(data.Data(), data.Size());
}

/////////////////////////////////////////////////////////////////////

static StrongRef<MaterialLibrary> g_MaterialLibrary;

core::StringView MaterialLibrary::SolidName("solid");
core::StringView MaterialLibrary::DebugOverlayName("debugOverlay");
core::StringView MaterialLibrary::TransparentName("transparent");

void MaterialLibrary::Initialize()
{
	if(g_MaterialLibrary)
		throw core::InvalidOperationException("Material library already initialized.");
	g_MaterialLibrary = LUX_NEW(MaterialLibrary);
}

MaterialLibrary* MaterialLibrary::Instance()
{
	return g_MaterialLibrary;
}

void MaterialLibrary::Destroy()
{
	g_MaterialLibrary.Reset();
}

MaterialLibrary::MaterialLibrary() :
	m_Driver(VideoDriver::Instance()),
	m_ShaderFactory(ShaderFactory::Instance())
{
	LX_CHECK_NULL_ARG(m_Driver);
	LX_CHECK_NULL_ARG(m_ShaderFactory);

	m_ShadowCasterShader = ShaderFactory::Instance()->CreateShaderFromMemory(
		EShaderLanguage::HLSL,
R"(
float4x4 scene_worldViewProj;
void mainVS(float3 pos : POSITION, out float4 hpos : POSITION, out float distance : TEXCOORD0)
{
	hpos = mul(float4(pos, 1.0), scene_worldViewProj);
	distance = hpos.z;
}
)", "vs_2_0",
R"(
float4 mainPS(float distance : TEXCOORD0) : COLOR0
{
	return float4(distance, 0, 0, 1);	
}
)", "ps_2_0", nullptr);
	// TODO: Move into dependency injection.
	{
		auto shader = m_ShaderFactory->GetFixedFunctionShader(FixedFunctionParameters::Lit({"texture"}, {TextureStageSettings()}));
		auto solid = CreateSolidMaterial(shader);
		SetMaterial(SolidName, solid);
	}

	{
		Pass pass;
		pass.fogEnabled = false;
		pass.lighting = ELightingFlag::Disabled;

		TextureStageSettings tss;
		tss.colorOperator = ETextureOperator::SelectArg1;
		tss.colorArg1 = ETextureArgument::Diffuse;
		pass.shader = m_ShaderFactory->GetFixedFunctionShader(FixedFunctionParameters::Unlit({"texture"}, {tss}, true));

		auto debug = CreateMaterial(pass, EMaterialReqFlag::None);
		SetMaterial(DebugOverlayName, debug);
	}

	{
		auto shader = m_ShaderFactory->GetFixedFunctionShader(FixedFunctionParameters::Lit({"texture"}, {TextureStageSettings()}));
		auto transparent = CreateTransparentMaterial(shader);
		SetMaterial(TransparentName, transparent);
	}

}

MaterialLibrary::~MaterialLibrary()
{
}

void MaterialLibrary::SetMaterial(
	core::StringView name, Material* material)
{
	auto& key = m_MaterialMap.At(name, m_MaterialList.Size());
	if(key == m_MaterialList.Size())
		m_MaterialList.PushBack(material);
	else
		m_MaterialList[key] = material;
}

StrongRef<Material> MaterialLibrary::GetMaterial(core::StringView name)
{
	auto itOpt = m_MaterialMap.Find(name);
	if(!itOpt.HasValue())
		throw core::ObjectNotFoundException(name);
	return m_MaterialList[itOpt.GetValue()->value];
}

StrongRef<Material> MaterialLibrary::TryGetMaterial(core::StringView name)
{
	auto itOpt = m_MaterialMap.Find(name);
	if(!itOpt.HasValue())
		return nullptr;
	return m_MaterialList[itOpt.GetValue()->value];
}

StrongRef<Material> MaterialLibrary::CloneMaterial(core::StringView name)
{
	return GetMaterial(name)->Clone();
}

StrongRef<Material> MaterialLibrary::CreateMaterial(const Pass& pass, EMaterialReqFlag reqs)
{
	return LUX_NEW(Material)(Material::Configuration(pass, reqs));
}
StrongRef<video::Material> MaterialLibrary::CreateMaterial(const Material::Configuration& config)
{
	return LUX_NEW(Material)(config);
}
StrongRef<Material> MaterialLibrary::CreateSolidMaterial(Shader* shader)
{
	Pass p;
	p.shader = shader;

	Pass p2;
	p2.shader = m_ShadowCasterShader;

	return LUX_NEW(Material)(Material::Configuration(p, EMaterialReqFlag::None).Tech(EMaterialTechnique::ShadowCaster, p2));
}

StrongRef<Material> MaterialLibrary::CreateTransparentMaterial(Shader* shader)
{
	Pass p;
	p.shader = shader;
	p.alpha.srcFactor = EBlendFactor::SrcAlpha;
	p.alpha.dstFactor = EBlendFactor::OneMinusSrcAlpha;
	p.alpha.blendOperator = EBlendOperator::Add;
	p.zWriteEnabled = false;
	p.fogEnabled = false;
	return CreateMaterial(p, EMaterialReqFlag::Transparent);
}

} // namespace video
} // namespace lux
