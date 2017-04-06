#include "MeshLoader3DS.h"
#include "video/SubMeshImpl.h"
#include "core/Logger.h"
#include "video/VideoDriver.h"
#include "math/matrix4.h"
#include "scene/SceneManager.h"
#include "io/file.h"
#include "io/filesystem.h"
#include "video/images/ImageSystem.h"
#include "video/MaterialLibrary.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/Texture.h"

#ifdef _DEBUG
#define _LUX_DEBUG_3DS_LOADER
#endif

namespace lux
{
namespace scene
{

namespace
{
enum E3DSChunk
{
	// Magic-Number der 3DS-Datei
	C3DS_MAIN3DS = 0x4D4D,

	// Haupt-Chunks
	C3DS_EDIT3DS = 0x3D3D,
	C3DS_KEYF3DS = 0xB000,
	C3DS_VERSION = 0x0002,
	C3DS_MESHVERSION = 0x3D3E,

	// Unter-Chunks von C3DS_EDIT3DS
	C3DS_EDIT_MATERIAL = 0xAFFF,
	C3DS_EDIT_OBJECT = 0x4000,

	// Unter-Chunks von C3DS_EDIT_MATERIAL
	C3DS_MATNAME = 0xA000,
	C3DS_MATAMBIENT = 0xA010,
	C3DS_MATDIFFUSE = 0xA020,
	C3DS_MATSPECULAR = 0xA030,
	C3DS_MATSHININESS = 0xA040,
	C3DS_MATSHIN2PCT = 0xA041,
	C3DS_TRANSPARENCY = 0xA050,
	C3DS_TRANSPARENCY_FALLOFF = 0xA052,
	C3DS_REFL_BLUR = 0xA053,
	C3DS_TWO_SIDE = 0xA081,
	C3DS_WIRE = 0xA085,
	C3DS_SHADING = 0xA100,
	C3DS_MATTEXMAP = 0xA200,
	C3DS_MATSPECMAP = 0xA204,
	C3DS_MATOPACMAP = 0xA210,
	C3DS_MATREFLMAP = 0xA220,
	C3DS_MATBUMPMAP = 0xA230,
	C3DS_MATMAPFILE = 0xA300,
	C3DS_MAT_TEXTILING = 0xA351,
	C3DS_MAT_USCALE = 0xA354,
	C3DS_MAT_VSCALE = 0xA356,
	C3DS_MAT_UOFFSET = 0xA358,
	C3DS_MAT_VOFFSET = 0xA35A,

	// Unter-Chunks von C3DS_EDIT_OBJECT
	C3DS_OBJTRIMESH = 0x4100,

	// Unter-Chunks von C3DS_OBJTRIMESH
	C3DS_TRIVERT = 0x4110,
	C3DS_POINTFLAGARRAY = 0x4111,
	C3DS_TRIFACE = 0x4120,
	C3DS_TRIFACEMAT = 0x4130,
	C3DS_TRIUV = 0x4140,
	C3DS_TRISMOOTH = 0x4150,
	C3DS_TRIMATRIX = 0x4160,
	C3DS_MESHCOLOR = 0x4165,
	C3DS_DIRECT_LIGHT = 0x4600,
	C3DS_DL_INNER_RANGE = 0x4659,
	C3DS_DL_OUTER_RANGE = 0x465A,
	C3DS_DL_MULTIPLIER = 0x465B,
	C3DS_CAMERA = 0x4700,
	C3DS_CAM_SEE_CONE = 0x4710,
	C3DS_CAM_RANGES = 0x4720,

	// Unter-Chunks von C3DS_KEYF3DS
	C3DS_KF_HDR = 0xB00A,
	C3DS_AMBIENT_TAG = 0xB001,
	C3DS_OBJECT_TAG = 0xB002,
	C3DS_CAMERA_TAG = 0xB003,
	C3DS_target_TAG = 0xB004,
	C3DS_LIGHTNODE_TAG = 0xB005,
	C3DS_KF_SEG = 0xB008,
	C3DS_KF_CURTIME = 0xB009,
	C3DS_KF_NODE_HDR = 0xB010,
	C3DS_PIVOTPOINT = 0xB013,
	C3DS_BOUNDBOX = 0xB014,
	C3DS_MORPH_SMOOTH = 0xB015,
	C3DS_POS_TRACK_TAG = 0xB020,
	C3DS_ROT_TRACK_TAG = 0xB021,
	C3DS_SCL_TRACK_TAG = 0xB022,
	C3DS_NODE_ID = 0xB030,

