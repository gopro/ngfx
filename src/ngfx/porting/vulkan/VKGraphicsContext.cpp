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
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
using namespace ngfx;
using namespace std;
#define MAX_DESCRIPTOR_SETS MAX_DESCRIPTORS * 4
#define MAX_DESCRIPTORS 1024

void VKGraphicsContext::create(const char *appName, bool enableDepthStencil,
                               bool debug) {
  this->debug = debug;
  vkInstance.create(appName, "Graphics Abstraction Engine", 0, debug);
  auto instance = vkInstance.v;
  if (debug)
    vkDebugMessenger.create(instance);
  vkPhysicalDevice.create(instance);
  vkDevice.create(&vkPhysicalDevice);
  vkCommandPool.create(vkDevice.v, vkDevice.queueFamilyIndices.graphics);
  vkQueue.create(this, vkDevice.queueFamilyIndices.graphics, 0);
  initDescriptorPool();
  vkDescriptorSetLayoutCache.create(vkDevice.v);
  this->enableDepthStencil = enableDepthStencil;
  depthFormat = PixelFormat(vkPhysicalDevice.depthFormat);
  vkQueryPool.create(vkDevice.v, VK_QUERY_TYPE_TIMESTAMP, 2);
}

VKGraphicsContext::~VKGraphicsContext() {
  VK_TRACE(vkDestroyDescriptorPool(vkDevice.v, vkDescriptorPool, nullptr));
  if (debug)
    vkDebugMessenger.destroy();
}

void VKGraphicsContext::initDescriptorPool() {
  VkResult vkResult;
  descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_DESCRIPTORS},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_DESCRIPTORS},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_DESCRIPTORS},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_DESCRIPTORS}};
  descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                              nullptr,
                              0,
                              MAX_DESCRIPTOR_SETS,
                              uint32_t(descriptorPoolSizes.size()),
                              descriptorPoolSizes.data()};
  V(vkCreateDescriptorPool(vkDevice.v, &descriptorPoolCreateInfo, nullptr,
                           &vkDescriptorPool));
};

RenderPass *VKGraphicsContext::getRenderPass(RenderPassConfig config) {
  for (auto &r : vkRenderPassCache) {
    if (r->config == config)
      return &r->vkRenderPass;
  }
  auto renderPassData = make_unique<VKRenderPassData>();
  initRenderPass(config, renderPassData->vkRenderPass);
  auto result = &renderPassData->vkRenderPass;
  vkRenderPassCache.emplace_back(std::move(renderPassData));
  return result;
}

