#include "MeshLoaderOBJ.h"
#include "video/VideoDriver.h"
#include "video/Texture.h"
#include "core/Logger.h"
#include "io/FileSystem.h"
#include "io/file.h"
#include "scene/SceneManager.h"
#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

namespace lux
{
namespace scene
{

#ifdef _DEBUG
//#define LUX_DEBUG_OBJ_LOADER
#endif

core::Name MeshLoaderOBJ::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Mesh)
		return core::Name::INVALID;

	string ext = io::GetFileExtension(file->GetDescription().GetName());
	if(ext.EqualCaseInsensitive("obj"))
		return core::ResourceType::Mesh;
	else
		return core::Name::INVALID;
}

const string& MeshLoaderOBJ::GetName() const
{
	static const string name = "Lux OBJ-Loader";
	return name;
}
//TODO Enable normal calculation from smoothing groups

static const u32 WORD_BUFFER_LENGTH = 512;

MeshLoaderOBJ::MeshLoaderOBJ(SceneManager* pSmgr) :
	m_SceneManager(pSmgr),
	m_Filesystem(pSmgr->GetFileSystem())
{
}

MeshLoaderOBJ::~MeshLoaderOBJ()
{
}

void MeshLoaderOBJ::CleanUp()
{
	m_Materials.Clear();
}


