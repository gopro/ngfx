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
#include <vulkan/vulkan.h>

namespace ngfx {
    struct VKBlit {
        struct Region {
            VkOffset3D p0, p1;
        };
        static void blitImage(
            VkCommandBuffer cmdBuffer,
            VkImage srcImage, uint32_t srcLevel, VkImage dstImage, uint32_t dstLevel,
            Region srcRegion, Region dstRegion,
            uint32_t srcBaseLayer = 0, uint32_t srcLayerCount = 1,
            uint32_t dstBaseLayer = 0, uint32_t dstLayerCount = 1,
            VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VkFilter filter = VK_FILTER_LINEAR);
    };
};
