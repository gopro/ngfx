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

namespace ngfx {
    enum MTLPrimitiveType {
        MTLPrimitiveTypePoint = 0,
        MTLPrimitiveTypeLine = 1,
        MTLPrimitiveTypeLineStrip = 2,
        MTLPrimitiveTypeTriangle = 3,
        MTLPrimitiveTypeTriangleStrip = 4
    };
    enum MTLTriangleFillMode {
        MTLTriangleFillModeFill = 0,
        MTLTriangleFillModeLines = 1
    };
    enum MTLBlendFactor {
        MTLBlendFactorZero = 0,
        MTLBlendFactorOne = 1,
        MTLBlendFactorSourceColor = 2,
        MTLBlendFactorOneMinusSourceColor = 3,
        MTLBlendFactorSourceAlpha = 4,
        MTLBlendFactorOneMinusSourceAlpha = 5,
        MTLBlendFactorDestinationColor = 6,
        MTLBlendFactorOneMinusDestinationColor = 7,
        MTLBlendFactorDestinationAlpha = 8,
        MTLBlendFactorOneMinusDestinationAlpha = 9,
        MTLBlendFactorSourceAlphaSaturated = 10,
        MTLBlendFactorBlendColor = 11,
        MTLBlendFactorOneMinusBlendColor = 12,
        MTLBlendFactorBlendAlpha = 13,
        MTLBlendFactorOneMinusBlendAlpha = 14
    };
    enum MTLBlendOperation {
        MTLBlendOperationAdd = 0,
        MTLBlendOperationSubtract = 1,
        MTLBlendOperationReverseSubtract = 2,
        MTLBlendOperationMin = 3,
        MTLBlendOperationMax = 4,
    };
    enum MTLColorWriteMask {
        MTLColorWriteMaskNone  = 0,
        MTLColorWriteMaskRed   = 0x1 << 3,
        MTLColorWriteMaskGreen = 0x1 << 2,
        MTLColorWriteMaskBlue  = 0x1 << 1,
        MTLColorWriteMaskAlpha = 0x1 << 0,
        MTLColorWriteMaskAll   = 0xf
    };
    enum MTLCullMode {
        MTLCullModeNone = 0,
        MTLCullModeFront = 1,
        MTLCullModeBack = 2
    };
    enum MTLWinding {
        MTLWindingClockwise = 0,
        MTLWindingCounterClockwise = 1
    };
    enum MTLPixelFormat {
        MTLPixelFormatInvalid = 0,

        /* Normal 8 bit formats */
        
        MTLPixelFormatA8Unorm      = 1,
        
        MTLPixelFormatR8Unorm                            = 10,
        MTLPixelFormatR8Unorm_sRGB  = 11,

        MTLPixelFormatR8Snorm      = 12,
        MTLPixelFormatR8Uint       = 13,
        MTLPixelFormatR8Sint       = 14,
        
        /* Normal 16 bit formats */

        MTLPixelFormatR16Unorm     = 20,
        MTLPixelFormatR16Snorm     = 22,
        MTLPixelFormatR16Uint      = 23,
        MTLPixelFormatR16Sint      = 24,
        MTLPixelFormatR16Float     = 25,

        MTLPixelFormatRG8Unorm                            = 30,
        MTLPixelFormatRG8Unorm_sRGB  = 31,
        MTLPixelFormatRG8Snorm                            = 32,
        MTLPixelFormatRG8Uint                             = 33,
        MTLPixelFormatRG8Sint                             = 34,

        /* Packed 16 bit formats */
        
        MTLPixelFormatB5G6R5Unorm  = 40,
        MTLPixelFormatA1BGR5Unorm  = 41,
        MTLPixelFormatABGR4Unorm   = 42,
        MTLPixelFormatBGR5A1Unorm  = 43,

        /* Normal 32 bit formats */

        MTLPixelFormatR32Uint  = 53,
        MTLPixelFormatR32Sint  = 54,
        MTLPixelFormatR32Float = 55,

        MTLPixelFormatRG16Unorm  = 60,
        MTLPixelFormatRG16Snorm  = 62,
        MTLPixelFormatRG16Uint   = 63,
        MTLPixelFormatRG16Sint   = 64,
        MTLPixelFormatRG16Float  = 65,

        MTLPixelFormatRGBA8Unorm      = 70,
        MTLPixelFormatRGBA8Unorm_sRGB = 71,
        MTLPixelFormatRGBA8Snorm      = 72,
        MTLPixelFormatRGBA8Uint       = 73,
        MTLPixelFormatRGBA8Sint       = 74,
        
        MTLPixelFormatBGRA8Unorm      = 80,
        MTLPixelFormatBGRA8Unorm_sRGB = 81,

