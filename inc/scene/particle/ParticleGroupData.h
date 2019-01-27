#ifndef INCLUDED_LUX_PARTICLE_GROUP_DATA_H
#define INCLUDED_LUX_PARTICLE_GROUP_DATA_H
#include "core/ReferenceCounted.h"
#include "core/lxPool.h"
#include "core/lxArray.h"

#include "math/Transformation.h"

#include "scene/particle/ParticleModel.h"

namespace lux
{
namespace scene
{
class AbstractParticleEmitter;
class AbstractParticleAffector;
class Node;

class ParticleGroupData : public ReferenceCounted
{
public:
	struct SystemData
	{
		int globalCount;
		int localCount;
		const StrongRef<AbstractParticleAffector>* affectors[2];
		int affectorsCounts[2];
		const StrongRef<AbstractParticleEmitter>* emitters;
		int emitterCount;

		Node* psSystem;
	};

private:
	struct EmitData
	{
		int total;
		int count;
		int emitter;

		EmitData() {}
		EmitData(int c, int e) :
			total(c),
			count(c),
			emitter(e)
		{
		}
	};

	struct CreationData
	{
		int count;
		math::Vector3F position;
		math::Vector3F velocity;
	};

public:
	LUX_API ParticleGroupData(ParticleModel* model, int capacity);
	LUX_API ~ParticleGroupData();

	LUX_API void Update(float secsPassed, const SystemData& data);
	LUX_API void AddParticle(int count, const math::Vector3F& position, const math::Vector3F& velocity);

	const core::Pool<Particle>& GetPool() const
	{
		return m_Pool;
	}
	int GetParticleCount() const { return GetPool().GetActiveCount(); }
	int GetParticleCapacity() const { return GetPool().Capactity(); }
	ParticleModel* GetModel()
	{
		return m_Model;
	}
private:
	bool LaunchParticle(Particle& particle, float secsPassed, const SystemData& data);
	bool AnimateParticle(Particle& particle, float secsPassed, const SystemData& data);

private:
	ParticleModel* m_Model;
	int m_ModelChangeId;

	core::Pool<Particle> m_Pool;
	core::Array<float> m_Data;

	core::Array<EmitData> m_EmitData;
	core::Array<CreationData> m_CreationData;

	core::Array<math::Transformation> m_EmitTransform;
	core::Array<math::Transformation> m_LastEmitTransform;

	core::Randomizer m_Random;

	int m_RotSpeedOffset;
	int m_AngleOffset;

	int m_CurEmitId;
};

} // namespace scene
} // namespace lux

#endif