	// Blickfeld Definitionen
	C3DS_VIEWPORT_LAYOUT = 0x7001,
	C3DS_VIEWPORT_DATA = 0x7011,
	C3DS_VIEWPORT_DATA_3 = 0x7012,
	C3DS_VIEWPORT_SIZE = 0x7020,

	// Verschiedene Farbformate
	C3DS_COL_RGB = 0x0010,
	C3DS_COL_TRU = 0x0011,
	C3DS_COL_LIN_24 = 0x0012,
	C3DS_COL_LIN_F = 0x0013,

	// Prozent-Chunk-Formate
	C3DS_PERCENTAGE_I = 0x0030,
	C3DS_PERCENTAGE_F = 0x0031,

	// Maximale Anzahl möglicher Chunk-Typen
	C3DS_CHUNK_MAX = 0xFFFF
};
}

// Konstruktor

bool MeshLoader3DS::CanLoadResource(core::Name type, io::File* file)
{
	if(type != core::ResourceType::Mesh && type != core::Name::INVALID)
		return false;

	if(!file)
		return false;

	S3DSChunkHeader header;
	if(file->ReadBinary(sizeof(header), &header) != sizeof(header))
		return false;
	if(header.ChunkID != C3DS_MAIN3DS)
		return false;
	return true;
}

void MeshLoader3DS::LoadResources(io::File* file, core::array<StrongRef<core::Resource>>& resources, core::array<string>& names)
{
	StrongRef<Mesh> m = CreateMesh(file);
	if(m != nullptr) {
		resources.Push_Back(m);
		names.Push_Back(string::EMPTY);
	}
}

const string& MeshLoader3DS::GetName() const
{
	static const string name = L"Lux 3DS-Loader";
	return name;
}

MeshLoader3DS::MeshLoader3DS(SceneManager* pSmgr)
	: m_SceneManager(pSmgr), m_pfVertices(nullptr),
	m_wVertexCount(0), m_pfTCoords(nullptr), m_wTCoordsCount(0), m_pwIndices(nullptr),
	m_wIndexCount(0), m_pdwSmoothingGroups(nullptr), m_Mesh(nullptr), m_pFile(nullptr)
{
	m_Filesystem = m_SceneManager->GetFileSystem();
}

// Destruktor
MeshLoader3DS::~MeshLoader3DS()
{
	CleanUp();
}

void MeshLoader3DS::CleanUp()
{
	LUX_FREE_ARRAY(m_pfVertices);
	m_pfVertices = nullptr;
	m_wVertexCount = 0;

	LUX_FREE_ARRAY(m_pwIndices);
	m_pwIndices = nullptr;
	m_wIndexCount = 0;

	LUX_FREE_ARRAY(m_pdwSmoothingGroups);
	m_pdwSmoothingGroups = nullptr;
	m_wFaceCount = 0;

	LUX_FREE_ARRAY(m_pfTCoords);
	m_pfTCoords = nullptr;
	m_wTCoordsCount = 0;

	m_MaterialGroups.Clear();
}

// Erstellt das Modell aus einer Datei, liefert NULL bei Fehler
StrongRef<Mesh> MeshLoader3DS::CreateMesh(io::File* file)
{
	SChunkData data;
	// Datei zum Lesen setzen
	m_pFile = file;

	// Passt die Magic-Number
	ReadChunkData(data);
	if(data.Header.ChunkID != C3DS_MAIN3DS)
		return nullptr;

	// Alle Listen leeren
	m_SubMeshNames.Clear();
	m_MaterialGroups.Clear();
	m_Materials.Clear();
	CleanUp();

	// Eine neue mesh erstellen
	m_Mesh = LUX_NEW(StaticMesh);
	if(ReadChunk(&data)) {
		// Datei erfolgreich geladen
		for(u32 i = 0; i < m_Mesh->GetSubMeshCount(); ++i) {
			video::SubMesh* subMesh = m_Mesh->GetSubMesh(i);
			// Leere Puffer löschen
			if(subMesh->GetVertexCount() == 0 || subMesh->GetIndexCount() == 0) {
				// index um 1 verringern, da der nächste Eintrag in der Liste um 1
				// nach vorne gerückt ist
				m_Mesh->RemoveSubMesh(i--);
				subMesh->Drop();
			}
		}

		// Die Bounding-box
		m_Mesh->RecalculateBoundingBox();
	} else {
		m_Mesh = nullptr;
	}

	return m_Mesh;
}

void MeshLoader3DS::ReadPercentageChunk(SChunkData* data, float& fPercentageOut)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading percent chunk.");
#endif