        /* Packed 32 bit formats */

        MTLPixelFormatRGB10A2Unorm = 90,
        MTLPixelFormatRGB10A2Uint  = 91,

        MTLPixelFormatRG11B10Float = 92,
        MTLPixelFormatRGB9E5Float = 93,

        MTLPixelFormatBGR10A2Unorm   = 94,

        MTLPixelFormatBGR10_XR       = 554,
        MTLPixelFormatBGR10_XR_sRGB  = 555,

        /* Normal 64 bit formats */

        MTLPixelFormatRG32Uint  = 103,
        MTLPixelFormatRG32Sint  = 104,
        MTLPixelFormatRG32Float = 105,

        MTLPixelFormatRGBA16Unorm  = 110,
        MTLPixelFormatRGBA16Snorm  = 112,
        MTLPixelFormatRGBA16Uint   = 113,
        MTLPixelFormatRGBA16Sint   = 114,
        MTLPixelFormatRGBA16Float  = 115,

        MTLPixelFormatBGRA10_XR       = 552,
        MTLPixelFormatBGRA10_XR_sRGB  = 553,

        /* Normal 128 bit formats */

        MTLPixelFormatRGBA32Uint  = 123,
        MTLPixelFormatRGBA32Sint  = 124,
        MTLPixelFormatRGBA32Float = 125,

        /* Compressed formats. */

        /* S3TC/DXT */
        MTLPixelFormatBC1_RGBA               = 130,
        MTLPixelFormatBC1_RGBA_sRGB          = 131,
        MTLPixelFormatBC2_RGBA               = 132,
        MTLPixelFormatBC2_RGBA_sRGB          = 133,
        MTLPixelFormatBC3_RGBA               = 134,
        MTLPixelFormatBC3_RGBA_sRGB          = 135,

        /* RGTC */
        MTLPixelFormatBC4_RUnorm             = 140,
        MTLPixelFormatBC4_RSnorm             = 141,
        MTLPixelFormatBC5_RGUnorm            = 142,
        MTLPixelFormatBC5_RGSnorm            = 143,

        /* BPTC */
        MTLPixelFormatBC6H_RGBFloat          = 150,
        MTLPixelFormatBC6H_RGBUfloat         = 151,
        MTLPixelFormatBC7_RGBAUnorm          = 152,
        MTLPixelFormatBC7_RGBAUnorm_sRGB     = 153,

        /* PVRTC */
        MTLPixelFormatPVRTC_RGB_2BPP         = 160,
        MTLPixelFormatPVRTC_RGB_2BPP_sRGB    = 161,
        MTLPixelFormatPVRTC_RGB_4BPP         = 162,
        MTLPixelFormatPVRTC_RGB_4BPP_sRGB    = 163,
        MTLPixelFormatPVRTC_RGBA_2BPP        = 164,
        MTLPixelFormatPVRTC_RGBA_2BPP_sRGB   = 165,
        MTLPixelFormatPVRTC_RGBA_4BPP        = 166,
        MTLPixelFormatPVRTC_RGBA_4BPP_sRGB   = 167,

        /* ETC2 */
        MTLPixelFormatEAC_R11Unorm           = 170,
        MTLPixelFormatEAC_R11Snorm           = 172,
        MTLPixelFormatEAC_RG11Unorm          = 174,
        MTLPixelFormatEAC_RG11Snorm          = 176,
        MTLPixelFormatEAC_RGBA8              = 178,
        MTLPixelFormatEAC_RGBA8_sRGB         = 179,

        MTLPixelFormatETC2_RGB8              = 180,
        MTLPixelFormatETC2_RGB8_sRGB         = 181,
        MTLPixelFormatETC2_RGB8A1            = 182,
        MTLPixelFormatETC2_RGB8A1_sRGB       = 183,

        /* ASTC */
        MTLPixelFormatASTC_4x4_sRGB          = 186,
        MTLPixelFormatASTC_5x4_sRGB          = 187,
        MTLPixelFormatASTC_5x5_sRGB          = 188,
        MTLPixelFormatASTC_6x5_sRGB          = 189,
        MTLPixelFormatASTC_6x6_sRGB          = 190,
        MTLPixelFormatASTC_8x5_sRGB          = 192,
        MTLPixelFormatASTC_8x6_sRGB          = 193,
        MTLPixelFormatASTC_8x8_sRGB          = 194,
        MTLPixelFormatASTC_10x5_sRGB         = 195,
        MTLPixelFormatASTC_10x6_sRGB         = 196,
        MTLPixelFormatASTC_10x8_sRGB         = 197,
        MTLPixelFormatASTC_10x10_sRGB        = 198,
        MTLPixelFormatASTC_12x10_sRGB        = 199,
        MTLPixelFormatASTC_12x12_sRGB        = 200,

