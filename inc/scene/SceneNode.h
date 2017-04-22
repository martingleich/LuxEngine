#ifndef INCLUDED_ISCENENODE_H
#define INCLUDED_ISCENENODE_H
#include "scene/SceneNodeComponent.h"
#include "math/aabbox3d.h"
#include "scene/Transformable.h"
#include "core/lxTreeNode.h"
#include "core/lxPool.h"
#include "core/lxString.h"
#include "video/Material.h"
#include "core/lxList.h"
#include "core/Referable.h"
#include "scene/collider/Collider.h"

namespace lux
{
namespace scene
{

namespace SceneNodeType
{
LUX_API extern const core::Name SkyBox;
LUX_API extern const core::Name Root;
LUX_API extern const core::Name Camera;
LUX_API extern const core::Name Mesh;
LUX_API extern const core::Name Light;
LUX_API extern const core::Name Empty;
}

class SceneManager;


enum EDebugData : u32
{
	EDD_NONE = 0,
	EDD_SUB_BBOX = 1,        // Alle Unterboundingboxen zeichen(z.B. Submeshes)
	EDD_MAIN_BBOX = 2,        // Nur die Hautpboundingbox zeichnen
	EDD_ALL_BBOX = EDD_SUB_BBOX | EDD_MAIN_BBOX,        // Sowohl Haupt- als auch Unterboundingbox zeichnen
};

// Eine Szenenknoten ist ein element des Szenengraphs
class SceneNode : public Referable, public scene::Transformable, private core::TreeNode, public input::EventReceiver
{
	friend struct RenderTransform;
public:
	typedef TreeNode::_Iterator<SceneNode>         ChildIterator;
	typedef TreeNode::_ConstIterator<SceneNode>    ChildConstIterator;

	struct ComponentEntry
	{
		ComponentEntry()
		{
		}
		ComponentEntry(SceneNodeComponent* c) : comp(c),
			markForDelete(false)
		{
		}

		StrongRef<SceneNodeComponent> comp;
		bool markForDelete;
	};
	typedef core::list<ComponentEntry> SceneNodeComponentList;

protected:
	struct RenderTransform
	{
		math::matrix4 world;
		math::matrix4 worldInvTrans;

		void Set(SceneNode* node)
		{
			node->GetAbsoluteTransform().ToMatrix(world);
			node->GetAbsoluteTransform().ToMatrixInv(worldInvTrans);
			worldInvTrans.Transpose();

			node->m_HasAbsTransChanged = false;
		}
	};

private:
	u32 m_Tags;

	u32 m_DebugFlags;
	bool m_IsVisible;
	bool m_ShouldRegister;
	bool m_IsEventReceiver;

	SceneNodeComponentList m_ComponentList;

	SceneManager* m_SceneManager;

	math::Transformation m_RelativeTrans;
	math::Transformation m_AbsoluteTrans;
	bool m_HasRelTransChanged;
	bool m_HasAbsTransChanged;

	StrongRef<Collider> m_Collider;

public:
	SceneNode()
		: m_HasRelTransChanged(true), m_HasAbsTransChanged(true), m_IsVisible(true), m_ShouldRegister(true),
		m_SceneManager(nullptr),
		m_DebugFlags(EDD_NONE), m_IsEventReceiver(false), m_Tags(0)
	{
		UpdateAbsTransform();
	}

	// Destuktor
	virtual ~SceneNode()
	{
		RemoveAllChildren();
		RemoveAllComponents();
		SetSceneManager(nullptr);
	}

	// Wird kurz vor dem Rendern aufgerufen
	// Hier muss man den Knoten zum Render anmelden
	// {sceneManager->RegisterNodeForRendering(this, ?);}
	virtual void OnRegisterSceneNode()
	{
		m_HasAbsTransChanged = false;

		ChildIterator entry = TreeNode::_GetChildrenFirst<SceneNode>();
		for(; entry != TreeNode::_GetChildrenEnd<SceneNode>(); ++entry) {
			if(entry->m_IsVisible && entry->m_ShouldRegister) {
				entry->OnRegisterSceneNode();
			}
		}
	}

	// Zeichnet den Knoten
	virtual void Render() = 0;

