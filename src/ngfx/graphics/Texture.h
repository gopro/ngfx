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
#include "ngfx/compute/ComputePipeline.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/GraphicsPipeline.h"
#include "ngfx/graphics/SamplerDesc.h"

namespace ngfx {
class Graphics;
class GraphicsContext;

/** \class Texture
 *
 *  This class provides support for GPU textures.
 *  A texture supports random sampling from a GPU shader.
 *  It also supports mipmapping: downscaled copies of the texture 
 *  which are used when the object is further away from the camera 
 *  to conserve memory bandwidth (LOD).  It supports 2D, 3D and 2D array textures
 *  as well as cubemap textures.  It supports single channel, dual channel, and RGBA textures 
 *  as well as video formats such as NV12.  The user can also wrap an existing handle as a texture.
 *  Additionally, texture handles can be shared across multiple graphics contexts, even across different 
 *  graphics API versions.  For example, when using DirectX 12 backend, a DirectX 11 texture can 
 *  be allocated on the shared heap and wrapped as a DirectX 12 texture, and vice versa.
 *  Another typical use case is a video buffer used by the video decoder can be wrapped as a GPU texture.
 */

class Texture {
public:
  /** Create a texture
   *  @param graphicsContext The graphics context
   *  @param graphics The graphics object
   *  @param data The texture data.  The user can pass nullptr to allocate a blank texture
   *  @param format The pixel format
   *  @param size   The texture size
   *  @param w      The texture width
   *  @param h      The texture height
   *  @param d      The texture depth
   *  @param arrayLayers The number of array layers
   *  @param imageUsageFlags A bitmask of flags describing the image usage
   *  @param textureType The texture type
   *  @param genMipmaps If true, generate mipmaps
   *  @param numSamples The number of samples when using multisampling
   *  @param samplerDesc The sampler description, or nullptr if the texture is not sampled
   *  @param dataPitch The pitch (size of each row of the input data in bytes, including padding, or -1 if there's no padding)
   */
  static Texture *
  create(GraphicsContext *graphicsContext, Graphics *graphics, void *data,
         PixelFormat format, uint32_t size, uint32_t w, uint32_t h, uint32_t d,
         uint32_t arrayLayers,
         ImageUsageFlags imageUsageFlags = ImageUsageFlags(
             IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
             IMAGE_USAGE_TRANSFER_DST_BIT),
         TextureType textureType = TEXTURE_TYPE_2D, bool genMipmaps = false,
         uint32_t numSamples = 1, SamplerDesc *samplerDesc = nullptr, int32_t dataPitch = -1);
  
  /** Create texture from existing handle
   *  @param graphicsContext The graphics context
   *  @param graphics The graphics object
   *  @param handle The external handle
   *  @param format The pixel format
   *  @param imageUsageFlags A bitmask of flags describing the image usage
   *  @param textureType The texture type
   *  @param numSamples The number of samples when using multisampling
   *  @param samplerDesc The sampler description, or nullptr if the texture is not sampled
   */
  static Texture*
  createFromHandle(GraphicsContext* graphicsContext, Graphics* graphics, void* handle, 
        PixelFormat format, uint32_t w, uint32_t h, uint32_t d, uint32_t arrayLayers,
        ImageUsageFlags imageUsageFlags = ImageUsageFlags(
            IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
            IMAGE_USAGE_TRANSFER_DST_BIT),
        TextureType textureType = TEXTURE_TYPE_2D,
        uint32_t numSamples = 1, SamplerDesc* samplerDesc = nullptr);
  virtual ~Texture() {}
  /** Upload data to texture */
  virtual void upload(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
                      uint32_t z = 0, int32_t w = -1, int32_t h = -1,
                      int32_t d = -1, int32_t arrayLayers = -1, int32_t numPlanes = -1,
                      int32_t dataPitch = -1) = 0;
  /** Download texture data */
  virtual void download(void *data, uint32_t size, uint32_t x = 0,
                        uint32_t y = 0, uint32_t z = 0, int32_t w = -1,
                        int32_t h = -1, int32_t d = -1,
                        int32_t arrayLayers = -1, int32_t numPlanes = -1) = 0;
  virtual void updateFromHandle(void* handle) = 0;
  /** Change the memory layout of the texture to switch to a different use case */
  virtual void changeLayout(CommandBuffer *commandBuffer,
                            ImageLayout imageLayout) = 0;
  /** Generate texture mipmaps */
  virtual void generateMipmaps(CommandBuffer *commandBuffer) = 0;
  /** Set resource name for debugging
   *  @param name The resource name
   */
  virtual void setName(const std::string& name) { this->name = name; }
  std::string name;
  PixelFormat format;
  uint32_t w = 0, h = 0, d = 1, arrayLayers = 1, mipLevels = 1, numSamples = 1;
  uint32_t size = 0;
  std::vector<uint32_t> planeWidth, planeHeight, planeSize;
  ImageUsageFlags imageUsageFlags;
  TextureType textureType;
};
} // namespace ngfx
