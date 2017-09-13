#ifndef INCLUDED_RENDER_TARGET_H
#define INCLUDED_RENDER_TARGET_H
#include "video/Texture.h"

namespace lux
{
namespace video
{

//! A rendertarget
/**
This class is a wrapper around a texture or something diffrent.
*/
class RenderTarget
{
public:
	//! Create a backbuffer rendertarget of a given size
	RenderTarget(const math::Dimension2U& size = math::Dimension2U(0, 0)) :
		m_Texture(nullptr),
		m_Size(size)
	{
	}

	//! Create a rendertarget from a texture
	/**
	The texture may be destroyed while in use as rendertarget
	\param t The texture to use as rendertarget. This texture must be a rendertarget texture.
	If null is passed the rendertarget represents the backbuffer
	*/
	RenderTarget(Texture* t) :
		m_Texture(t)
	{
		if(m_Texture) {
			m_Size = m_Texture->GetSize();
			if(!m_Texture->IsRendertarget())
				throw core::InvalidArgumentException("target", "Must be a rendertarget texture");
		}
	}

	bool operator==(const RenderTarget& other) const
	{
		return m_Texture == other.m_Texture;
	}

	bool operator!=(const RenderTarget& other) const
	{
		return !(*this == other);
	}

	//! Get the rendertarget texture
	/**
	May be null, to represent the backbuffer
	*/
	Texture* GetTexture() const
	{
		return m_Texture;
	}

	//! The size of the rendertarget texture
	const math::Dimension2U& GetSize() const
	{
		return m_Size;
	}

	//! Represents the rendertarget the backbuffer
	bool IsBackbuffer() const
	{
		return (m_Texture == nullptr);
	}

protected:
	StrongRef<Texture> m_Texture;
	math::Dimension2U m_Size;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_RENDER_TARGET_H