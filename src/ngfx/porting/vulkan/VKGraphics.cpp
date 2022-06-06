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
#include "ngfx/porting/vulkan/VKGraphics.h"
#include "ngfx/porting/vulkan/VKBuffer.h"
#include "ngfx/porting/vulkan/VKComputePipeline.h"
#include "ngfx/porting/vulkan/VKConfig.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKGraphicsPipeline.h"
#include "ngfx/porting/vulkan/VKRenderPass.h"
#include "ngfx/porting/vulkan/VKTexture.h"
using namespace ngfx;

void VKGraphics::beginRenderPass(CommandBuffer *commandBuffer,
                                 RenderPass *renderPass,
                                 Framebuffer *framebuffer, glm::vec4 clearColor,
                                 float clearDepth, uint32_t clearStencil) {
  currentRenderPass = renderPass;
  currentFramebuffer = framebuffer;
  auto &vkCommandBuffer = vk(commandBuffer)->v;
  auto vkFramebuffer = vk(framebuffer);
  auto &vkAttachmentInfos = vkFramebuffer->vkAttachmentInfos;
  std::vector<VkClearValue> clearValues(vkAttachmentInfos.size());
  for (uint32_t j = 0; j < vkAttachmentInfos.size(); j++) {
    auto &info = vkAttachmentInfos[j];
    bool depthStencilAttachment =
        info.imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (depthStencilAttachment)
      clearValues[j].depthStencil = {clearDepth, clearStencil};
    else
      clearValues[j].color = {
          {clearColor[0], clearColor[1], clearColor[2], clearColor[3]}};
  }
  VkRenderPassBeginInfo renderPassBeginInfo = {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      nullptr,
      vk(renderPass)->v,
      vk(framebuffer)->v,
      {{0, 0}, {vk(framebuffer)->w, vk(framebuffer)->h}},
      uint32_t(clearValues.size()),
      clearValues.data()};
  VK_TRACE(vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo,
                                VK_SUBPASS_CONTENTS_INLINE));

  auto vkRenderPass = vk(renderPass);
  for (uint32_t j = 0; j < framebuffer->attachments.size(); j++) {
    auto &attachment = framebuffer->attachments[j];
    auto vkTexture = (VKTexture *)attachment.texture;
    uint32_t baseIndex =
        attachment.layer * vkTexture->vkImage.createInfo.mipLevels +
        attachment.level;
    vkTexture->vkImage.imageLayout[baseIndex] =
        vkRenderPass->createInfo.pAttachments[j].initialLayout;
  }
}

void VKGraphics::endRenderPass(CommandBuffer *commandBuffer) {
  VK_TRACE(vkCmdEndRenderPass(vk(commandBuffer)->v));

  auto vkRenderPass = vk(currentRenderPass);
  auto framebuffer = currentFramebuffer;
  for (uint32_t j = 0; j < framebuffer->attachments.size(); j++) {
    auto &attachment = framebuffer->attachments[j];
    auto vkTexture = (VKTexture *)attachment.texture;
    uint32_t baseIndex =
        attachment.layer * vkTexture->vkImage.createInfo.mipLevels +
        attachment.level;
    vkTexture->vkImage.imageLayout[baseIndex] =
        vkRenderPass->createInfo.pAttachments[j].finalLayout;
  }

  currentRenderPass = nullptr;
}

void VKGraphics::beginProfile(CommandBuffer *commandBuffer) {
    auto *vkCtx = vk(ctx);
    auto *vkCommandBuffer = vk(commandBuffer)->v;
    vkCmdResetQueryPool(vkCommandBuffer, vkCtx->vkQueryPool.v, 0, 2);
    vkCmdWriteTimestamp(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vkCtx->vkQueryPool.v, 0);
}

