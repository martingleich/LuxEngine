#ifndef INCLUDED_PIPELINESETTINGS_H
#define INCLUDED_PIPELINESETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{

enum EPipelineFlag
{
	// Als Gitternetz zeichnen
	EPF_WIREFRAME = 0x1,

	// Als Punktwolke zeichnen
	EPF_POINTCLOUD = 0x2,

	// Flat- oder Gouraud-Shading
	EPF_GOURAUD_SHADING = 0x4,

	// Beleuchtung an oder aus
	EPF_LIGHTING = 0x8,

	// Wird der ZBuffer benutzt
	EPF_ZBUFFER = 0x10,

	// Wird in den ZBuffer geschrieben
	EPF_ZWRITE_ENABLED = 0x20,

	// Werden die Hinterseiten weggeschnitten
	EPF_BACK_FACE_CULLING = 0x40,

	// Werden die Vorderseiten weggeschnitten
	EPF_FRONT_FACE_CULLING = 0x80,

	// Wird der Bilineare Texturfilter benutzt
	EPF_FILTER = 0x100,

	// Wird der Trilineare Texturfilter benutzt(Überschreibt Bilinear und UseMIPMaps)
	EPF_TRILINEAR_FILTER = 0x200,

	// Wird der Anisotropische Texturfilter benutzt
	EPF_ANISOTROPIC = 0x400,

	// Reagiert das material auf Nebel
	EPF_FOG_ENABLED = 0x800,

	//Sollen MIP-Maps benutzt werden
	EPF_USE_MIP_MAPS = 0x1000,

	EPF_NORMALIZE_NORMALS = 0x2000,

	EPF_POLYGON_OFFSET = 0x4000,

	EPF_COLOR_PLANE = 0x8000,
	EPF_MAG_FILTER = 0x10000,
	EPF_MIN_FILTER = 0x20000,
};


// Die Vergleichsfunktion für den Z-Buffer
enum EZComparisonFunc : u32
{
	// Der Z-Test wird nie bestanden
	EZCF_NEVER = 1,
	// zWert(Neu) < zWert(Alt)
	EZCF_LESS = 2,
	// zWert(Neu) = zWert(Alt)
	EZCF_EQUAL = 3,
	// zWert(Neu) <= zWert(Alt)
	EZCF_LESS_EQUAL = 4,
	// zWert(Neu) > zWert(Alt)
	EZCF_GREATER = 5,
	// zWert(Neu) != zWert(Alt)
	EZCF_NOT_EQUAL = 6,
	// zWert(Neu) >= zWert(Alt)
	EZCF_GREATER_EQUAL = 7,
	// Der Z-Test wird immer bestanden
	EZCF_ALWAYS = 8,
};

enum EColorPlane : u32
{
	ECP_NONE = 0x0,
	ECP_ALPHA = 0x1,
	ECP_RED = 0x2,
	ECP_GREEN = 0x4,
	ECP_BLUE = 0x8,

	ECP_RGB = ECP_RED | ECP_GREEN | ECP_BLUE,
	ECP_ALL = ECP_RGB | ECP_ALPHA
};

enum EPolygonOffset : u32
{
	EPO_FRONT = 0,
	EPO_BACK = 1
};

// Blendfaktor für Alphablending
enum EBlendFactor : u32
{
	// Alpha ist immer 0
	EBF_ZERO = 0,
	// Alpha ist immer 1
	EBF_ONE,
	// Alpha ist gleich Quellalpha
	EBF_SRC_ALPHA,
	// Alpha ist gleich 1-Quellalpha
	EBF_ONE_MINUS_SRC_ALPHA,
	// Alpha ist gleich Zielalpha
	EBF_DST_ALPHA,
	// Alpha ist gleich 1-Zielalpha
	EBF_ONE_MINUS_DST_ALPHA,
};

// Wie werden die alte und neue Farbe nach Multiplikation mit
// dem Blendfaktor verknüpft.
// Alle Operationen erfolgen Komponentenweise
enum EBlendOperator : u32
{
	// Es findet kein Alphablending stat
	EBO_NONE = 0,
	// Alte Farbe + Neue Farbe = Ergebnis
	EBO_ADD,
	// Neue Farbe - Alte Farbe = Ergebnis
	EBO_SUBTRACT,
	// Alte Farbe - Neue Farbe = Ergebnis
	EBO_REVSUBTRACT,
	// Min(Alte Farbe, Neue Farbe) = Ergebnis
	EBO_MIN,
	// Max(Alte Farbe, Neue Farbe) = Ergebnis
	EBO_MAX
};

enum ETextureFilter : u32
{
	ETF_POINT,
	ETF_LINEAR,
	ETF_ANISOTROPIC,
};