	// Animiert den Knoten
	// time = Sekunden seit dem letzten Frame
	virtual void Animate(float time)
	{
		SceneNodeComponentList::Iterator entry = m_ComponentList.First();
		while(entry != m_ComponentList.End()) {
			SceneNodeComponent* component = entry->comp;
			if(!entry->markForDelete) {
				if(component->IsAnimated() && component->IsActive())
					component->Animate(time);
			}

			SceneNodeComponentList::Iterator next_entry;
			if(entry->markForDelete) {
				entry->comp->SetParent(nullptr);
				next_entry = m_ComponentList.Erase(entry);
			} else {
				next_entry = entry;
				++next_entry;
			}

			entry = next_entry;
		}

		UpdateAbsTransform();

		ChildIterator child = TreeNode::_GetChildrenFirst<SceneNode>();
		while(child != TreeNode::_GetChildrenEnd<SceneNode>()) {
			child->Animate(time);
			++child;
		}
	}

	// Fügt einen neuen Component zum Scene-Node hinzu
	virtual StrongRef<SceneNodeComponent> AddComponent(SceneNodeComponent* animator)
	{
		if(animator) {
			if(animator->IsEventReceiver())
				this->SetEventReceiver(true);
			if(animator->GetParent())
				animator->GetParent()->RemoveComponent(animator);
			animator->SetParent(this);
			m_ComponentList.Push_back(ComponentEntry(animator));
		}
		return animator;
	}

	// Fragt alle Componenten des Scene-Nodes ab
	const SceneNodeComponentList& GetComponents() const
	{
		return m_ComponentList;
	}

	SceneNodeComponentList& GetComponents()
	{
		return m_ComponentList;
	}

	StrongRef<SceneNodeComponent> GetComponentByType(core::Name type) const
	{
		for(auto it = m_ComponentList.First(); it != m_ComponentList.End(); ++it) {
			if(type == it->comp->GetReferableSubType())
				return it->comp;
		}

		return nullptr;
	}

	bool HasComponentType(core::Name type) const
	{
		return (GetComponentByType(type) != nullptr);
	}

	//! Add a tag to this scene node.
	/**
	Each scene node can have up to 32 tags, each represented by a bit in an integer.
	These tags are never manipulates by the engine, and are under full user control.
	They can be used to identify groups of scene nodes.
	\param tag The tags to add to this scene node.
	*/
	void AddTag(u32 tag)
	{
		m_Tags |= tag;
	}

	//! Remove tags from the scene node.
	void RemoveTag(u32 tag)
	{
		m_Tags &= ~tag;
	}

	//! Check if all given tags are set in this scene node.
	bool HasTag(u32 tag) const
	{
		return (tag == 0 || ((m_Tags&tag) == tag));
	}

	//! Removes a component from the scene node
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use SceneNode::MarkForDelete.
	\param comp The component to remove from the node.
	\return Was the component found, and deleted.
	*/
	virtual bool RemoveComponent(SceneNodeComponent* comp)
	{
		// Component suchen und löschen
		SceneNodeComponentList::Iterator entry = m_ComponentList.First();

		for(; entry != m_ComponentList.End(); ++entry) {
			if(entry->comp == comp) {
				m_ComponentList.Erase(entry);
				comp->SetParent(nullptr);
				return true;
			}
		}

		return false;
	}

	// Löscht alle Componenten
	virtual void RemoveAllComponents()
	{
		m_ComponentList.Clear();
	}

	// Liefert die Bounding-box des Knotens
	virtual const math::aabbox3df& GetBoundingBox() const
	{
		return math::aabbox3df::EMPTY;
	}

	virtual void SetRelativeTransform(const math::Transformation& t)
	{
		m_HasRelTransChanged = true;
		m_RelativeTrans = t;
	}

	// Gibt die absolute Transformation aus
	virtual const math::Transformation& GetAbsoluteTransform() const
	{
		return m_AbsoluteTrans;
	}

	// Gibt die Relative Transformation aus
	virtual const math::Transformation& GetRelativeTransform() const
	{
		return m_RelativeTrans;
	}

	// Die Anzahl der Materialien des Knotens
	virtual size_t GetMaterialCount() const
	{
		return 0;
	}

