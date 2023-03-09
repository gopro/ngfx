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
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include <dxgi1_4.h>
#include <wrl.h>
using namespace ngfx;
using namespace std;
using Microsoft::WRL::ComPtr;
#define MAX_DESCRIPTORS 1024

void D3DGraphicsContext::create(const char *appName, bool enableDepthStencil, bool debug) {
  HRESULT hResult;
  this->debug = debug;
  this->enableDepthStencil = enableDepthStencil;

  UINT dxgiFactoryFlags = 0;
#ifdef ENABLE_GPU_CAPTURE_SUPPORT
  enableGPUCapture = getenv("NGFX_GPU_CAPTURE");
  if (enableGPUCapture) {
      gpuCapture.reset(GPUCapture::create());
  }
#endif

  if (debug) {
    ComPtr<ID3D12Debug1> debugController;
    V(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
    if (ENABLE_GPU_VALIDATION)
        debugController->SetEnableGPUBasedValidation(true);

    // Enable additional debug layers.
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
  }
  V(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&d3dFactory)));
  d3dDevice.create(this);
  if (debug) {
      ID3D12InfoQueue* infoQueue = nullptr;
      d3dDevice.v->QueryInterface(&infoQueue);
      infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

      //filter warnings
      D3D12_INFO_QUEUE_FILTER filter{};
      D3D12_MESSAGE_SEVERITY severityList[] = { D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_SEVERITY_CORRUPTION };
      filter.AllowList.NumSeverities = sizeof(severityList) / sizeof(severityList[0]);
      filter.AllowList.pSeverityList = severityList;
      infoQueue->PushStorageFilter(&filter);
  }
  std::vector<PixelFormat> depthStencilFormatCandidates = {
      PixelFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT),
      PixelFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
      PixelFormat(DXGI_FORMAT_D16_UNORM)
  };
  depthStencilFormat = PixelFormat(findSupportedFormat(depthStencilFormatCandidates, D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL));
  depthFormat = depthStencilFormat;
  if (onSelectDepthStencilFormats)
      onSelectDepthStencilFormats(depthStencilFormatCandidates, depthFormat, depthStencilFormat);

  d3dCommandQueue.create(this);
  createDescriptorHeaps();
  d3dCopyCommandList.create(d3dDevice.v.Get());
  d3dComputeCommandList.create(d3dDevice.v.Get());
  d3dQueryTimestampHeap.create(d3dDevice.v.Get(), D3D12_QUERY_HEAP_TYPE_TIMESTAMP, 2);
  d3dTimestampResultBuffer.create(this, 2 * sizeof(uint64_t));
#ifdef ENABLE_GPU_CAPTURE_SUPPORT
  if (enableGPUCapture) {
      gpuCapture->begin();
  }
#endif
}

D3DGraphicsContext::~D3DGraphicsContext() {
#ifdef ENABLE_GPU_CAPTURE_SUPPORT
    if (enableGPUCapture)
        gpuCapture->end();
#endif
}

