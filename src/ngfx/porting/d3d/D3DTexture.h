/*
 * Copyright 2020 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once
#include "ngfx/core/DebugUtil.h"
#include "ngfx/graphics/Texture.h"
#include "ngfx/porting/d3d/D3DBuffer.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DDescriptorHandle.h"
#include "ngfx/porting/d3d/D3DReadbackBuffer.h"
#include "ngfx/porting/d3d/D3DSampler.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
class D3DGraphics;
class D3DGraphicsContext;
class D3DBlitOp;
class D3DTexture : public Texture {
public:
  void create(D3DGraphicsContext *ctx, D3DGraphics *graphics, void *data,
              uint32_t size, uint32_t w, uint32_t h, uint32_t d,
              uint32_t arrayLayers, DXGI_FORMAT format,
              ImageUsageFlags usageFlags, TextureType textureType,
              bool genMipmaps, uint32_t numSamples,
              const D3DSamplerDesc *samplerDesc = nullptr);
  void upload(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
              uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
              int32_t arrayLayers = -1, int32_t numPlanes = -1) override;
   void download(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
                uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
                int32_t arrayLayers = -1) override;
  void changeLayout(CommandBuffer *commandBuffer,
                    ImageLayout imageLayout) override;
  void
  resourceBarrier(D3DCommandList *cmdList, D3D12_RESOURCE_STATES newState,
                  UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
  void generateMipmaps(CommandBuffer *commandBuffer) override;
  ComPtr<ID3D12Resource> v;
  D3DDescriptorHandle getRtvDescriptor(uint32_t level = 0,
                                       uint32_t baseLayer = 0,
                                       uint32_t layerCount = 1);
  D3DSampler* getSampler(D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT);
  D3DDescriptorHandle getSrvDescriptor(uint32_t baseMipLevel,
                                       uint32_t numMipLevels, uint32_t plane = 0);
  D3DDescriptorHandle getUavDescriptor(uint32_t mipLevel);
  struct RtvData {
    D3D12_RENDER_TARGET_VIEW_DESC desc;
    D3DDescriptorHandle handle;
  };
  std::vector<RtvData> rtvDescriptorCache;
  std::vector<std::unique_ptr<D3DSampler>> samplerCache;
  std::vector<D3DDescriptorHandle> defaultSrvDescriptor;
  uint32_t numPlanes = 1;
  D3DDescriptorHandle defaultRtvDescriptor{},
      defaultUavDescriptor{},
      dsvDescriptor{};
  D3DSampler *defaultSampler = nullptr;
  
  struct SrvData {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    D3DDescriptorHandle handle;
    uint32_t plane = 0;
  };
  std::vector<SrvData> srvDescriptorCache;
  struct UavData {
      D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
      D3DDescriptorHandle handle;
  };
  std::vector<UavData> uavDescriptorCache;
  D3D12_RESOURCE_DESC resourceDesc;
  D3D12_RESOURCE_FLAGS resourceFlags;
std::vector<D3D12_RESOURCE_STATES> currentResourceState;
uint32_t numSubresources = 0;
bool isRenderTarget = false;

private:
  void getResourceDesc();
  void createResource();
  void createDepthStencilView();
  void downloadFn(D3DCommandList *cmdList, D3DReadbackBuffer &readbackBuffer,
                  D3D12_BOX &srcRegion,
                  D3D12_PLACED_SUBRESOURCE_FOOTPRINT &dstFootprint);
  void uploadFn(D3DCommandList *cmdList, void *data, uint32_t size,
                D3DBuffer *stagingBuffer, uint32_t x = 0, uint32_t y = 0,
                uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
                int32_t arrayLayers = -1, int32_t numPlanes = -1);
  void generateMipmapsFn(D3DCommandList *cmdList);
  D3DGraphicsContext *ctx = nullptr;
  D3DGraphics *graphics = nullptr;
  ID3D12Device* d3dDevice = nullptr;
  struct GenMipmapData {
      std::vector<D3DBlitOp> ops;
  };
  std::unique_ptr<GenMipmapData> genMipmapData;
};
D3D_CAST(Texture);
} // namespace ngfx
