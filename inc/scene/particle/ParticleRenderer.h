#ifndef INCLUDED_PARTICLERENDERER_H
#define INCLUDED_PARTICLERENDERER_H
#include "core/Referable.h"
#include "video/SpriteBank.h"
#include "math/Vector3.h"

namespace lux
{
namespace video
{
class Renderer;
}
namespace scene
{

class RendererMachine;
class ParticleGroupData;

class ParticleRenderer : public Referable
{
public:
	StrongRef<ParticleRenderer> Clone()
	{
		return CloneImpl().StaticCastStrong<ParticleRenderer>();
	}
	virtual StrongRef<RendererMachine> GetMachine() const = 0;
};

class RendererMachine : public Referable
{
public:
	virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer = nullptr) = 0;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_IPARTICLERENDERER