bool MeshLoaderOBJ::LoadResource(io::File* file, core::Resource* dst)
{
	const long filesize = file->GetSize();
	if(!filesize)
		return nullptr;

	core::array<math::vector3f> vertexBuffer;
	core::array<math::vector3f> NormalBuffer;
	core::array<math::vector2f> TCoordBuffer;

	SObjMtl* pCurrMtl = LUX_NEW(SObjMtl);
	m_Materials.Push_Back(pCurrMtl);
	u32 SmoothingGroup = 0;

	const io::FileDescription fileDesc = file->GetDescription();

	char* pcBuffer = LUX_NEW_ARRAY(char, filesize);
	memset(pcBuffer, 0, filesize);
	file->ReadBinary(filesize, pcBuffer);
	const char* const pcBufferEnd = pcBuffer + filesize;

	// Inhalt der Datei verarbeiten
	const char* pcBuffPtr = pcBuffer;
	string grpName, mtlName;
	bool mtlChanged = false;

	while(pcBuffPtr != pcBufferEnd) {
		switch(pcBuffPtr[0]) {
		case 'm':    // mtllib
		{
			char name[WORD_BUFFER_LENGTH];
			pcBuffPtr = CopyNextWord(pcBuffPtr, pcBufferEnd, name, WORD_BUFFER_LENGTH);
#ifdef LUX_DEBUG_OBJ_LOADER
			log::Debug << "Loading material file.;
#endif
				ReadMaterial(name, fileDesc);
		}
		break;

		case 'v':    // Vertexdaten
			switch(pcBuffPtr[1]) {
			case ' ':    // Position
			{
				math::vector3f vPos;
				pcBuffPtr = Read3DVec(pcBuffPtr, pcBufferEnd, vPos);
				vertexBuffer.Push_Back(vPos);
			}
			break;

			case 'n':    // Normal
			{
				math::vector3f vNor;
				pcBuffPtr = Read3DVec(pcBuffPtr, pcBufferEnd, vNor);
				NormalBuffer.Push_Back(vNor);
			}
			break;

			case 't':    // Texturkoordinate
			{
				math::vector2f vTex;
				pcBuffPtr = Read2DVec(pcBuffPtr, pcBufferEnd, vTex);
				TCoordBuffer.Push_Back(vTex);
			}
			break;
			}
			break;

		case 'g':    // Gruppenname
		{
			char Grp[WORD_BUFFER_LENGTH];
			pcBuffPtr = CopyNextWord(pcBuffPtr, pcBufferEnd, Grp, WORD_BUFFER_LENGTH);
#ifdef LUX_DEBUG_OBJ_LOADER
			log::Debug << "Setting group " << Grp << ".";
#endif
			if(Grp[0] != '\0')
				grpName = Grp;
			else
				grpName = "default";

			mtlChanged = true;
		}
		break;

		case 's':    // Glättung
		{
			char Smooth[WORD_BUFFER_LENGTH];
			pcBuffPtr = CopyNextWord(pcBuffPtr, pcBufferEnd, Smooth, WORD_BUFFER_LENGTH);
#ifdef LUX_DEBUG_OBJ_LOADER
			log::Debug << "Setting smoothing group " << Smooth << ".";
#endif
			if(strcmp(Smooth, "off"))
				SmoothingGroup = 0;
			else
				SmoothingGroup = atol(Smooth);
		}
		break;

		case 'u':    // usemtl
		{
			char matName[WORD_BUFFER_LENGTH];
			pcBuffPtr = CopyNextWord(pcBuffPtr, pcBufferEnd, matName, WORD_BUFFER_LENGTH);
#ifdef LUX_DEBUG_OBJ_LOADER
			log::Debug << "Setting material " << matName << ".";
#endif
			mtlName = matName;
			mtlChanged = true;
		}
		break;

		case 'f':    // Face
		{
			char VertexWord[WORD_BUFFER_LENGTH];
			video::Vertex3D v;

			// Vertexfarbe = aktuelle Streufarbe
			if(mtlChanged) {
				SObjMtl* useMtl = FindMaterial(mtlName, grpName);
				if(useMtl)
					pCurrMtl = useMtl;
				mtlChanged = false;
			}

			if(pCurrMtl)
				v.color = pCurrMtl->Meshbuffer->GetMaterial().diffuse;

			//v.color = video::Color::White;

			// Vertexdaten des Faces laden
			const string WordBuffer = CopyLine(pcBuffPtr, pcBufferEnd);
			const char* pcLinePtr = WordBuffer.Data();
			const char* const pcEndPtr = pcLinePtr + WordBuffer.Size();

			core::array<s32> FaceCorners;
			FaceCorners.Reserve(32);

			// Alle Vertizes einlesen
			pcLinePtr = MoveToNextWord(pcLinePtr, pcEndPtr);
			while(pcLinePtr[0] != '\0') {
				s32 Idx[3];
				Idx[1] = Idx[2] = -1;

				// Nächsten Vertexblock einlesen
				u32 WLength = CopyWord(pcLinePtr, pcEndPtr, VertexWord, WORD_BUFFER_LENGTH);
				RetrieveVertexIndices(VertexWord, VertexWord + WLength + 1, Idx,
					vertexBuffer.Size(),
					TCoordBuffer.Size(),
					NormalBuffer.Size());
				v.position = vertexBuffer[Idx[0]];
				if(Idx[1] != -1)
					v.texture = TCoordBuffer[Idx[1]];
				else
					v.texture.Set(0.0f, 0.0f);

				if(Idx[2] != -1)
					v.normal = NormalBuffer[Idx[2]];
				else {
					v.normal.Set(0.0f, 0.0f, 0.0f);
					pCurrMtl->RecalculateNormals = true;
				}

				s32 VertexLocation;
				std::map<video::Vertex3D, s32>::iterator it = pCurrMtl->VertMap.find(v);

				video::VertexBuffer* pVB = pCurrMtl->Meshbuffer->GetVertices();
				video::IndexBuffer* pIB = pCurrMtl->Meshbuffer->GetIndices();
				if(it != pCurrMtl->VertMap.end()) {
					VertexLocation = it->second;
				} else {
					if(!pVB) {
						// Vertexbuffer erstellen
						pVB = m_SceneManager->GetDriver()->GetBufferManager()->CreateVertexBuffer();
						pIB = m_SceneManager->GetDriver()->GetBufferManager()->CreateIndexBuffer();

						pVB->SetFormat(video::VertexFormat::STANDARD);
						pVB->SetHWMapping(video::EHardwareBufferMapping::Static);
						pIB->SetType(video::EIndexFormat::Bit16);
						pIB->SetHWMapping(video::EHardwareBufferMapping::Static);
						pCurrMtl->Meshbuffer->SetBuffer(pVB, pIB);
					}

					VertexLocation = pVB->AddVertex(&v);
					pCurrMtl->VertMap[v] = VertexLocation;
				}

				FaceCorners.Push_Back(VertexLocation);

				// Nächster Vertex
				pcLinePtr = MoveToNextWord(pcLinePtr, pcEndPtr);
			}

			// Das Face in Dreiecke zerlegen
			for(u32 i = 1; i < FaceCorners.Size() - 1; ++i) {
				// Dreieck hinzufügen
				pCurrMtl->Meshbuffer->GetIndices()->AddIndex(&FaceCorners[i + 1]);
				pCurrMtl->Meshbuffer->GetIndices()->AddIndex(&FaceCorners[i]);
				pCurrMtl->Meshbuffer->GetIndices()->AddIndex(&FaceCorners[0]);
			}

			FaceCorners.Resize(0);
			FaceCorners.Reserve(32);
		}
		break;

		case '#':    // Kommentar
		default:
			break;
		}

		// Den Rest der Zeile überlesen
		pcBuffPtr = MoveToNextLine(pcBuffPtr, pcBufferEnd);
	}

	// Das Modell erstellen
	Mesh* mesh = dynamic_cast<Mesh*>(dst);
	if(mesh) {
		for(u32 m = 0; m < m_Materials.Size(); ++m) {
			if(m_Materials[m]->Meshbuffer->GetIndexCount() > 0) {
				m_Materials[m]->Meshbuffer->RecalculateBoundingBox();
				m_Materials[m]->Meshbuffer->GetIndices()->Update();
				m_Materials[m]->Meshbuffer->GetVertices()->Update();
				mesh->AddSubMesh(m_Materials[m]->Meshbuffer);
			}
		}

		mesh->RecalculateBoundingBox();
	}

	// Alles aufräumen
	LUX_FREE_ARRAY(pcBuffer);
	CleanUp();

	return true;
}

