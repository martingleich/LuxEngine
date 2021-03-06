#include "FontRaster.h"

#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/Texture.h"
#include "video/VertexTypes.h"
#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"

LX_REGISTER_REFERABLE_CLASS(lux::gui::FontRaster, "lux.resource.Font");

namespace lux
{
namespace gui
{

namespace
{
core::StringView g_VSCode =
R"(
struct VS_OUT
{
	float4 hpos : POSITION;
	float4 uv_pos : TEXCOORD0;
};

float4x4 scene_viewProj;

VS_OUT mainVS(float3 position : POSITION, float2 uv : TEXCOORD0)
{
	VS_OUT Out;
	Out.hpos = mul(float4(position, 1), scene_viewProj);
	Out.uv_pos.xy = uv;
	Out.uv_pos.zw = position.xy;
	return Out;
}
)";
core::StringView g_PSCode = R"(
sampler2D param_texture;
float4 param_fontColor;
float4 param_borderColor;

float4 mainPS(float4 uv_pos : TEXCOORD0) : COLOR0
{
	float2 uv = uv_pos.xy;
	float2 pos = uv_pos.zw;
	float4 value = tex2D(param_texture, uv);

	float alpha = value.a;
	float inner = value.r;
	float4 color = lerp(param_borderColor, param_fontColor, inner);

	return float4(color.rgb, alpha*color.a);
}
)";

class ShaderParamLoader : public video::ShaderParamSetCallback
{
public:
	struct Data : public video::ShaderParamSetCallback::Data
	{
		video::TextureLayer texture;
		video::ColorF borderColor;
		video::ColorF fontColor;
	};
	int m_TexId;
	int m_BorderColorId;
	int m_FontColorId;
	video::Shader* m_Shader;
	void Init(video::Shader* shader)
	{
		m_TexId = shader->GetParamId("texture");
		m_BorderColorId = shader->GetParamId("borderColor");
		m_FontColorId = shader->GetParamId("fontColor");
		m_Shader = shader;
	}

	void SendShaderSettings(ShaderParamSetCallback::Data* data) const override
	{
		auto dat = dynamic_cast<Data*>(data);
		LX_CHECK_NULL_ARG(dat);
		m_Shader->SetParam(m_TexId, &dat->texture);
		m_Shader->SetParam(m_BorderColorId, &dat->borderColor);
		m_Shader->SetParam(m_FontColorId, &dat->fontColor);
	}
};

WeakRef<video::Shader> g_FontShader;
ShaderParamLoader g_ParamLoader;

StrongRef<video::Shader> EnsureFontShader()
{
	if(g_FontShader)
		return g_FontShader;

	auto shaderFactory = video::ShaderFactory::Instance();
	StrongRef<video::Shader> shader;
	if(shaderFactory->IsShaderSupported(video::EShaderLanguage::HLSL, "vs_2_0", "ps_2_0")) {
		// Use real shader
		shader = shaderFactory->CreateShaderFromMemory(
			video::EShaderLanguage::HLSL,
			g_VSCode, "vs_2_0",
			g_PSCode, "ps_2_0");
	} else {
		// Use fixed function shader
		video::TextureStageSettings tss;
		tss.alphaArg1 = video::ETextureArgument::Texture;
		tss.alphaArg2 = video::ETextureArgument::Diffuse;
		tss.alphaOperator = video::ETextureOperator::Modulate;
		tss.colorArg1 = video::ETextureArgument::Texture;
		tss.colorArg2 = video::ETextureArgument::Diffuse;
		tss.colorOperator = video::ETextureOperator::Modulate;
		shader = shaderFactory->GetFixedFunctionShader(
			video::FixedFunctionParameters::Unlit({"textures"}, {tss}, true));
	}
	g_ParamLoader.Init(shader);
	g_FontShader = shader.GetWeak();
	return shader;
}

}

FontRaster::FontRaster()
{
}

FontRaster::~FontRaster()
{
}

void FontRaster::Init(const FontCreationData& data)
{
	m_Desc = data.desc;

	m_CharHeight = data.charHeight;
	m_CharMap = data.charMap;
	m_BaseLine = data.baseLine;
	m_BaseSettings = data.baseSettings;

	// Set replacement character.
	core::String errorChars = "�? ";
	for(auto c : errorChars.CodePoints()) {
		auto jtOpt = m_CharMap.Find(c);
		if(jtOpt.HasValue()) {
			m_ErrorChar = jtOpt.GetValue()->value;
			break;
		}
	}

	LoadImageData(data.image, data.imageSize, data.channelCount);

	InitPass();
}

