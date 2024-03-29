#include "D3DTexture.h"
#include "D3DBlitOp.h"
#include "D3DBuffer.h"
#include "D3DDebugUtil.h"
#include "D3DFence.h"
#include "D3DGraphicsContext.h"
#include "ngfx/core/StringUtil.h"
#include "ngfx/core/Timer.h"
using namespace ngfx;
using namespace std;

D3DTexture::~D3DTexture() {}

DXGI_FORMAT D3DTexture::getViewFormat(DXGI_FORMAT resourceFormat, uint32_t planeIndex) {
    DXGI_FORMAT format;
    switch (resourceFormat) {
    case DXGI_FORMAT_R16_TYPELESS:
        format = DXGI_FORMAT_R16_UNORM;
        break;
    case DXGI_FORMAT_R24G8_TYPELESS:
        format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        break;
    case DXGI_FORMAT_NV12:
        format = (planeIndex == 0) ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8_UNORM;
        break;
    default:
        format = resourceFormat;
    }
    return format;
}

void D3DTexture::getResourceDesc() {
    resourceFlags = D3D12_RESOURCE_FLAG_NONE;
    if (imageUsageFlags & IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (imageUsageFlags & IMAGE_USAGE_STORAGE_BIT)
        resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    auto d3dDevice = ctx->d3dDevice.v.Get();
    bool isSampled = (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT);
    if (textureType == TEXTURE_TYPE_3D) {
        resourceDesc =
            CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT(format), w, h, d, 1, resourceFlags);
    }
    else {
        DXGI_FORMAT texFormat = DXGI_FORMAT(format);
        if (texFormat == DXGI_FORMAT_D16_UNORM && isSampled)
            texFormat = DXGI_FORMAT_R16_TYPELESS;
        else if (texFormat == DXGI_FORMAT_D24_UNORM_S8_UINT && isSampled)
            texFormat = DXGI_FORMAT_R24G8_TYPELESS;
        else if (texFormat == DXGI_FORMAT_D32_FLOAT && isSampled)
            texFormat = DXGI_FORMAT_R32_TYPELESS;
        else if (texFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT && isSampled)
            texFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        resourceDesc =
            CD3DX12_RESOURCE_DESC::Tex2D(texFormat, w, h, d * arrayLayers,
                mipLevels, numSamples, 0, resourceFlags);
    }
}

void D3DTexture::createResource() {
    HRESULT hResult;
    D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT(format), {0.0f, 0.0f, 0.0f, 0.0f} };
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    V(d3dDevice->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, isRenderTarget ? &clearValue : nullptr,
        IID_PPV_ARGS(&v)));
}

void D3DTexture::createDepthStencilView() {
    auto dsvDescriptorHeap = &ctx->d3dDsvDescriptorHeap;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT(format);
    dsvDesc.ViewDimension = (numSamples > 1) ? D3D12_DSV_DIMENSION_TEXTURE2DMS
        : D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDescriptor = make_unique<D3DDescriptorHandle>();
    dsvDescriptorHeap->getHandle(*dsvDescriptor);
    D3D_TRACE(d3dDevice->CreateDepthStencilView(
        v.Get(), &dsvDesc, dsvDescriptor->cpuHandle));
}

