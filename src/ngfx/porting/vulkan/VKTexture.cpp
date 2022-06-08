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
#include "ngfx/porting/vulkan/VKTexture.h"
#include "ngfx/porting/vulkan/VKBlit.h"
#include "ngfx/porting/vulkan/VKBuffer.h"
#include "ngfx/porting/vulkan/VKCommandBuffer.h"
#include "ngfx/porting/vulkan/VKComputePipeline.h"
#include "ngfx/porting/vulkan/VKGraphicsPipeline.h"
#include "ngfx/porting/vulkan/VKQueue.h"
#include "ngfx/graphics/FormatUtil.h"
#include <algorithm>
using namespace ngfx;

void VKTexture::create(VKGraphicsContext *ctx, void *data, uint32_t size,
                       VkExtent3D extent, uint32_t arrayLayers, VkFormat format,
                       VkImageUsageFlags imageUsageFlags,
                       VkImageViewType imageViewType, bool genMipmaps,
                       VKSamplerCreateInfo *pSamplerCreateInfo,
                       uint32_t numSamples) {
  this->ctx = ctx;
  this->w = extent.width;
  this->h = extent.height;
  this->d = extent.depth;
  this->arrayLayers = arrayLayers;
  this->numSamples = numSamples;
  this->vkFormat = format;
  this->imageUsageFlags = imageUsageFlags;
  this->genMipmaps = genMipmaps;
  this->samplerCreateInfo.reset(pSamplerCreateInfo);
  numPlanes = 1; //TODO
  const std::vector<VkFormat> depthFormats = {
      VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT};
  depthTexture = std::find(depthFormats.begin(), depthFormats.end(), format) !=
                 depthFormats.end();
  this->aspectFlags =
      depthTexture ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
  VkImageType imageType;
  if (imageViewType == VK_IMAGE_VIEW_TYPE_1D ||
      imageViewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY)
    imageType = VK_IMAGE_TYPE_1D;
  else if (imageViewType == VK_IMAGE_VIEW_TYPE_2D ||
           imageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
           imageViewType == VK_IMAGE_VIEW_TYPE_CUBE)
    imageType = VK_IMAGE_TYPE_2D;
  else
    imageType = VK_IMAGE_TYPE_3D;
  this->mipLevels =
      genMipmaps ? floor(log2(float(glm::min(extent.width, extent.height)))) + 1
                 : 1;
  vkImage.create(&ctx->vkDevice, extent, format, imageUsageFlags, imageType,
                 mipLevels, arrayLayers, numSamples,
                 (imageViewType == VK_IMAGE_VIEW_TYPE_CUBE)
                     ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
                     : 0);
  vkDefaultImageView = getImageView(imageViewType, mipLevels, arrayLayers);
  auto &copyCommandBuffer = ctx->vkCopyCommandBuffer;
  std::unique_ptr<VKBuffer> stagingBuffer;
  if (data) {
    stagingBuffer.reset(new VKBuffer());
    stagingBuffer->create(ctx, data, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  }
  copyCommandBuffer.begin();
  uploadFn(copyCommandBuffer.v, data, size, stagingBuffer.get());

  if (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT) {
    if (genMipmaps)
      samplerCreateInfo->maxLod = mipLevels;
    if (!sampler)
        initSampler();
  }

  if (imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    vkImage.changeLayout(
        copyCommandBuffer.v, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, aspectFlags);
  } else if (imageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
    vkImage.changeLayout(copyCommandBuffer.v, VK_IMAGE_LAYOUT_GENERAL,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, aspectFlags, 0,
                         mipLevels, 0, this->arrayLayers);
  } else if (imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
    vkImage.changeLayout(
        copyCommandBuffer.v, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        aspectFlags, 0, mipLevels, 0, this->arrayLayers);
  }
  copyCommandBuffer.end();
  vk(ctx->queue)->submit(&copyCommandBuffer, 0, {}, {}, nullptr);
  ctx->queue->waitIdle();
}

VkDescriptorSet VKTexture::getSamplerDescriptorSet(VkCommandBuffer cmdBuffer) {
    if (!samplerDescriptorSet)
        initSamplerDescriptorSet(cmdBuffer);
    return samplerDescriptorSet;
}

VkDescriptorSet VKTexture::getStorageImageDescriptorSet(VkCommandBuffer cmdBuffer) {
    if (!storageImageDescriptorSet)
        initStorageImageDescriptorSet(cmdBuffer);
    return storageImageDescriptorSet;
}


void VKTexture::generateMipmaps(CommandBuffer *commandBuffer) {
  generateMipmapsFn(vk(commandBuffer)->v);
}

void VKTexture::generateMipmapsFn(VkCommandBuffer cmdBuffer) {
  vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_ACCESS_TRANSFER_READ_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, aspectFlags, 0, 1, 0,
                       arrayLayers);

  for (uint32_t j = 1; j < mipLevels; j++) {
    vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, aspectFlags, j, 1, 0,
                         arrayLayers);

    VKBlit::blitImage(
        cmdBuffer, vkImage.v, j - 1, vkImage.v, j,
        {{0, 0, 0},
         {int32_t(glm::max(w >> (j - 1), 1u)),
          int32_t(glm::max(h >> (j - 1), 1u)), 1}},
        {{0, 0, 0},
         {int32_t(glm::max(w >> j, 1u)), int32_t(glm::max(h >> j, 1u)), 1}},
        0, arrayLayers, 0, arrayLayers);

    vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, aspectFlags, j, 1, 0,
                         arrayLayers);
  }
}

