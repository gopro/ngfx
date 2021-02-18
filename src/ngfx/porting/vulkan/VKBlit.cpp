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
#include "ngfx/porting/vulkan/VKBlit.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
using namespace ngfx;

void VKBlit::blitImage(VkCommandBuffer cmdBuffer, VkImage srcImage,
                       uint32_t srcLevel, VkImage dstImage, uint32_t dstLevel,
                       Region srcRegion, Region dstRegion,
                       uint32_t srcBaseLayer, uint32_t srcLayerCount,
                       uint32_t dstBaseLayer, uint32_t dstLayerCount,
                       VkImageLayout srcImageLayout,
                       VkImageLayout dstImageLayout, VkFilter filter) {
  VkImageBlit b0 = {
      {VK_IMAGE_ASPECT_COLOR_BIT, srcLevel, srcBaseLayer, srcLayerCount},
      {srcRegion.p0, srcRegion.p1},
      {VK_IMAGE_ASPECT_COLOR_BIT, dstLevel, dstBaseLayer, dstLayerCount},
      {dstRegion.p0, dstRegion.p1}};
  VK_TRACE(vkCmdBlitImage(cmdBuffer, srcImage, srcImageLayout, dstImage,
                          dstImageLayout, 1, &b0, filter));
}