	// Liefert ein material des Knotens
	virtual video::Material& GetMaterial(size_t mat)
	{
		LUX_UNUSED(mat);
		return video::WorkMaterial;
	}

	// Ist der Knoten selbst sichtbar
	virtual bool IsVisible() const
	{
		return m_IsVisible;
	}

	// Ist der Knoten wirklich sichtbar also auch sein Vater und dessen Vater...
	virtual bool IsTrulyVisible() const
	{
		const SceneNode* node = this;
		while(node) {
			if(node->IsVisible() == false)
				return false;

			node = node->GetParent();
		}

		return true;
	}

	// Setzt die Sichtbarkeit des Knotens
	virtual void SetVisible(bool visible)
	{
		m_IsVisible = visible;
	}

	// Fügt dem Knoten ein Kind hinzu
	// Wenn das Kind bereits einen Vater hat wird er von zuerst von dort gelöscht
	virtual bool AddChild(SceneNode* child)
	{
		if(child && (child != this)) {
			if(m_SceneManager != child->m_SceneManager) {
				bool r = child->SetSceneManager(m_SceneManager);
				if(!r)
					return false;
			}

			child->Grab();

			// Der Vater gibt sein Kind frei
			SceneNode* parent = child->GetParent();
			if(parent)
				child->Drop();

			// Und bei uns anhängen
			TreeNode::_AddChild(child);

			return true;
		}

		return false;
	}

	// Löscht einen Kindknoten, wenn keine anderen Verweise auf ihn existieren
	// wird er gelöscht
	// Rückgabe: True wenn er entfernt werden konnte ansonsten false
	virtual bool RemoveChild(SceneNode* child)
	{
		ChildIterator it = TreeNode::_GetChildrenFirst<SceneNode>();
		for(; it != TreeNode::_GetChildrenEnd<SceneNode>(); ++it) {
			if(it.Pointer() == (TreeNode*)child) {
				it->_RemoveFromParent();
				it->Drop();
				return true;
			}
		}

		return false;
	}

	// Löscht alle Kinder dieses Knotens
	virtual void RemoveAllChildren()
	{
		auto lambda = [](TreeNode* node) -> bool { (static_cast<SceneNode*>(node))->Drop(); return true; };

		this->_RemoveAllChilds(true, lambda);
	}

	// Löscht diesen Knoten aus der Szene
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use SceneManager::AddToDeletionQueue or SceneNode::MarkForDelete
	*/
	virtual void Remove()
	{
		SceneNode* parent = GetParent();
		if(parent)
			parent->RemoveChild(this);
	}

	LUX_API void MarkForDelete();

	//! Marks a scene node componenent for deletion.
	/**
	This method can be called at any time in the program.
	The component will be removed before the next frame.
	\param comp The component to remove.
	*/
	void MarkForDelete(SceneNodeComponent* comp)
	{
		auto& comps = GetComponents();
		for(auto it = comps.First(); it != comps.End(); ++it) {
			if(it->comp == comp)
				it->markForDelete = true;
		}
	}

	// Gibt die Liste der Kinderknoten zurück
	ChildIterator GetChildrenFirst()
	{
		return TreeNode::_GetChildrenFirst<SceneNode>();
	}

	ChildIterator GetChildrenEnd()
	{
		return TreeNode::_GetChildrenEnd<SceneNode>();
	}

	// Setzt den Vater dieses Knotens
	virtual bool SetParent(SceneNode* newParent)
	{
		if(newParent == GetParent())
			return true;

		bool result = newParent->AddChild(this);
		if(!result)
			return false;
		m_HasRelTransChanged = true;
		return true;
	}

	// Aktualisiert die Absolute Transformation
	// ACHTUNG: Arbeitet nicht rekursiv, also müssen die Eltern vorher auch aktualisiert werden
	void UpdateAbsTransform()
	{
		SceneNode* parent = GetParent();
		if(parent && parent != (SceneNode*)m_SceneManager) {
			if(parent->m_HasAbsTransChanged || m_HasRelTransChanged) {
				m_AbsoluteTrans = parent->m_AbsoluteTrans.CombineLeft(m_RelativeTrans);
				m_HasAbsTransChanged = true;
			}
		} else {
			if(m_HasRelTransChanged) {
				m_AbsoluteTrans = m_RelativeTrans;
				m_HasAbsTransChanged = true;
			}
		}

		m_HasRelTransChanged = false;
	}

