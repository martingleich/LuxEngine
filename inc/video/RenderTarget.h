#ifndef INCLUDED_RENDER_TARGET_H
#define INCLUDED_RENDER_TARGET_H
#include "video/Texture.h"

namespace lux
{
namespace video
{

struct RenderTarget
{
	RenderTarget(const math::dimension2du& size = math::dimension2du(0, 0)) :
		m_Texture(nullptr),
		m_Size(size)
	{
	}

	RenderTarget(Texture* t) :
		m_Texture(t)
	{
		if(m_Texture)
			m_Size = m_Texture->GetSize();
	}

	bool operator==(const RenderTarget& other) const
	{
		return m_Texture == other.m_Texture;
	}

	bool operator!=(const RenderTarget& other) const
	{
		return !(*this == other);
	}

	Texture* GetTexture() const
	{
		return m_Texture;
	}

	const math::dimension2du& GetSize() const
	{
		return m_Size;
	}

	operator bool() const
	{
		return HasTexture();
	}

	bool HasTexture() const
	{
		return (m_Texture != nullptr);
	}

	bool IsValid() const
	{
		return m_Texture && m_Texture->IsRendertarget();
	}

protected:
	WeakRef<Texture> m_Texture;
	math::dimension2du m_Size;
};

}
}

#endif // #ifndef INCLUDED_RENDER_TARGET_H