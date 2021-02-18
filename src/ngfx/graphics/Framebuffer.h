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
#include "ngfx/graphics/RenderPass.h"
#include "ngfx/graphics/Texture.h"
#include <vector>

namespace ngfx {
class Framebuffer {
public:
  struct Attachment {
    Texture *texture = nullptr;
    uint32_t level = 0, layer = 0;
  };
  static Framebuffer *create(Device *device, RenderPass *renderPass,
                             const std::vector<Attachment> &attachments,
                             uint32_t w, uint32_t h, uint32_t layers = 1);
  virtual ~Framebuffer() {}
  uint32_t w, h, layers, numAttachments;
  std::vector<Attachment> attachments;
};
} // namespace ngfx