void VKTexture::upload(void *data, uint32_t size, uint32_t x, uint32_t y,
                       uint32_t z, int32_t w, int32_t h, int32_t d,
                       int32_t arrayLayers, int32_t numPlanes /* TODO */,
                       int32_t dataPitch /* TODO */) {
  auto &copyCommandBuffer = ctx->vkCopyCommandBuffer;
  std::unique_ptr<VKBuffer> stagingBuffer;
  if (data) {
    stagingBuffer.reset(new VKBuffer());
    stagingBuffer->create(ctx, data, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  }
  copyCommandBuffer.begin();
  uploadFn(copyCommandBuffer.v, data, size, stagingBuffer.get(), x, y, z, w, h,
           d, arrayLayers, numPlanes, dataPitch);
  if (imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    vkImage.changeLayout(
        copyCommandBuffer.v, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, aspectFlags);
  } else if (imageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
    vkImage.changeLayout(copyCommandBuffer.v, VK_IMAGE_LAYOUT_GENERAL,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, aspectFlags, 0,
                         mipLevels, 0, this->arrayLayers);
  } else if (imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
    vkImage.changeLayout(
        copyCommandBuffer.v, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        aspectFlags, 0, mipLevels, 0, this->arrayLayers);
  }
  copyCommandBuffer.end();
  vk(ctx->queue)->submit(&copyCommandBuffer, 0, {}, {}, nullptr);
  ctx->queue->waitIdle();
}

void VKTexture::uploadFn(VkCommandBuffer cmdBuffer, void *data, uint32_t size,
                         VKBuffer *stagingBuffer, uint32_t x, uint32_t y,
                         uint32_t z, int32_t w, int32_t h, int32_t d,
                         int32_t arrayLayers, int32_t numPlanes, int32_t dataPitch) {
  if (data) {
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
    uint32_t bpp = FormatUtil::getBytesPerPixel(format);
    vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, aspectFlags, 0, 1, 0,
                         arrayLayers);
    std::vector<VkBufferImageCopy> bufferCopyRegions = {
        {0,
         dataPitch == -1 ? 0 : uint32_t(dataPitch / bpp),
         0,
         {aspectFlags, 0, 0, uint32_t(arrayLayers)},
         {int32_t(x), int32_t(y), int32_t(z)},
         {uint32_t(w), uint32_t(h), uint32_t(d)}}};
    VK_TRACE(vkCmdCopyBufferToImage(
        cmdBuffer, stagingBuffer->v, vkImage.v, vkImage.imageLayout[0],
        uint32_t(bufferCopyRegions.size()), bufferCopyRegions.data()));
  }
  if (data && mipLevels != 1)
    generateMipmapsFn(cmdBuffer);
}