uint64_t VKGraphics::endProfile(CommandBuffer *commandBuffer) {
    auto *vkCtx = vk(ctx);
    auto *vkCommandBuffer = vk(commandBuffer)->v;
    vkCmdWriteTimestamp(vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vkCtx->vkQueryPool.v, 1);
    uint64_t t[2];
    vkGetQueryPoolResults(vkCtx->vkDevice.v, vkCtx->vkQueryPool.v, 0, 2, sizeof(t), t,
                          sizeof(t[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    return t[1] - t[0];
}

void VKGraphics::bindComputePipeline(CommandBuffer *commandBuffer,
                                     ComputePipeline *computePipeline) {
  VK_TRACE(vkCmdBindPipeline(vk(commandBuffer)->v,
                             VK_PIPELINE_BIND_POINT_COMPUTE,
                             vk(computePipeline)->v));
  currentPipeline = computePipeline;
}

void VKGraphics::bindGraphicsPipeline(CommandBuffer *commandBuffer,
                                      GraphicsPipeline *graphicsPipeline) {
  VK_TRACE(vkCmdBindPipeline(vk(commandBuffer)->v,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             vk(graphicsPipeline)->v));
  currentPipeline = graphicsPipeline;
}

void VKGraphics::bindTexture(CommandBuffer *commandBuffer, Texture *texture,
                             uint32_t set) {
  auto vkTexture = vk(texture);
  VkPipelineLayout pipelineLayout;
  VkPipelineBindPoint pipelineBindPoint;
  VkDescriptorSet *descriptorSet = nullptr;
  if (VKGraphicsPipeline *graphicsPipeline =
          dynamic_cast<VKGraphicsPipeline *>(currentPipeline)) {
    if (!(vkTexture->imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)) {
      NGFX_ERR("incorrect image usage flags: missing IMAGE_USAGE_SAMPLED_BIT");
    }
    pipelineLayout = graphicsPipeline->pipelineLayout;
    pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    descriptorSet = &vkTexture->samplerDescriptorSet;
  } else if (VKComputePipeline *computePipeline =
                 dynamic_cast<VKComputePipeline *>(currentPipeline)) {
    if (!(vkTexture->imageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)) {
      NGFX_ERR("incorrect image usage flags: missing IMAGE_USAGE_STORAGE_BIT");
    }
    pipelineLayout = computePipeline->pipelineLayout;
    pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    descriptorSet = &vkTexture->storageImageDescriptorSet;
  } else
    NGFX_ERR();
  VK_TRACE(vkCmdBindDescriptorSets(vk(commandBuffer)->v, pipelineBindPoint,
                                   pipelineLayout, set, 1, descriptorSet, 0,
                                   nullptr));
}

void VKGraphics::bindVertexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                  uint32_t location, uint32_t stride) {
  VkDeviceSize offsets[] = {0};
  VK_TRACE(vkCmdBindVertexBuffers(vk(commandBuffer)->v, location, 1,
                                  &vk(buffer)->v, offsets));
}
void VKGraphics::bindIndexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                 IndexFormat indexFormat) {
  VkDeviceSize offset = 0;
  VK_TRACE(vkCmdBindIndexBuffer(vk(commandBuffer)->v, vk(buffer)->v, offset,
                                VkIndexType(indexFormat)));
}
static void bindBufferFN0(CommandBuffer *commandBuffer, Buffer *buffer,
                          uint32_t set, Pipeline *currentPipeline,
                          const VkDescriptorSet *descriptorSet) {
  VkPipelineLayout pipelineLayout;
  VkPipelineBindPoint pipelineBindPoint;
  if (VKGraphicsPipeline *graphicsPipeline =
          dynamic_cast<VKGraphicsPipeline *>(currentPipeline)) {
    pipelineLayout = graphicsPipeline->pipelineLayout;
    pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  } else if (VKComputePipeline *computePipeline =
                 dynamic_cast<VKComputePipeline *>(currentPipeline)) {
    pipelineLayout = computePipeline->pipelineLayout;
    pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
  } else
    NGFX_ERR();
  VK_TRACE(vkCmdBindDescriptorSets(vk(commandBuffer)->v, pipelineBindPoint,
                                   pipelineLayout, set, 1, descriptorSet, 0,
                                   nullptr));
}

void VKGraphics::bindUniformBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                   uint32_t set,
                                   ShaderStageFlags shaderStageFlags) {
  bindBufferFN0(commandBuffer, buffer, set, currentPipeline,
                &vk(buffer)->getUboDescriptorSet(shaderStageFlags));
}

void VKGraphics::bindStorageBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                   uint32_t set,
                                   ShaderStageFlags shaderStageFlags, bool) {
  bindBufferFN0(commandBuffer, buffer, set, currentPipeline,
                &vk(buffer)->getSsboDescriptorSet(shaderStageFlags));
}

void VKGraphics::dispatch(CommandBuffer *commandBuffer, uint32_t groupCountX,
                          uint32_t groupCountY, uint32_t groupCountZ,
                          int32_t threadsPerGroupX, int32_t threadsPerGroupY,
                          int32_t threadsPerGroupZ) {
  VK_TRACE(vkCmdDispatch(vk(commandBuffer)->v, groupCountX, groupCountY,
                         groupCountZ));
}

void VKGraphics::draw(CommandBuffer *commandBuffer, uint32_t vertexCount,
                      uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) {
  VK_TRACE(vkCmdDraw(vk(commandBuffer)->v, vertexCount, instanceCount,
                     firstVertex, firstInstance));
}
void VKGraphics::drawIndexed(CommandBuffer *cmdBuffer, uint32_t indexCount,
                             uint32_t instanceCount, uint32_t firstIndex,
                             int32_t vertexOffset, uint32_t firstInstance) {
  VK_TRACE(vkCmdDrawIndexed(vk(cmdBuffer)->v, indexCount, instanceCount,
                            firstIndex, vertexOffset, firstInstance));
}

void VKGraphics::setViewport(CommandBuffer *commandBuffer, Rect2D r) {
  viewport = r;
  VkViewport vkViewport = {float(r.x), float(r.y), float(r.w),
                           float(r.h), 0.0f,       1.0f};
  VK_TRACE(vkCmdSetViewport(vk(commandBuffer)->v, 0, 1, &vkViewport));
}
void VKGraphics::setScissor(CommandBuffer *commandBuffer, Rect2D r) {
  scissorRect = r;
#ifdef ORIGIN_BOTTOM_LEFT
  auto &v = viewport;
  VkRect2D vkScissorRect = {{r.x, int32_t(v.h) - r.y - int32_t(r.h)},
                            {r.w, r.h}};
#else
  VkRect2D vkScissorRect = {{r.x, r.y}, {r.w, r.h}};
#endif
  VK_TRACE(vkCmdSetScissor(vk(commandBuffer)->v, 0, 1, &vkScissorRect));
}

void VKGraphics::waitIdle(CommandBuffer *cmdBuffer) {
  vk(ctx)->vkDevice.waitIdle();
}

Graphics *Graphics::create(GraphicsContext *ctx) {
  VKGraphics *vkGraphics = new VKGraphics();
  vkGraphics->ctx = ctx;
  vkGraphics->create();
  return vkGraphics;
}
