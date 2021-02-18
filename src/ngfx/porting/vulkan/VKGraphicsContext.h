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
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/graphics/Window.h"
#include "ngfx/porting/vulkan/VKCommandBuffer.h"
#include "ngfx/porting/vulkan/VKCommandPool.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKDescriptorSetLayoutCache.h"
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKFence.h"
#include "ngfx/porting/vulkan/VKFramebuffer.h"
#include "ngfx/porting/vulkan/VKImage.h"
#include "ngfx/porting/vulkan/VKInstance.h"
#include "ngfx/porting/vulkan/VKPhysicalDevice.h"
#include "ngfx/porting/vulkan/VKPipelineCache.h"
#include "ngfx/porting/vulkan/VKQueue.h"
#include "ngfx/porting/vulkan/VKRenderPass.h"
#include "ngfx/porting/vulkan/VKSemaphore.h"
#include "ngfx/porting/vulkan/VKSwapchain.h"
//#define ENABLE_DEPTH_STENCIL

namespace ngfx {
class VKGraphicsContext : public GraphicsContext {
public:
  void create(const char *appName, bool enableDepthStencil, bool debug);
  virtual ~VKGraphicsContext();
  void setSurface(Surface *surface) override;
  CommandBuffer *drawCommandBuffer(int32_t index = -1) override;
  CommandBuffer *copyCommandBuffer() override;
  CommandBuffer *computeCommandBuffer() override;
  void createBindings();
  VKInstance vkInstance;
  VKPhysicalDevice vkPhysicalDevice;
  VKDevice vkDevice;
  VKCommandPool vkCommandPool;
  VKQueue vkQueue;
  std::unique_ptr<VKSwapchain> vkSwapchain;
  std::vector<VKCommandBuffer> vkDrawCommandBuffers;
  VKCommandBuffer vkCopyCommandBuffer, vkComputeCommandBuffer;
  VKImage vkDepthStencilImage, vkMultisampleColorImage, vkMultisampleDepthImage;
  VKImageView vkDepthStencilImageView, vkMultisampleColorImageView,
      vkMultisampleDepthImageView;
  struct VKRenderPassData {
    RenderPassConfig config;
    VKRenderPass vkRenderPass;
  };
  RenderPass *getRenderPass(RenderPassConfig config) override;
  std::vector<std::unique_ptr<VKRenderPassData>> vkRenderPassCache;
  VKRenderPass *vkDefaultRenderPass = nullptr,
               *vkDefaultOffscreenRenderPass = nullptr;
  VKPipelineCache vkPipelineCache;
  std::vector<VKFramebuffer> vkSwapchainFramebuffers;
  std::vector<VKFence> vkWaitFences;
  VKFence vkComputeFence;
  VKSemaphore vkPresentCompleteSemaphore, vkRenderCompleteSemaphore;
  VkDescriptorPool vkDescriptorPool;
  VKDescriptorSetLayoutCache vkDescriptorSetLayoutCache;
  bool offscreen = true;
  uint32_t numSamples = 1;
  std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
  VKImageCreateInfo msColorImageCreateInfo;
  VKImageCreateInfo msDepthImageCreateInfo;

private:
  void initDescriptorPool();
  void initRenderPass(const RenderPassConfig &config, VKRenderPass &renderPass);
  void initRenderPassMSAA(const RenderPassConfig &config,
                          VKRenderPass &renderPass);
  void initOffscreenRenderPass(const RenderPassConfig &config,
                               VKRenderPass &renderPass);
  void initOffscreenRenderPassMSAA(const RenderPassConfig &config,
                                   VKRenderPass &renderPass);
  void createSwapchainFramebuffers(int w, int h);
  void initSemaphores(VkDevice device);
  void initFences(VkDevice device);
};
inline VKGraphicsContext *vk(GraphicsContext *g) {
  return (VKGraphicsContext *)g;
}
} // namespace ngfx
