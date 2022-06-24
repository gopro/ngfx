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
#include "ngfx/graphics/ImageUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <memory>
#include <stb_image.h>
#include <stb_image_write.h>
using namespace ngfx;

ImageData ImageUtil::load(const char* filename) {
    ImageData v;
    v.data = stbi_load(filename, &v.w, &v.h, &v.numChannels, 4);
    v.size = v.w * v.h * 4;
    return v;   
}

void ImageUtil::storePNG(const char* filename, const ImageData& v) {
    stbi_write_png(filename, v.w, v.h, v.numChannels, v.data, v.w * v.numChannels);
}
