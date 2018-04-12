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
LUX_API extern const Name ImageList; //!< Resources of type \ref lux::video::ImageList
LUX_API extern const Name Texture; //!< Resources of type \ref lux::video::Texture
LUX_API extern const Name CubeTexture; //!< Resources of type \ref lux::video::CubeTexture
LUX_API extern const Name Mesh; //!< Resources of type \ref lux::video::Mesh
LUX_API extern const Name Font; //!< Resources of type \ref lux::gui::Font
LUX_API extern const Name Sound; //!< Resources of type \ref lux::sound::SoundSource
LUX_API extern const Name Material; //!< Resources of type \ref lux::sound::SoundSource
}

class Resource;
class OriginResourceLoader;

struct ResourceOrigin
{
	ResourceOrigin() :
		loader(nullptr)
	{}
	ResourceOrigin(OriginResourceLoader* l, const core::String& s) :
		str(s),
		loader(l)
	{
	}

	core::String str;
	OriginResourceLoader* loader;

	inline void Load(Resource* dst) const;
	inline bool IsAvailable() const
	{
		return loader != nullptr;
	}
};

class OriginResourceLoader
{
public:
	virtual void LoadResource(const ResourceOrigin& origin, Resource* dst) const = 0;
};

void ResourceOrigin::Load(Resource* dst) const
{
	if(loader)
		loader->LoadResource(*this, dst);
}

//! A engine resource object
class Resource : public Referable
{
public:
	Resource(const ResourceOrigin& origin) :
		m_Origin(origin),
		m_Loaded(origin.loader?false:true)
	{
	}

	Resource(const Resource& other) :
		m_Origin(other.m_Origin),
		m_Loaded(other.m_Loaded)
	{

	}

	Resource() :
		m_Loaded(true)
	{
	}

	virtual ~Resource() {}

	virtual void Load()
	{
		if(!m_Loaded)
			m_Origin.Load(this);
	}

	virtual void Reload()
	{
		Unload();
		Load();
	}

	virtual void Unload()
	{
		m_Loaded = false;
	}

	bool IsLoaded() const
	{
		return m_Loaded;
	}

	void SetLoaded(bool loaded)
	{
		m_Loaded = loaded;
	}

	const ResourceOrigin& GetOrigin() const
	{
		return m_Origin;
	}

private:
	ResourceOrigin m_Origin;
	bool m_Loaded;
};

#define LX_REGISTER_RESOURCE_CLASS(type, class) \
static ::lux::Referable* LX_CONCAT(InternalCreatorFunc_, __LINE__)(const void* origin) { return LUX_NEW(class)(origin?*(lux::core::ResourceOrigin*)origin:lux::core::ResourceOrigin()); } \
static ::lux::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject(::lux::core::Name(type), &LX_CONCAT(InternalCreatorFunc_, __LINE__));

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_RESOURCE_H