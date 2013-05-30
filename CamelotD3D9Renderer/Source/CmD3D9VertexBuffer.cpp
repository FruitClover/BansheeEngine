/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "CmD3D9VertexBuffer.h"
#include "CmD3D9Mappings.h"
#include "CmException.h"
#include "CmD3D9HardwareBufferManager.h"
#include "CmD3D9RenderSystem.h"
#include "CmD3D9Device.h"
#include "CmD3D9ResourceManager.h"

namespace CamelotFramework {

	//---------------------------------------------------------------------
    D3D9VertexBuffer::D3D9VertexBuffer(HardwareBufferManager* mgr, UINT32 vertexSize, 
        UINT32 numVertices, GpuBufferUsage usage, 
        bool useSystemMemory)
		: VertexBuffer(mgr, vertexSize, numVertices, usage, useSystemMemory)
    {    }
	//---------------------------------------------------------------------
    D3D9VertexBuffer::~D3D9VertexBuffer()
    {	    }
	//---------------------------------------------------------------------
    void* D3D9VertexBuffer::lockImpl(UINT32 offset, 
        UINT32 length, GpuLockOptions options)
    {		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (options != GBL_READ_ONLY)
		{
			DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

			while (it != mMapDeviceToBufferResources.end())
			{
				BufferResources* bufferResources = it->second;

				bufferResources->mOutOfDate = true;

				if(bufferResources->mLockLength > 0)
				{
					UINT32 highPoint = std::max( offset + length, 
						bufferResources->mLockOffset + bufferResources->mLockLength );
					bufferResources->mLockOffset = std::min( bufferResources->mLockOffset, offset );
					bufferResources->mLockLength = highPoint - bufferResources->mLockOffset;
				}
				else
				{
					if (offset < bufferResources->mLockOffset)
						bufferResources->mLockOffset = offset;
					if (length > bufferResources->mLockLength)
						bufferResources->mLockLength = length;
				}
				
				if (bufferResources->mLockOptions != GBL_WRITE_ONLY_DISCARD)
					bufferResources->mLockOptions = options;					

				++it;
			}
		}

		return mSystemMemoryBuffer + offset;		
    }
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::unlockImpl(void)
    {
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();
		// TODO PORT - Don't know what the next frame number is. Add a method for that
		//UINT32 nextFrameNumber = Root::getSingleton().getNextFrameNumber();
		UINT32 nextFrameNumber = 0;

		while (it != mMapDeviceToBufferResources.end())
		{
			BufferResources* bufferResources = it->second;

			if (bufferResources->mOutOfDate && 
				bufferResources->mBuffer != NULL&&
				nextFrameNumber - bufferResources->mLastUsedFrame <= 1)
				updateBufferResources(mSystemMemoryBuffer, bufferResources);

			++it;
		}			
    }
	//---------------------------------------------------------------------
    void D3D9VertexBuffer::readData(UINT32 offset, UINT32 length, 
        void* pDest)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, GBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::writeData(UINT32 offset, UINT32 length, 
		const void* pSource,
		bool discardWholeBuffer)
	{
		// There is no functional interface in D3D, just do via manual 
		// lock, copy & unlock
		void* pDst = this->lock(offset, length, 
			discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_READ_WRITE);
		memcpy(pDst, pSource, length);
		this->unlock();
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{			
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
			createBuffer(d3d9Device, mBufferDesc.Pool);
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{	
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

		if (it != mMapDeviceToBufferResources.end())	
		{								
			SAFE_RELEASE(it->second->mBuffer);

			if(it->second != nullptr)
				CM_DELETE(it->second, BufferResources, PoolAlloc);

			mMapDeviceToBufferResources.erase(it);
		}	
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
	{	
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (mBufferDesc.Pool == D3DPOOL_DEFAULT)
		{
			DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

			if (it != mMapDeviceToBufferResources.end())
			{				
				SAFE_RELEASE(it->second->mBuffer);	
			}					
		}
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
	{		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
			createBuffer(d3d9Device, mBufferDesc.Pool);
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::createBuffer(IDirect3DDevice9* d3d9Device, D3DPOOL ePool)
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		BufferResources* bufferResources;		
		HRESULT hr;

		DeviceToBufferResourcesIterator it;

		// Find the vertex buffer of this device.
		it = mMapDeviceToBufferResources.find(d3d9Device);
		if (it != mMapDeviceToBufferResources.end())
		{
			bufferResources = it->second;
			SAFE_RELEASE(bufferResources->mBuffer);
		}
		else
		{
			bufferResources = cm_new<BufferResources, PoolAlloc>();			
			mMapDeviceToBufferResources[d3d9Device] = bufferResources;
		}

		bufferResources->mBuffer = NULL;
		bufferResources->mOutOfDate = true;
		bufferResources->mLockOffset = 0;
		bufferResources->mLockLength = getSizeInBytes();
		bufferResources->mLockOptions = GBL_READ_WRITE;

		// TODO PORT - Don't know what the next frame number is. Add a method for that
		//bufferResources->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();
		bufferResources->mLastUsedFrame = 0;
		
		// Create the vertex buffer
		hr = d3d9Device->CreateVertexBuffer(
			static_cast<UINT>(mSizeInBytes), 
			D3D9Mappings::get(mUsage), 
			0, // No FVF here, thank you.
			ePool,
			&bufferResources->mBuffer,
			NULL);

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			CM_EXCEPT(RenderingAPIException, "Cannot restore D3D9 vertex buffer: " + msg);
		}

		hr = bufferResources->mBuffer->GetDesc(&mBufferDesc);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			CM_EXCEPT(RenderingAPIException, "Cannot get D3D9 Vertex buffer desc: " + msg);
		}		
	}
	//---------------------------------------------------------------------
	IDirect3DVertexBuffer9* D3D9VertexBuffer::getD3D9VertexBuffer(void)
	{
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToBufferResourcesIterator it;

		// Find the vertex buffer of this device.
		it = mMapDeviceToBufferResources.find(d3d9Device);

		// Case vertex buffer was not found for the current device -> create it.		
		if (it == mMapDeviceToBufferResources.end() || it->second->mBuffer == NULL)		
		{						
			createBuffer(d3d9Device, mBufferDesc.Pool);
			it = mMapDeviceToBufferResources.find(d3d9Device);			
		}

		if (it->second->mOutOfDate)
			updateBufferResources(mSystemMemoryBuffer, it->second);
		
		// TODO PORT - Don't know what the next frame number is. Add a method for that
		//it->second->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();
		it->second->mLastUsedFrame = 0;

		return it->second->mBuffer;
	}	
	//---------------------------------------------------------------------
	bool D3D9VertexBuffer::updateBufferResources(const char* systemMemoryBuffer,
		BufferResources* bufferResources)
	{		
		assert(bufferResources != NULL);
		assert(bufferResources->mBuffer != NULL);
		assert(bufferResources->mOutOfDate);
				
		void* dstBytes;
		HRESULT hr;
		

		// Lock the buffer.
		hr = bufferResources->mBuffer->Lock(
			static_cast<UINT>(bufferResources->mLockOffset), 
			static_cast<UINT>(bufferResources->mLockLength), 
			&dstBytes,
			D3D9Mappings::get(bufferResources->mLockOptions, mUsage));

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			CM_EXCEPT(RenderingAPIException, "Cannot lock D3D9 vertex buffer: " + msg);
		}

		memcpy(dstBytes, systemMemoryBuffer + bufferResources->mLockOffset, bufferResources->mLockLength);

		// Unlock the buffer.
		hr = bufferResources->mBuffer->Unlock();
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			CM_EXCEPT(RenderingAPIException, "Cannot unlock D3D9 vertex buffer: " + msg);
		}

		bufferResources->mOutOfDate = false;
		bufferResources->mLockOffset = mSizeInBytes;
		bufferResources->mLockLength = 0;
		bufferResources->mLockOptions = GBL_READ_WRITE;

		return true;		
	}
	//--------------------------------------------------------------------
	void D3D9VertexBuffer::initialize_internal()
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		D3DPOOL eResourcePool;

#if CM_D3D_MANAGE_BUFFERS
		eResourcePool = mSystemMemory ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
#else
		eResourcePool = mSystemMemory ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
#endif       		

		// Set the desired memory pool.
		mBufferDesc.Pool = eResourcePool;

		// Allocate the system memory buffer.
		mSystemMemoryBuffer = (char*)cm_alloc<ScratchAlloc>(getSizeInBytes());
		memset(mSystemMemoryBuffer, 0, getSizeInBytes());	

		// Case we have to create this buffer resource on loading.
		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
		{
			for (UINT32 i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
			{
				IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

				createBuffer(d3d9Device, mBufferDesc.Pool);
			}
		}

		VertexBuffer::initialize_internal();
	}
	//---------------------------------------------------------------------
	void D3D9VertexBuffer::destroy_internal()
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

			DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

		while (it != mMapDeviceToBufferResources.end())
		{
			SAFE_RELEASE(it->second->mBuffer);

			if(it->second != nullptr)
				CM_DELETE(it->second, BufferResources, PoolAlloc);

			++it;
		}	
		mMapDeviceToBufferResources.clear();   

		if(mSystemMemoryBuffer != nullptr)
			cm_free<ScratchAlloc>(mSystemMemoryBuffer);

		VertexBuffer::destroy_internal();
	}
}