DXGI_FORMAT D3DGraphicsContext::findSupportedFormat(const std::vector<PixelFormat>& formats, D3D12_FORMAT_SUPPORT1 formatSupport1) {
    HRESULT hResult;
    for (const auto format : formats) {
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT(format);
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { dxgiFormat };
        V(d3dDevice.v->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)));
        if (formatSupport.Support1 & formatSupport1)
            return dxgiFormat;
    }
    return DXGI_FORMAT_UNKNOWN;
}
void D3DGraphicsContext::createDescriptorHeaps() {
  d3dRtvDescriptorHeap.create(d3dDevice.v.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                              MAX_DESCRIPTORS, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
  d3dCbvSrvUavDescriptorHeap.create(
      d3dDevice.v.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
      MAX_DESCRIPTORS * 3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
  d3dSamplerDescriptorHeap.create(
      d3dDevice.v.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, MAX_DESCRIPTORS,
      D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE); // the maximum number of samplers in a shader-visible descriptor heap is only 2048 (see D3D12 Hardware Tiers on MSDN)
  d3dDsvDescriptorHeap.create(d3dDevice.v.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                              MAX_DESCRIPTORS, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
}

void D3DGraphicsContext::setSurface(Surface *surface) {
  defaultOffscreenSurfaceFormat = PIXELFORMAT_RGBA8_UNORM;
  if (surface && !surface->offscreen) {
    offscreen = false;
    d3dSwapchain = make_unique<D3DSwapchain>();
    d3dSwapchain->create(this, d3d(surface));
    surfaceFormat = PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    numDrawCommandBuffers = d3dSwapchain->numImages;
    currentImageIndex = 0;
  } else {
    offscreen = true;
    numDrawCommandBuffers = 1;
    surfaceFormat = defaultOffscreenSurfaceFormat;
    currentImageIndex = -1;
  }
  d3dDrawCommandLists.resize(numDrawCommandBuffers);
  for (auto &cmdList : d3dDrawCommandLists) {
    cmdList.create(d3dDevice.v.Get());
  }
  d3dOffscreenDrawCommandList.create(d3dDevice.v.Get());
  if (surface && numSamples != 1) {
    NGFX_TODO("");
  }
  if (surface && enableDepthStencil) {
    d3dDepthStencilView.reset((D3DTexture *)Texture::create(
        this, nullptr, nullptr, depthStencilFormat, surface->w * surface->h * 4,
        surface->w, surface->h, 1, 1,
        IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
    if (numSamples != 1) {
      NGFX_TODO("");
    }
  }
  std::optional<AttachmentDescription> depthAttachmentDescription;
  if (enableDepthStencil)
    depthAttachmentDescription = { depthStencilFormat, nullopt, nullopt, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_DONT_CARE };
  else
    depthAttachmentDescription = nullopt;
  if (surface && !surface->offscreen) {
    RenderPassConfig onscreenRenderPassConfig = {
        {{surfaceFormat, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_PRESENT_SRC, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE }},
        depthAttachmentDescription,
        false,
        numSamples};
    d3dDefaultRenderPass =
        (D3DRenderPass *)getRenderPass(onscreenRenderPassConfig);
  }
  defaultOffscreenSurfaceFormat = PIXELFORMAT_RGBA8_UNORM;
  RenderPassConfig offscreenRenderPassConfig = {
      {{defaultOffscreenSurfaceFormat, nullopt, nullopt, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE }},
      depthAttachmentDescription,
      false,
      numSamples};
  d3dDefaultOffscreenRenderPass =
      (D3DRenderPass *)getRenderPass(offscreenRenderPassConfig);
  if (surface && !surface->offscreen) {
    createSwapchainFramebuffers(surface->w, surface->h);
  }
  // initSemaphores(vkDevice.v);
  createFences(d3dDevice.v.Get());
  createBindings();
}

RenderPass *D3DGraphicsContext::getRenderPass(RenderPassConfig config) {
  for (auto &r : d3dRenderPassCache) {
    if (r->config == config)
      return &r->d3dRenderPass;
  }
  auto renderPassData = make_unique<D3DRenderPassData>();
  createRenderPass(config, renderPassData->d3dRenderPass);
  auto result = &renderPassData->d3dRenderPass;
  d3dRenderPassCache.emplace_back(std::move(renderPassData));
  return result;
}

void D3DGraphicsContext::createRenderPass(const RenderPassConfig &config,
                                          D3DRenderPass &renderPass) {
  D3D12_RESOURCE_STATES
      initialResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET,
      finalResourceState =
          (config.colorAttachmentDescriptions[0].finalLayout ==
           IMAGE_LAYOUT_PRESENT_SRC)
              ? D3D12_RESOURCE_STATE_PRESENT
              : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
  const AttachmentDescription *colorAttachment = &config.colorAttachmentDescriptions[0];
  const AttachmentDescription* depthAttachment = config.depthStencilAttachmentDescription.has_value() ?
      &config.depthStencilAttachmentDescription.value() :
      nullptr;
  renderPass.create(this, initialResourceState, finalResourceState,
      colorAttachment->loadOp, colorAttachment->storeOp,
      depthAttachment ? depthAttachment->loadOp : ATTACHMENT_LOAD_OP_CLEAR,
      depthAttachment ? depthAttachment->storeOp : ATTACHMENT_STORE_OP_DONT_CARE);
}

void D3DGraphicsContext::createFences(ID3D12Device *device) {
  d3dDrawFences.resize(numDrawCommandBuffers);
  for (auto &fence : d3dDrawFences) {
    fence.create(device, D3DFence::SIGNALED);
  }
  d3dOffscreenFence.create(device, D3DFence::SIGNALED);
  d3dCopyFence.create(device, D3DFence::SIGNALED);
  d3dComputeFence.create(device);
}

void D3DGraphicsContext::createSwapchainFramebuffers(int w, int h) {
  // Create frame buffers for every swap chain image
  d3dSwapchainFramebuffers.resize(d3dSwapchain->numImages);
  for (uint32_t i = 0; i < d3dSwapchainFramebuffers.size(); i++) {
    // TODO: add support for MSAA
    std::vector<D3DFramebuffer::D3DAttachment> attachments(enableDepthStencil ? 2 : 1);
    attachments[0].createFromSwapchainImage(d3dSwapchain.get(), i);
    if (enableDepthStencil) {
        attachments[1].createFromDepthStencilAttachment(d3dDepthStencilView.get());
    }
    d3dSwapchainFramebuffers[i].create(attachments, w, h);
  }
}

CommandBuffer *D3DGraphicsContext::drawCommandBuffer(int32_t index) {
  if (index == -1)
    index = currentImageIndex;
  return index == -1 ? 
    &d3dOffscreenDrawCommandList : 
    &d3dDrawCommandLists[index];
}

CommandBuffer *D3DGraphicsContext::copyCommandBuffer() {
  return &d3dCopyCommandList;
}

CommandBuffer *D3DGraphicsContext::computeCommandBuffer() {
  return &d3dComputeCommandList;
}

void D3DGraphicsContext::createBindings() {
  device = &d3dDevice;
  queue = &d3dCommandQueue;
  defaultRenderPass =
      offscreen ? d3dDefaultOffscreenRenderPass : d3dDefaultRenderPass;
  defaultOffscreenRenderPass = d3dDefaultOffscreenRenderPass;
  swapchain = d3dSwapchain.get();
  frameFences.resize(d3dDrawFences.size());
  for (int j = 0; j < d3dDrawFences.size(); j++)
    frameFences[j] = &d3dDrawFences[j];
  offscreenFence = &d3dOffscreenFence;
  swapchainFramebuffers.resize(d3dSwapchainFramebuffers.size());
  for (int j = 0; j < d3dSwapchainFramebuffers.size(); j++)
    swapchainFramebuffers[j] = &d3dSwapchainFramebuffers[j];
  pipelineCache = &d3dPipelineCache;
}

GraphicsContext *GraphicsContext::create(const char *appName,
                                         bool enableDepthStencil, bool debug,
                                         OnSelectDepthStencilFormats onSelectDepthStencilFormats) {
  NGFX_LOG("debug: %s", (debug) ? "true" : "false");
  auto d3dGraphicsContext = new D3DGraphicsContext();
  d3dGraphicsContext->onSelectDepthStencilFormats = onSelectDepthStencilFormats;
  d3dGraphicsContext->create(appName, enableDepthStencil, debug);
  return d3dGraphicsContext;
}
