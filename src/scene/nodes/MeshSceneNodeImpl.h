#ifndef INCLUDED_CMESHSCENENODE_H
#define INCLUDED_CMESHSCENENODE_H
#include "scene/nodes/MeshSceneNode.h"
#include "scene/mesh/Mesh.h"


namespace lux
{
namespace scene
{

class MeshSceneNodeImpl : public MeshSceneNode
{
private:
	// Kopiert die Materialien der Sub-Meshes in eine interne Liste
	void CopyMaterials();

public:
	// Konstruktor
	MeshSceneNodeImpl();

	MeshSceneNodeImpl(const MeshSceneNodeImpl& other) = default;

	// Destruktor
	virtual ~MeshSceneNodeImpl();

	// Reigstireit den Knoten
	virtual void OnRegisterSceneNode();

	// Zeichnet den Knoten
	virtual void Render();

	// Liefert die Boundig-box des Knoten
	virtual const math::aabbox3df& GetBoundingBox() const
	{
		return m_BoundingBox;
	}

	// Liefert ein material des Knotens
	virtual video::Material* GetMaterial(size_t index);
	virtual void SetMaterial(size_t index, video::Material* m);

	// Liefert die Anzahl der Materialien
	virtual size_t GetMaterialCount() const;

	// Das gesetzte Modell
	virtual StrongRef<Mesh> GetMesh()
	{
		return m_Mesh;
	}

	// Setzt das Modell neu
	virtual void SetMesh(Mesh* mesh);

	// Benutzt der Scene-Node nur die Materialien der Sub-Meshes
	// d.h. Wenn man das material der Sub-Meshes ändert, ändert es sich in allen
	// Scene-Nodes mit.
	virtual void SetReadMaterialsOnly(bool state)
	{
		m_OnlyReadMaterials = state;
		if(!m_OnlyReadMaterials)
			CopyMaterials();
	}

	// Benutzt der Scene-Node nur die Materialien der Sub-Meshes
	// d.h. Wenn man das material der Sub-Meshes ändert, ändert es sich in allen
	// Scene-Nodes mit.
	virtual bool GetReadMaterialsOnly() const
	{
		return m_OnlyReadMaterials;
	}

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	StrongRef<Mesh> m_Mesh;
	bool m_OnlyReadMaterials;    // Benutzt der Scene-Node nur die Materialien der Sub-Meshes
									// d.h. Wenn man das material der Sub-Meshes ändert, ändert es sich in allen
									// Scene-Nodes mit.
	core::array<StrongRef<video::Material>> m_Materials;    // Die Materialien des Knotens falls nicht m_OnlyReadMaterials

	math::aabbox3df m_BoundingBox;    // Damit wenn keine mesh geladen auch eine Bounding-box vorhanden ist

	RenderTransform m_RenderTransform;

};

}    

}    


#endif