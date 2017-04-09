#ifndef INCLUDED_RESOURCE_H
#define INCLUDED_RESOURCE_H
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
LUX_API extern const Name Mesh; //!< Resources of type \ref lux::scene::Mesh
LUX_API extern const Name Font; //!< Resources of type \ref lux::gui::Font
LUX_API extern const Name Sound; //!< Resources of type \ref lux::sound::SoundSource
}

class Resource;
class ResourceOrigin
{
public:
	string str;
};

class OriginResourceLoader
{
public:
	virtual bool LoadResource(const ResourceOrigin& origin, Resource* dst) const = 0;
};

//! A engine resource object
class Resource : public Referable
{
public:
	Resource() :
		m_Loader(nullptr),
		m_Loaded(false)
	{
	}

	virtual ~Resource()
	{
	}

	Name GetReferableType() const
	{
		return ReferableType::Resource;
	}

	virtual bool Load()
	{
		if(m_Loaded)
			return true;

		bool result = false;
		if(m_Loader)
			result = m_Loader->LoadResource(m_Origin, this);

		if(result)
			m_Loaded = true;

		return m_Loaded;
	}

	virtual void Unload()
	{
		m_Loaded = false;
	}

	void SetLoaded(bool loaded)
	{
		m_Loaded = loaded;
	}

	bool IsLoaded() const
	{
		return m_Loaded;
	}

	void SetOrigin(const OriginResourceLoader* loader, const ResourceOrigin& origin)
	{
		m_Loader = loader;
		m_Origin = origin;
	}

	const OriginResourceLoader* GetOriginLoader() const
	{
		return m_Loader;
	}

	const ResourceOrigin& GetOrigin() const
	{
		return m_Origin;
	}

private:
	const OriginResourceLoader* m_Loader;
	ResourceOrigin m_Origin;
	bool m_Loaded;
};

}
}

#endif // #ifndef INCLUDED_RESOURCE_H