	SChunkData chunk;
	ReadChunkData(chunk);

	switch(chunk.Header.ChunkID) {
	case C3DS_PERCENTAGE_I:
		fPercentageOut = m_pFile->Read<short>()*0.01f;
		chunk.Read += 2;
		break;
	case C3DS_PERCENTAGE_F:
		// Ausgabe
		fPercentageOut = m_pFile->Read<float>();
		chunk.Read += sizeof(float);
		break;
	default:
		log::Debug("Unknown percent chunk found.");
		m_pFile->Seek(chunk.Header.ChunkSize - chunk.Read);
		chunk.Read = chunk.Header.ChunkSize;
		break;
	}

	data->Read += chunk.Read;
}

void MeshLoader3DS::ReadColorChunk(SChunkData* data, video::Color& ColorOut)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Lade Farb-Chunk.");
#endif    

	SChunkData chunk;
	ReadChunkData(chunk);

	switch(chunk.Header.ChunkID) {
	case C3DS_COL_TRU:
	case C3DS_COL_LIN_24:
	{
		// True-color
		u8 c[3];
		m_pFile->ReadBinary(sizeof(char) * 3, c);
		ColorOut.Set(255, c[0], c[1], c[2]);
		chunk.Read += sizeof(c);
	}
	break;

	case C3DS_COL_RGB:
	case C3DS_COL_LIN_F:
	{
		// High-color
		float c[3];
		m_pFile->ReadBinary(sizeof(c), c);
		ColorOut.Set(1.0f, c[0], c[1], c[2]);
		chunk.Read += sizeof(c);
	}
	break;

	default:
		log::Debug("Unknown color chunk found.");
		m_pFile->Seek(chunk.Header.ChunkSize - chunk.Read);
		chunk.Read = chunk.Header.ChunkSize;
		break;
	}

	data->Read += chunk.Read;
}