class PipelineSettings
{
public:
	struct SAlphaBlend
	{
		EBlendFactor SrcBlend : 3;
		EBlendFactor DstBlend : 3;
		EBlendOperator Operator : 3;

		SAlphaBlend() : SrcBlend(EBF_ONE), DstBlend(EBF_ZERO), Operator(EBO_NONE)
		{
		}
	};

	// Reagiert das material auf Nebel
	// default: false
	bool FogEnabled : 1;

	// Beleuchung an oder aus
	// default: true;
	bool Lighting : 1;

	SAlphaBlend Blending;

	// Die Größe von Punkten
	// default: 1.0
	float Thickness;

	// Vergleichsfunktion des ZBuffers
	EZComparisonFunc ZBufferFunc : 4;

	// Wird in den ZBuffer geschrieben
	// Achtung bei Transparenten Materialien
	// default: true
	bool ZWriteEnabled : 1;

	// Als Gitternetz oder als Dreiecke zeichnen
	// default: false
	bool Wireframe : 1;

	//! Should all Normals normalized before Rendering.
	/**
		default: true
	*/
	bool NormalizeNormals : 1;

	// Als Punktwolke oder als Dreiecke zeichnen
	// default: false
	bool Pointcloud : 1;

	// Flat- oder Gouraud-Shading
	// default: true;
	bool GouraudShading : 1;

	// Werden die Dreiecke die wegzeigen nicht gezeichnet
	// default: true
	bool BackfaceCulling : 1;

	// Werden die Dreiecke die herzeigen nicht gezeichnet
	// default: false
	bool FrontfaceCulling : 1;

	// Sollen MIP-Maps benutzt werden
	bool UseMIPMaps : 1;

	// Die geschriebenen Farbebenen
	EColorPlane ColorPlane : 4;

	// Polygon offset von 0-7
	u8 PolygonOffsetFactor : 3;

	// Polygon offset Richtung
	EPolygonOffset PolygonOffsetDirection : 1;

	ETextureFilter MinFilter;
	ETextureFilter MagFilter;
	bool TrilinearFilter : 1;
	u16 Anisotropic : 16;

	PipelineSettings() : Thickness(1.0f), ZBufferFunc(EZCF_LESS_EQUAL), Wireframe(false), Pointcloud(false),
		GouraudShading(true), ZWriteEnabled(true), BackfaceCulling(true), FrontfaceCulling(false),
		FogEnabled(true), UseMIPMaps(true), Lighting(true), NormalizeNormals(true),
		ColorPlane(ECP_ALL), PolygonOffsetFactor(0), PolygonOffsetDirection(EPO_FRONT),
		MinFilter(ETF_LINEAR), MagFilter(ETF_LINEAR), TrilinearFilter(false), Anisotropic(0)
	{
	}

	bool operator!=(const PipelineSettings& other) const
	{
		return  Thickness != other.Thickness ||
			Wireframe != other.Wireframe ||
			Pointcloud != other.Pointcloud ||
			GouraudShading != other.GouraudShading ||
			Lighting != other.Lighting ||
			ZWriteEnabled != other.ZWriteEnabled ||
			BackfaceCulling != other.BackfaceCulling ||
			FrontfaceCulling != other.FrontfaceCulling ||
			FogEnabled != other.FogEnabled ||
			ZBufferFunc != other.ZBufferFunc ||
			NormalizeNormals != other.NormalizeNormals ||
			UseMIPMaps != other.UseMIPMaps ||
			ColorPlane != other.ColorPlane ||
			PolygonOffsetFactor != other.PolygonOffsetFactor ||
			PolygonOffsetDirection != other.PolygonOffsetDirection ||
			MagFilter != other.MagFilter ||
			MinFilter != other.MinFilter ||
			TrilinearFilter != other.TrilinearFilter ||
			Anisotropic != other.Anisotropic;
	}

	bool operator==(const PipelineSettings& other) const
	{
		return !(*this != other);
	}