const char* MeshLoaderOBJ::ReadTextures(const char* pFrom, const char* const pTo,
	SObjMtl* pCurrMaterial, const io::FileDescription& fileDesc)
{
	u8 type = 0;    // map_Kd - Streufarbe
				// map_Ks - Glanzfarbe
				// map_Ka - Hintergrund
				// map_Ns - Selbsterhellung
	if((!strncmp(pFrom, "map_bump", 8)) || (!strncmp(pFrom, "bump", 4)))
		type = 1; // normal map
	else if((!strncmp(pFrom, "map_d", 5)) || (!strncmp(pFrom, "map_opacity", 11)))
		type = 2; // opacity map
	else if(!strncmp(pFrom, "map_refl", 8))
		type = 3; // reflection map

	// Texturname laden
	char textureNameBuf[WORD_BUFFER_LENGTH];
	pFrom = CopyNextWord(pFrom, pTo, textureNameBuf, WORD_BUFFER_LENGTH);

	// TODO: Textur-Optionen abhandlen
	/*
	float bumpiness = 6.0f;
	bool clamp = false;
	while(textureNameBuf[0] == '-')
	{

		if (!strncmp(pFrom, "-bm", 3))
		{
			pFrom = CopyNextWord(pFrom, pTo, textureNameBuf, WORD_BUFFER_LENGTH);
			pCurrMaterial->Meshbuffer->GetMaterial().MaterialTypeParam=core::fast_atof(textureNameBuf);
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			continue;
		}
		else
		if (!strncmp(bufPtr,"-blendu",7))
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		else
		if (!strncmp(bufPtr,"-blendv",7))
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		else
		if (!strncmp(bufPtr,"-cc",3))
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		else
		if (!strncmp(bufPtr,"-clamp",6))
			bufPtr = readBool(bufPtr, clamp, bufEnd);
		else
		if (!strncmp(bufPtr,"-texres",7))
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		else
		if (!strncmp(bufPtr,"-type",5))
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		else
		if (!strncmp(bufPtr,"-mm",3))
		{
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
		}
		else
		if (!strncmp(bufPtr,"-o",2)) // texture coord translation
		{
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			// next parameters are optional, so skip rest of loop if no number is found
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
		}
		else
		if (!strncmp(bufPtr,"-s",2)) // texture coord scale
		{
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			// next parameters are optional, so skip rest of loop if no number is found
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
		}
		else
		if (!strncmp(bufPtr,"-t",2))
		{
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			// next parameters are optional, so skip rest of loop if no number is found
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
			bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
			if (!core::isdigit(textureNameBuf[0]))
				continue;
		}
		// get next word
		bufPtr = goAndCopyNextWord(textureNameBuf, bufPtr, WORD_BUFFER_LENGTH, bufEnd);
	}
	*/

	io::path texname = textureNameBuf;
	texname.Replace("\\", "/");

	StrongRef<video::Texture> texture;
	if(!texname.IsEmpty()) {
		if(m_Filesystem->ExistFile(texname)) {
			texture = m_SceneManager->GetResourceSystem()->GetResource(
				core::ResourceType::Texture, texname);
		} else {
			// Über einen relativen Pfad laden
			io::FileDescription texFile = io::ConcatFileDesc(fileDesc, texname);

			texture = m_SceneManager->GetResourceSystem()->GetResource(
				core::ResourceType::Texture, 
				m_Filesystem->OpenFile(texFile));
		}
	}

	if(texture) {
		// Standardtexture
		if(type == 0) {
			pCurrMaterial->Meshbuffer->GetMaterial().SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer("solid"));
			pCurrMaterial->Meshbuffer->GetMaterial().Layer(0) = (video::BaseTexture*)texture;
		}

		// TODO: Andere Texturverwendungen, noch nicht implementiert
		/*else if (type==1)
		{
			if (newTexture)
				sceneManager->getVideoDriver()->makeNormalMapTexture(texture, bumpiness);
			currMaterial->Meshbuffer->material.setTexture(1, texture);
			currMaterial->Meshbuffer->material.MaterialType=video::EMT_PARALLAX_MAP_SOLID;
			currMaterial->Meshbuffer->material.MaterialTypeParam=0.035f;
		}
		else if (type==2)
		{
			currMaterial->Meshbuffer->material.setTexture(0, texture);
			currMaterial->Meshbuffer->material.MaterialType=video::EMT_TRANSPARENT_ADD_COLOR;
		}*/

		// Streufarbe auf Weiß setzen
		pCurrMaterial->Meshbuffer->GetMaterial().diffuse.Set(
			pCurrMaterial->Meshbuffer->GetMaterial().diffuse.GetAlpha(), 1.0f, 1.0f, 1.0f);
	}

	return pFrom;
}

