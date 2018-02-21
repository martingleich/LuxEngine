#include "video/Material.h"

LX_REFERABLE_MEMBERS_SRC(lux::video::Material, "lux.Material");

namespace lux
{
namespace video
{

Material::Material() :
	m_Requirement(EMaterialRequirement::None)
{
}

Material::~Material()
{
}

} // namespace video
} // namespace lux
