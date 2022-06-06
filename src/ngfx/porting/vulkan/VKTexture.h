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
#include "ngfx/graphics/Texture.h"
#include "ngfx/porting/vulkan/VKBuffer.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKImage.h"
#include "ngfx/porting/vulkan/VKImageView.h"
#include "ngfx/porting/vulkan/VKSamplerCreateInfo.h"

namespace ngfx {
class VKTexture : public Texture {
public:
  void create(VKGraphicsContext *ctx, void *data, uint32_t size,
              VkExtent3D extent, uint32_t arrayLayers, VkFormat format,
              VkImageUsageFlags imageUsageFlags, VkImageViewType imageViewType,
              bool genMipmaps, VKSamplerCreateInfo *pSamplerCreateInfo,
              uint32_t numSamples = 1);
  virtual ~VKTexture();
  void upload(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
              uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
              int32_t arrayLayers = -1, int32_t numPlanes = -1,
              int32_t dataPitch = -1) override;
  void download(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
                uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
                int32_t arrayLayers = -1, int32_t numPlanes = -1) override;
  void updateFromHandle(void* handle) override {
      NGFX_TODO();
  }
  void changeLayout(CommandBuffer *commandBuffer,
                    ImageLayout imageLayout) override;
  void generateMipmaps(CommandBuffer *commandBuffer) override;
  VKImageView *getImageView(VkImageViewType imageViewType, uint32_t mipLevels,
                            uint32_t arrayLayers, uint32_t baseMipLevel = 0,
                            uint32_t baseArrayLayer = 0);
  VkFormat vkFormat;
  VKImage vkImage;
  std::vector<std::unique_ptr<VKImageView>> vkImageViewCache;
  VKImageView *vkDefaultImageView = nullptr;
  VkDescriptorSet samplerDescriptorSet = 0, storageImageDescriptorSet = 0;
  VkSampler sampler = 0;
  VkImageAspectFlags aspectFlags;
  bool depthTexture = false;
  bool genMipmaps = false;
  std::unique_ptr<VKSamplerCreateInfo> samplerCreateInfo;

private:
  void initSamplerDescriptorSet(VkCommandBuffer cmdBuffer);
  void initStorageImageDescriptorSet(VkCommandBuffer cmdBuffer);
  void initSampler();
  void uploadFn(VkCommandBuffer cmdBuffer, void *data, uint32_t size,
                VKBuffer *stagingBuffer, uint32_t x = 0, uint32_t y = 0,
                uint32_t z = 0, int32_t w = -1, int32_t h = -1, int32_t d = -1,
                int32_t arrayLayers = -1);
  void downloadFn(VkCommandBuffer cmdBuffer, void *data, uint32_t size,
                  VKBuffer *stagingBuffer, uint32_t x = 0, uint32_t y = 0,
                  uint32_t z = 0, int32_t w = -1, int32_t h = -1,
                  int32_t d = -1, int32_t arrayLayers = -1);
  void generateMipmapsFn(VkCommandBuffer cmdBuffer);
  VKGraphicsContext *ctx;
};
VK_CAST(Texture);
} // namespace ngfx
