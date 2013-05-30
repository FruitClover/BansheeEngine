#include "CmD3D11GpuBuffer.h"
#include "CmD3D11GpuBufferView.h"
#include "CmD3D11RenderSystem.h"
#include "CmD3D11HardwareBuffer.h"
#include "CmD3D11Device.h"
#include "CmException.h"

namespace CamelotFramework
{
	D3D11GpuBuffer::D3D11GpuBuffer(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter) 
		: GpuBuffer(elementCount, elementSize, type, usage, randomGpuWrite, useCounter), mBuffer(nullptr)
	{

	}

	D3D11GpuBuffer::~D3D11GpuBuffer()
	{ }

	void D3D11GpuBuffer::initialize_internal()
	{
		D3D11HardwareBuffer::BufferType bufferType;
		D3D11RenderSystem* d3d11rs = static_cast<D3D11RenderSystem*>(D3D11RenderSystem::instancePtr());

		switch(mType)
		{
		case GBT_STRUCTURED:
			bufferType = D3D11HardwareBuffer::BT_STRUCTURED;
			break;
		case GBT_RAW:
			bufferType = D3D11HardwareBuffer::BT_RAW;
			break;
		case GBT_INDIRECTARGUMENT:
			bufferType = D3D11HardwareBuffer::BT_INDIRECTARGUMENT;
			break;
		case GBT_APPENDCONSUME:
			bufferType = D3D11HardwareBuffer::BT_APPENDCONSUME;
			break;
		default:
			CM_EXCEPT(InvalidParametersException, "Unsupported buffer type " + toString(mType));
		}

		mBuffer = cm_new<D3D11HardwareBuffer, PoolAlloc>(bufferType, mUsage, mElementCount, mElementSize, 
			d3d11rs->getPrimaryDevice(), false, false, mRandomGpuWrite, mUseCounter);

		GpuBuffer::initialize_internal();
	}

	void D3D11GpuBuffer::destroy_internal()
	{
		CM_DELETE(mBuffer, D3D11HardwareBuffer, PoolAlloc);

		GpuBuffer::destroy_internal();
	}

	void* D3D11GpuBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		return mBuffer->lock(offset, length, options);
	}

	void D3D11GpuBuffer::unlock()
	{
		mBuffer->unlock();
	}

	void D3D11GpuBuffer::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		mBuffer->readData(offset, length, pDest);
	}

	void D3D11GpuBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, bool discardWholeBuffer)
	{
		mBuffer->writeData(offset, length, pSource, discardWholeBuffer);
	}

	void D3D11GpuBuffer::copyData(GpuBuffer& srcBuffer, UINT32 srcOffset, 
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{
		D3D11GpuBuffer* d3d11SrcBuffer = static_cast<D3D11GpuBuffer*>(&srcBuffer);

		mBuffer->copyData(*d3d11SrcBuffer->mBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}

	ID3D11Buffer* D3D11GpuBuffer::getDX11Buffer() const 
	{ 
		return mBuffer->getD3DBuffer(); 
	}

	GpuBufferView* D3D11GpuBuffer::createView()
	{
		return cm_new<D3D11GpuBufferView, PoolAlloc>();
	}

	void D3D11GpuBuffer::destroyView(GpuBufferView* view)
	{
		if(view != nullptr)
			cm_delete<PoolAlloc>(view);
	}
}