// Liest das gegebene material ein
void MeshLoaderOBJ::ReadMaterial(const char* pcFilename, const io::FileDescription& fileDesc)
{
	const io::path realFile = pcFilename;
	StrongRef<io::File> mtlReader;

	if(m_Filesystem->ExistFile(realFile))
		mtlReader = m_Filesystem->OpenFile(realFile);
	if(mtlReader == nullptr) {
		io::FileDescription newFile = io::ConcatFileDesc(fileDesc, realFile);
		mtlReader = m_Filesystem->OpenFile(newFile);
	}
	if(!mtlReader) {
		log::Warning("Can't open the material file: ~s.", realFile);
		return;
	}



	const long filesize = mtlReader->GetSize();
	if(!filesize) {
		log::Warning("Empty material file is ignored: ~s.", realFile);
		return;
	}

	char* pBuffer = LUX_NEW_ARRAY(char, filesize);
	mtlReader->ReadBinary(filesize, (void*)pBuffer);
	const char* pBufferEnd = pBuffer + filesize;

	SObjMtl* currMaterial = 0;

	const char* pBufferPtr = pBuffer;
	while(pBufferPtr != pBufferEnd) {
		switch(*pBufferPtr) {
		case 'n': // newmtl
		{
			// Letztes material speichern
			if(currMaterial)
				m_Materials.Push_Back(currMaterial);

			// Neuen Materialname einlesen
			char mtlNameBuf[WORD_BUFFER_LENGTH];
			pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, mtlNameBuf, WORD_BUFFER_LENGTH);

			currMaterial = LUX_NEW(SObjMtl);
			currMaterial->name = mtlNameBuf;
		}
		break;
		case 'i': // illum - Beleuchtungsmodell
			if(currMaterial) {
				const u32 COLOR_BUFFER_LENGTH = 16;
				char illumStr[COLOR_BUFFER_LENGTH];

				pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, illumStr, COLOR_BUFFER_LENGTH);
				currMaterial->Illumination = (u8)(atol(illumStr));
			}
			break;
		case 'N':
			if(currMaterial) {
				switch(pBufferPtr[1]) {
				case 's': // Ns - Glanzkraft
				{
					const u32 COLOR_BUFFER_LENGTH = 16;
					char nsStr[COLOR_BUFFER_LENGTH];

					pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, nsStr, COLOR_BUFFER_LENGTH);
					float shininessValue = (float)(atof(nsStr));

					currMaterial->Meshbuffer->GetMaterial().shininess = shininessValue;
				}
				break;
				case 'i': // Ni - Brechungsindex ( verwerfen)
				{
					char tmpbuf[WORD_BUFFER_LENGTH];
					pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, tmpbuf, WORD_BUFFER_LENGTH);
				}
				break;
				}
			}
			break;
		case 'K':
			if(currMaterial) {
				switch(pBufferPtr[1]) {
				case 'd':        // Kd = Streufarbe
				{
					pBufferPtr = ReadColor(pBufferPtr, pBufferEnd, currMaterial->Meshbuffer->GetMaterial().diffuse);
				}
				break;

				case 's':        // Ks = Glanzfarbe
				{
					pBufferPtr = ReadColor(pBufferPtr, pBufferEnd, currMaterial->Meshbuffer->GetMaterial().specular);
				}
				break;

				case 'a':        // Ka = Hintergrundfarbe
				{
					video::Colorf AmbientColor;
					pBufferPtr = ReadColor(pBufferPtr, pBufferEnd, AmbientColor);
					currMaterial->Meshbuffer->GetMaterial().ambient = AmbientColor.GetLuminance() / 255.0f;
				}
				break;
				case 'e':        // Ke = emissive
				{
					pBufferPtr = ReadColor(pBufferPtr, pBufferEnd, currMaterial->Meshbuffer->GetMaterial().emissive);
				}
				break;
				}
			}
			break;
		case 'b': // bump
		case 'm': // texture maps
			if(currMaterial) {
				pBufferPtr = ReadTextures(pBufferPtr, pBufferEnd, currMaterial, fileDesc);
			}
			break;
		case 'd': // d - Transparenz
			if(currMaterial) {
				const u32 COLOR_BUFFER_LENGTH = 16;
				char dStr[COLOR_BUFFER_LENGTH];

				pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, dStr, COLOR_BUFFER_LENGTH);
				float dValue = (float)(atof(dStr));

				currMaterial->Meshbuffer->GetMaterial().diffuse.SetAlpha(dValue);
				// TODO: Richtiges material setzen
			}
			break;
		case 'T':
			if(currMaterial) {
				switch(pBufferPtr[1]) {
				case 'f':        // Tf - Transmitivity
					const u32 COLOR_BUFFER_LENGTH = 16;
					char redStr[COLOR_BUFFER_LENGTH];
					char greenStr[COLOR_BUFFER_LENGTH];
					char blueStr[COLOR_BUFFER_LENGTH];

					pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, redStr, COLOR_BUFFER_LENGTH);
					pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, greenStr, COLOR_BUFFER_LENGTH);
					pBufferPtr = CopyNextWord(pBufferPtr, pBufferEnd, blueStr, COLOR_BUFFER_LENGTH);

					float transparency = (float)((atof(redStr) + atof(greenStr) + atof(blueStr)) / 3);

					currMaterial->Meshbuffer->GetMaterial().diffuse.SetAlpha(transparency);
					// TODO: Materialtyp setzen
				}
			}
			break;
		default: // Kommentar oder wird nicht verarbeitet
			break;
		}
		// Und zur nächsten Zeile
		pBufferPtr = MoveToNextLine(pBufferPtr, pBufferEnd);
	}

	// Ende der Datei wenn ein material erstellt wurde speichern
	if(currMaterial) {
		if(currMaterial->Meshbuffer->GetMaterial().GetRenderer() == nullptr)
			currMaterial->Meshbuffer->GetMaterial().SetRenderer(m_SceneManager->GetMaterialLibrary()->GetMaterialRenderer("solid"));

		m_Materials.Push_Back(currMaterial);
	}

	LUX_FREE_ARRAY(pBuffer);
}

