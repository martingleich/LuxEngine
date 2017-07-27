#ifndef INCLUDED_PARTICLE_GROUP_DATA_H
#define INCLUDED_PARTICLE_GROUP_DATA_H
#include "core/ReferenceCounted.h"

#include "core/lxPool.h"
#include "core/lxArray.h"
#include "core/lxRandom.h"

#include "math/Transformation.h"

#include "Particle.h"

namespace lux
{
namespace scene
{
class ParticleModel;
class ParticleEmitter;
class ParticleAffector;
class Node;

class ParticleGroupData : public ReferenceCounted
{
public:
	struct AffectorEntry
	{
		ParticleAffector* a;
		Node* n;

		explicit AffectorEntry(ParticleAffector* _a, Node* _n = nullptr) :
			a(_a),
			n(_n)
		{
		}

		ParticleAffector* operator->() { return a; }
		const ParticleAffector* operator->() const { return a; }
	};

	struct EmitterEntry
	{
		math::Transformation transform;
		math::Transformation lastTransform;
		ParticleEmitter* e;
		Node* n;

		explicit EmitterEntry(ParticleEmitter* _e, Node* _n = nullptr) :
			e(_e),
			n(_n)
		{
		}

		ParticleEmitter* operator->() { return e; }
		const ParticleEmitter* operator->() const { return e; }
	};

	struct SystemData
	{
		AffectorEntry* globals;
		size_t globalCount;
		AffectorEntry* locals;
		size_t localCount;

		EmitterEntry* emitters;
		size_t emitterCount;

		const Node* owner;
		const Node* psSystem;
	};

private:
	struct EmitData
	{
		u32 total;
		u32 count;
		EmitterEntry* entry;

		EmitData() {}
		EmitData(u32 c, EmitterEntry* e) :
			total(c),
			count(c),
			entry(e)
		{
		}
	};

	struct CreationData
	{
		u32 count;
		ParticleEmitter* emitter;

		math::vector3f position;
		math::vector3f velocity;

		CreationData()
		{
		}
	};

public:
	LUX_API ParticleGroupData(ParticleModel* model, u32 capacity);
	LUX_API ~ParticleGroupData();

	LUX_API void Update(float secsPassed, const SystemData& data);

	LUX_API void AddParticle(u32 count, const math::vector3f& position, const math::vector3f& velocity);
	LUX_API void AddParticle(u32 count, ParticleEmitter* emitter);

	LUX_API u32 GetParticleCount() const;
	LUX_API u32 GetParticleCapacity() const;

	LUX_API const core::Pool<Particle>& GetPool() const;
	LUX_API ParticleModel* GetModel();

private:
	bool LaunchParticle(Particle& particle, float secsPassed);
	bool AnimateParticle(Particle& particle, float secsPassed, const SystemData& data);

private:
	ParticleModel* m_Model;

	core::Pool<Particle> m_Pool;
	core::Array<float> m_Data;

	core::Array<EmitData> m_EmitData;
	core::Array<CreationData> m_CreationData;

	core::Randomizer m_Random;

	int m_RotSpeedOffset;
	int m_AngleOffset;

	u32 m_CurEmitId;
};

} // namespace scene
} // namespace lux

#endif
