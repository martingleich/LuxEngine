#ifndef INCLUDED_LUX_PARTICLERENDERER_H
#define INCLUDED_LUX_PARTICLERENDERER_H
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
class ParticleGroupData;

class ParticleRenderer : public core::Referable
{
public:
	StrongRef<ParticleRenderer> Clone() { return CloneImpl().StaticCastStrong<ParticleRenderer>(); }
	virtual void Render(video::Renderer* videoRenderer, ParticleGroupData* group) = 0;

};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_LUX_IPARTICLERENDERER