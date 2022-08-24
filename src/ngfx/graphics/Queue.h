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
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/Fence.h"
#include "ngfx/graphics/GraphicsCore.h"
#include "ngfx/graphics/Swapchain.h"
#include <vector>

namespace ngfx {
    /** \class Queue
    *
    *  This class provides a base class for a queue.
    *  The user records graphics or compute commands to a command buffer,
    *  and these are submitted to the GPU via a queue.
    *  There can be more than one queue and they can have different capabilities
       (e.g. graphics, compute, and transfer)
    */
class Queue {
public:
  virtual ~Queue() {}
  /** Queue the swapchain image for presenting to the display */
  virtual void present() = 0;
  /** Submit the command buffer to the GPU for processing.  
      This is an asynchronous operation */
  virtual void submit(CommandBuffer *commandBuffer) = 0;
  /** Wait for the GPU to finish executing commands.
      The user can also use a fence to be notified when the commands finish 
      executing on the GPU */
  virtual void waitIdle() = 0;
};
} // namespace ngfx