	void SetFlag(EPipelineFlag Flag, bool Set)
	{
		switch(Flag) {
		case EPF_WIREFRAME:
			Wireframe = Set; break;
		case EPF_POINTCLOUD:
			Pointcloud = Set; break;
		case EPF_GOURAUD_SHADING:
			GouraudShading = Set; break;
		case EPF_LIGHTING:
			Lighting = Set; break;
		case EPF_ZBUFFER:
			ZBufferFunc = Set ? EZCF_LESS_EQUAL : EZCF_ALWAYS; break;
		case EPF_ZWRITE_ENABLED:
			ZWriteEnabled = Set; break;
		case EPF_BACK_FACE_CULLING:
			BackfaceCulling = Set; break;
		case EPF_FRONT_FACE_CULLING:
			FrontfaceCulling = Set; break;
		case EPF_FILTER:
			MagFilter = Set ? ETF_LINEAR : ETF_POINT;
			MinFilter = Set ? ETF_LINEAR : ETF_POINT; break;
		case EPF_MAG_FILTER:
			MagFilter = Set ? ETF_LINEAR : ETF_POINT; break;
		case EPF_MIN_FILTER:
			MinFilter = Set ? ETF_LINEAR : ETF_POINT; break;
		case EPF_TRILINEAR_FILTER:
			TrilinearFilter = Set; break;
		case EPF_ANISOTROPIC:
			Anisotropic = Set ? 65535 : 0; break;
		case EPF_FOG_ENABLED:
			FogEnabled = Set; break;
		case EPF_USE_MIP_MAPS:
			UseMIPMaps = Set; break;
		case EPF_NORMALIZE_NORMALS:
			NormalizeNormals = Set; break;
		case EPF_POLYGON_OFFSET:
			PolygonOffsetFactor = Set ? 1 : 0; break;
		case EPF_COLOR_PLANE:
			ColorPlane = Set ? ECP_ALL : ECP_NONE; break;
		}
	}
};

class PipelineOverwrite
{
public:
	//! The override settings
	PipelineSettings Override;

	//! The values to override, OR´D combination of Pipeline settings
	u32 Flags;

	//! Is the override active
	bool Enabled;

	//! default Constructor
	PipelineOverwrite() : Flags(0), Enabled(false)
	{
	}

	//! Apply the choosen overwrites to some settings
	void ApplyTo(PipelineSettings& settings) const
	{
		if(true) {
			if((Flags&EPF_WIREFRAME) != 0)
				settings.Wireframe = Override.Wireframe;
			if((Flags&EPF_POINTCLOUD) != 0)
				settings.Pointcloud = Override.Pointcloud;
			if((Flags&EPF_GOURAUD_SHADING) != 0)
				settings.GouraudShading = Override.GouraudShading;
			if((Flags&EPF_LIGHTING) != 0)
				settings.Lighting = Override.Lighting;
			if((Flags&EPF_ZBUFFER) != 0)
				settings.ZBufferFunc = Override.ZBufferFunc;
			if((Flags&EPF_ZWRITE_ENABLED) != 0)
				settings.ZWriteEnabled = Override.ZWriteEnabled;
			if((Flags&EPF_BACK_FACE_CULLING) != 0)
				settings.BackfaceCulling = Override.BackfaceCulling;
			if((Flags&EPF_FRONT_FACE_CULLING) != 0)
				settings.FrontfaceCulling = Override.FrontfaceCulling;
			if((Flags&EPF_FILTER) != 0) {
				settings.MinFilter = Override.MinFilter;
				settings.MagFilter = Override.MagFilter;
			}
			if((Flags&EPF_MIN_FILTER) != 0)
				settings.MinFilter = Override.MinFilter;
			if((Flags&EPF_MAG_FILTER) != 0)
				settings.MagFilter = Override.MagFilter;
			if((Flags&EPF_TRILINEAR_FILTER) != 0)
				settings.TrilinearFilter = Override.TrilinearFilter;
			if((Flags&EPF_ANISOTROPIC) != 0)
				settings.Anisotropic = Override.Anisotropic;
			if((Flags&EPF_FOG_ENABLED) != 0)
				settings.FogEnabled = Override.FogEnabled;
			if((Flags&EPF_USE_MIP_MAPS) != 0)
				settings.UseMIPMaps = Override.UseMIPMaps;
			if((Flags&EPF_NORMALIZE_NORMALS) != 0)
				settings.NormalizeNormals = Override.NormalizeNormals;
			if((Flags&EPF_COLOR_PLANE) != 0)
				settings.ColorPlane = Override.ColorPlane;
			if((Flags&EPF_POLYGON_OFFSET) != 0) {
				settings.PolygonOffsetDirection = Override.PolygonOffsetDirection;
				settings.PolygonOffsetFactor = Override.PolygonOffsetFactor;
			}
		}
	}

	u32 GetFlags(u32 flags)
	{
		return (Flags&flags);
	}

	void ClearFlags(u32 flags)
	{
		Flags = Flags&(~flags);
	}

	void SetFlags(u32 flags)
	{
		Flags = Flags | flags;
	}

	void SetFlags(u32 flags, u32 value)
	{
		Flags = (Flags & (~flags)) | (flags&value);
	}
};


} 

} 


#endif // INCLUDED_PIPELINESETTINGS_H
