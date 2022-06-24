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

namespace ngfx {

class TextureUtil {
public:
  static Texture* load(GraphicsContext *ctx, Graphics *graphics, 
      const char *filename, ImageUsageFlags imageUsageFlags = ImageUsageFlags(
          IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT | 
          IMAGE_USAGE_TRANSFER_DST_BIT), TextureType textureType = TEXTURE_TYPE_2D,
      bool genMipmaps = false, uint32_t numSamples = 1, SamplerDesc *samplerDesc = nullptr);
  static void storeJPEG(const char* filename, Texture* texture, int quality = 90);
  static void storePNG(const char* filename, Texture* texture);
};
} // namespace ngfx