        MTLPixelFormatASTC_4x4_LDR           = 204,
        MTLPixelFormatASTC_5x4_LDR           = 205,
        MTLPixelFormatASTC_5x5_LDR           = 206,
        MTLPixelFormatASTC_6x5_LDR           = 207,
        MTLPixelFormatASTC_6x6_LDR           = 208,
        MTLPixelFormatASTC_8x5_LDR           = 210,
        MTLPixelFormatASTC_8x6_LDR           = 211,
        MTLPixelFormatASTC_8x8_LDR           = 212,
        MTLPixelFormatASTC_10x5_LDR          = 213,
        MTLPixelFormatASTC_10x6_LDR          = 214,
        MTLPixelFormatASTC_10x8_LDR          = 215,
        MTLPixelFormatASTC_10x10_LDR         = 216,
        MTLPixelFormatASTC_12x10_LDR         = 217,
        MTLPixelFormatASTC_12x12_LDR         = 218,
        MTLPixelFormatGBGR422 = 240,
        MTLPixelFormatBGRG422 = 241,

        /* Depth */

        MTLPixelFormatDepth16Unorm           = 250,
        MTLPixelFormatDepth32Float  = 252,

        /* Stencil */

        MTLPixelFormatStencil8        = 253,

        /* Depth Stencil */
        
        MTLPixelFormatDepth24Unorm_Stencil8   = 255,
        MTLPixelFormatDepth32Float_Stencil8   = 260,

        MTLPixelFormatX32_Stencil8   = 261,
        MTLPixelFormatX24_Stencil8   = 262,
    };

    enum MTLIndexType {
        MTLIndexTypeUInt16 = 0,
        MTLIndexTypeUInt32 = 1,
    };

    enum MTLTextureType {
        MTLTextureType1D = 0,
        MTLTextureType1DArray = 1,
        MTLTextureType2D = 2,
        MTLTextureType2DArray = 3,
        MTLTextureType2DMultisample = 4,
        MTLTextureTypeCube = 5,
        MTLTextureTypeCubeArray = 6,
        MTLTextureType3D = 7,
        MTLTextureType2DMultisampleArray = 8,
        MTLTextureTypeTextureBuffer = 9
    };

    enum MTLVertexFormat {
        MTLVertexFormatInvalid = 0,
        
        MTLVertexFormatUChar2 = 1,
        MTLVertexFormatUChar3 = 2,
        MTLVertexFormatUChar4 = 3,
        
        MTLVertexFormatChar2 = 4,
        MTLVertexFormatChar3 = 5,
        MTLVertexFormatChar4 = 6,
        
        MTLVertexFormatUChar2Normalized = 7,
        MTLVertexFormatUChar3Normalized = 8,
        MTLVertexFormatUChar4Normalized = 9,
        
        MTLVertexFormatChar2Normalized = 10,
        MTLVertexFormatChar3Normalized = 11,
        MTLVertexFormatChar4Normalized = 12,
        
        MTLVertexFormatUShort2 = 13,
        MTLVertexFormatUShort3 = 14,
        MTLVertexFormatUShort4 = 15,
        
        MTLVertexFormatShort2 = 16,
        MTLVertexFormatShort3 = 17,
        MTLVertexFormatShort4 = 18,
        
        MTLVertexFormatUShort2Normalized = 19,
        MTLVertexFormatUShort3Normalized = 20,
        MTLVertexFormatUShort4Normalized = 21,
        
        MTLVertexFormatShort2Normalized = 22,
        MTLVertexFormatShort3Normalized = 23,
        MTLVertexFormatShort4Normalized = 24,
        
        MTLVertexFormatHalf2 = 25,
        MTLVertexFormatHalf3 = 26,
        MTLVertexFormatHalf4 = 27,
        
        MTLVertexFormatFloat = 28,
        MTLVertexFormatFloat2 = 29,
        MTLVertexFormatFloat3 = 30,
        MTLVertexFormatFloat4 = 31,
        
        MTLVertexFormatInt = 32,
        MTLVertexFormatInt2 = 33,
        MTLVertexFormatInt3 = 34,
        MTLVertexFormatInt4 = 35,
        
        MTLVertexFormatUInt = 36,
        MTLVertexFormatUInt2 = 37,
        MTLVertexFormatUInt3 = 38,
        MTLVertexFormatUInt4 = 39,
        
        MTLVertexFormatInt1010102Normalized = 40,
        MTLVertexFormatUInt1010102Normalized = 41
    };

    
}

