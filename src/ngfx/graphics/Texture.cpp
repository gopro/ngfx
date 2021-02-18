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
#include "ngfx/graphics/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <memory>
using namespace ngfx;

Texture* Texture::create(GraphicsContext* ctx, Graphics* graphics, const char* filename, ImageUsageFlags imageUsageFlags,
        TextureType textureType, bool genMipmaps, FilterMode minFilter, FilterMode magFilter, FilterMode mipFilter, uint32_t numSamples) {
    int w, h, channels;
    std::unique_ptr<stbi_uc> data(stbi_load(filename, &w, &h, &channels, 4));
    assert(data);
    Texture* texture = create(ctx, graphics, data.get(), PIXELFORMAT_RGBA8_UNORM, w * h * 4, w, h, 1, 1, imageUsageFlags,
        textureType, genMipmaps, minFilter, magFilter, mipFilter, numSamples);
    return texture;
}