// Findet und liefert das material mit dem Namen
MeshLoaderOBJ::SObjMtl* MeshLoaderOBJ::FindMaterial(const string& mtlName, const string& grpName)
{
	MeshLoaderOBJ::SObjMtl* pTmp = nullptr;

	// Ist das material schon geladen, falls ja direkt ausgeben
	// Falls nur der name übereinstimmt eine neue Gruppe erstellen
	for(u32 dwMat = 0; dwMat < m_Materials.Size(); ++dwMat) {
		if(m_Materials[dwMat]->name == mtlName)
			return m_Materials[dwMat];
		else
			pTmp = m_Materials[dwMat];
	}

	if(pTmp) {
		m_Materials.Push_Back(LUX_NEW(MeshLoaderOBJ::SObjMtl)(*pTmp));
		MeshLoaderOBJ::SObjMtl* out = m_Materials[m_Materials.Size() - 1];
		out->Group = grpName;
		return out;
	} else if(!grpName.IsEmpty()) {
		m_Materials.Push_Back(LUX_NEW(SObjMtl)(*m_Materials[0]));
		MeshLoaderOBJ::SObjMtl* out = m_Materials[m_Materials.Size() - 1];
		out->Group = grpName;
		return out;
	}

	return nullptr;
}

// Verknüpft die Vertizes und Indizes in der Datei miteinander
bool MeshLoaderOBJ::RetrieveVertexIndices(const char* pFrom, const char* pTo, s32* indices,
	u32 vbsize, u32 vtsize, u32 vnsize)
{
	char acWord[16] = "";
	const char* p = MoveToFirstWord(pFrom, pTo);
	u32 idxType = 0;    // 0 = posIdx, 1 = texcoordIdx, 2 = normalIdx

	u32 i = 0;
	while(p != pTo) {
		if((core::IsDigit(*p)) || (*p == '-')) {
			// Nummer speichern
			acWord[i++] = *p;
		} else if(*p == '/' || *p == ' ' || *p == '\0') {
			// Nummer vorbei beenden und konvertieren
			acWord[i] = '\0';
			// Wenn keine Zahl gefunden wurde wird sie später zu -1 also ungültig und wird verworfen
			indices[idxType] = atol(acWord);
			if(indices[idxType] < 0) {
				// Relative Codierung übersetzen
				switch(idxType) {
				case 0:
					indices[idxType] += vbsize;
					break;
				case 1:
					indices[idxType] += vtsize;
					break;
				case 2:
					indices[idxType] += vnsize;
					break;
				}
			}
			// Ins 0-basierte index System umrechnen
			else
				indices[idxType] -= 1;

			// Puffer zurücksetzen
			acWord[0] = '\0';
			i = 0;

			// Der nächste Indextyp
			if(*p == '/') {
				if(++idxType > 2) {
					// Wir verarbeiten nur die Grundvertizes
					idxType = 0;
				}
			} else {
				// Alle fehlende Wert als ungülig definieren
				while(++idxType < 3)
					indices[idxType] = -1;
				++p;
				break;
			}
		}

		// Das nächste Zeichen
		++p;
	}

	return true;
}

