/*
 * Copyright 2022 GoPro Inc.
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
#include "Texture.h"
#include "ImageUtil.h"

namespace ngfx {

/** \class TextureUtil
 * 
 *  This module provides various utility functions for importing and exporting images 
 *  into GPU textures
 */

class TextureUtil {
public:
  /** Load texture from image data */
  static Texture* load(GraphicsContext* ctx, Graphics* graphics,
        const ImageData &imageData, ImageUsageFlags imageUsageFlags = ImageUsageFlags(
            IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
            IMAGE_USAGE_TRANSFER_DST_BIT), TextureType textureType = TEXTURE_TYPE_2D,
      bool genMipmaps = false, uint32_t numSamples = 1, SamplerDesc* samplerDesc = nullptr);
  /** Load texture from an image file (e.g. PNG or JPEG) */
  static Texture* load(GraphicsContext *ctx, Graphics *graphics, 
      const char *filename, ImageUsageFlags imageUsageFlags = ImageUsageFlags(
          IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT | 
          IMAGE_USAGE_TRANSFER_DST_BIT), TextureType textureType = TEXTURE_TYPE_2D,
      bool genMipmaps = false, uint32_t numSamples = 1, SamplerDesc *samplerDesc = nullptr);
  /** Download texture contents into image data object */
  static void download(Texture* texture, ImageData& v);
  /** Store the GPU texture as a JPEG image */
  static void storeJPEG(std::string filename, Texture* texture, int quality = 90);
  /** Store the GPU texture as a PNG image */
  static void storePNG(std::string filename, Texture* texture);
};
} // namespace ngfx