	// Liefert den Vater diese Knotens zurück
	SceneNode* GetParent() const
	{
		return (SceneNode*)TreeNode::_GetParent();
	}

	// Gibt den Scenemanager dieses Knotens zurück
	SceneManager* GetSceneManager() const
	{
		return m_SceneManager;
	}

	LUX_API SceneNode* GetRoot();

	// Setzt welche Debug-Daten angezweigt werden sollen
	// debugData kann mittels OR auf mehrere Statusse gleichzeitig gesetzt werden
	// WICHTIG: Nicht alle Knoten unterstützen Alle Debuginformationen
	void SetDebugData(EDebugData debugData, bool state)
	{
		if(state)
			m_DebugFlags |= debugData;
		else
			m_DebugFlags &= ~debugData;
	}
	bool GetDebugData(EDebugData debugData)
	{
		return (m_DebugFlags & debugData) != 0;
	}

	// Setzt den neuen sceneManager für diesen Knoten und alle seine Kinder
	virtual bool SetSceneManager(SceneManager* newSmgr)
	{
		if(GetParent() != nullptr && GetParent()->GetSceneManager() != newSmgr)
			return false;

		if(m_SceneManager == newSmgr)
			return true;

		bool ReceiveEvents = IsEventReceiver();
		if(ReceiveEvents)
			this->SetEventReceiver(false);
		m_SceneManager = newSmgr;
		if(newSmgr) {
			if(ReceiveEvents)
				this->SetEventReceiver(true);
		}

		// Scene-Manager für die Kinder neu setzen
		ChildIterator entry = TreeNode::_GetChildrenFirst<SceneNode>();
		for(; entry != TreeNode::_GetChildrenEnd<SceneNode>(); ++entry)
			entry->SetSceneManager(newSmgr);

		return true;
	}

	StrongRef<Collider> SetCollider(Collider* collider)
	{
		m_Collider = collider;
		return m_Collider;
	}

	StrongRef<Collider> GetCollider() const
	{
		return m_Collider;
	}

	LUX_API EResult ExecuteQuery(Query* query, QueryCallback* queryCallback);

	virtual StrongRef<Referable> Clone() const
	{
		return nullptr;
	}

	core::Name GetReferableType() const
	{
		return ReferableType::SceneNode;
	}

	bool OnEvent(const input::Event& event)
	{
		auto& comp = GetComponents();
		for(auto it = comp.First(); it != comp.End(); ++it)
			if(it->comp->IsEventReceiver() && it->comp->IsActive())
				if(it->comp->OnEvent(event))
					return true;

		return false;
	}

	bool IsEventReceiver() const
	{
		return m_IsEventReceiver;
	}

	LUX_API void SetEventReceiver(bool receiver);

protected:
	SceneNode(const SceneNode& other)
	{
		m_Tags = other.m_Tags;
		m_RelativeTrans = other.m_RelativeTrans;
		m_AbsoluteTrans = other.m_AbsoluteTrans;
		m_HasRelTransChanged = true;
		m_HasAbsTransChanged = true;

		m_Collider = other.m_Collider;

		m_DebugFlags = other.m_DebugFlags;

		m_IsVisible = other.m_IsVisible;
		m_IsEventReceiver = other.m_IsEventReceiver;

		// Kinder kopieren
		ChildIterator entry = TreeNode::_GetChildrenFirst<SceneNode>();
		for(; entry != TreeNode::_GetChildrenEnd<SceneNode>(); ++entry) {
			StrongRef<SceneNode> s = (StrongRef<SceneNode>)entry->Clone();
			s->SetParent(this);
		}

		// Componenten kopieren
		SceneNodeComponentList::Iterator animIt = m_ComponentList.First();
		for(; animIt != m_ComponentList.End(); ++animIt) {
			StrongRef<SceneNodeComponent> animator = animIt->comp->Clone();
			if(animator)
				AddComponent(animator);
		}
	}

	void SetRegisterFlag(bool shouldRegister)
	{
		m_ShouldRegister = shouldRegister;
	}
};

}    

}    


#endif