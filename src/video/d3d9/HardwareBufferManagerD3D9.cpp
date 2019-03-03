#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "HardwareBufferManagerD3D9.h"
#include "core/Logger.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "VideoDriverD3D9.h"
#include "video/d3d9/D3DHelper.h"
#include "platform/D3D9Exception.h"

namespace lux
{
namespace video
{

BufferManagerD3D9::BufferManagerD3D9(VideoDriver* driver) :
	BufferManagerNull(driver)
{
	m_D3DDevice = reinterpret_cast<IDirect3DDevice9*>(m_Driver->GetLowLevelDevice());

	VideoDriverD3D9* drv = reinterpret_cast<VideoDriverD3D9*>(driver);
	if((drv->GetCaps().Caps2 & D3DDEVCAPS2_STREAMOFFSET) != 0)
		m_AllowStreamOffset = true;
	else
		m_AllowStreamOffset = false;
}

BufferManagerD3D9::~BufferManagerD3D9()
{
	m_D3DDevice->SetStreamSource(0, nullptr, 0, 0);
	m_D3DDevice->SetIndices(nullptr);
}

void BufferManagerD3D9::RemoveInternalBuffer(HardwareBuffer* buffer, void* handle)
{
	auto idOpt = m_HardwareBuffers.LinearSearch(buffer);
	if(!idOpt.HasValue())
		return;
	int id = idOpt.GetValue();
	m_HardwareBuffers.Erase(id);

	ULONG remaining = 0;
	switch(buffer->GetBufferType()) {
	case EHardwareBufferType::Index:
	{
		IDirect3DIndexBuffer9* d3dBuffer = (IDirect3DIndexBuffer9*)handle;
		if(d3dBuffer)
			remaining = d3dBuffer->Release();
		break;
	}
	case EHardwareBufferType::Vertex:
	{
		IDirect3DVertexBuffer9* d3dBuffer = (IDirect3DVertexBuffer9*)handle;
		if(d3dBuffer)
			remaining = d3dBuffer->Release();
		break;
	}
	default:
		throw core::GenericRuntimeException("Unsupported hardwarebuffer type.");
	}
	lxAssert(remaining == 0);
}

void* BufferManagerD3D9::UpdateVertexBuffer(VertexBuffer* buffer, void* handle)
{
	IDirect3DVertexBuffer9* d3dBuffer = reinterpret_cast<IDirect3DVertexBuffer9*>(handle);

	const EHardwareBufferMapping HWMapping = buffer->GetHWMapping();
	int beginDirty;
	int endDirty;
	buffer->GetDirty(beginDirty, endDirty);

	UINT size = (UINT)buffer->GetSize();
	UINT stride = (UINT)buffer->GetStride();
	DWORD mapping = (HWMapping == EHardwareBufferMapping::Dynamic ? D3DUSAGE_DYNAMIC : 0);
	mapping |= D3DUSAGE_WRITEONLY;

	UINT oldSize = 0;
	UINT oldMapping = 0xFFFFFFFF;
	if(d3dBuffer) {
		D3DVERTEXBUFFER_DESC desc;
		d3dBuffer->GetDesc(&desc);
		oldSize = desc.Size;
		oldMapping = desc.Usage;
	}

	HRESULT hr;
	if(size*stride > oldSize ||
		mapping != oldMapping) {
		IDirect3DVertexBuffer9* newD3DBuffer;

		if(FAILED(hr = m_D3DDevice->CreateVertexBuffer(size*stride,
			mapping,
			0,
			D3DPOOL_DEFAULT,
			&newD3DBuffer,
			nullptr))) {
			throw core::D3D9Exception(hr);
		}

		// Rewrite whole buffer
		beginDirty = 0;
		endDirty = size - 1;

		if(d3dBuffer)
			d3dBuffer->Release();
		d3dBuffer = newD3DBuffer;
	}

	void* data;
	if(FAILED(hr = d3dBuffer->Lock(beginDirty * stride,
		(endDirty - beginDirty + 1) * stride,
		&data,
		0))) {
		throw core::D3D9Exception(hr);
	}

	// Use const version, to not update the dirty region
	const void* target = buffer->Pointer_c(beginDirty, endDirty - beginDirty + 1);
	memcpy(data, target, (endDirty - beginDirty + 1)*stride);

	if(FAILED(hr = d3dBuffer->Unlock()))
		throw core::D3D9Exception(hr);

	return d3dBuffer;
}

void* BufferManagerD3D9::UpdateIndexBuffer(IndexBuffer* buffer, void* handle)
{
	IDirect3DIndexBuffer9* d3dBuffer = reinterpret_cast<IDirect3DIndexBuffer9*>(handle);

	const EHardwareBufferMapping HWMapping = buffer->GetHWMapping();
	int beginDirty;
	int endDirty;
	buffer->GetDirty(beginDirty, endDirty);

	auto size = (UINT)buffer->GetSize();
	auto stride = (UINT)buffer->GetStride();
	D3DFORMAT format = buffer->GetFormat() == EIndexFormat::Bit16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
	DWORD mapping = (HWMapping == EHardwareBufferMapping::Dynamic ? D3DUSAGE_DYNAMIC : 0);
	mapping |= D3DUSAGE_WRITEONLY;

	UINT oldSize = 0;
	D3DFORMAT oldFormat = D3DFMT_UNKNOWN;
	UINT oldMapping = 0xFFFFFFFF;
	if(d3dBuffer) {
		D3DINDEXBUFFER_DESC desc;
		d3dBuffer->GetDesc(&desc);
		oldSize = desc.Size;
		oldFormat = desc.Format;
		oldMapping = desc.Usage;
	}

	HRESULT hr;
	if(size*stride > oldSize ||
		format != oldFormat ||
		mapping != oldMapping) {
		IDirect3DIndexBuffer9* newD3DBuffer;
		if(FAILED(hr = m_D3DDevice->CreateIndexBuffer(size*stride,
			mapping,
			format,
			D3DPOOL_DEFAULT,
			&newD3DBuffer,
			nullptr))) {
			throw core::D3D9Exception(hr);
		}

		// Rewrite whole buffer
		beginDirty = 0;
		endDirty = size - 1;

		if(d3dBuffer)
			d3dBuffer->Release();
		d3dBuffer = newD3DBuffer;
	}

	void* data;
	if(FAILED(hr = d3dBuffer->Lock(beginDirty * stride,
		(endDirty - beginDirty + 1) * stride,
		&data,
		0))) {
		throw core::D3D9Exception(hr);
	}

	// Use const version, to not update the dirty region
	const void* target = buffer->Pointer_c(beginDirty, endDirty - beginDirty + 1);
	memcpy(data, target, (endDirty - beginDirty + 1)*stride);

	if(FAILED(hr = d3dBuffer->Unlock()))
		throw core::D3D9Exception(hr);

	return d3dBuffer;
}

void* BufferManagerD3D9::UpdateInternalBuffer(HardwareBuffer* buffer, void* handle)
{
	// Is the buffer a new one
	if(handle == nullptr)
		m_HardwareBuffers.PushBack(buffer);

	switch(buffer->GetBufferType()) {
	case EHardwareBufferType::Index:
		return UpdateIndexBuffer(
			static_cast<IndexBuffer*>(buffer),
			handle);

	case EHardwareBufferType::Vertex:
		return UpdateVertexBuffer(
			static_cast<VertexBuffer*>(buffer),
			handle);

	default:
		throw core::GenericRuntimeException("Unsupported hardwarebuffer type.");
	}
}

void BufferManagerD3D9::EnableHardwareBuffer(const HardwareBuffer* buffer, const void* handle)
{
	switch(buffer->GetBufferType()) {
	case EHardwareBufferType::Index:
	{
		IDirect3DIndexBuffer9* d3dBuffer = (IDirect3DIndexBuffer9*)handle;
		if(handle) {
			m_IStream.data = nullptr;
			m_IStream.offset = 0;
			HRESULT hr = m_D3DDevice->SetIndices(d3dBuffer);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		} else {
			m_IStream.data = buffer->Pointer_c(0, buffer->GetSize());
			m_IStream.offset = 0;
		}
	}
	break;
	case EHardwareBufferType::Vertex:
	{
		IDirect3DVertexBuffer9* d3dBuffer = (IDirect3DVertexBuffer9*)handle;
		if(handle) {
			m_VStream.data = nullptr;
			m_VStream.offset = 0;

			HRESULT hr;
			if(m_AllowStreamOffset) {
				hr = m_D3DDevice->SetStreamSource(0, d3dBuffer, m_VStream.offset, buffer->GetStride());
				m_VStream.offset = 0;
			} else {
				hr = m_D3DDevice->SetStreamSource(0, d3dBuffer, 0, buffer->GetStride());
			}

			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		} else {
			m_VStream.data = buffer->Pointer_c(0, buffer->GetSize());
			m_VStream.offset = 0;
		}
	}
	break;
	}
}

bool BufferManagerD3D9::GetVertexStream(VertexStream& vs) const
{
	vs = m_VStream;
	return true;
}

bool BufferManagerD3D9::GetIndexStream(IndexStream& is) const
{
	is = m_IStream;
	return true;
}

void BufferManagerD3D9::ResetStreams()
{
	m_D3DDevice->SetIndices(nullptr);
	m_D3DDevice->SetStreamSource(0, nullptr, 0, 0);
}

void BufferManagerD3D9::ReleaseHardwareBuffers()
{
	m_D3DDevice->SetStreamSource(0, nullptr, 0, 0);
	m_D3DDevice->SetIndices(nullptr);

	for(auto hb : m_HardwareBuffers) {
		auto unknown = reinterpret_cast<IUnknown*>(hb->GetHandle());
		unknown->Release();
		hb->SetHandle(nullptr);
	}
}

void BufferManagerD3D9::RestoreHardwareBuffers()
{
	auto oldBuffers = m_HardwareBuffers;
	m_HardwareBuffers.Clear();
	for(auto hb : oldBuffers) {
		hb->SetDirty(0, hb->GetSize() - 1);
		hb->Update();
	}
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9