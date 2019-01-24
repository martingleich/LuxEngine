#ifndef INCLUDED_LUX_RESOURCE_H
#define INCLUDED_LUX_RESOURCE_H
#include "core/Referable.h"

namespace lux
{

namespace io
{
class File;
}

namespace core
{

//! The diffrent resources types native to the engine.
namespace ResourceType
{
LUX_API extern const Name Image; //!< Resources of type \ref lux::video::Image
LUX_API extern const Name Texture; //!< Resources of type \ref lux::video::Texture
LUX_API extern const Name CubeTexture; //!< Resources of type \ref lux::video::CubeTexture
LUX_API extern const Name Mesh; //!< Resources of type \ref lux::video::Mesh
LUX_API extern const Name Font; //!< Resources of type \ref lux::gui::Font
LUX_API extern const Name Sound; //!< Resources of type \ref lux::sound::SoundSource
LUX_API extern const Name Material; //!< Resources of type \ref lux::video::Material
LUX_API extern const Name Animation; //!< Resources of type \ref lux::scene::Animation
}

class OriginResourceLoader
{
public:
	virtual void LoadResource(const core::String& origin, core::Referable* dst) const = 0;
	virtual const core::String& GetName() const = 0;
};

class ResourceOrigin
{
public:
	ResourceOrigin() :
		m_Loader(nullptr)
	{}
	ResourceOrigin(OriginResourceLoader* l, const core::String& s) :
		m_String(s),
		m_Loader(l)
	{
	}

	OriginResourceLoader* GetLoader() const { return m_Loader; }
	const core::String& GetString() const { return m_String; }

	bool operator==(const ResourceOrigin& other) const { return m_Loader == other.m_Loader && m_String == other.m_String; }
	bool operator!=(const ResourceOrigin& other) const { return m_Loader != other.m_Loader || m_String != other.m_String; }

private:
	core::String m_String;
	OriginResourceLoader* m_Loader;
};

template <>
struct HashType<ResourceOrigin>
{
	unsigned int operator()(const ResourceOrigin& origin) const
	{
		SequenceHasher seq;
		seq.Add(core::HashType<OriginResourceLoader*>()(origin.GetLoader()));
		seq.Add(core::HashType<core::String>()(origin.GetString()));
		return seq.GetHash();
	}
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_RESOURCE_H