void VKTexture::download(void *data, uint32_t size, uint32_t x, uint32_t y,
                         uint32_t z, int32_t w, int32_t h, int32_t d,
                         int32_t arrayLayers, int32_t numPlanes /* TODO */) {
  auto &copyCommandBuffer = ctx->vkCopyCommandBuffer;
  std::unique_ptr<VKBuffer> stagingBuffer;
  stagingBuffer.reset(new VKBuffer());
  stagingBuffer->create(ctx, nullptr, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  copyCommandBuffer.begin();
  downloadFn(copyCommandBuffer.v, data, size, stagingBuffer.get(), x, y, z, w,
             h, d, arrayLayers);
  copyCommandBuffer.end();
  vk(ctx->queue)->submit(&copyCommandBuffer, 0, {}, {}, nullptr);
  ctx->queue->waitIdle();
  stagingBuffer->download(data, size);
}

void VKTexture::downloadFn(VkCommandBuffer cmdBuffer, void *data, uint32_t size,
                           VKBuffer *stagingBuffer, uint32_t x, uint32_t y,
                           uint32_t z, int32_t w, int32_t h, int32_t d,
                           int32_t arrayLayers) {
  if (w == -1)
    w = this->w;
  if (h == -1)
    h = this->h;
  if (d == -1)
    d = this->d;
  if (arrayLayers == -1)
    arrayLayers = this->arrayLayers;

  vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_ACCESS_TRANSFER_READ_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT);
  std::vector<VkBufferImageCopy> bufferCopyRegions = {
      {0,
       0,
       0,
       {aspectFlags, 0, 0, 1},
       {int32_t(x), int32_t(y), int32_t(z)},
       {uint32_t(w), uint32_t(h), uint32_t(d)}}};
  VK_TRACE(vkCmdCopyImageToBuffer(
      cmdBuffer, vkImage.v, vkImage.imageLayout[0], stagingBuffer->v,
      uint32_t(bufferCopyRegions.size()), bufferCopyRegions.data()));
}

VKTexture::~VKTexture() {
  if (sampler)
    VK_TRACE(vkDestroySampler(ctx->vkDevice.v, sampler, nullptr));
}

void VKTexture::initSampler() {
  VkResult vkResult;
  V(vkCreateSampler(ctx->vkDevice.v, samplerCreateInfo.get(), nullptr,
                    &sampler));
}

void VKTexture::initSamplerDescriptorSet(VkCommandBuffer cmdBuffer) {
  VkResult vkResult;

  vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_ACCESS_SHADER_READ_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, aspectFlags, 0,
                       mipLevels, 0, this->arrayLayers);

  VkDescriptorPool descriptorPool = ctx->vkDescriptorPool;
  VkDescriptorSetLayout descriptorSetLayout =
      ctx->vkDescriptorSetLayoutCache.get(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  VkDescriptorSetAllocateInfo allocInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, descriptorPool,
      1, &descriptorSetLayout};
  V(vkAllocateDescriptorSets(ctx->vkDevice.v, &allocInfo,
                             &samplerDescriptorSet));
  VkDescriptorImageInfo descriptorImageInfo = {sampler, vkDefaultImageView->v,
                                               vkImage.imageLayout[0]};
  VkWriteDescriptorSet writeDescriptorSet = {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      nullptr,
      samplerDescriptorSet,
      0,
      0,
      1,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      &descriptorImageInfo,
      nullptr,
      nullptr};
  VK_TRACE(vkUpdateDescriptorSets(ctx->vkDevice.v, 1, &writeDescriptorSet, 0,
                                  nullptr));
}

