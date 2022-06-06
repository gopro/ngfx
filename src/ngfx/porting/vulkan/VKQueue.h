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
#include "ngfx/graphics/Queue.h"
#include "ngfx/porting/vulkan/VKDevice.h"
#include "ngfx/porting/vulkan/VKUtil.h"

namespace ngfx {
class VKGraphicsContext;
class VKQueue : public Queue {
public:
  void create(VKGraphicsContext *ctx, int queueFamilyIndex, int queueIndex);
  virtual ~VKQueue();
  void present() override;
  void submit(CommandBuffer *commandBuffer) override;
  void submit(CommandBuffer *commandBuffer, VkPipelineStageFlags waitStageMask,
              const std::vector<Semaphore *> &waitSemaphores,
              const std::vector<Semaphore *> &signalSemaphores,
              Fence *waitFence);
  void waitIdle() override;
  VkQueue v = VK_NULL_HANDLE;

private:
  VKGraphicsContext *ctx;
};
VK_CAST(Queue);
} // namespace ngfx