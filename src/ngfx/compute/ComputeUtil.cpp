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

#include "ngfx/compute/ComputeUtil.h"
using namespace ngfx;
using namespace glm;

vec4 ComputeUtil::imageLoad(image_t src, ivec2 coord) {
    int x = clamp(coord.x, 0, src.w - 1);
    int y = clamp(coord.y, 0, src.h - 1);
    return vec4(src.data[y * src.w + x]) / 255.0f;
}

void ComputeUtil::imageStore(image_t dst, ivec2 coord, vec4 v) {
    int x = coord.x, y = coord.y;
    u8vec4* dstPtr = &dst.data[y * (dst.w) + x];
    *dstPtr = u8vec4(v * 255.0f);
}

void ComputeUtil::convolve(image_t src, image_t dst, kernel_t kernel) {
    int kw2 = kernel.w / 2, kh2 = kernel.h / 2;
    for (int j = 0; j < dst.h; j++) {
        for (int k = 0; k < dst.w; k++) {
            vec4 s(0.0f);
            for (int i0 = 0; i0 < kernel.h; i0++) {
                for (int i1 = 0; i1 < kernel.w; i1++) {
                    vec4 srcData = imageLoad(src, ivec2(k + i1 - kw2, j + i0 - kh2));
                    s += srcData * kernel.data[i0 * kernel.w + i1];
                }
            }
            imageStore(dst, ivec2(k, j), s);
        }
    }
}

void ComputeUtil::transpose(image_t src, image_t dst) {
    for (int j = 0; j < dst.h; j++) {
        for (int k = 0; k < dst.w; k++) {
            u8vec4* dstPtr = &dst.data[j * (dst.w) + k];
            u8vec4* srcPtr = &src.data[k * (src.w) + j];
            *dstPtr = *srcPtr;
        }
    }
}
