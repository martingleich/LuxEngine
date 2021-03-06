#ifndef INCLUDED_LUX_HARDWAREBUFFERMANAGER_H
#define INCLUDED_LUX_HARDWAREBUFFERMANAGER_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace video
{

class HardwareBuffer;
class IndexBuffer;
class VertexBuffer;
class VideoDriver;

//! Manages the hardwarebuffers for a driver
class BufferManager : public ReferenceCounted
{
public:
	virtual ~BufferManager() {}

	//! Start deferring the following updates.
	/**
	Stops Hardwarebuffer updates from immediatly affecting the hardwarebuffer.
	This allows to merge multiple changes to the same buffer and other optizations.
	EndDeferred must be called after calling BeginDeferred.
	*/
	//virtual void BeginDeferred() = 0;

	//! Stop deferring and flushes all deferred updates.
	//virtual void EndDeferred() = 0;

	//! Informs the buffermanager that a new buffer was created
	/**
	\param buffer The buffer which was created
	*/
	virtual void AddBuffer(HardwareBuffer* buffer) = 0;

	//! Informs the buffermanager that a buffer was removed
	/**
	\param buffer The buffer which was removed
	*/
	virtual void RemoveBuffer(HardwareBuffer* buffer) = 0;

	//! Update a single buffer
	/**
	Update the data in a single buffer and resets its dirty area.
	\param buffer The buffer which should be updated
	*/
	virtual void UpdateBuffer(HardwareBuffer* buffer) = 0;

	//! Enable a single hardwarebuffer for rendering
	/**
	Calls to IVideoDriver::DrawPrimitiveList must used the buffer used here
	for rendering.
	\param buffer The buffer to set
	*/
	virtual void EnableBuffer(const HardwareBuffer* buffer) = 0;

	//! Create a new index buffer
	virtual StrongRef<IndexBuffer> CreateIndexBuffer() = 0;

	//! Create a new vertex buffer
	virtual StrongRef<VertexBuffer> CreateVertexBuffer() = 0;
};

} // namespace vidoe
} // namespace lux

#endif // !INCLUDED_LUX_HARDWAREBUFFERMANAGER_H
