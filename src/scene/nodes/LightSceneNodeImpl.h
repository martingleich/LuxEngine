#ifndef INCLUDED_CLIGHTSCENENODE_H
#define INCLUDED_CLIGHTSCENENODE_H
#include "scene/nodes/LightSceneNode.h"

namespace lux
{
namespace scene
{

class LightSceneNodeImpl : public LightSceneNode
{
private:
	video::LightData m_LightData;

public:
	LightSceneNodeImpl();
	LightSceneNodeImpl(const LightSceneNodeImpl& other) = default;

	virtual ~LightSceneNodeImpl()
	{
	}

	virtual void SetLightData(const video::LightData& light);

	virtual const video::LightData& GetLightData() const;
	virtual video::LightData& GetLightData();

	virtual void SetRange(float range);
	virtual float GetRange() const;

	virtual void SetColor(const video::Colorf& color);
	virtual const video::Colorf& GetColor() const;

	virtual void SetLightType(video::LightData::EType type);
	virtual video::LightData::EType GetLightType() const;

	virtual const math::vector3f& GetDirection();

	virtual void OnRegisterSceneNode();
	virtual void Render();

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;
};

}    // namespace scene
}    // namespace lux

#endif