void VKTexture::initStorageImageDescriptorSet(VkCommandBuffer cmdBuffer) {
  VkResult vkResult;
  vkImage.changeLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL,
                       VK_ACCESS_SHADER_READ_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, aspectFlags, 0,
                       mipLevels, 0, this->arrayLayers);
  VkDescriptorPool descriptorPool = ctx->vkDescriptorPool;
  VkDescriptorSetLayout descriptorSetLayout =
      ctx->vkDescriptorSetLayoutCache.get(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  VkDescriptorSetAllocateInfo allocInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, descriptorPool,
      1, &descriptorSetLayout};
  V(vkAllocateDescriptorSets(ctx->vkDevice.v, &allocInfo,
                             &storageImageDescriptorSet));
  VkDescriptorImageInfo descriptorImageInfo = {sampler, vkDefaultImageView->v,
                                               vkImage.imageLayout[0]};
  VkWriteDescriptorSet writeDescriptorSet = {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      nullptr,
      storageImageDescriptorSet,
      0,
      0,
      1,
      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      &descriptorImageInfo,
      nullptr,
      nullptr};
  VK_TRACE(vkUpdateDescriptorSets(ctx->vkDevice.v, 1, &writeDescriptorSet, 0,
                                  nullptr));
}

void VKTexture::changeLayout(CommandBuffer *commandBuffer,
                             ImageLayout imageLayout) {
  if (imageLayout == IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    vkImage.changeLayout(vk(commandBuffer)->v, VkImageLayout(imageLayout),
                         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         aspectFlags, 0, mipLevels, 0, arrayLayers);
  } else if (imageLayout == IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    vkImage.changeLayout(vk(commandBuffer)->v, VkImageLayout(imageLayout),
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, aspectFlags, 0,
                         mipLevels, 0, arrayLayers);
  } else if (imageLayout == IMAGE_LAYOUT_GENERAL) {
    vkImage.changeLayout(vk(commandBuffer)->v, VkImageLayout(imageLayout),
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, aspectFlags, 0,
                         mipLevels, 0, arrayLayers);
  } else if (imageLayout == IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    vkImage.changeLayout(vk(commandBuffer)->v, VkImageLayout(imageLayout),
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                         aspectFlags, 0, mipLevels, 0, arrayLayers);
  }
}

VKImageView *VKTexture::getImageView(VkImageViewType imageViewType,
                                     uint32_t mipLevels, uint32_t arrayLayers,
                                     uint32_t baseMipLevel,
                                     uint32_t baseArrayLayer) {
  VKImageViewCreateInfo imageViewCreateInfo(vkImage.v, imageViewType, vkFormat,
                                            aspectFlags, mipLevels, arrayLayers,
                                            baseMipLevel, baseArrayLayer);
  for (auto &imageView : vkImageViewCache) {
    if (imageView->createInfo == imageViewCreateInfo) {
      return imageView.get();
    }
  }
  auto vkImageView = std::make_unique<VKImageView>();
  vkImageView->create(ctx->vkDevice.v, imageViewCreateInfo);
  auto result = vkImageView.get();
  vkImageViewCache.push_back(std::move(vkImageView));
  return result;
}

Texture *Texture::create(GraphicsContext* graphicsContext, Graphics* graphics, void* data,
    PixelFormat format, uint32_t size, uint32_t w, uint32_t h, uint32_t d,
    uint32_t arrayLayers,
    ImageUsageFlags imageUsageFlags,
    TextureType textureType, bool genMipmaps,
    uint32_t numSamples, SamplerDesc* samplerDesc, int32_t dataPitch) {
  VKTexture *vkTexture = new VKTexture();
  VKSamplerCreateInfo *samplerCreateInfo = nullptr;
  if (samplerDesc && (imageUsageFlags & IMAGE_USAGE_SAMPLED_BIT)) {
    samplerCreateInfo = new VKSamplerCreateInfo;
    samplerCreateInfo->minFilter = VkFilter(samplerDesc->minFilter);
    samplerCreateInfo->magFilter = VkFilter(samplerDesc->magFilter);
    samplerCreateInfo->mipmapMode = (samplerDesc->mipFilter == FILTER_LINEAR)
                                        ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                                        : VK_SAMPLER_MIPMAP_MODE_NEAREST;
  }
  vkTexture->create(vk(graphicsContext), data, size, {w, h, d}, arrayLayers,
                    VkFormat(format), imageUsageFlags,
                    VkImageViewType(textureType), genMipmaps, samplerCreateInfo,
                    numSamples);
  vkTexture->format = format;
  return vkTexture;
}
