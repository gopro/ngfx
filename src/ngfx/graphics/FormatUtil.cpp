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
#include "ngfx/graphics/FormatUtil.h"
#include <map>
using namespace ngfx;
using namespace std;

#define FN0(s, t0, t1) \
  { PIXELFORMAT_R##s##_##t0, 1 * (s/8)    }, \
  { PIXELFORMAT_RG##s##_##t0, 2 * (s/8)   }, \
  { PIXELFORMAT_RGBA##s##_##t0, 4 * (s/8) }

int FormatUtil::getBytesPerPixel(PixelFormat format) {
    static const std::map<PixelFormat, int> bppMap = {
        { PIXELFORMAT_UNDEFINED, -1 },
        FN0(8, UNORM, UNORM),
        FN0(16, UINT, UINT),
        FN0(16, SFLOAT, SFLOAT),
        FN0(32, UINT, UINT),
        FN0(32, SFLOAT, SFLOAT),
        { PIXELFORMAT_BGRA8_UNORM, 4 },
        { PIXELFORMAT_D16_UNORM, 2 },
        { PIXELFORMAT_D24_UNORM, 4 },
        { PIXELFORMAT_D24_UNORM_S8_UINT, 4 },
        { PIXELFORMAT_D32_SFLOAT_S8_UINT, 8 }
    };
    return bppMap.at(format);
}