void FontRaster::Draw(
	const FontRenderSettings& _settings,
	const core::StringView& text,
	const math::Vector2F& position,
	const math::RectF* userClip)
{
	if(text.IsEmpty())
		return;

	auto settings = GetFinalFontSettings(_settings);

	const float slanting = m_CharHeight * settings.slanting * settings.scale;
	const float charHeight = m_CharHeight * settings.scale;
	const float charSpace = settings.charDistance * settings.scale;

	auto renderer = video::VideoDriver::Instance()->GetRenderer();

	math::RectI clipRect = renderer->GetScissorRect();
	if(userClip) {
		clipRect.FitInto(math::RectI(
			(u32)math::Max(0.0f, userClip->left),
			(u32)math::Max(0.0f, userClip->top),
			(u32)math::Max(0.0f, std::ceil(userClip->right)),
			(u32)math::Max(0.0f, std::ceil(userClip->bottom))));
	}

	if(position.y + charHeight + 1 < (float)clipRect.top
		|| position.y - 1 > (float)clipRect.bottom)
		return;

	video::ScissorRectToken tok;
	if(userClip)
		renderer->SetScissorRect(clipRect, &tok);

	ShaderParamLoader::Data shaderData;
	shaderData.texture = video::TextureLayer(m_Texture);
	shaderData.borderColor = settings.borderColor;
	shaderData.fontColor = settings.color;
	if(!m_Pass.shader)
		renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);

	renderer->SendPassSettingsEx(video::ERenderMode::Mode2D, m_Pass, false, &g_ParamLoader, &shaderData);
	math::Vector2F cursor = position;
	video::Vertex2D vertices[600];
	u32 vertexCursor = 0;
	for(u32 character : text.CodePoints()) {
		const CharInfo& info = GetCharInfo(character);

		if(character == ' ') {
			cursor.x += (info.A + info.B + info.C) * settings.scale * settings.wordDistance;
			cursor.x += charSpace;
			continue;
		}

		const float charWidth = info.B * settings.scale;

		cursor.x += info.A * settings.scale;

		// Top-Left
		vertices[vertexCursor].position.x = std::floor(cursor.x + slanting);
		vertices[vertexCursor].position.y = std::floor(cursor.y);
		vertices[vertexCursor].texture.x = info.left;
		vertices[vertexCursor].texture.y = info.top;

		// Top-Right
		vertices[vertexCursor + 1].position.x = std::floor(cursor.x + charWidth + slanting);
		vertices[vertexCursor + 1].position.y = std::floor(cursor.y);
		vertices[vertexCursor + 1].texture.x = info.right;
		vertices[vertexCursor + 1].texture.y = info.top;

		// Lower-Right
		vertices[vertexCursor + 2].position.x = std::floor(cursor.x + charWidth);
		vertices[vertexCursor + 2].position.y = std::floor(cursor.y + charHeight);
		vertices[vertexCursor + 2].texture.x = info.right;
		vertices[vertexCursor + 2].texture.y = info.bottom;

		vertices[vertexCursor + 3] = vertices[vertexCursor];
		vertices[vertexCursor + 4] = vertices[vertexCursor + 2];

		// Lower-Left
		vertices[vertexCursor + 5].position.x = std::floor(cursor.x);
		vertices[vertexCursor + 5].position.y = std::floor(cursor.y + charHeight);
		vertices[vertexCursor + 5].texture.x = info.left;
		vertices[vertexCursor + 5].texture.y = info.bottom;

		cursor.x += charWidth + info.C * settings.scale + charSpace;

		float maxX = math::Max(
			vertices[vertexCursor + 1].position.x,
			vertices[vertexCursor + 2].position.x) + 1;
		float minX = math::Min(
			vertices[vertexCursor].position.x,
			vertices[vertexCursor + 5].position.x) - 1;
		if(minX > 0) {
			if(minX > clipRect.right)
				break; // Abort rendering loop
		}

		if(maxX < 0 || maxX < clipRect.left)
			continue; // Abort this character

		// Add this character to the chunk
		vertexCursor += 6;

		if(vertexCursor >= 600) {
			renderer->Draw(video::RenderRequest::FromMemory(
				video::EPrimitiveType::Triangles, vertexCursor / 3,
				vertices, vertexCursor, video::VertexFormat::STANDARD_2D));

			vertexCursor = 0;
		}
	}

	if(vertexCursor > 0) {
		renderer->Draw(video::RenderRequest::FromMemory(
			video::EPrimitiveType::Triangles, vertexCursor / 3,
			vertices, vertexCursor, video::VertexFormat::STANDARD_2D));
	}
}

