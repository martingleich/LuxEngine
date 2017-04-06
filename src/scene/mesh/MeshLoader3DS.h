#ifndef INCLUDED_C3DSMESHLOADER_H
#define INCLUDED_C3DSMESHLOADER_H
#include "resources/ResourceSystem.h"
#include "video/Material.h"
#include "StaticMesh.h"
#include "core/lxArray.h"
#include "math/matrix4.h"

namespace lux
{
namespace io
{
class FileSystem;
}
namespace scene
{
class SceneManager;

class MeshLoader3DS : public core::ResourceLoader
{
private:
	// Der Chunkheader der 3DS-Datei
#pragma pack(push, 1)
	struct S3DSChunkHeader
	{
		u16    ChunkID;
		u32 ChunkSize;
	};
#pragma pack(pop)

	struct SChunkData
	{
		SChunkData() : Read(0)
		{
		}

		u32 Read;
		S3DSChunkHeader Header;
	};

	// Ein 3DS-material
	struct S3DSMaterial
	{
		void Clear()
		{
			material.Clear();

			name = "";
			filename[0] = "";
			filename[1] = "";
			filename[2] = "";
			filename[3] = "";
			filename[4] = "";
			Strenght[0] = 0.0f;
			Strenght[1] = 0.0f;
			Strenght[2] = 0.0f;
			Strenght[3] = 0.0f;
			Strenght[4] = 0.0f;
		}

		video::Material material;        // Das material selbst
		string name;            // Der name des Materials
		io::path filename[5];    // Namen für Texturen o.Ä.
		float Strenght[5];    // Allgemeine Werte z.B. 

		S3DSMaterial()
		{
			Clear();
		}

		S3DSMaterial(const S3DSMaterial& other) : material(other.material)
		{
			name = other.name;
			filename[0] = other.filename[0];
			filename[1] = other.filename[1];
			filename[2] = other.filename[2];
			filename[3] = other.filename[3];
			filename[4] = other.filename[4];
			Strenght[0] = other.Strenght[0];
			Strenght[1] = other.Strenght[1];
			Strenght[2] = other.Strenght[2];
			Strenght[3] = other.Strenght[3];
			Strenght[4] = other.Strenght[4];
		}
	};

	// Eine Materialgruppe
	struct SMaterialGroup
	{
		SMaterialGroup() : FaceCount(0), Faces(nullptr)
		{
		}

		SMaterialGroup(const SMaterialGroup& other)
		{
			*this = other;
		}

		~SMaterialGroup()
		{
			Clear();
		}

		void Clear()
		{
			LUX_FREE_ARRAY(Faces);
			Faces = nullptr;
			FaceCount = 0;
		}

		SMaterialGroup& operator=(const SMaterialGroup& other)
		{
			MaterialName = other.MaterialName;
			FaceCount = other.FaceCount;
			Faces = LUX_NEW_ARRAY(u16, FaceCount);
			for(u32 dw = 0; dw < FaceCount; ++dw)
				Faces[dw] = other.Faces[dw];

			return *this;
		}

		string  MaterialName;
		u16 FaceCount;
		u16* Faces;
	};

	WeakRef<SceneManager> m_SceneManager;
	StrongRef<io::FileSystem>    m_Filesystem;

	float* m_pfVertices;        // Die Vertizes, nicht als Vektor gespeichert das der Aufbau
								// der Vektoren in 3DS-Dateien anders ist und so möglichst
								// wenig konvertiert werden muss
	u16 m_wVertexCount;
	float* m_pfTCoords;
	u16 m_wTCoordsCount;

	u16* m_pwIndices;
	u16 m_wIndexCount;
	u16 m_wFaceCount;

	core::array<SMaterialGroup> m_MaterialGroups;
	u32* m_pdwSmoothingGroups;
	core::array<u16> m_TempIndices;

	core::array<S3DSMaterial> m_Materials;
	core::array<string> m_SubMeshNames;
	math::matrix4 m_mTransformation;
	math::aabbox3df m_BoundingBox;

	StrongRef<StaticMesh> m_Mesh;

	StrongRef<io::File> m_pFile;

private:
	bool ReadChunk(SChunkData* parent);
	void ReadMaterialChunk(SChunkData* parent);
	void ReadFrameChunk(SChunkData* parent);
	void ReadTrackChunk(SChunkData& data, video::SubMesh* subMesh, const math::vector3f& vPivot);
	void ReadObjectChunk(SChunkData* parent);
	void ReadPercentageChunk(SChunkData* pChunk, float& Percentage);
	void ReadColorChunk(SChunkData* chunk, video::Color& out);

	void ReadChunkData(SChunkData& data);
	void ReadString(SChunkData& data, string& out);
	void ReadVertices(SChunkData& data);
	void ReadIndices(SChunkData& data);
	void ReadMaterialGroup(SChunkData& data);
	void ReadTextureCoords(SChunkData& data);

	void ComposeObject(const string& name);
	void LoadMaterials();
	void CleanUp();

public:
	MeshLoader3DS(SceneManager* pSmgr);
	~MeshLoader3DS();

	StrongRef<Mesh> CreateMesh(io::File* file);

	bool CanLoadResource(core::Name type, io::File* file);
	void LoadResources(io::File* file, core::array<StrongRef<core::Resource>>& resources, core::array<string>& names);
	const string& GetName() const;
};

}    // namespace scene
}    // namespace lux

#endif