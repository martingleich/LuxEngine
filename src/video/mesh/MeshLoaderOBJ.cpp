#include "MeshLoaderOBJ.h"

#include "core/ResourceSystem.h"

#include "core/Logger.h"

#include "video/MaterialLibrary.h"
#include "video/VideoDriver.h"
#include "video/Texture.h"
#include "video/VertexTypes.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/HardwareBufferManager.h"

#include "core/lxMemory.h"
#include "core/lxHashMap.h"

#include <sstream>
#include <vector>
#include <tinyobjloader/tiny_obj_loader.h>

namespace lux
{
namespace video
{

///////////////////////////////////////////////////////////////////////////////

core::Name MeshLoaderOBJ::GetResourceType(io::File* file, core::Name requestedType)
{
	if(!requestedType.IsEmpty() && requestedType != core::ResourceType::Mesh)
		return core::Name::INVALID;

	core::String ext = file->GetPath().GetFileExtension();
	if(ext.Equal("obj", core::EStringCompare::CaseInsensitive))
		return core::ResourceType::Mesh;
	else
		return core::Name::INVALID;
}

const core::String& MeshLoaderOBJ::GetName() const
{
	static const core::String name = "Lux OBJ-Loader";
	return name;
}

class LuxObjMaterialReader : public tinyobj::MaterialReader
{
public:
	LuxObjMaterialReader(const io::Path& baseDir)
		: m_BaseDir(baseDir)
	{
	}
	virtual ~LuxObjMaterialReader() {}
	virtual bool operator()(const std::string &matId,
		std::vector<tinyobj::material_t> *materials,
		std::map<std::string, int> *matMap, std::string *err)
	{
		StrongRef<io::File> mtlFile;
		io::Path matPath(matId.data());

		auto fileSys = io::FileSystem::Instance();
		if(fileSys->ExistFile(matPath))
			mtlFile = fileSys->OpenFile(matPath);
		if(!mtlFile) {
			auto newFile = matPath.GetResolved(m_BaseDir);
			mtlFile = fileSys->OpenFile(newFile);
		}

		if(!mtlFile) {
			std::stringstream ss;
			ss << "WARN: Material file [ " << matId << " ] not found." << std::endl;
			if(err) {
				(*err) += ss.str();
			}
			return false;
		}

		auto filesize = mtlFile->GetSize();
		if(!filesize)
			throw core::FileFormatException("Can't load streaming file", "obj");

		core::RawMemory memory(core::SafeCast<size_t>(filesize));
		mtlFile->ReadBinary(filesize, memory);
		std::stringstream fileStream;
		fileStream.write(memory, filesize);

		std::string warning;
		tinyobj::LoadMtl(matMap, materials, &fileStream, &warning);

		if(!warning.empty()) {
			if(err) {
				(*err) += warning;
			}
		}

		return true;
	}

private:
	io::Path m_BaseDir;
};

struct ObjLoader
{
public:
	ObjLoader(io::File* file, core::Referable* resource)
	{
		auto mesh = dynamic_cast<video::Mesh*>(resource);
		if(!mesh)
			throw core::InvalidOperationException("Wrong resource type passed");

		basePath = file->GetPath().GetFileDir();
		auto filesize = file->GetSize();
		core::RawMemory memory(core::SafeCast<size_t>(filesize));
		file->ReadBinary(filesize, memory);
		std::stringstream fileStream;
		fileStream.write(memory, filesize);

		LuxObjMaterialReader mtlReader(basePath);
		std::string error;
		bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, &fileStream, &mtlReader, true);
		if(!result)
			throw core::FileFormatException("Invalid file format", "obj");

		if(shapes.empty())
			throw core::FileFormatException("File contains no geometry", "obj");

		ConvertMaterials();
		invalidMaterial = video::MaterialLibrary::Instance()->CloneMaterial(video::MaterialLibrary::DebugOverlayName);
		invalidMaterial->SetDiffuse(video::Color::Pink);

		size_t indexCount = 0;
		for(const auto& s : shapes)
			indexCount += s.mesh.indices.size();
		if(indexCount > INT_MAX)
			throw core::FileFormatException("To many indices", "obj");

		bool use32BitIndices = attrib.vertices.size() > std::numeric_limits<u16>::max();

		auto geo = video::VideoDriver::Instance()->CreateGeometry();
		vertexBuffer = geo->GetVertices();
		indexBuffer = geo->GetIndices();
		if(use32BitIndices)
			indexBuffer->SetFormat(video::EIndexFormat::Bit32);
		else
			indexBuffer->SetFormat(video::EIndexFormat::Bit16);
		indexBuffer->Reserve((int)indexCount);