void D3DTexture::create(D3DGraphicsContext* ctx, D3DGraphics* graphics,
    void* data, uint32_t size, uint32_t w, uint32_t h,
    uint32_t d, uint32_t arrayLayers, DXGI_FORMAT format,
    ImageUsageFlags usageFlags, TextureType textureType,
    bool genMipmaps, uint32_t numSamples,
    const D3DSamplerDesc *samplerDesc, int32_t dataPitch) {
    HRESULT hResult;
    this->ctx = ctx;
    this->graphics = graphics;
    this->w = w;
    this->h = h;
    this->d = d;
    this->arrayLayers = arrayLayers;
    this->size = size;
    this->format = PixelFormat(format);
    this->textureType = textureType;
    this->mipLevels =
        genMipmaps ? uint32_t(floor(log2(float(min(w, h))))) + 1 : 1;
    if (genMipmaps)
        usageFlags |= IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    this->imageUsageFlags = usageFlags;
    this->numSamples = numSamples;
    d3dDevice = ctx->d3dDevice.v.Get();
    numPlanes = D3D12GetFormatPlaneCount(d3dDevice, format);
    numSubresources = numPlanes * arrayLayers * mipLevels;
    currentResourceState.resize(numSubresources);
    defaultSrvDescriptor.resize(numPlanes);
    defaultRtvDescriptor.resize(numPlanes);
    defaultUavDescriptor.resize(numPlanes);
    planeWidth.resize(numPlanes);
    planeHeight.resize(numPlanes);
    planeSize.resize(numPlanes);
    for (uint32_t j = 0; j < numPlanes; j++) {
        //in the case of NV12:
        // the first plane is width w , height h, and each pixel is a Y value
        // the second plane is width w / 2, height h / 2, and each pixel is a UV value
        planeWidth[j] = (format == DXGI_FORMAT_NV12 && j == 1) ? w / 2 : w;
        planeHeight[j] = (format == DXGI_FORMAT_NV12 && j == 1) ? h / 2 : h;
        planeSize[j] = (format == DXGI_FORMAT_NV12) ? planeWidth[j] * planeHeight[j] : size;
    }

    getResourceDesc();
    isRenderTarget =
        (resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
    createResource();

    for (auto& s : currentResourceState)
        s = D3D12_RESOURCE_STATE_COPY_DEST;

    bool isSampled = (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT);
    if (isSampled) {
        for (uint32_t j = 0; j < numPlanes; j++) {
            defaultSrvDescriptor[j] = getSrvDescriptor(0, mipLevels, j);
        }
        if (samplerDesc)
            defaultSampler = getSampler(samplerDesc->Filter);
    }
    if (imageUsageFlags & IMAGE_USAGE_STORAGE_BIT) {
        for (uint32_t j = 0; j < numPlanes; j++) {
            defaultUavDescriptor[j] = getUavDescriptor(0, j);
        }
    }

    if (isRenderTarget) {
        for (uint32_t j = 0; j < numPlanes; j++)
            defaultRtvDescriptor[j] = getRtvDescriptor(0, 0, 1, j);
    }

    if (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        createDepthStencilView();
    }

    upload(data, size, 0, 0, 0, -1, -1, -1, -1, -1, dataPitch);
}

void D3DTexture::createFromHandle(D3DGraphicsContext* ctx, D3DGraphics* graphics, void* handle,
    uint32_t w, uint32_t h, uint32_t d,
    uint32_t arrayLayers, DXGI_FORMAT format,
    ImageUsageFlags usageFlags, TextureType textureType,
    uint32_t numSamples,
    const D3DSamplerDesc* samplerDesc) {
    HRESULT hResult;
    this->ctx = ctx;
    this->graphics = graphics;
    this->w = w;
    this->h = h;
    this->d = d;
    this->arrayLayers = arrayLayers;
    this->format = PixelFormat(format);
    this->textureType = textureType;
    this->imageUsageFlags = usageFlags;
    this->numSamples = numSamples;
    numPlanes = D3D12GetFormatPlaneCount(d3dDevice, format);
    d3dDevice = ctx->d3dDevice.v.Get();
    numSubresources = numPlanes * arrayLayers * mipLevels;
    currentResourceState.resize(numSubresources);
    defaultSrvDescriptor.resize(numPlanes);
    planeWidth.resize(numPlanes);
    planeHeight.resize(numPlanes);
    planeSize.resize(numPlanes);
    for (uint32_t j = 0; j < numPlanes; j++) {
        planeWidth[j] = w;
        planeHeight[j] = (format == DXGI_FORMAT_NV12 && j == 1) ? h / 2 : h;
        planeSize[j] = (format == DXGI_FORMAT_NV12) ? planeWidth[j] * planeHeight[j] : size;
    }

    getResourceDesc();
    isRenderTarget =
        (resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    if (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT) {
        if (samplerDesc)
            defaultSampler = getSampler(samplerDesc->Filter);
    }

    updateFromHandle(handle);
}

void D3DTexture::updateFromHandle(void* handle) {
    v = (ID3D12Resource*)handle;

    for (auto& s : currentResourceState)
        s = D3D12_RESOURCE_STATE_COMMON;

    auto commandList = d3d(ctx->drawCommandBuffer(ctx->currentImageIndex));

    resourceBarrierTransition(commandList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    if (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT) {
        for (uint32_t j = 0; j < numPlanes; j++) {
            defaultSrvDescriptor[j] = getSrvDescriptor(0, mipLevels, j);
        }
    }
}

D3DSampler* D3DTexture::getSampler(D3D12_FILTER filter) {
    for (auto& sampler : samplerCache) {
        if (sampler->desc.Filter == filter)
            return sampler.get();
    }
    // Create sampler
    D3DSamplerDesc samplerDesc;
    samplerDesc.Filter = filter;
    auto sampler = make_unique<D3DSampler>();
    sampler->create(ctx, samplerDesc);
    D3DSampler* r = sampler.get();
    samplerCache.emplace_back(std::move(sampler));
    return r;
}

D3DDescriptorHandle* D3DTexture::getSrvDescriptor(uint32_t baseMipLevel,
    uint32_t numMipLevels, uint32_t plane) {
    for (auto& srvData : srvDescriptorCache) {
        if (textureType == TEXTURE_TYPE_2D &&
            srvData.desc.Texture2D.MostDetailedMip == baseMipLevel &&
            srvData.desc.Texture2D.MipLevels == numMipLevels &&
            srvData.plane == plane)
            return srvData.handle;
        else if (textureType == TEXTURE_TYPE_2D_ARRAY &&
            srvData.desc.Texture2DArray.MostDetailedMip == baseMipLevel &&
            srvData.desc.Texture2DArray.MipLevels == numMipLevels &&
            srvData.plane == plane)
            return srvData.handle;
        if (textureType == TEXTURE_TYPE_3D &&
            srvData.desc.Texture3D.MostDetailedMip == baseMipLevel &&
            srvData.desc.Texture3D.MipLevels == numMipLevels &&
            srvData.plane == plane)
            return srvData.handle;
        else if (textureType == TEXTURE_TYPE_CUBE &&
            srvData.desc.TextureCube.MostDetailedMip == baseMipLevel &&
            srvData.desc.TextureCube.MipLevels == numMipLevels &&
            srvData.plane == plane)
            return srvData.handle;
    }
    // Create shader resource view
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = getViewFormat(resourceDesc.Format, plane);
    if (srvDesc.Format == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS && plane == 1)
        return nullptr;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION(textureType);
    if (textureType == TEXTURE_TYPE_2D) {
        srvDesc.Texture2D.MostDetailedMip = baseMipLevel;
        srvDesc.Texture2D.MipLevels = numMipLevels;
        srvDesc.Texture2D.PlaneSlice = plane;
    }
    else if (textureType == TEXTURE_TYPE_2D_ARRAY) {
        srvDesc.Texture2DArray.MostDetailedMip = baseMipLevel;
        srvDesc.Texture2DArray.MipLevels = numMipLevels;
    }
    else if (textureType == TEXTURE_TYPE_3D) {
        srvDesc.Texture3D.MostDetailedMip = baseMipLevel;
        srvDesc.Texture3D.MipLevels = numMipLevels;
    }
    else if (textureType == TEXTURE_TYPE_CUBE) {
        srvDesc.TextureCube.MostDetailedMip = baseMipLevel;
        srvDesc.TextureCube.MipLevels = numMipLevels;
    }
    auto& cbvSrvUavDescriptorHeap = ctx->d3dCbvSrvUavDescriptorHeap;
    auto d3dDevice = ctx->d3dDevice.v.Get();
    auto handle = make_unique<D3DDescriptorHandle>();
    cbvSrvUavDescriptorHeap.getHandle(*handle);
    D3D_TRACE(d3dDevice->CreateShaderResourceView(
        v.Get(), &srvDesc, handle->cpuHandle));
    SrvData srvData;
    srvData.desc = srvDesc;
    srvData.handle = handle.get();
    srvData.plane = plane;
    cbvSrvUavDescriptors.emplace_back(std::move(handle));
    auto result = srvData.handle;
    srvDescriptorCache.emplace_back(std::move(srvData));
    return result;
}

D3DDescriptorHandle* D3DTexture::getUavDescriptor(uint32_t mipLevel, uint32_t plane) {
    for (auto& uavData : uavDescriptorCache) {
        if (textureType == TEXTURE_TYPE_2D &&
            uavData.desc.Texture2D.MipSlice == mipLevel
            && uavData.desc.Texture2D.PlaneSlice == plane)
            return uavData.handle;
    }
    // Create an unordered access view
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = getViewFormat(resourceDesc.Format, plane);
    if (textureType == TEXTURE_TYPE_CUBE) {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.ArraySize = 6;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.PlaneSlice = plane;
    }
    else {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = mipLevel;
        uavDesc.Texture2D.PlaneSlice = plane;
    }
    auto& cbvSrvUavDescriptorHeap = ctx->d3dCbvSrvUavDescriptorHeap;
    auto d3dDevice = ctx->d3dDevice.v.Get();
    auto handle = make_unique<D3DDescriptorHandle>();
    cbvSrvUavDescriptorHeap.getHandle(*handle);
    D3D_TRACE(d3dDevice->CreateUnorderedAccessView(
        v.Get(), nullptr, &uavDesc, handle->cpuHandle));
    UavData uavData;
    uavData.desc = uavDesc;
    uavData.handle = handle.get();
    cbvSrvUavDescriptors.emplace_back(std::move(handle));
    auto result = uavData.handle;
    uavDescriptorCache.emplace_back(std::move(uavData));
    return result;
}

D3D12_RENDER_TARGET_VIEW_DESC
D3DTexture::getRtvDesc(TextureType textureType, DXGI_FORMAT format, uint32_t numSamples,
    uint32_t level, uint32_t baseLayer, uint32_t layerCount, uint32_t plane) {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = getViewFormat(format, plane);
    if (textureType == TEXTURE_TYPE_2D) {
        if (numSamples > 1) {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        }
        else {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = level;
        }
    }
    else if (textureType == TEXTURE_TYPE_2D_ARRAY ||
        textureType == TEXTURE_TYPE_CUBE) {
        if (numSamples > 1) {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
            rtvDesc.Texture2DMSArray.FirstArraySlice = baseLayer;
            rtvDesc.Texture2DMSArray.ArraySize = layerCount;
        }
        else {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = level;
            rtvDesc.Texture2DArray.FirstArraySlice = baseLayer;
            rtvDesc.Texture2DArray.ArraySize = layerCount;
        }
    }
    else if (textureType == TEXTURE_TYPE_3D) {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice = level;
    }
    return rtvDesc;
}
D3DDescriptorHandle* D3DTexture::getRtvDescriptor(uint32_t level,
    uint32_t baseLayer,
    uint32_t layerCount,
    uint32_t planeIndex) {
    for (auto& rtvData : rtvDescriptorCache) {
        if (textureType == TEXTURE_TYPE_2D && numSamples > 1)
            return rtvData.handle;
        else if (textureType == TEXTURE_TYPE_2D &&
            rtvData.desc.Texture2D.MipSlice == level)
            return rtvData.handle;
        else if ((textureType == TEXTURE_TYPE_2D_ARRAY ||
            textureType == TEXTURE_TYPE_CUBE) &&
            numSamples > 1 &&
            rtvData.desc.Texture2DMSArray.FirstArraySlice == baseLayer &&
            rtvData.desc.Texture2DMSArray.ArraySize == layerCount)
            return rtvData.handle;
        else if ((textureType == TEXTURE_TYPE_2D_ARRAY ||
            textureType == TEXTURE_TYPE_CUBE) &&
            rtvData.desc.Texture2DArray.MipSlice == level &&
            rtvData.desc.Texture2DArray.FirstArraySlice == baseLayer &&
            rtvData.desc.Texture2DArray.ArraySize == layerCount)
            return rtvData.handle;
        else if (textureType == TEXTURE_TYPE_3D &&
            rtvData.desc.Texture3D.MipSlice == level)
            return rtvData.handle;
    }
    // Create render target view
    auto rtvDescriptorHeap = &ctx->d3dRtvDescriptorHeap;
    auto d3dDevice = ctx->d3dDevice.v.Get();
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
        getRtvDesc(textureType, DXGI_FORMAT(format), numSamples, level, baseLayer,
            layerCount, planeIndex);
    auto handle = make_unique<D3DDescriptorHandle>();
    rtvDescriptorHeap->getHandle(*handle);
    D3D_TRACE(d3dDevice->CreateRenderTargetView(
        v.Get(), &rtvDesc, handle->cpuHandle));
    RtvData rtvData;
    rtvData.desc = rtvDesc;
    rtvData.handle = handle.get();
    rtvDescriptors.emplace_back(std::move(handle));
    auto result = rtvData.handle;
    rtvDescriptorCache.emplace_back(std::move(rtvData));
    return result;
}

void D3DTexture::generateMipmapsFn(D3DCommandList* cmdList) {
    genMipmapData.reset(new GenMipmapData());
    for (uint32_t j = 1; j < mipLevels; j++) {
        D3DBlitOp op(
            ctx, this, j - 1, this, j,
            { {0, 0, 0},
             {int32_t(glm::max(w >> (j - 1), 1u)),
              int32_t(glm::max(h >> (j - 1), 1u)), 1} },
            { {0, 0, 0},
             {int32_t(glm::max(w >> j, 1u)), int32_t(glm::max(h >> j, 1u)), 1} },
            0, arrayLayers, 0, arrayLayers);
        op.apply(ctx, cmdList, graphics);
        genMipmapData->ops.emplace_back(std::move(op));
    }
}

void D3DTexture::upload(void* data, uint32_t size, uint32_t x, uint32_t y,
    uint32_t z, int32_t w, int32_t h, int32_t d,
    int32_t arrayLayers, int32_t numPlanes, int32_t dataPitch) {
    auto& copyCommandList = ctx->d3dCopyCommandList;
    D3DBuffer stagingBuffer; 
    if (w == -1)
        w = this->w;
    if (h == -1)
        h = this->h;
    if (d == -1)
        d = this->d;
    if (arrayLayers == -1)
        arrayLayers = this->arrayLayers;
    if (numPlanes == -1)
        numPlanes = this->numPlanes;
    copyCommandList.begin();
    if (data) {
        uint64_t stagingBufferSize;
        D3D_TRACE(stagingBufferSize =
            GetRequiredIntermediateSize(v.Get(), 0, arrayLayers * numPlanes));
        stagingBuffer.create(ctx, nullptr, uint32_t(stagingBufferSize),
            D3D12_HEAP_TYPE_UPLOAD);
        stagingBuffer.v->SetName(L"StagingBuffer");
        uploadFn(&copyCommandList, data, size, &stagingBuffer, x, y, z, w, h, d,
            arrayLayers, numPlanes, dataPitch);
    }
    D3D12_RESOURCE_STATES resourceState =
        (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        ? D3D12_RESOURCE_STATE_DEPTH_WRITE
        : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; //TODO: optimize
    resourceBarrierTransition(&copyCommandList, resourceState);
    copyCommandList.end();
    ctx->d3dCommandQueue.submit(copyCommandList.v.Get(), nullptr);
    ctx->d3dCommandQueue.waitIdle();
    if (data && mipLevels != 1) {
        D3DCommandList cmdList;
        ComPtr<ID3D12CommandAllocator> cmdAllocator;
        HRESULT hResult;
        auto d3dDevice = ctx->d3dDevice.v.Get();
        cmdList.create(d3dDevice);
        cmdList.begin();
        generateMipmapsFn(&cmdList);
        cmdList.end();
        ctx->d3dCommandQueue.submit(cmdList.v.Get(), nullptr);
        ctx->d3dCommandQueue.waitIdle();
    }
}

void D3DTexture::generateMipmaps(CommandBuffer* commandBuffer) {
    generateMipmapsFn(d3d(commandBuffer));
}

//TODO: support sub-region update (use CopySubResourceRegion)
void D3DTexture::uploadFn(D3DCommandList* cmdList, void* data, uint32_t,
    D3DBuffer* stagingBuffer, uint32_t, uint32_t,
    uint32_t, int32_t, int32_t, int32_t,
    int32_t, int32_t, int32_t dataPitch) {
    if (data) {
        resourceBarrierTransition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
        vector<D3D12_SUBRESOURCE_DATA> textureData(arrayLayers * numPlanes);
        uint8_t* srcData = (uint8_t*)data;
        uint32_t subresourceIndex = 0;
        for (uint32_t i = 0; i < numPlanes; i++) {
            uint32_t rowPitch = (dataPitch == -1) ? planeSize[i] / (d * arrayLayers * planeHeight[i]) : dataPitch;
            uint32_t slicePitch = rowPitch * planeHeight[i];
            for (uint32_t j = 0; j < arrayLayers; j++) {
                textureData[subresourceIndex++] = { srcData, long(rowPitch), slicePitch };
                srcData += slicePitch;
            }
        }
        uint64_t bufferSize =
            UpdateSubresources(cmdList->v.Get(), v.Get(), stagingBuffer->v.Get(), 0,
                0, arrayLayers * numPlanes, textureData.data());
        assert(bufferSize);
    }
}

void D3DTexture::download(void* data, uint32_t size, uint32_t x, uint32_t y,
    uint32_t z, int32_t w, int32_t h, int32_t d,
    int32_t arrayLayers, int32_t numPlanes) {
    auto& copyCommandList = ctx->d3dCopyCommandList;
    const bool flipY = false; //TODO:move to param
    if (w == -1)
        w = this->w;
    if (h == -1)
        h = this->h;
    if (d == -1)
        d = this->d;
    if (arrayLayers == -1)
        arrayLayers = this->arrayLayers;
    if (numPlanes == -1)
        numPlanes = this->numPlanes;
    D3D12_RESOURCE_DESC desc = v->GetDesc();
    struct SubresourceData {
        D3DReadbackBuffer readbackBuffer;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
        uint32_t numRows = 0;
        uint64_t srcSize = 0;
        uint64_t rowSizeBytes = 0;
        int w = 0, h = 0;
    };
    std::vector<SubresourceData> subresourceData(numPlanes);
    copyCommandList.begin();
    for (int j = 0; j < subresourceData.size(); j++) {
        auto& s = subresourceData[j];
        s.w = (w * planeWidth[j]) / planeWidth[0];
        s.h = (h * planeHeight[j]) / planeHeight[0];
        D3D_TRACE(ctx->d3dDevice.v->GetCopyableFootprints(
            &desc, j, 1, 0, &s.footprint, &s.numRows, &s.rowSizeBytes, &s.srcSize));
        s.readbackBuffer.create(ctx, uint32_t(s.srcSize));
        D3D12_BOX srcRegion = { 0, 0, 0, UINT(s.w), UINT(s.h), 1 };
        downloadFn(&copyCommandList, s.readbackBuffer, srcRegion, s.footprint, j);
    }
    copyCommandList.end();
    ctx->d3dCommandQueue.submit(&copyCommandList);
    ctx->d3dCommandQueue.waitIdle();

    uint8_t* dataPtr = (uint8_t*)data;
    for (int i = 0; i < numPlanes; i++) {
        auto& s = subresourceData[i];
        void* readbackBufferPtr = s.readbackBuffer.map();
        uint8_t* srcPtr = (uint8_t*)readbackBufferPtr,
            * dstPtr =
            &((uint8_t*)dataPtr)[flipY ? (uint64_t(s.h) - 1) * s.rowSizeBytes : 0];
        int64_t dstInc = flipY ? -s.rowSizeBytes : s.rowSizeBytes;
        for (uint32_t j = 0; j < s.h; j++) {
            memcpy(dstPtr, srcPtr, s.rowSizeBytes);
            srcPtr += s.footprint.Footprint.RowPitch;
            dstPtr += dstInc;
        }
        s.readbackBuffer.unmap();
        dataPtr += s.numRows * s.rowSizeBytes;
    }
}

void D3DTexture::downloadFn(D3DCommandList* cmdList,
    D3DReadbackBuffer& readbackBuffer,
    D3D12_BOX& srcRegion,
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& dstFootprint,
    int subresourceIndex) {
    resourceBarrierTransition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE, subresourceIndex);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {
        readbackBuffer.v.Get(),
        D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        {dstFootprint} };
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {
        v.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, UINT64(subresourceIndex) };

    D3D_TRACE(cmdList->v->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation,
        &srcRegion));
    D3D12_RESOURCE_STATES resourceState =
        (imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        ? D3D12_RESOURCE_STATE_DEPTH_WRITE
        : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    resourceBarrierTransition(cmdList, resourceState, subresourceIndex);
    for (auto& s : currentResourceState)
        s = resourceState;
}

void D3DTexture::changeLayout(CommandBuffer *commandBuffer,
    ImageLayout imageLayout) {
    D3D12_RESOURCE_STATES resourceState;
    switch (imageLayout) {
    case IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        break;
    case IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        break;
    case IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        resourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        break;
    case IMAGE_LAYOUT_GENERAL:
        resourceState = D3D12_RESOURCE_STATE_COMMON;
        break;
    case IMAGE_LAYOUT_UNORDERED_ACCESS:
        resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        break;
    default:
        NGFX_ERR("imageLayout: %d", imageLayout);
    };
    resourceBarrierTransition(d3d(commandBuffer), resourceState);
}

template<typename T>
static bool allEqual(const std::vector<T>& v) {
    if (v.empty())
        return false;
    auto v0 = v[0];
    for (int j = 1; j < v.size(); j++)
        if (v[j] != v0)
            return false;
    return true;
}
void D3DTexture::resourceBarrierTransition(D3DCommandList* cmdList,
    D3D12_RESOURCE_STATES newState,
    UINT subresource) {
    uint32_t j0, j1;
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        if (currentResourceState[0] != newState && allEqual(currentResourceState)) {
            auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(v.Get(), currentResourceState[0],
                newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
            D3D_TRACE(cmdList->v->ResourceBarrier(1, &resourceBarrier));
            std::fill(currentResourceState.begin(), currentResourceState.end(), newState);
            return;
        }
        j0 = 0;
        j1 = numSubresources;
    }
    else {
        j0 = subresource;
        j1 = j0 + 1;
    }
    std::vector<CD3DX12_RESOURCE_BARRIER> resourceBarriers;
    for (uint32_t j = j0; j < j1; j++) {
        if (currentResourceState[j] == newState) {
            continue;
        }
        resourceBarriers.push_back(
            CD3DX12_RESOURCE_BARRIER::Transition(v.Get(), currentResourceState[j],
                newState, j)
        );
        currentResourceState[j] = newState;
    }
    if (!resourceBarriers.empty())
        D3D_TRACE(cmdList->v->ResourceBarrier(resourceBarriers.size(), resourceBarriers.data()));
}

void D3DTexture::setName(const std::string& name) {
    v->SetName(StringUtil::toWString(name).c_str());
    this->name = name;
}

Texture* Texture::create(GraphicsContext* ctx, Graphics* graphics, void* data,
    PixelFormat format, uint32_t size, uint32_t w,
    uint32_t h, uint32_t d, uint32_t arrayLayers,
    ImageUsageFlags imageUsageFlags,
    TextureType textureType, bool genMipmaps,
    uint32_t numSamples, SamplerDesc *samplerDesc, int32_t dataPitch) {
    D3DTexture* d3dTexture = new D3DTexture();
    unique_ptr<D3DSamplerDesc> d3dSamplerDesc;
    if (samplerDesc) {
        d3dSamplerDesc.reset(new D3DSamplerDesc(samplerDesc));
    }
    d3dTexture->create(d3d(ctx), (D3DGraphics*)graphics, data, size, w, h, d,
        arrayLayers, DXGI_FORMAT(format), imageUsageFlags,
        textureType, genMipmaps, numSamples, d3dSamplerDesc.get(), dataPitch);
    return d3dTexture;
}

Texture* Texture::createFromHandle(GraphicsContext* ctx, Graphics* graphics, void* handle,
    PixelFormat format, uint32_t w, uint32_t h, uint32_t d, uint32_t arrayLayers,
    ImageUsageFlags imageUsageFlags,
    TextureType textureType,
    uint32_t numSamples, SamplerDesc* samplerDesc) {
    D3DTexture* d3dTexture = new D3DTexture();
    unique_ptr<D3DSamplerDesc> d3dSamplerDesc;
    if (samplerDesc) {
        d3dSamplerDesc.reset(new D3DSamplerDesc(samplerDesc));
    }
    d3dTexture->createFromHandle(d3d(ctx), (D3DGraphics*)graphics, handle, w, h, d,
        arrayLayers, DXGI_FORMAT(format), imageUsageFlags,
        textureType, numSamples, d3dSamplerDesc.get());
    return d3dTexture;
}
