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
#include "ngfx/porting/metal/MTLGraphicsCoreDefines.h"

namespace ngfx {
enum PrimitiveTopology {
  PRIMITIVE_TOPOLOGY_POINT_LIST = MTLPrimitiveTypePoint,
  PRIMITIVE_TOPOLOGY_LINE_LIST = MTLPrimitiveTypeLine,
  PRIMITIVE_TOPOLOGY_LINE_STRIP = MTLPrimitiveTypeLineStrip,
  PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = MTLPrimitiveTypeTriangle,
  PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = MTLPrimitiveTypeTriangleStrip
};
enum PolygonMode {
  POLYGON_MODE_FILL = MTLTriangleFillModeFill,
  POLYGON_MODE_LINE = MTLTriangleFillModeLines,
};
enum BlendFactor {
  BLEND_FACTOR_ZERO = MTLBlendFactorZero,
  BLEND_FACTOR_ONE = MTLBlendFactorOne,
  BLEND_FACTOR_SRC_COLOR = MTLBlendFactorSourceColor,
  BLEND_FACTOR_ONE_MINUS_SRC_COLOR = MTLBlendFactorOneMinusSourceColor,
  BLEND_FACTOR_DST_COLOR = MTLBlendFactorDestinationColor,
  BLEND_FACTOR_ONE_MINUS_DST_COLOR = MTLBlendFactorOneMinusDestinationColor,
  BLEND_FACTOR_SRC_ALPHA = MTLBlendFactorSourceAlpha,
  BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = MTLBlendFactorOneMinusSourceAlpha,
  BLEND_FACTOR_DST_ALPHA = MTLBlendFactorDestinationAlpha,
  BLEND_FACTOR_ONE_MINUS_DST_ALPHA = MTLBlendFactorOneMinusDestinationAlpha,
  BLEND_FACTOR_CONSTANT_COLOR = MTLBlendFactorBlendColor,
  BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = MTLBlendFactorOneMinusBlendColor,
  BLEND_FACTOR_CONSTANT_ALPHA = MTLBlendFactorBlendAlpha,
  BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = MTLBlendFactorOneMinusBlendAlpha,
  BLEND_FACTOR_SRC_ALPHA_SATURATE = MTLBlendFactorSourceAlphaSaturated
};
enum BlendOp {
  BLEND_OP_ADD = MTLBlendOperationAdd,
  BLEND_OP_SUBTRACT = MTLBlendOperationSubtract,
  BLEND_OP_REVERSE_SUBTRACT = MTLBlendOperationReverseSubtract,
  BLEND_OP_MIN = MTLBlendOperationMin,
  BLEND_OP_MAX = MTLBlendOperationMax
};
enum BufferUsageFlagBits {
  BUFFER_USAGE_TRANSFER_SRC_BIT,
  BUFFER_USAGE_TRANSFER_DST_BIT,
  BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  BUFFER_USAGE_STORAGE_BUFFER_BIT,
  BUFFER_USAGE_VERTEX_BUFFER_BIT,
  BUFFER_USAGE_INDEX_BUFFER_BIT
};
enum ColorComponentFlagBits {
  COLOR_COMPONENT_R_BIT = MTLColorWriteMaskRed,
  COLOR_COMPONENT_G_BIT = MTLColorWriteMaskGreen,
  COLOR_COMPONENT_B_BIT = MTLColorWriteMaskBlue,
  COLOR_COMPONENT_A_BIT = MTLColorWriteMaskAlpha
};
enum CommandBufferLevel {
  COMMAND_BUFFER_LEVEL_PRIMARY,
  COMMAND_BUFFER_LEVEL_SECONDARY
};
enum CullModeFlags {
  CULL_MODE_NONE = MTLCullModeNone,
  CULL_MODE_FRONT_BIT = MTLCullModeFront,
  CULL_MODE_BACK_BIT = MTLCullModeBack
};
enum FenceCreateFlagBits {
  FENCE_CREATE_SIGNALED_BIT,
};
enum FrontFace {
  FRONT_FACE_COUNTER_CLOCKWISE = MTLWindingCounterClockwise,
  FRONT_FACE_CLOCKWISE = MTLWindingClockwise
};
enum ImageUsageFlagBits {
  IMAGE_USAGE_TRANSFER_SRC_BIT = 1,
  IMAGE_USAGE_TRANSFER_DST_BIT = 2,
  IMAGE_USAGE_SAMPLED_BIT = 4,
  IMAGE_USAGE_STORAGE_BIT = 8,
  IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 16,
  IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 32,
  IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT = 64,
  IMAGE_USAGE_INPUT_ATTACHMENT_BIT = 128,
};
enum PipelineStageFlagBits { PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
enum ShaderStageFlagBits {
  SHADER_STAGE_VERTEX_BIT = 1,
  SHADER_STAGE_TESSELLATION_CONTROL_BIT = 2,
  SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 4,
  SHADER_STAGE_GEOMETRY_BIT = 8,
  SHADER_STAGE_FRAGMENT_BIT = 16,
  SHADER_STAGE_COMPUTE_BIT = 32,
  SHADER_STAGE_ALL_GRAPHICS = 31,
  SHADER_STAGE_ALL
};
enum TextureType {
  TEXTURE_TYPE_2D = MTLTextureType2D,
  TEXTURE_TYPE_3D = MTLTextureType3D,
  TEXTURE_TYPE_CUBE = MTLTextureTypeCube,
  TEXTURE_TYPE_2D_ARRAY = MTLTextureType2DArray
};

#define DEFINE_PIXELFORMATS(s, t0, t1)                                         \
  PIXELFORMAT_R##s##_##t0 = MTLPixelFormatR##s##t1,                            \
  PIXELFORMAT_RG##s##_##t0 = MTLPixelFormatRG##s##t1,                          \
  PIXELFORMAT_RGBA##s##_##t0 = MTLPixelFormatRGBA##s##t1

enum PixelFormat {
  PIXELFORMAT_UNDEFINED = MTLPixelFormatInvalid,
  DEFINE_PIXELFORMATS(8, UNORM, Unorm),
  DEFINE_PIXELFORMATS(16, UINT, Uint),
  DEFINE_PIXELFORMATS(16, SFLOAT, Float),
  DEFINE_PIXELFORMATS(32, UINT, Uint),
  DEFINE_PIXELFORMATS(32, SFLOAT, Float),
  PIXELFORMAT_BGRA8_UNORM = MTLPixelFormatBGRA8Unorm,
  PIXELFORMAT_D16_UNORM = MTLPixelFormatDepth16Unorm,
  PIXELFORMAT_D24_UNORM = MTLPixelFormatDepth24Unorm_Stencil8,
  PIXELFORMAT_D24_UNORM_S8 = MTLPixelFormatDepth24Unorm_Stencil8
};

enum IndexFormat {
  INDEXFORMAT_UINT16 = MTLIndexTypeUInt16,
  INDEXFORMAT_UINT32 = MTLIndexTypeUInt32
};

#define DEFINE_VERTEXFORMATS(t0, t1)                                           \
  VERTEXFORMAT_##t0 = MTLVertexFormat##t1,                                     \
  VERTEXFORMAT_##t0##2 = MTLVertexFormat##t1##2,                               \
  VERTEXFORMAT_##t0##3 = MTLVertexFormat##t1##3,                               \
  VERTEXFORMAT_##t0##4 = MTLVertexFormat##t1##4

enum VertexFormat { DEFINE_VERTEXFORMATS(FLOAT, Float) };

enum DescriptorType {
  DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
  DESCRIPTOR_TYPE_STORAGE_IMAGE,
  DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  DESCRIPTOR_TYPE_STORAGE_BUFFER
};
enum VertexInputRate { VERTEX_INPUT_RATE_VERTEX, VERTEX_INPUT_RATE_INSTANCE };

enum FilterMode { FILTER_NEAREST, FILTER_LINEAR };

enum SamplerAddressMode {
	CLAMP_TO_EDGE = MTLSamplerAddressModeClampToEdge,
	MIRROR_CLAMP_TO_EDGE = MTLSamplerAddressModeMirrorClampToEdge,
	REPEAT = MTLSamplerAddressModeRepeat,
	MIRROR_REPEAT = MTLSamplerAddressModeMirrorRepeat
};

enum ImageLayout {
  IMAGE_LAYOUT_UNDEFINED,
  IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  IMAGE_LAYOUT_GENERAL,
  IMAGE_LAYOUT_PRESENT_SRC
};
} // namespace ngfx