		u32 realFaceId = 0;
		int prevMaterialId = -1;
		for(const auto& s : shapes) {
			const auto& objmesh = s.mesh;
			u32 faceId = 0;
			u32 indexCounter = 0;
			for(auto i : objmesh.indices) {
				int matId = objmesh.material_ids[faceId];

				auto mat = GetMaterial(matId);
				u32 index32 = GetIndex(i, mat);
				u16 index16 = (u16)index32;
				if(use32BitIndices)
					indexBuffer->AddIndex(&index32);
				else
					indexBuffer->AddIndex(&index16);
				indexCounter = (indexCounter + 1) % 3;
				if(indexCounter == 0) {
					if(materialRanges.IsEmpty() || matId != prevMaterialId) {
						MaterialRange newRange;
						newRange.first = realFaceId;
						newRange.last = realFaceId;
						newRange.matId = matId;
						materialRanges.PushBack(newRange);
						prevMaterialId = matId;
					} else {
						materialRanges.Back().last++;
					}
					++faceId;
					++realFaceId;
				}
			}
		}

		indexBuffer->Update();
		vertexBuffer->Update();

		geo->RecalculateBoundingBox();
		mesh->SetGeometry(geo);
		mesh->SetMaterial(invalidMaterial);
		mesh->RecalculateBoundingBox();

		// Assign material groups
		for(auto& matRange : materialRanges) {
			if(matRange.matId != -1)
				mesh->SetMaterialRange(GetMaterial(matRange.matId), matRange.first, matRange.last);
		}
	}

	u32 GetIndex(tinyobj::index_t idx, const video::Material* material)
	{
		auto it = indexMap.find(idx);
		if(it == indexMap.end()) {
			u32 newIdx = vertexBuffer->GetCursor();
			video::Vertex3D vert;
			if(idx.vertex_index != -1)
				vert.position.Set(attrib.vertices[3 * idx.vertex_index], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
			if(idx.normal_index != -1)
				vert.normal.Set(attrib.normals[3 * idx.normal_index], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2]);
			if(idx.texcoord_index != -1)
				vert.texture.Set(attrib.texcoords[2 * idx.texcoord_index], attrib.texcoords[2 * idx.texcoord_index + 1]);
			if(material)
				vert.color = material->GetDiffuse().ToHex();
			else
				vert.color = video::Color::Pink;
			vertexBuffer->AddVertex(&vert);

			indexMap[idx] = newIdx;
			return newIdx;
		} else {
			return it->second;
		}
	}

	struct IndexTLess
	{
		bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const
		{
			if(a.vertex_index != b.vertex_index)
				return a.vertex_index < b.vertex_index;
			if(a.normal_index != b.normal_index)
				return a.normal_index < b.normal_index;
			if(a.texcoord_index != b.texcoord_index)
				return a.texcoord_index < b.texcoord_index;
			return false;
		}
	};

	void ConvertMaterials()
	{
		for(auto& mat : materials)
			luxMaterials.PushBack(ConvertMaterial(mat));
	}

	StrongRef<Texture> LoadTexture(const io::Path& path)
	{
		StrongRef<video::Texture> texture;
		if(io::FileSystem::Instance()->ExistFile(path)) {
			texture = core::ResourceSystem::Instance()->GetResource(
				core::ResourceType::Texture, path.AsView()).AsStrong<video::Texture>();
		} else {
			io::Path texFile = path.GetResolved(basePath);
			texture = core::ResourceSystem::Instance()->GetResource(
				core::ResourceType::Texture,
				io::FileSystem::Instance()->OpenFile(texFile)).AsStrong<video::Texture>();
		}
		return texture;
	}

	StrongRef<video::Material> ConvertMaterial(const tinyobj::material_t& mat)
	{
		StrongRef<Material> lxm;
		if(mat.dissolve != 1)
			lxm = video::MaterialLibrary::Instance()->CloneMaterial(video::MaterialLibrary::TransparentName);
		else
			lxm = video::MaterialLibrary::Instance()->CloneMaterial(video::MaterialLibrary::SolidName);

		lxm->SetDiffuse(video::ColorF(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], mat.dissolve));
		lxm->SetEmissive(video::ColorF(mat.emission[0], mat.emission[1], mat.emission[2]).GetLuminance());
		video::ColorF spec = video::ColorF(mat.specular[0], mat.specular[1], mat.specular[2]);
		if(math::IsZero(spec.r) && math::IsZero(spec.g) && math::IsZero(spec.b)) {
			lxm->SetSpecularIntensity(1);
			lxm->SetSpecularHardness(0);
		} else {
			lxm->SetSpecularIntensity(spec.GetLuminance());
			lxm->SetSpecularHardness(mat.shininess);
		}

		if(!mat.diffuse_texname.empty()) {
			io::Path texname = mat.diffuse_texname.data();
			lxm->SetTexture(0, LoadTexture(texname));
		}

		return lxm;
	}

	StrongRef<video::Material> GetMaterial(int id)
	{
		if(id >= 0)
			return luxMaterials[id];
		return invalidMaterial;
	}

	std::map<tinyobj::index_t, u32, IndexTLess> indexMap;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	core::Array<StrongRef<Material>> luxMaterials;
	StrongRef<Material> invalidMaterial;

	struct MaterialRange
	{
		int matId = -1;
		u32 first;
		u32 last;
	};

	core::Array<MaterialRange> materialRanges;

	IndexBuffer* indexBuffer;
	VertexBuffer* vertexBuffer;

	io::Path basePath;
};

void MeshLoaderOBJ::LoadResource(io::File* file, core::Referable* dst)
{
	ObjLoader(file, dst);
}

}
}
