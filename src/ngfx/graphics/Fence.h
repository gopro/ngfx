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
#include "ngfx/graphics/Device.h"
#include "ngfx/graphics/GraphicsCore.h"

/** \class Fence
 * 
 *  This class implements a fence synchronization mechanism.
 *  The CPU waits for the fence to be signaled by the GPU when an operation is completed.
 */

namespace ngfx {
class Fence {
public:
  /** Create the fence object
   *  @param device The GPU device handle
   *  @param flags Additional fence create flags (optional) */
  static Fence *create(Device *device, FenceCreateFlags flags = 0);
  /** Destroy the fence object */
  virtual ~Fence() {}
  /** Wait for the fence to be signaled by the GPU */
  virtual void wait() = 0;
  /** Reset the fence */
  virtual void reset() = 0;
};
} // namespace ngfx