// Geht zum ersten Wort
const char* MeshLoaderOBJ::MoveToFirstWord(const char* pFrom, const char* const pTo, bool bAcrossNewLines)
{
	if(bAcrossNewLines) {
		while((pFrom != pTo) && core::IsSpace(*pFrom))
			++pFrom;
	} else {
		while((pFrom != pTo) && core::IsSpace(*pFrom) && (*pFrom != '\n'))
			++pFrom;
	}

	return pFrom;
}

// Bewegt den Cursor zum nächsten druckbaren Zeichen
const char* MeshLoaderOBJ::MoveToNextWord(const char* pFrom, const char* pTo, bool bAcrossNewLines)
{
	while(pFrom != pTo && !core::IsSpace(*pFrom))
		++pFrom;

	return MoveToFirstWord(pFrom, pTo, bAcrossNewLines);
}

// Bewegt den Cursor zum nächsten druckbaren Zeichen in der nächsten Zeile
const char* MeshLoaderOBJ::MoveToNextLine(const char* pFrom, const char* pTo)
{
	while(pFrom != pTo) {
		if(*pFrom == '\n' || *pFrom == '\r')
			break;
		++pFrom;
	}

	return MoveToFirstWord(pFrom, pTo, true);
}

// Liest das aktuelle Wort am Cursor aus
u32 MeshLoaderOBJ::CopyWord(const char* pFrom, const char* pTo, char* out, u32 dwMaxLength)
{
	if(!dwMaxLength)
		return 0;
	if(!pFrom) {
		*out = 0;
		return 0;
	}

	u32 i = 0;
	while(pFrom[i]) {
		if(core::IsSpace(pFrom[i]) || &(pFrom[i]) == pTo)
			break;
		++i;
	}

	u32 length = math::Min<u32>(i, dwMaxLength - 1);
	for(u32 j = 0; j < length; ++j)
		out[j] = pFrom[j];

	out[length] = 0;
	return length;
}