void MeshLoader3DS::ReadMaterialChunk(SChunkData* parent)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading material chunk.");
#endif

	u16 wMatSection = 0;
	S3DSMaterial CurrentMaterial;
	CurrentMaterial.material.SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer(L"solid"));

	while(parent->Read < parent->Header.ChunkSize) {
		SChunkData data;
		ReadChunkData(data);

		switch(data.Header.ChunkID) {
		case C3DS_MATNAME:
			ReadString(data, CurrentMaterial.name);
			break;
		case C3DS_MATAMBIENT:
		{
			video::Color AmbientColor;
			ReadColorChunk(&data, AmbientColor);
			// Wir benutzen die Helligkeit als Hintergrundfaktor
			CurrentMaterial.material.ambient = AmbientColor.GetLuminance();
		}
		break;
		case C3DS_MATDIFFUSE:
			ReadColorChunk(&data, CurrentMaterial.material.diffuse);
			break;
		case C3DS_MATSPECULAR:
			ReadColorChunk(&data, CurrentMaterial.material.specular);
			break;
		case C3DS_MATSHININESS:
			ReadPercentageChunk(&data, CurrentMaterial.material.shininess);
			CurrentMaterial.material.shininess = (1.0f - CurrentMaterial.material.shininess)*128.0f;
			break;
		case C3DS_TRANSPARENCY:
		{
			float fPercentage;
			ReadPercentageChunk(&data, fPercentage);
			if(fPercentage > 0.0f) {
				CurrentMaterial.material.SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer(L"transparent_alpha"));
				CurrentMaterial.material.Param("AlphaFactor") = fPercentage;
			} else {
				CurrentMaterial.material.SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer(L"solid"));
			}
		}
		break;
		/*
		MARK
		case C3DS_WIRE:
			CurrentMaterial.material->Wireframe = true;
			break;
		case C3DS_TWO_SIDE:
			CurrentMaterial.material->BackfaceCulling = false;
			break;
		case C3DS_SHADING:
			{
				u16 wFlags = m_pFile->Read<u16>();
				switch(wFlags)
				{
				case 0:
					CurrentMaterial.material->Wireframe = true;
					break;
				case 1:
					CurrentMaterial.material->Wireframe = false;
					CurrentMaterial.material->GouraudShading = false;
					break;
				case 2:
					CurrentMaterial.material->Wireframe = false;
					CurrentMaterial.material->GouraudShading = true;
					break;
				default:
					break;
				}

				data.Read = data.Header.ChunkSize;
			}
			break;
		*/
		case C3DS_MATTEXMAP:
		case C3DS_MATSPECMAP:
		case C3DS_MATOPACMAP:
		case C3DS_MATREFLMAP:
		case C3DS_MATBUMPMAP:
		{
			wMatSection = data.Header.ChunkID;
			short sTest;
			const u32 pos = m_pFile->GetCursor();
			sTest = m_pFile->Read<short>();
			m_pFile->Seek(pos, io::ESeekOrigin::Start);
			if((sTest == C3DS_PERCENTAGE_I) ||
				(sTest == C3DS_PERCENTAGE_F))
				switch(wMatSection) {
				case C3DS_MATTEXMAP:
					ReadPercentageChunk(&data, CurrentMaterial.Strenght[0]);
					break;
				case C3DS_MATSPECMAP:
					ReadPercentageChunk(&data, CurrentMaterial.Strenght[1]);
					break;
				case C3DS_MATOPACMAP:
					ReadPercentageChunk(&data, CurrentMaterial.Strenght[2]);
					break;
				case C3DS_MATBUMPMAP:
					ReadPercentageChunk(&data, CurrentMaterial.Strenght[4]);
					break;
				}
		}
		break;
		case C3DS_MATMAPFILE:
		{
			// Texturname einlesen
			string sTexName;
			ReadString(data, sTexName);
			io::path texPath = io::path::FromAscii(sTexName.c_str());
			switch(wMatSection) {
			case C3DS_MATTEXMAP:
				CurrentMaterial.filename[0] = texPath;
				break;
			case C3DS_MATSPECMAP:
				CurrentMaterial.filename[1] = texPath;
				break;
			case C3DS_MATOPACMAP:
				CurrentMaterial.filename[2] = texPath;
				break;
			case C3DS_MATREFLMAP:
				CurrentMaterial.filename[3] = texPath;
				break;
			case C3DS_MATBUMPMAP:
				CurrentMaterial.filename[4] = texPath;
				break;
			}
		}
		break;
		case C3DS_MAT_TEXTILING:
		{
			short sFlags;
			sFlags = m_pFile->Read<short>();
			data.Read += 2;
		}
		break;
		case C3DS_MAT_USCALE:
		case C3DS_MAT_VSCALE:
		case C3DS_MAT_UOFFSET:
		case C3DS_MAT_VOFFSET:
		{
			float fValue = m_pFile->Read<float>();
			LUX_UNUSED(fValue);
			u32 dwTex = 0;
			if(wMatSection != C3DS_MATTEXMAP)
				dwTex = 1;
			u32 dwRow = 0;
			u32 dwCol = 0;
			if(data.Header.ChunkID == C3DS_MAT_VSCALE) {
				dwRow = 1;
				dwCol = 1;
			} else if(data.Header.ChunkID == C3DS_MAT_UOFFSET) {
				dwRow = 2;
				dwCol = 0;
			} else if(data.Header.ChunkID == C3DS_MAT_VOFFSET) {
				dwRow = 2;
				dwCol = 1;
			}
			data.Read += sizeof(float);
		}
		break;
		default:
			// Unbekannter Chunk -> Ignorieren
			m_pFile->Seek(data.Header.ChunkSize - data.Read);
			data.Read = data.Header.ChunkSize;
		}

		parent->Read += data.Read;
	}

	// material fertig
	if(CurrentMaterial.material.GetRenderer() == nullptr)
		CurrentMaterial.material.SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer(L"solid"));

	// material hinzufügen
	m_Materials.Push_Back(CurrentMaterial);
}

void MeshLoader3DS::ReadTrackChunk(SChunkData& data, video::SubMesh* subMesh, const math::vector3f& vPivot)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading track chunk.");
#endif

	LUX_UNUSED(subMesh);
	LUX_UNUSED(vPivot);

	m_pFile->Seek(data.Header.ChunkSize - data.Read);
	data.Read = data.Header.ChunkSize;
}

void MeshLoader3DS::ReadFrameChunk(SChunkData* parent)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading animation chunk.");
#endif

	SChunkData data;
	ReadChunkData(data);

	m_pFile->Seek(data.Header.ChunkSize - data.Read);
	data.Read = data.Header.ChunkSize;

	parent->Read += data.Read;
}