float FontRaster::GetTextWidth(const FontRenderSettings& settings, const core::StringView& text)
{
	float width = 0;
	IterateCarets(settings, text, [&](float f, int byte) -> bool {
		LUX_UNUSED(byte);
		width = f;
		return true;
	});
	return width;
}

int FontRaster::GetCaretFromOffset(const FontRenderSettings& settings, const core::StringView& text, float XPosition)
{
	if(XPosition < 0.0f)
		return 0;
	if(text.IsEmpty())
		return 0;
	float lastCaret = 0;
	int finalByte;
	IterateCarets(settings, text, [&](float f, int byte)->bool
	{
		if(XPosition >= lastCaret && XPosition <= f) {
			const float d1 = XPosition - lastCaret;
			const float d2 = f - XPosition;
			if(d1 > d2)
				finalByte = byte;
			return false;
		}
		lastCaret = f;
		finalByte = byte;
		return true;
	});

	return finalByte;
}

void FontRaster::GetTextCarets(const FontRenderSettings& settings, const core::StringView& text, core::Array<FontCaret>& carets)
{
	IterateCarets(settings, text, [&](float f, int byte)->bool
	{
		carets.EmplaceBack(f, byte);
		return true;
	});
}

const CharInfo& FontRaster::GetCharInfo(u32 c)
{
	return m_CharMap.Get(c, m_ErrorChar);
}

FontRenderSettings FontRaster::GetFinalFontSettings(const FontRenderSettings& _settings)
{
	auto settings = _settings;
	settings.charDistance += m_BaseSettings.charDistance;
	settings.lineDistance *= m_BaseSettings.lineDistance;
	settings.scale *= m_BaseSettings.scale;
	settings.slanting += m_BaseSettings.slanting;
	settings.wordDistance *= m_BaseSettings.wordDistance;
	return settings;
}

void FontRaster::InitPass()
{
	m_Pass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_Pass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_Pass.alpha.blendOperator = video::EBlendOperator::Add;
	m_Pass.zWriteEnabled = false;
	m_Pass.zBufferFunc = video::EComparisonFunc::Always;
	m_Pass.culling = video::EFaceSide::None;
	m_Pass.shader = EnsureFontShader();
}

void FontRaster::LoadImageData(
	const u8* imageData,
	math::Dimension2I imageSize,
	int channelCount)
{
	lxAssert(imageData);
	lxAssert(channelCount > 0);

	m_ImageSize = imageSize;
	m_ChannelCount = channelCount;

	// Create backup image
	int imageBytes = m_ImageSize.GetArea() * m_ChannelCount;
	m_Image.Resize(imageBytes);
	memcpy(m_Image.Data(), imageData, imageBytes);

	// Create texture
	m_Texture = video::VideoDriver::Instance()->CreateTexture(
		m_ImageSize, video::ColorFormat::A8R8G8B8, 1, false);
	m_Texture->SetFiltering(video::BaseTexture::Filter::Point);

	video::TextureLock lock(m_Texture, video::BaseTexture::ELockMode::Overwrite);

	const u8* srcRow = imageData;
	auto srcPitch = m_ChannelCount * m_ImageSize.width;
	u8* texRow = lock.data;
	for(int y = 0; y < m_ImageSize.width; ++y) {
		const u8* srcPixel = srcRow;
		u8* texPixel = texRow;

		// Copy to backup
		for(int x = 0; x < m_ImageSize.width; ++x) {
			u8 inner = m_ChannelCount == 2 ? srcPixel[1] : 255;
			*texPixel++ = inner;        // Blue
			*texPixel++ = inner;        // Green
			*texPixel++ = inner;        // Red
			*texPixel++ = srcPixel[0];        // Alpha

			srcPixel += m_ChannelCount;
		}

		texRow += lock.pitch;
		srcRow += srcPitch;
	}
}

const core::HashMap<u32, CharInfo>& FontRaster::GetCharMap() const
{
	return m_CharMap;
}

void FontRaster::GetRawData(const void*& ptr, int& width, int& height, int& channels)
{
	ptr = m_Image.Data();
	width = m_ImageSize.width;
	height = m_ImageSize.height;
	channels = m_ChannelCount;
}

const FontDescription& FontRaster::GetDescription() const
{
	return m_Desc;
}

const FontRenderSettings& FontRaster::GetBaseFontSettings()
{
	return m_BaseSettings;
}

float FontRaster::GetBaseLine() const
{
	return m_BaseLine;
}

float FontRaster::GetFontHeight() const
{
	return m_CharHeight;
}

core::Name FontRaster::GetReferableType() const
{
	return core::ResourceType::Font;
}

StrongRef<core::Referable> FontRaster::Clone() const
{
	return LUX_NEW(FontRaster)(*this);
}

} // namespace gui
} // namespace lux

