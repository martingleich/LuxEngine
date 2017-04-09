#ifndef INCLUDED_COBJMESHLOADER_H
#define INCLUDED_COBJMESHLOADER_H
#include "StaticMesh.h"
#include "video/VertexTypes.h"
#include "video/SubMeshImpl.h"
#include "io/path.h"
#include <map>

namespace lux
{
namespace scene
{
class SceneManager;

class MeshLoaderOBJ : public core::ResourceLoader
{
public:
	MeshLoaderOBJ(SceneManager* pSmgr);
	virtual ~MeshLoaderOBJ();

	core::Name GetResourceType(io::File* file, core::Name requestedType);
	bool LoadResource(io::File* file, core::Resource* dst);
	const string& GetName() const;

private:

	// Eine subMesh der OBJ-Datei
	struct SObjMtl
	{
		SObjMtl() : Meshbuffer(0), Bumpiness(1.0f), Illumination(0),
			RecalculateNormals(false)
		{
			Meshbuffer = LUX_NEW(video::SubMeshImpl);
			Meshbuffer->GetMaterial().shininess = 0.0f;
			Meshbuffer->GetMaterial().ambient = 0.2f;
			Meshbuffer->GetMaterial().diffuse = video::Color(204, 204, 204, 255);
			Meshbuffer->GetMaterial().specular = video::Color(255, 255, 255, 255);
		}

		SObjMtl(const SObjMtl& other)
			: name(other.name), Group(other.Group),
			Bumpiness(other.Bumpiness), Illumination(other.Illumination),
			RecalculateNormals(false)
		{
			Meshbuffer = LUX_NEW(video::SubMeshImpl);
			Meshbuffer->SetMaterial(other.Meshbuffer->GetMaterial());
		}

		std::map<video::Vertex3D, s32> VertMap;
		StrongRef<video::SubMeshImpl> Meshbuffer;
		string name;
		string Group;
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
	string CopyLine(const char* pFrom, const char* pTo);

	// Kombination aus MoveToNextWord und CopyWord
	const char* CopyNextWord(const char* pFrom, const char* pTo, char* out, u32 dwMaxLength);

	// Liest die Texturdaten ein
	const char* ReadTextures(const char* pFrom, const char* const pTo, SObjMtl* pCurrMaterial, const io::FileDescription& fileDesc);

	// Liest das gegebene material ein
	void ReadMaterial(const char* pcFilename, const io::FileDescription& fileDesc);

	// Findet und liefert das material mit dem Namen
	SObjMtl* FindMaterial(const string& mtlName, const string& grpName);

	// Liest eine RGB-Farbe
	const char*  ReadColor(const char* pFrom, const char* pTo, video::Colorf& out);
	// Liest einen 3D-Vektor
	const char*  Read3DVec(const char* pFrom, const char* pTo, math::vector3f& out);
	// Liest einen 2D-Vektor
	const char*  Read2DVec(const char* pFrom, const char* pTo, math::vector2f& out);
	// Liest eine Wahrheitswert
	const char*  ReadBool(const char* pFrom, const char* pTo, bool& out);

	// Verknüpft die Vertizes und Indizes in der Datei miteinander
	bool RetrieveVertexIndices(const char* pFrom, const char* pTo, s32* indices, u32 vbsize, u32 vtsize, u32 vnsize);

	// Räumt alles aus
	void CleanUp();

	WeakRef<scene::SceneManager> m_SceneManager;
	StrongRef<io::FileSystem> m_Filesystem;

	core::array<SObjMtl*> m_Materials;
};

}    

}    


#endif
