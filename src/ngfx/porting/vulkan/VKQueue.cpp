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
#include "ngfx/porting/vulkan/VKQueue.h"
#include "ngfx/porting/vulkan/VKCommandBuffer.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#include "ngfx/porting/vulkan/VKFence.h"
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKSemaphore.h"
#include "ngfx/porting/vulkan/VKSwapchain.h"
using namespace ngfx;

void VKQueue::create(VKGraphicsContext *ctx, int queueFamilyIndex,
                     int queueIndex) {
  this->ctx = ctx;
  VK_TRACE(vkGetDeviceQueue(ctx->vkDevice.v, queueFamilyIndex, queueIndex, &v));
}
VKQueue::~VKQueue() {}

void VKQueue::present() {
  VkResult vkResult;
  Swapchain *swapChain = ctx->swapchain;
  uint32_t currentImageIndex = ctx->currentImageIndex;
  const std::vector<VkSemaphore> vkWaitSemaphores = {
      ctx->vkRenderCompleteSemaphore.v};
  VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  nullptr,
                                  uint32_t(vkWaitSemaphores.size()),
                                  vkWaitSemaphores.data(),
                                  1,
                                  &vk(swapChain)->v,
                                  &currentImageIndex,
                                  nullptr};
  V(vkQueuePresentKHR(v, &presentInfo));
}

void VKQueue::submit(CommandBuffer *commandBuffer) {
  if (commandBuffer == &ctx->vkComputeCommandBuffer) {
    submit(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
           ctx->computeFence);
  } else if (commandBuffer == &ctx->vkCopyCommandBuffer) {
    submit(commandBuffer, 0, {}, {}, nullptr);
  } else if (ctx->offscreen && commandBuffer == &ctx->vkDrawCommandBuffers[0]) {
    submit(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, {}, {},
           nullptr);
  } else {
    submit(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
           {ctx->presentCompleteSemaphore}, {ctx->renderCompleteSemaphore},
           ctx->frameFences[ctx->currentImageIndex]);
  }
}
void VKQueue::submit(CommandBuffer *commandBuffer,
                     VkPipelineStageFlags waitStageMask,
                     const std::vector<Semaphore *> &waitSemaphores,
                     const std::vector<Semaphore *> &signalSemaphores,
                     Fence *waitFence) {
  VkResult vkResult;
  std::vector<VkSemaphore> vkWaitSemaphores(waitSemaphores.size());
  for (size_t j = 0; j < waitSemaphores.size(); j++)
    vkWaitSemaphores[j] = vk(waitSemaphores[j])->v;
  std::vector<VkSemaphore> vkSignalSemaphores(signalSemaphores.size());
  for (size_t j = 0; j < signalSemaphores.size(); j++)
    vkSignalSemaphores[j] = vk(signalSemaphores[j])->v;
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             nullptr,
                             uint32_t(vkWaitSemaphores.size()),
                             vkWaitSemaphores.data(),
                             &waitStageMask,
                             1,
                             &vk(commandBuffer)->v,
                             uint32_t(vkSignalSemaphores.size()),
                             vkSignalSemaphores.data()};
  V(vkQueueSubmit(v, 1, &submitInfo,
                  waitFence ? vk(waitFence)->v : VK_NULL_HANDLE));
}

void VKQueue::waitIdle() {
  VkResult vkResult;
  V(vkQueueWaitIdle(v));
}