// Liest die aktuelle Zeile am Cursor aus
string MeshLoaderOBJ::CopyLine(const char* pFrom, const char* pTo)
{
	const char* pTmp = pFrom;
	while(pTmp < pTo) {
		if(*pTmp == '\n' || *pTmp == '\r')
			break;
		++pTmp;
	}

	return string(pFrom, (u32)(pTmp - pFrom + ((pTmp < pTo) ? 1 : 0)));
}

// Kombination aus MoveToNextWord und CopyWord
const char* MeshLoaderOBJ::CopyNextWord(const char* pFrom, const char* pTo, char* out, u32 dwMaxLength)
{
	pFrom = MoveToNextWord(pFrom, pTo, false);
	CopyWord(pFrom, pTo, out, dwMaxLength);
	return pFrom;
}

// Liest eine RGB-Farbe
const char*  MeshLoaderOBJ::ReadColor(const char* pFrom, const char* pTo, video::Colorf& out)
{
	const u32 BUFFER_LENGHT = 16;
	char aBuffer[BUFFER_LENGHT];

	out.SetAlpha(1.0f);

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.SetRed((float)atof(aBuffer));

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.SetGreen((float)atof(aBuffer));

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.SetBlue((float)atof(aBuffer));

	return pFrom;
}

// Liest einen 3D-Vektor
const char*  MeshLoaderOBJ::Read3DVec(const char* pFrom, const char* pTo, math::vector3f& out)
{
	const u32 BUFFER_LENGHT = 256;
	char aBuffer[BUFFER_LENGHT];

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.x = -(float)(atof(aBuffer));        // OBJ-Dateien benutzen ein linkshändiges Koordinatensystem

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.y = (float)(atof(aBuffer));

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.z = (float)(atof(aBuffer));

	return pFrom;
}

// Liest einen 2D-Vektor
const char*  MeshLoaderOBJ::Read2DVec(const char* pFrom, const char* pTo, math::vector2f& out)
{
	const u32 BUFFER_LENGHT = 256;
	char aBuffer[BUFFER_LENGHT];

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.x = (float)(atof(aBuffer));

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out.y = 1.0f - (float)(atof(aBuffer));    // in das normale Koordinatensystem umrechnen

	return pFrom;
}

// Liest eine Wahrheitswert
const char*  MeshLoaderOBJ::ReadBool(const char* pFrom, const char* pTo, bool& out)
{
	const u32 BUFFER_LENGHT = 8;
	char aBuffer[BUFFER_LENGHT];

	pFrom = CopyNextWord(pFrom, pTo, aBuffer, BUFFER_LENGHT);
	out = (strcmp(aBuffer, "off") != 0);

	return pFrom;
}

}    // namespace scene
}    // namespace lux
