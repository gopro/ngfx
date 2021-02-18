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
#include "ngfx/graphics/GraphicsCore.h"

/** \class CommandBuffer
 * 
 *  This class supports GPU command buffer operations,
 *  including recording draw commands and submitting them to the GPU.
 *  It supports primary and secondary command buffers.
 *  Secondary command buffers can be recorded in parallel, using multiple threads,
 *  and can be added to a primary command buffer.
 */

namespace ngfx {
class GraphicsContext;
class CommandBuffer {
public:
  /** Create the command buffer
   *  @param ctx The graphics context
   *  @param level The command buffer level
   */
  static CommandBuffer *
  create(GraphicsContext *ctx,
         CommandBufferLevel level = COMMAND_BUFFER_LEVEL_PRIMARY);
  /** Destroy the command buffer */
  virtual ~CommandBuffer() {}
  /** Begin recording */
  virtual void begin() = 0;
  /** End recording */
  virtual void end() = 0;
};
}; // namespace ngfx