bool MeshLoader3DS::ReadChunk(SChunkData* parent)
{
	while(parent->Read < parent->Header.ChunkSize) {
		SChunkData data;
		ReadChunkData(data);

		switch(data.Header.ChunkID) {
		case C3DS_VERSION:
		{
			u16 wVersion = m_pFile->Read<u16>();
			m_pFile->Seek(data.Header.ChunkSize - data.Read - 2);
			data.Read += data.Header.ChunkSize - data.Read;
			if(wVersion != 0x03)
				return false;
		}
		break;
		case C3DS_EDIT_MATERIAL:
			ReadMaterialChunk(&data);
			break;
		case C3DS_KEYF3DS:
			ReadFrameChunk(&data);
			break;
		case C3DS_EDIT3DS:
			break;
		case C3DS_MESHVERSION:
		case 0x01:
		{
			u32 dwVersion;
			dwVersion = m_pFile->Read<u32>();
			data.Read += sizeof(u32);
		}
		break;
		case C3DS_EDIT_OBJECT:
		{
			string name;
			ReadString(data, name);
			ReadObjectChunk(&data);
			ComposeObject(name);
		}
		break;
		default:
			// Unbekannter Chunk -> ignorieren
			m_pFile->Seek(data.Header.ChunkSize - data.Read);
			data.Read = data.Header.ChunkSize;
		}

		parent->Read += data.Read;
	}

	return true;
}

void MeshLoader3DS::ReadObjectChunk(SChunkData* parent)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading object chunk.");
#endif

	while(parent->Read < parent->Header.ChunkSize) {
		SChunkData data;
		ReadChunkData(data);

		switch(data.Header.ChunkID) {
		case C3DS_OBJTRIMESH:
			ReadObjectChunk(&data);
			break;
		case C3DS_TRIVERT:
			ReadVertices(data);
			break;
		case C3DS_POINTFLAGARRAY:
		{
			u16 wNumVertices;
			u16 wFlags;
			wNumVertices = m_pFile->Read<u16>();
			for(u16 wVertex = 0; wVertex < wNumVertices; ++wVertex)
				wFlags = m_pFile->Read<u16>();

			data.Read += (wNumVertices + 1) * sizeof(u16);
		}
		break;
		case C3DS_TRIFACE:
			ReadIndices(data);
			// Glättungs- und Materialgruppen einlesen
			ReadObjectChunk(&data);
			break;
		case C3DS_TRIFACEMAT:
			ReadMaterialGroup(data);
			break;
		case C3DS_TRIUV:
			ReadTextureCoords(data);
			break;
		case C3DS_TRIMATRIX:
		{
			float fMatrix[4][3];
			m_pFile->ReadBinary(12 * sizeof(float), &fMatrix);
			m_mTransformation.MakeIdent();
			for(int i = 0; i < 4; ++i) {
				for(int j = 0; j < 3; ++j) {
					m_mTransformation(i, j) = fMatrix[i][j];
				}
			}

			data.Read += sizeof(float) * 12;
		}
		break;
		case C3DS_MESHCOLOR:
		{
			char cFlag;
			cFlag = m_pFile->Read<char>();
			++data.Read;
		}
		break;
		case C3DS_TRISMOOTH:
		{
			m_pdwSmoothingGroups = LUX_NEW_ARRAY(u32, m_wFaceCount);
			m_pFile->ReadBinary(m_wFaceCount * sizeof(u32), m_pdwSmoothingGroups);
			data.Read += m_wFaceCount * sizeof(u32);
		}
		break;
		default:
			// Unbekannter Chunk -> Ignorieren
			m_pFile->Seek(data.Header.ChunkSize - data.Read);
			data.Read = data.Header.ChunkSize;
		}

		parent->Read += data.Read;
	}
}

