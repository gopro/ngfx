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

/** \class Framebuffer
 * 
 *  This class defines the interface for a framebuffer object.
 *  It supports rendering to a destination surface, such as a texture
 *  or a window surface.
 */

namespace ngfx {
class Framebuffer {
public:
  struct Attachment {
    /** The destination texture */
    Texture *texture = nullptr;
    uint32_t level = 0, /**< The destination texture mipmap level */
             layer = 0; /**< The destination texture array layer index */
    /** Get the attachment subresource index */
    inline uint32_t subresourceIndex() {
        return layer * texture->mipLevels + level;
    };
  };
  /** Create a framebuffer object
   *  @param device The graphics device
   *  @param renderPass The renderPass object
   *  @param attachments The output attachments
   *  @param w The destination width
   *  @param h The destination height
   *  @param layers The number of output layers
   */
  static Framebuffer *create(Device *device, RenderPass *renderPass,
                             const std::vector<Attachment> &attachments,
                             uint32_t w, uint32_t h, uint32_t layers = 1);
  /** Destroy the framebuffer */
  virtual ~Framebuffer() {}
  uint32_t w, /**< The output width */
           h, /**< The output height */
           layers, /**< The number of output layers */
           numAttachments; /**< The number of attachments */
  std::vector<Attachment> attachments; /**< The vector of output attachments */
};
} // namespace ngfx
