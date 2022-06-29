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

#include "TextureUtil.h"
using namespace ngfx;
using namespace std;

Texture* TextureUtil::load(GraphicsContext* ctx, Graphics* graphics,
    const char* filename, ImageUsageFlags imageUsageFlags, TextureType textureType,
    bool genMipmaps, uint32_t numSamples, SamplerDesc* samplerDesc) {
    ImageData v;
    ImageUtil::load(filename, v);
    return Texture::create(ctx, graphics, v.data, PIXELFORMAT_RGBA8_UNORM,
        v.size, v.w, v.h, 1, 1, imageUsageFlags, textureType, genMipmaps, numSamples, samplerDesc);
}

void TextureUtil::download(Texture* texture, ImageData &v) {
    v.data = malloc(texture->size);
    v.size = texture->size;
    v.w = texture->w;
    v.h = texture->h;
    v.numChannels = 4;
    texture->download(v.data, v.size);
}

void TextureUtil::storeJPEG(std::string filename, Texture* texture, int quality) {
    ImageData v;
    download(texture, v);
    ImageUtil::storeJPEG(filename, v, quality);
}

void TextureUtil::storePNG(std::string filename, Texture* texture) {
    ImageData v;
    download(texture, v);
    ImageUtil::storePNG(filename, v);
}