void MeshLoader3DS::ComposeObject(const string& name)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Creating 3ds object.");
#endif

	if(m_Mesh->GetSubMeshCount() != m_Materials.Size())
		LoadMaterials();

	if(m_MaterialGroups.IsEmpty()) {
		// Keine Materialgruppen vorhanden also alle hinzufügen
		SMaterialGroup Group;
		Group.FaceCount = m_wFaceCount;
		Group.Faces = LUX_NEW_ARRAY(u16, Group.FaceCount);
		for(u16 dw = 0; dw < Group.FaceCount; ++dw)
			Group.Faces[dw] = dw;
		m_MaterialGroups.Push_Back(Group);

		// Wenn kein material vorhanden ist, ein simples ohne Textur hinzufügen
		if(m_Materials.IsEmpty()) {
			S3DSMaterial Mat;
			Mat.material.SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer(L"solid"));
			m_Materials.Push_Back(Mat);
			video::SubMeshImpl* pSub = LUX_NEW(video::SubMeshImpl);
			m_Mesh->AddSubMesh(pSub);
			pSub->SetMaterial(Mat.material);
			pSub->Drop();
			m_SubMeshNames.Push_Back("");
		}
	}

	video::Material Mat;
	for(u32 dw = 0; dw < m_MaterialGroups.Size(); ++dw) {
		video::SubMeshImpl* pSub = nullptr;
		u32 dwSubPos;

		// Die passende Sub-mesh finden
		for(dwSubPos = 0; dwSubPos < m_Materials.Size(); ++dwSubPos) {
			if(m_MaterialGroups[dw].MaterialName == m_Materials[dwSubPos].name) {
				pSub = (StrongRef<video::SubMeshImpl>)m_Mesh->GetSubMesh(dwSubPos);
				Mat = m_Materials[dwSubPos].material;
				m_SubMeshNames[dwSubPos] = name;
				break;
			}
		}

		if(pSub) {
			video::Vertex3D Vertex;
			Vertex.color = (u32)(Mat.diffuse);
			Vertex.normal.Set(0.0f, 0.0f, 0.0f);
			math::aabbox3df BoundingBox;
			if(m_MaterialGroups[dw].FaceCount > 0)
				BoundingBox.Set(m_pfVertices[0],
				m_pfVertices[2],
				m_pfVertices[1]);
			u32 vertexCount = pSub->GetVertices() ? pSub->GetVertices()->GetCursor() : 0;

			if(vertexCount == 0) {
				// Vertex und Indexbuffer erstellen
				video::VertexBuffer*pVB = m_SceneManager->GetDriver()->GetBufferManager()->CreateVertexBuffer();
				video::IndexBuffer*pIB = m_SceneManager->GetDriver()->GetBufferManager()->CreateIndexBuffer();

				pVB->SetFormat(video::VertexFormat::STANDARD);
				pVB->SetHWMapping(video::EHardwareBufferMapping::Static);
				pVB->SetSize(m_MaterialGroups[dw].FaceCount * 3);
				pIB->SetType(video::EIndexFormat::Bit16);
				pIB->SetHWMapping(video::EHardwareBufferMapping::Static);
				pIB->SetSize(m_MaterialGroups[dw].FaceCount * 3);
				vertexCount = 1;
				pSub->SetBuffer(pVB, pIB);
				pVB->Drop();
				pIB->Drop();
			} else {
				//------------------------------------------------------------------------
				// Vertexbuffer anpassen
				pSub->GetVertices()->SetSize(vertexCount + m_MaterialGroups[dw].FaceCount * 3);

				// Indexbuffer anpassen
				pSub->GetIndices()->SetSize(pSub->GetIndexCount() + m_MaterialGroups[dw].FaceCount * 3);
				//------------------------------------------------------------------------
			}

			for(int iFace = 0; iFace < m_MaterialGroups[dw].FaceCount; ++iFace) {
				vertexCount = pSub->GetVertices()->GetCursor();

				for(int iVertex = 0; iVertex < 3; ++iVertex) {
					int iIndex = m_pwIndices[m_MaterialGroups[dw].Faces[iFace] * 4 + iVertex];

					if(m_wVertexCount > iIndex) {
						Vertex.position.Set(m_pfVertices[iIndex * 3],
							m_pfVertices[iIndex * 3 + 2],
							m_pfVertices[iIndex * 3 + 1]);
					}

					if(m_wTCoordsCount > iIndex) {
						Vertex.texture.Set(m_pfTCoords[iIndex * 2],
							1.0f - m_pfTCoords[iIndex * 2 + 1]);
					}

					BoundingBox.AddPoint(Vertex.position);
					pSub->GetVertices()->AddVertex(&Vertex);        // Cursor wird automatisch verschoben
				}


				// Normalvektor berechnen
				video::Vertex3D* v = (video::Vertex3D*)pSub->GetVertices()->Pointer(vertexCount, 3);
				math::plane3df plane(v[0].position,
					v[2].position,
					v[1].position);

				v[0].normal = plane.Normal;
				v[1].normal = plane.Normal;
				v[2].normal = plane.Normal;

				// Indizes hinzufügen
				u32 dwTemp;
				pSub->GetIndices()->AddIndex(&vertexCount);
				dwTemp = vertexCount + 2;
				pSub->GetIndices()->AddIndex(&dwTemp);
				dwTemp = vertexCount + 1;
				pSub->GetIndices()->AddIndex(&dwTemp);
			}

			// Bounding-box setzen
			pSub->SetBoundingBox(BoundingBox);

			// Vertex- und Indexbuffer dirty setzten
			pSub->GetVertices()->Update();
			pSub->GetIndices()->Update();
		}
	}

	// Alles wieder Aufräumen
	CleanUp();
}