void VKGraphicsContext::initRenderPass(const RenderPassConfig &config,
                                       VKRenderPass &renderPass) {
  std::vector<VkAttachmentDescription> attachments;
  uint32_t depthAttachmentBaseIndex = 0;
  for (uint32_t j = 0; j < config.numColorAttachments(); j++) {
    auto &colorAttachmentDesc = config.colorAttachmentDescriptions[j];
    VkFormat colorFormat = VkFormat(colorAttachmentDesc.format);
    VkImageLayout initialLayout =
        (colorAttachmentDesc.initialLayout)
            ? VkImageLayout(*colorAttachmentDesc.initialLayout)
            : VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout =
        (colorAttachmentDesc.finalLayout)
            ? VkImageLayout(*colorAttachmentDesc.finalLayout)
            : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments.push_back(
        {0, colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout});
  }
  auto &depthStencilAttachmentDesc = config.depthStencilAttachmentDescription;
  if (depthStencilAttachmentDesc) {
    depthAttachmentBaseIndex = attachments.size();
    VkFormat depthFormat = VkFormat(depthStencilAttachmentDesc->format);
    VkImageLayout initialLayout =
        (depthStencilAttachmentDesc->initialLayout)
            ? VkImageLayout(*depthStencilAttachmentDesc->initialLayout)
            : VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout =
        (depthStencilAttachmentDesc->finalLayout)
            ? VkImageLayout(*depthStencilAttachmentDesc->finalLayout)
            : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments.push_back(
        {0, depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout});
  }

  std::vector<VkAttachmentReference> colorReferences(
      config.numColorAttachments());
  for (uint32_t j = 0; j < colorReferences.size(); j++) {
    colorReferences[j] = {j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  }
  VkAttachmentReference depthReference = {
      depthAttachmentBaseIndex,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  std::vector<VkSubpassDescription> subpasses = {
      {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr,
       uint32_t(colorReferences.size()), colorReferences.data(), nullptr,
       config.depthStencilAttachmentDescription ? &depthReference : nullptr, 0,
       nullptr}};

  // Subpass dependencies for layout transitions
  std::vector<VkSubpassDependency> dependencies = {
      {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
       VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
       VK_DEPENDENCY_BY_REGION_BIT},
      {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
       VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
       VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT}};
  renderPass.create(vkDevice.v, attachments, subpasses, dependencies);
}

#if 0

void VKGraphicsContext::initRenderPassMSAA(const RenderPassConfig &config, VKRenderPass& renderPass) {
    std::vector<VkAttachmentDescription> attachments;
    uint32_t depthAttachmentBaseIndex = 0;
    for (uint32_t j = 0; j<config.numColorAttachments(); j++) {
        auto &colorAttachmentDesc = config.colorAttachmentDescriptions[j];
        VkFormat colorFormat = VkFormat(colorAttachmentDesc.format);
        attachments.push_back({
            0, colorFormat, VkSampleCountFlagBits(config.numSamples),
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });
        attachments.push_back({
            0, colorFormat, VkSampleCountFlagBits(1),
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        });
    }

    if (config.depthStencilAttachmentDescription) {
        depthAttachmentBaseIndex = attachments.size();
        VkFormat depthFormat = VkFormat(config.depthStencilAttachmentDescription->format);
        attachments.push_back({
            0, depthFormat, VkSampleCountFlagBits(config.numSamples),
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        });
        if (config.enableDepthStencilResolve) {
            attachments.push_back({
                0, depthFormat, VkSampleCountFlagBits(1),
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            });
        }
    }

    std::vector<VkAttachmentReference> colorReferences(config.numColorAttachments());
    for (uint32_t j = 0; j<colorReferences.size(); j++) {
        colorReferences[j] = { 2 * j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }
    std::vector<VkAttachmentReference> resolveReferences(config.numColorAttachments());
    for (uint32_t j = 0; j<resolveReferences.size(); j++) {
        resolveReferences[j] = { 2 * j + 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }
    VkAttachmentReference depthReference = { depthAttachmentBaseIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    std::vector<VkSubpassDescription> subpasses = {
        {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, nullptr,
            uint32_t(colorReferences.size()), colorReferences.data(),
            resolveReferences.data(),
            config.depthStencilAttachmentDescription ? &depthReference : nullptr,
            0, nullptr
        }
    };

    // Subpass dependencies for layout transitions
    std::vector<VkSubpassDependency> dependencies = {
        {
            VK_SUBPASS_EXTERNAL, 0,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            0, VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        }
    };
    renderPass.create(vkDevice.v, attachments, subpasses, dependencies);
}

void VKGraphicsContext::initOffscreenRenderPass(const RenderPassConfig &config, VKRenderPass& renderPass) {
    VkFormat colorFormat = VkFormat(defaultOffscreenSurfaceFormat),
             depthFormat = vkPhysicalDevice.depthFormat;
    std::vector<VkAttachmentDescription> attachments;
    uint32_t depthAttachmentBaseIndex = 0;
    for (uint32_t j = 0; j<config.numColorAttachments(); j++) {
        attachments.push_back({
            0, colorFormat, VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });
    }
    if (config.depthStencilAttachmentDescription) {
        depthAttachmentBaseIndex = attachments.size();
        attachments.push_back({
            0, depthFormat, VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        });
    }

    std::vector<VkAttachmentReference> colorReferences(config.numColorAttachments());
    for (uint32_t j = 0; j<colorReferences.size(); j++) {
        colorReferences[j] = { j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }
    VkAttachmentReference depthReference = {
        depthAttachmentBaseIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    std::vector<VkSubpassDescription> subpasses = {
        {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, nullptr,
            uint32_t(colorReferences.size()), colorReferences.data(),
            nullptr,
            config.depthStencilAttachmentDescription ? &depthReference : nullptr,
            0, nullptr
        }
    };

    // Subpass dependencies for layout transitions
    std::vector<VkSubpassDependency> dependencies = {
        {
            VK_SUBPASS_EXTERNAL, 0,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            0, VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        }
    };
    renderPass.create(vkDevice.v, attachments, subpasses, dependencies);
}

void VKGraphicsContext::initOffscreenRenderPassMSAA(const RenderPassConfig &config, VKRenderPass& renderPass) {
    VkFormat colorFormat = VkFormat(defaultOffscreenSurfaceFormat),
             depthFormat = vkPhysicalDevice.depthFormat;
    std::vector<VkAttachmentDescription> attachments;
    uint32_t depthAttachmentBaseIndex = 0;
    for (uint32_t j = 0; j<config.numColorAttachments(); j++) {
        attachments.push_back({
            0, colorFormat, VkSampleCountFlagBits(config.numSamples),
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });
        attachments.push_back({
            0, colorFormat, VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });
    }

    if (config.depthStencilAttachmentDescription) {
        depthAttachmentBaseIndex = attachments.size();
        attachments.push_back({
            0, depthFormat, VkSampleCountFlagBits(config.numSamples),
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        });
        if (config.enableDepthStencilResolve) {
            attachments.push_back({
                0, depthFormat, VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            });
        }
    }

    std::vector<VkAttachmentReference> colorReferences(config.numColorAttachments());
    for (uint32_t j = 0; j<colorReferences.size(); j++) {
        colorReferences[j] = { 2 * j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }
    std::vector<VkAttachmentReference> resolveReferences(config.numColorAttachments());
    for (uint32_t j = 0; j<resolveReferences.size(); j++) {
        resolveReferences[j] = { 2 * j + 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }
    VkAttachmentReference depthReference = { depthAttachmentBaseIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    std::vector<VkSubpassDescription> subpasses = {
        {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, nullptr,
            uint32_t(colorReferences.size()), colorReferences.data(),
            resolveReferences.data(),
            config.depthStencilAttachmentDescription ? &depthReference : nullptr,
            0, nullptr
        }
    };

    // Subpass dependencies for layout transitions
    std::vector<VkSubpassDependency> dependencies = {
        {
            VK_SUBPASS_EXTERNAL, 0,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            0, VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        }
    };
    renderPass.create(vkDevice.v, attachments, subpasses, dependencies);
}

#endif

void VKGraphicsContext::createSwapchainFramebuffers(int w, int h) {
  // Create frame buffers for every swap chain image
  vkSwapchainFramebuffers.resize(vkSwapchain->numImages);
  for (uint32_t i = 0; i < vkSwapchainFramebuffers.size(); i++) {
    std::vector<VKFramebuffer::VKAttachmentDescriptor> attachments;
    if (numSamples != 1)
      attachments.push_back({&vkMultisampleColorImageView,
                             {vkMultisampleColorImage.createInfo.usage}});
    attachments.push_back(
        {&vkSwapchain->imageViews[i], {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT}});
    if (enableDepthStencil) {
      if (numSamples != 1)
        attachments.push_back({&vkMultisampleDepthImageView,
                               {vkMultisampleDepthImage.createInfo.usage}});
      attachments.push_back(
          {&vkDepthStencilImageView, {vkDepthStencilImage.createInfo.usage}});
    }
    vkSwapchainFramebuffers[i].create(vkDevice.v, vkDefaultRenderPass->v,
                                      attachments, w, h);
  }
}

void VKGraphicsContext::initSemaphores(VkDevice device) {
  vkPresentCompleteSemaphore.create(device);
  vkRenderCompleteSemaphore.create(device);
}
void VKGraphicsContext::initFences(VkDevice device) {
  vkWaitFences.resize(numDrawCommandBuffers);
  for (auto &fence : vkWaitFences)
    fence.create(device, VK_FENCE_CREATE_SIGNALED_BIT);
  vkComputeFence.create(device);
}
void VKGraphicsContext::setSurface(Surface *surface) {
  if (surface && !surface->offscreen) {
    offscreen = false;
    vkSwapchain = make_unique<VKSwapchain>(this, vk(surface));
    surfaceFormat = PixelFormat(vkSwapchain->surfaceFormat.format);
    numDrawCommandBuffers = vkSwapchain->numImages;
  } else {
    offscreen = true;
    numDrawCommandBuffers = 1;
  }
  vkDrawCommandBuffers.resize(numDrawCommandBuffers);
  for (auto &cmdBuffer : vkDrawCommandBuffers) {
    cmdBuffer.create(vkDevice.v, vkCommandPool.v);
  }
  vkCopyCommandBuffer.create(vkDevice.v, vkCommandPool.v);
  vkComputeCommandBuffer.create(vkDevice.v, vkCommandPool.v);
  if (surface && numSamples != 1) {
    msColorImageCreateInfo = {};
    msColorImageCreateInfo.format = VkFormat(surfaceFormat);
    msColorImageCreateInfo.extent = {surface->w, surface->h, 1};
    msColorImageCreateInfo.samples = VkSampleCountFlagBits(numSamples);
    msColorImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    vkMultisampleColorImage.create(&vkDevice,
                                   msColorImageCreateInfo); // TODO: use
                                                            // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
                                                            // on mobile GPU
    vkMultisampleColorImageView.create(vkDevice.v, vkMultisampleColorImage.v,
                                       VK_IMAGE_VIEW_TYPE_2D,
                                       VkFormat(surfaceFormat));
  }
  if (surface && enableDepthStencil) {
    vkDepthStencilImage.create(&vkDevice, {surface->w, surface->h, 1},
                               vkPhysicalDevice.depthFormat,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkDepthStencilImageView.create(
        vkDevice.v, vkDepthStencilImage.v, VK_IMAGE_VIEW_TYPE_2D,
        vkPhysicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (numSamples != 1) {
      msDepthImageCreateInfo = {};
      msDepthImageCreateInfo.format = vkPhysicalDevice.depthFormat;
      msDepthImageCreateInfo.extent = {surface->w, surface->h, 1};
      msDepthImageCreateInfo.samples = VkSampleCountFlagBits(numSamples);
      msDepthImageCreateInfo.usage =
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
      vkMultisampleDepthImage.create(&vkDevice,
                                     msDepthImageCreateInfo); // TODO: use
                                                              // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
                                                              // on mobile GPU
      vkMultisampleDepthImageView.create(
          vkDevice.v, vkMultisampleDepthImage.v, VK_IMAGE_VIEW_TYPE_2D,
          vkPhysicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
  }
  std::optional<AttachmentDescription> depthAttachmentDescription;
  if (enableDepthStencil)
    depthAttachmentDescription = { depthFormat, nullopt, nullopt };
  else
    depthAttachmentDescription = nullopt;
  if (surface && !surface->offscreen) {
    RenderPassConfig onscreenRenderPassConfig = {
        {{surfaceFormat, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_PRESENT_SRC}},
        depthAttachmentDescription,
        false,
        numSamples};
    vkDefaultRenderPass =
        (VKRenderPass *)getRenderPass(onscreenRenderPassConfig);
  }
  defaultOffscreenSurfaceFormat = PixelFormat(VK_FORMAT_R8G8B8A8_UNORM);
  RenderPassConfig offscreenRenderPassConfig = {
      { { defaultOffscreenSurfaceFormat, nullopt, nullopt } },
      depthAttachmentDescription,
      false,
      numSamples};
  vkDefaultOffscreenRenderPass =
      (VKRenderPass *)getRenderPass(offscreenRenderPassConfig);
  vkPipelineCache.create(vkDevice.v);
  if (surface && !surface->offscreen)
    createSwapchainFramebuffers(surface->w, surface->h);
  initSemaphores(vkDevice.v);
  initFences(vkDevice.v);
  createBindings();
  pipelineCache = &vkPipelineCache;
}

CommandBuffer *VKGraphicsContext::drawCommandBuffer(int32_t index) {
  if (index == -1)
    index = currentImageIndex;
  return &vkDrawCommandBuffers[index];
}
CommandBuffer *VKGraphicsContext::copyCommandBuffer() {
  return &vkCopyCommandBuffer;
}
CommandBuffer *VKGraphicsContext::computeCommandBuffer() {
  return &vkComputeCommandBuffer;
}

void VKGraphicsContext::createBindings() {
  device = &vkDevice;
  queue = &vkQueue;
  defaultRenderPass =
      offscreen ? vkDefaultOffscreenRenderPass : vkDefaultRenderPass;
  defaultOffscreenRenderPass = vkDefaultOffscreenRenderPass;
  swapchain = vkSwapchain.get();
  frameFences.resize(vkWaitFences.size());
  for (size_t j = 0; j < vkWaitFences.size(); j++)
    frameFences[j] = &vkWaitFences[j];
  computeFence = &vkComputeFence;
  swapchainFramebuffers.resize(vkSwapchainFramebuffers.size());
  for (size_t j = 0; j < vkSwapchainFramebuffers.size(); j++)
    swapchainFramebuffers[j] = &vkSwapchainFramebuffers[j];
  presentCompleteSemaphore = &vkPresentCompleteSemaphore;
  renderCompleteSemaphore = &vkRenderCompleteSemaphore;
}
GraphicsContext *GraphicsContext::create(const char *appName,
                                         bool enableDepthStencil, bool debug) {
  NGFX_LOG("debug: %s", (debug) ? "true" : "false");
  auto vkGraphicsContext = new VKGraphicsContext();
  vkGraphicsContext->create(appName, enableDepthStencil, debug);
  return vkGraphicsContext;
}
