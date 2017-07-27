#ifndef INCLUDED_COBJMESHLOADER_H
#define INCLUDED_COBJMESHLOADER_H
#include "StaticMesh.h"
#include "video/VertexTypes.h"
#include "video/mesh/GeometryImpl.h"
#include "io/path.h"
#include "video/MaterialLibrary.h"
#include "resources/ResourceLoader.h"
#include "resources/ResourceSystem.h"

#include <map>

namespace lux
{
namespace video
{
class SceneManager;

class MeshLoaderOBJ : public core::ResourceLoader
{
public:
	MeshLoaderOBJ(video::VideoDriver* driver, video::MaterialLibrary* matLib);

	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
	const String& GetName() const;

private:
	// Eine subMesh der OBJ-Datei
	struct SObjMtl
	{
		SObjMtl() : Meshbuffer(0), Bumpiness(1.0f), Illumination(0),
			RecalculateNormals(false)
		{
			Meshbuffer = LUX_NEW(video::GeometryImpl);
		}

		SObjMtl(const SObjMtl& other)
			: name(other.name), Group(other.Group),
			Bumpiness(other.Bumpiness), Illumination(other.Illumination),
			RecalculateNormals(false)
		{
			Meshbuffer = LUX_NEW(video::GeometryImpl);
			material = other.material;
		}

		std::map<video::Vertex3D, s32> VertMap;
		StrongRef<video::GeometryImpl> Meshbuffer;
		StrongRef<video::Material> material;
		String name;
		String Group;
		float Bumpiness;
		u8 Illumination;
		bool RecalculateNormals;
	};

	const char* MoveToFirstWord(const char* pFrom, const char* const pTo, bool bAcrossNewLines = true);

	// Bewegt den Cursor zum nächsten druckbaren Zeichen
	const char* MoveToNextWord(const char* pFrom, const char* pTo, bool bAcrossNewLines = true);

	// Bewegt den Cursor zum nächsten druckbaren Zeichen in der nächsten Zeile
	const char* MoveToNextLine(const char* pFrom, const char* pTo);

	// Liest das aktuelle Wort am Cursor aus
	u32 CopyWord(const char* pFrom, const char* pTo, char* out, u32 dwMaxLength);

	// Liest die aktuelle Zeile am Cursor aus
	String CopyLine(const char* pFrom, const char* pTo);

	// Kombination aus MoveToNextWord und CopyWord
	const char* CopyNextWord(const char* pFrom, const char* pTo, char* out, u32 dwMaxLength);

	// Liest die Texturdaten ein
	const char* ReadTextures(const char* pFrom, const char* const pTo, SObjMtl* pCurrMaterial, const io::FileDescription& fileDesc);

	// Liest das gegebene material ein
	void ReadMaterial(const char* pcFilename, const io::FileDescription& fileDesc);

	// Findet und liefert das material mit dem Namen
	SObjMtl* FindMaterial(const String& mtlName, const String& grpName);

	const char*  ReadColor(const char* pFrom, const char* pTo, video::Colorf& out);
	const char*  Read3DVec(const char* pFrom, const char* pTo, math::Vector3F& out);
	const char*  Read2DVec(const char* pFrom, const char* pTo, math::Vector2F& out);
	const char*  ReadBool(const char* pFrom, const char* pTo, bool& out);

	bool RetrieveVertexIndices(const char* pFrom, const char* pTo, s32* indices, u32 vbsize, u32 vtsize, u32 vnsize);

	void CleanUp();

private:
	WeakRef<video::MaterialLibrary> m_MatLib;
	WeakRef<video::VideoDriver> m_Driver;

	core::Array<SObjMtl*> m_Materials;
};

}
}

#endif