void MeshLoader3DS::LoadMaterials()
{
	// Für jedes material eine Sub-mesh erstellen
	string sModelFilename = m_pFile->GetName();

	if(m_Materials.IsEmpty())
		log::Warning("No materials found in the 3ds file.");

	m_SubMeshNames.Reserve(m_Materials.Size());
	for(u32 dwMat = 0; dwMat < m_Materials.Size(); ++dwMat) {
		m_SubMeshNames.Push_Back("");
		video::SubMeshImpl* pSub = LUX_NEW(video::SubMeshImpl);
		m_Mesh->AddSubMesh(pSub);

		pSub->SetMaterial(m_Materials[dwMat].material);

		// Erste Texturschicht
		if(m_Materials[dwMat].filename[0].Length()) {
			StrongRef<video::Texture> texture = nullptr;
			if(m_Filesystem->ExistFile(m_Materials[dwMat].filename[0]))
				texture = m_SceneManager->GetResourceSystem()->GetResource(
				core::ResourceType::Texture, m_Materials[dwMat].filename[0]);

			if(!texture) {
				io::path filename = io::GetFileDir(sModelFilename);
				filename += '/';
				filename += io::GetFilenameOnly(m_Materials[dwMat].filename[0]);
				if(m_Filesystem->ExistFile(filename))
					texture = m_SceneManager->GetResourceSystem()->GetResource(
					core::ResourceType::Texture, filename);
			}

			if(!texture) {
				log::Warning << "Can't load texture of material:" << m_Materials[dwMat].filename[0].c_str() << ".";
			} else {
				if(pSub->GetMaterial().GetTextureCount() > 0)
					pSub->GetMaterial().Layer(0) = texture;
			}
		}

		// Zweite Texturschicht
		if(m_Materials[dwMat].filename[1].Length()) {
			StrongRef<video::Texture> texture = nullptr;
			if(m_Filesystem->ExistFile(m_Materials[dwMat].filename[1])) {
				texture = m_SceneManager->GetResourceSystem()->GetResource(
					core::ResourceType::Texture, m_Materials[dwMat].filename[1]);
			}

			if(!texture) {
				io::path filename = io::GetFileDir(sModelFilename);
				filename += '/';
				filename += io::GetFilenameOnly(m_Materials[dwMat].filename[1]);
				if(m_Filesystem->ExistFile(filename)) {
					texture = m_SceneManager->GetResourceSystem()->GetResource(
						core::ResourceType::Texture, filename);
				}
			}

			if(!texture) {
				log::Warning << "Can't load texture of material:" << m_Materials[dwMat].filename[1].c_str() << ".";
			} else {
				if(pSub->GetMaterial().GetTextureCount() > 1)
					pSub->GetMaterial().Layer(1) = texture;
			}
		}

		// Dritte Texturschicht
		if(m_Materials[dwMat].filename[2].Length()) {
			StrongRef<video::Texture> texture = nullptr;
			if(m_Filesystem->ExistFile(m_Materials[dwMat].filename[2])) {
				texture = m_SceneManager->GetResourceSystem()->GetResource(
					core::ResourceType::Texture, m_Materials[dwMat].filename[2]);
			}

			if(!texture) {
				io::path filename = io::GetFileDir(sModelFilename);
				filename += '/';
				filename += io::GetFilenameOnly(m_Materials[dwMat].filename[2]);
				if(m_Filesystem->ExistFile(filename)) {
					texture = m_SceneManager->GetResourceSystem()->GetResource(
						core::ResourceType::Texture, filename);
				}
			}

			if(!texture) {
				log::Warning << "Can't load texture of material:" << m_Materials[dwMat].filename[2].c_str() << ".";
			} else {
				if(pSub->GetMaterial().GetTextureCount() > 2)
					pSub->GetMaterial().Layer(2) = texture;
			}
		}

		// Vierte Texturschicht
		if(m_Materials[dwMat].filename[3].Length()) {
			StrongRef<video::Texture> texture = nullptr;
			if(m_Filesystem->ExistFile(m_Materials[dwMat].filename[3])) {
				texture = m_SceneManager->GetResourceSystem()->GetResource(
					core::ResourceType::Texture, m_Materials[dwMat].filename[3]);
			}

			if(!texture) {
				io::path filename = io::GetFileDir(sModelFilename);
				filename += '/';
				filename += io::GetFilenameOnly(m_Materials[dwMat].filename[3]);
				if(m_Filesystem->ExistFile(filename)) {
					texture = m_SceneManager->GetResourceSystem()->GetResource(
						core::ResourceType::Texture, filename);
				}
			}

			if(!texture) {
				log::Warning << "Can't load texture of material:" << m_Materials[dwMat].filename[3].c_str() << ".";
			} else {
				if(pSub->GetMaterial().GetTextureCount() > 3)
					pSub->GetMaterial().Layer(3) = texture;
			}
		}

		pSub->Drop();
	}
}

