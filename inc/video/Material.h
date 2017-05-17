#ifndef INCLUDED_MATERIAL_H
#define INCLUDED_MATERIAL_H
#include "video/Color.h"
#include "MaterialRenderer.h"
#include "MaterialLayer.h"

namespace lux
{
namespace video
{

// Woher kommen die Alphawerte
enum EAlphaSource
{
	// Aus der Vertexfarbe
	EAS_VERTEX_COLOR = 1,
	// Aus der gesetzten Textur
	EAS_TEXTURE = 2,
	// Aus vertex und textur
	EAS_VERTEX_AND_TEXTURE = 3,
};

// Benutzt ein Blendingfaktor Alphawerte
inline bool TextureBlendFunc_HasAlpha(const EBlendFactor factor)
{
	switch(factor) {
	case EBF_SRC_ALPHA:
	case EBF_ONE_MINUS_SRC_ALPHA:
	case EBF_DST_ALPHA:
	case EBF_ONE_MINUS_DST_ALPHA:
		return true;
	default:
		return false;
	}
}

// Packt SrcFactor, DstFactor, Modulate und Alphaquelle in einen float für "fUnknown1"
inline u32 Pack_TextureBlendFunc(EBlendFactor SrcFactor,
	EBlendFactor DstFactor,
	EBlendOperator Operator = EBO_ADD,
	EAlphaSource AlphaSrc = EAS_TEXTURE,
	u32 Value = 0)
{
	// 0000000000000000aaaaoooossssdddd
	return (Value << 16) | ((AlphaSrc << 12) & 0xF000) | ((Operator << 8) & 0xF00) | ((SrcFactor << 4) & 0xF0) | (DstFactor & 0xF);
}

// Entpackt SrcFactor, DstFactor, Modulate und Alphaquelle auf "fUnknwon1"
inline void Unpack_TextureBlendFunc(EBlendFactor& SrcFactor,
	EBlendFactor& DstFactor,
	EBlendOperator& Operator,
	EAlphaSource& AlphaSrc,
	u32& Value,
	u32 Packed)
{
	AlphaSrc = EAlphaSource((Packed & 0x0000F000) >> 12);
	Operator = EBlendOperator((Packed & 0x00000F00) >> 8);
	SrcFactor = EBlendFactor((Packed & 0x000000F0) >> 4);
	DstFactor = EBlendFactor((Packed & 0x0000000F));
	Value = (Packed & 0xFFFF0000) >> 16;
}

class Material : public RenderData
{
public:
	core::PackageParam Param(const string& name)
	{
		return m_Puffer.FromName(name, false);
	}

	core::PackageParam Param(const string& name) const
	{
		return m_Puffer.FromName(name, true);
	}

	core::PackageParam Param(u32 id)
	{
		return m_Puffer.FromID(id, false);
	}

	core::PackageParam Param(u32 id) const
	{
		return m_Puffer.FromID(id, true);
	}

	core::PackageParam Layer(u32 layer)
	{
		return m_Puffer.FromType(core::Type::Texture, layer, false);
	}

	core::PackageParam Layer(u32 layer) const
	{
		return m_Puffer.FromType(core::Type::Texture, layer, false);
	}

	u32 GetTextureCount() const
	{
		return m_Puffer.GetTextureCount();
	}

	MaterialRenderer* GetRenderer() const
	{
		return (MaterialRenderer*)m_Type;
	}

	void SetRenderer(MaterialRenderer* renderer)
	{
		m_Type = (RenderType*)renderer;
		if(renderer)
			m_Puffer.SetType(&m_Type->GetPackage());
		else
			m_Puffer.SetType(nullptr);
	}

public:
	//-------------------------------------------------------------
	// Echte Materialdaten
	// Diese Daten enthält jedes Material per definition, alle anderen Parameter sind mittels Param-Methode erreichbar

	// Hintergrundfaktor
	// Wie stark reagiert das Material auf die globale Hintergrundfarbe
	// default: 1.0
	float ambient;

	// Streufarbe
	// default: LightGray
	Colorf diffuse;

	// Eigenfarbe
	// Wie stark erhellt sich das Material selbst
	// default: Black
	Colorf emissive;

	// Glanzfarbe
	// Wie glänzt das Material
	// default: White
	Colorf specular;

	// Glanzkraft
	// 0 = Kein Glanz; Ansonten desto größer der Wert, desto schärferer Glanzpunkt
	// default: 0.0
	float shininess;

	// Konstuktor
	Material() :
		ambient(1.0f),
		diffuse(Color::LightGray),
		emissive(Color::Black),
		specular(Color::White),
		shininess(0.0f)
	{
	}

	Material(MaterialRenderer* renderer) :
		RenderData((RenderType*)renderer),
		ambient(1.0f),
		diffuse(Color::LightGray),
		emissive(Color::Black),
		specular(Color::White),
		shininess(0.0f)
	{
	}

	Material(const Material& other)
	{
		this->Set(other);
	}

	~Material()
	{
		SetRenderer(nullptr);
	}

	bool operator==(const Material& Other) const
	{
		return  m_Puffer == Other.m_Puffer &&
			ambient == Other.ambient &&
			diffuse == Other.diffuse &&
			emissive == Other.emissive &&
			specular == Other.specular &&
			shininess == Other.shininess;
	}

	bool operator!=(const Material& Other) const
	{
		return !(*this == Other);
	}

	Material& operator=(const Material& other)
	{
		return this->Set(other);
	}

	void Clear()
	{
		m_Puffer.Reset();
	}

	const core::PackagePuffer& GetPuffer() const
	{
		return m_Puffer;
	}

	Material& Set(const Material& Other)
	{
		if(this == &Other)
			return *this;

		m_Puffer = Other.m_Puffer;
		m_Type = Other.m_Type;

		ambient = Other.ambient;
		diffuse = Other.diffuse;
		emissive = Other.emissive;
		specular = Other.specular;
		shininess = Other.shininess;

		return *this;
	}
};

LUX_API extern Material IdentityMaterial;
LUX_API extern Material WorkMaterial;

}    

}    


#endif










