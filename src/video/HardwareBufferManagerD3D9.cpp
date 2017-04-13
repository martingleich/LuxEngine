#include "HardwareBufferManagerD3D9.h"
#include "core/Logger.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "VideoDriverD3D9.h"

namespace lux
{
namespace video
{
// TODO: Handle lost resources
BufferManagerD3D9::BufferManagerD3D9(VideoDriver* driver) : BufferManagerNull(driver),
m_VertexBuffer(nullptr),
m_IndexBuffer(nullptr)
{
	m_D3DDevice = reinterpret_cast<IDirect3DDevice9*>(m_Driver->GetDevice());

	m_UsedStreams = 0;
	m_MaxStreamCount = m_Driver->GetDeviceCapability(EDriverCaps::MaxStreams);
	VideoDriverD3D9* drv = reinterpret_cast<VideoDriverD3D9*>(driver);
	if((drv->GetCaps().Caps2 & D3DDEVCAPS2_STREAMOFFSET) != 0)
		m_AllowStreamOffset = true;
	else
		m_AllowStreamOffset = false;

	m_VStreams.Reserve(m_MaxStreamCount);
}

BufferManagerD3D9::~BufferManagerD3D9()
{
}

void BufferManagerD3D9::RemoveInternalBuffer(HardwareBuffer* buffer, void* handle)
{
	switch(buffer->GetBufferType()) {
	case EHardwareBufferType::Index:
	{
		IDirect3DIndexBuffer9* d3dBuffer = (IDirect3DIndexBuffer9*)handle;
		if(d3dBuffer)
			d3dBuffer->Release();
	}
	break;
	case EHardwareBufferType::Vertex:
	{
		IDirect3DVertexBuffer9* d3dBuffer = (IDirect3DVertexBuffer9*)handle;
		if(d3dBuffer)
			d3dBuffer->Release();
	}
	break;
	default:
		assertNeverReach("Unsupported hardwarebuffer type.");
	}
}

void* BufferManagerD3D9::UpdateVertexBuffer(VertexBuffer* buffer, void* handle)
{
	IDirect3DVertexBuffer9* d3dBuffer = reinterpret_cast<IDirect3DVertexBuffer9*>(handle);

	const EHardwareBufferMapping HWMapping = buffer->GetHWMapping();
	u32 beginDirty;
	u32 endDirty;
	buffer->GetDirty(beginDirty, endDirty);

	u32 size = buffer->GetSize();
	u32 stride = buffer->GetStride();
	u32 mapping = (HWMapping == EHardwareBufferMapping::Dynamic ? D3DUSAGE_DYNAMIC : 0);
	mapping |= D3DUSAGE_WRITEONLY;

	u32 oldSize = 0;
	u32 oldMapping = 0xFFFFFFFF;
	if(d3dBuffer) {
		D3DVERTEXBUFFER_DESC desc;
		d3dBuffer->GetDesc(&desc);
		oldSize = desc.Size;
		oldMapping = desc.Usage;
	}

	if(size*stride > oldSize ||
		mapping != oldMapping) {
		IDirect3DVertexBuffer9* newD3DBuffer;

		if(FAILED(m_D3DDevice->CreateVertexBuffer(size*stride,
			mapping,
			0,
			D3DPOOL_DEFAULT,
			&newD3DBuffer,
			nullptr))) {
			log::Error("Failed to create the vertexbuffer.");
			return nullptr;
		}

		if(d3dBuffer)
			d3dBuffer->Release();
		d3dBuffer = newD3DBuffer;
	}

	void* data;
	if(FAILED(d3dBuffer->Lock(beginDirty * stride,
		(endDirty - beginDirty + 1) * stride,
		&data,
		(HWMapping == EHardwareBufferMapping::Dynamic) ? D3DLOCK_DISCARD : 0))) {
		log::Error("Failed to lock the indexbuffer.");
		return false;
	}

	// Use const version, to not update the dirty region
	const void* target = buffer->Pointer_c(beginDirty, endDirty - beginDirty + 1);
	memcpy(data, target, (endDirty - beginDirty + 1)*stride);

	d3dBuffer->Unlock();

	return d3dBuffer;
}

void* BufferManagerD3D9::UpdateIndexBuffer(IndexBuffer* buffer, void* handle)
{
	IDirect3DIndexBuffer9* d3dBuffer = reinterpret_cast<IDirect3DIndexBuffer9*>(handle);

	const EHardwareBufferMapping HWMapping = buffer->GetHWMapping();
	u32 beginDirty;
	u32 endDirty;
	buffer->GetDirty(beginDirty, endDirty);

	u32 size = buffer->GetSize();
	u32 stride = buffer->GetStride();
	D3DFORMAT format = buffer->GetType() == EIndexFormat::Bit16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
	u32 mapping = (HWMapping == EHardwareBufferMapping::Dynamic ? D3DUSAGE_DYNAMIC : 0);
	mapping |= D3DUSAGE_WRITEONLY;

	u32 oldSize = 0;
	D3DFORMAT oldFormat = D3DFMT_UNKNOWN;
	u32 oldMapping = 0xFFFFFFFF;
	if(d3dBuffer) {
		D3DINDEXBUFFER_DESC desc;
		d3dBuffer->GetDesc(&desc);
		oldSize = desc.Size;
		oldFormat = desc.Format;
		oldMapping = desc.Usage;
	}

	if(size*stride > oldSize ||
		format != oldFormat ||
		mapping != oldMapping) {
		IDirect3DIndexBuffer9* newD3DBuffer;
		if(FAILED(m_D3DDevice->CreateIndexBuffer(size*stride,
			mapping,
			format,
			D3DPOOL_DEFAULT,
			&newD3DBuffer,
			nullptr))) {
			log::Error("Failed to create the indexbuffer.");
			return nullptr;
		}

		// Den alten Puffer freigeben und den neuen Einsetzten
		if(d3dBuffer) d3dBuffer->Release();
		d3dBuffer = newD3DBuffer;
	}

	void* data;
	if(FAILED(d3dBuffer->Lock(beginDirty * stride,
		(endDirty - beginDirty + 1) * stride,
		&data,
		(HWMapping == EHardwareBufferMapping::Dynamic) ? D3DLOCK_DISCARD : 0))) {
		log::Error("Der Indexpuffer konnte nicht gesperrt werden!");
		return nullptr;
	}

	// Use const version, to not update the dirty region
	const void* target = buffer->Pointer_c(beginDirty, endDirty - beginDirty + 1);
	memcpy(data, target, (endDirty - beginDirty + 1)*stride);

	d3dBuffer->Unlock();
	return d3dBuffer;
}

void* BufferManagerD3D9::UpdateInternalBuffer(HardwareBuffer* buffer, void* handle)
{
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
		assertNeverReach("Unsupported hardware buffer type.");
		return nullptr;
	}
}

bool BufferManagerD3D9::EnableHardwareBuffer(u32 streamID, const HardwareBuffer* buffer, const void* handle)
{
	if(streamID > m_MaxStreamCount)
		return false;

	bool out = true;
	switch(buffer->GetBufferType()) {
	case EHardwareBufferType::Index:
	{
		IDirect3DIndexBuffer9* d3dBuffer = (IDirect3DIndexBuffer9*)handle;
		if(handle) {
			m_IStream.data = nullptr;
			m_IStream.offset = 0;
			HRESULT hr = m_D3DDevice->SetIndices(d3dBuffer);
			if(FAILED(hr))
				out = false;
		} else {
			m_IStream.data = buffer->Pointer_c(0, buffer->GetSize());
			m_IStream.offset = 0;
		}
	}
	break;
	case EHardwareBufferType::Vertex:
	{
		IDirect3DVertexBuffer9* d3dBuffer = (IDirect3DVertexBuffer9*)handle;
		VertexStream& vs = m_VStreams[streamID];
		if(handle) {
			vs.data = nullptr;
			vs.offset = 0;

			HRESULT hr;
			if(m_AllowStreamOffset) {
				hr = m_D3DDevice->SetStreamSource(streamID, d3dBuffer, vs.offset, buffer->GetStride());
				vs.offset = 0;
			} else {
				hr = m_D3DDevice->SetStreamSource(streamID, d3dBuffer, 0, buffer->GetStride());
			}

			if(FAILED(hr))
				out = false;
		} else {
			vs.data = buffer->Pointer_c(0, buffer->GetSize());
			vs.offset = 0;
		}

		if(out)
			m_UsedStreams |= (1 << streamID);
	}
	break;
	}

	return out;
}

bool BufferManagerD3D9::GetVertexStream(u32 streamID, VertexStream& vs) const
{
	if((m_UsedStreams & (1 << streamID)) == 0)
		return false;

	vs = m_VStreams[streamID];
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

	int i = 0;
	while(m_UsedStreams & (1 << i)) {
		m_D3DDevice->SetStreamSource(i, nullptr, 0, 0);
		++i;
	}

	m_UsedStreams = 0;
}
}
}