void MeshLoader3DS::ReadTextureCoords(SChunkData& data)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading texture coordinate chunk.");
#endif

	m_wTCoordsCount = m_pFile->Read<u16>();
	data.Read += sizeof(u16);

	const u32 iTCoordsBufferByteSize = m_wTCoordsCount * sizeof(float) * 2;
	if(data.Header.ChunkSize - data.Read != iTCoordsBufferByteSize) {
		log::Error("Invalid texture coordinate chunk.");
		return;
	}

	m_pfTCoords = LUX_NEW_ARRAY(float, m_wTCoordsCount * 2);
	m_pFile->ReadBinary(iTCoordsBufferByteSize, m_pfTCoords);
	data.Read += iTCoordsBufferByteSize;
}

void MeshLoader3DS::ReadMaterialGroup(SChunkData& data)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading material group chunk.");
#endif

	SMaterialGroup Group;

	ReadString(data, Group.MaterialName);
	Group.FaceCount = m_pFile->Read<u16>();
	data.Read += sizeof(u16);

	// Dreiecke einlesen
	Group.Faces = LUX_NEW_ARRAY(u16, Group.FaceCount);
	m_pFile->ReadBinary(sizeof(u16)*Group.FaceCount, Group.Faces);
	data.Read += sizeof(u16) * Group.FaceCount;

	m_MaterialGroups.Push_Back(Group);
}

void MeshLoader3DS::ReadIndices(SChunkData& data)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading index chunk.");
#endif

	m_wFaceCount = m_pFile->Read<u16>();
	data.Read += sizeof(u16);

	const int iIndexBufferByteSize = m_wFaceCount * sizeof(u16) * 4;

	// Alle 3 Indizes kommt ein zusätliches Flag
	m_pwIndices = LUX_NEW_ARRAY(u16, m_wFaceCount * 4);
	m_pFile->ReadBinary(iIndexBufferByteSize, m_pwIndices);
	data.Read += iIndexBufferByteSize;
}

void MeshLoader3DS::ReadVertices(SChunkData& data)
{
#ifdef _LUX_DEBUG_3DS_LOADER
	log::Debug("Loading vertex chunk.");
#endif

	m_wVertexCount = m_pFile->Read<u16>();
	data.Read += sizeof(u16);

	const u32 iVertexBufferByteSize = m_wVertexCount * sizeof(float) * 3;
	if(data.Header.ChunkSize - data.Read != iVertexBufferByteSize) {
		return;
	}

	m_pfVertices = LUX_NEW_ARRAY(float, m_wVertexCount * 3);
	m_pFile->ReadBinary(iVertexBufferByteSize, m_pfVertices);
	data.Read += iVertexBufferByteSize;
}

void MeshLoader3DS::ReadChunkData(SChunkData& data)
{
	m_pFile->ReadBinary(sizeof(S3DSChunkHeader), &data.Header);
	data.Read += sizeof(S3DSChunkHeader);
}

void MeshLoader3DS::ReadString(SChunkData& data, string& out)
{
	out = m_pFile->ReadString();
	data.Read += out.Length() + 1;
}

}    // namespace scene
}    // namespace lux