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

#include "ngfx/porting/metal/MTLGraphicsPipeline.h"
#include "ngfx/porting/metal/MTLPipelineUtil.h"
#include "ngfx/porting/metal/MTLShaderModule.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/porting/metal/MTLCommandBuffer.h"
#include "ngfx/porting/metal/MTLRenderCommandEncoder.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

void MTLGraphicsPipeline::create(MTLGraphicsContext* ctx, const State& state, MTLVertexDescriptor* vertexDescriptor,
         const Shaders& shaders, ::MTLPixelFormat colorFormat, ::MTLPixelFormat depthStencilFormat) {
    NSError* error;
    auto device = ctx->mtlDevice.v;
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineStateDescriptor.label = @"";
    pipelineStateDescriptor.vertexFunction = shaders.VS;
    pipelineStateDescriptor.fragmentFunction = shaders.PS;
    auto &blendParams = state.blendParams;
    for (uint32_t j = 0; j<state.numColorAttachments; j++) {
        auto colorAttachment = pipelineStateDescriptor.colorAttachments[j];
        colorAttachment.pixelFormat = colorFormat;
        colorAttachment.blendingEnabled = state.blendEnable;
        colorAttachment.sourceRGBBlendFactor = ::MTLBlendFactor(blendParams.srcColorBlendFactor);
        colorAttachment.sourceAlphaBlendFactor = ::MTLBlendFactor(blendParams.srcAlphaBlendFactor);
        colorAttachment.destinationRGBBlendFactor = ::MTLBlendFactor(blendParams.dstColorBlendFactor);
        colorAttachment.destinationAlphaBlendFactor = ::MTLBlendFactor(blendParams.dstAlphaBlendFactor);
        colorAttachment.rgbBlendOperation = ::MTLBlendOperation(blendParams.colorBlendOp);
        colorAttachment.alphaBlendOperation = ::MTLBlendOperation(blendParams.alphaBlendOp);
        colorAttachment.writeMask = state.colorWriteMask;
    }
    
    pipelineStateDescriptor.rasterSampleCount = state.numSamples;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.depthAttachmentPixelFormat = depthStencilFormat;
    const std::vector<MTLPixelFormat> stencilFormats = {
        MTLPixelFormatStencil8 ,MTLPixelFormatDepth24Unorm_Stencil8,
        MTLPixelFormatDepth32Float_Stencil8,
    };
    if (std::find(stencilFormats.begin(), stencilFormats.end(), depthStencilFormat) != stencilFormats.end()) {
        pipelineStateDescriptor.stencilAttachmentPixelFormat = depthStencilFormat;
    }
    
    MTLPipelineOption options = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
    mtlPipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor options:options reflection:&reflection error:&error];
    [pipelineStateDescriptor release];
    NSCAssert(mtlPipelineState, @"Failed to create pipeline state: %@", error);
    
    mtlPrimitiveType = ::MTLPrimitiveType(state.primitiveTopology);
    mtlCullMode = ::MTLCullMode(state.cullModeFlags);
    mtlFrontFaceWinding = ::MTLWinding(state.frontFace);
    
    if (state.depthTestEnable || state.stencilEnable) {
        MTLDepthStencilDescriptor *depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDesc.depthCompareFunction = (state.depthTestEnable) ?
            ::MTLCompareFunction(state.depthFunc) :
            ::MTLCompareFunctionAlways;
        depthStencilDesc.depthWriteEnabled = state.depthWriteEnable;
        auto &stencilParams = state.stencilParams;
        if (state.stencilEnable) {
            MTLStencilDescriptor *frontFaceStencil = [[MTLStencilDescriptor alloc] init];
            frontFaceStencil.readMask = stencilParams.stencilReadMask;
            frontFaceStencil.writeMask = stencilParams.stencilWriteMask;
            frontFaceStencil.stencilFailureOperation = ::MTLStencilOperation(stencilParams.frontStencilFailOp);
            frontFaceStencil.depthFailureOperation = ::MTLStencilOperation(stencilParams.frontStencilDepthFailOp);
            frontFaceStencil.depthStencilPassOperation = ::MTLStencilOperation(stencilParams.frontStencilPassOp);
            frontFaceStencil.stencilCompareFunction = ::MTLCompareFunction(stencilParams.frontStencilFunc);
            depthStencilDesc.frontFaceStencil = frontFaceStencil;

            MTLStencilDescriptor *backFaceStencil = [[MTLStencilDescriptor alloc] init];
            backFaceStencil.readMask = stencilParams.stencilReadMask;
            backFaceStencil.writeMask = stencilParams.stencilWriteMask;
            backFaceStencil.stencilFailureOperation = ::MTLStencilOperation(stencilParams.backStencilFailOp);
            backFaceStencil.depthFailureOperation = ::MTLStencilOperation(stencilParams.backStencilDepthFailOp);
            backFaceStencil.depthStencilPassOperation = ::MTLStencilOperation(stencilParams.backStencilPassOp);
            backFaceStencil.stencilCompareFunction = ::MTLCompareFunction(stencilParams.backStencilFunc);
            depthStencilDesc.backFaceStencil = backFaceStencil;

            stencilRef = stencilParams.stencilRef;
        }
        mtlDepthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
    }
}

GraphicsPipeline* GraphicsPipeline::create(GraphicsContext *graphicsContext, const State &state,
                                           VertexShaderModule *vs, FragmentShaderModule *fs,
                                           PixelFormat colorFormat, PixelFormat depthStencilFormat,
                                           std::vector<VertexInputAttributeDescription> vertexAttributes,
                                           std::set<std::string> instanceAttributes) {
    MTLGraphicsPipeline* mtlGraphicsPipeline = new MTLGraphicsPipeline();
    auto& descriptorBindings = mtlGraphicsPipeline->descriptorBindings;
    
    MTLPipelineUtil::parseDescriptors(vs->descriptors, descriptorBindings);
    MTLPipelineUtil::parseDescriptors(fs->descriptors, descriptorBindings);
    
    uint32_t numVSDescriptors = vs->descriptors.size();
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor new];
    auto& vertexAttributeBindings = mtlGraphicsPipeline->vertexAttributeBindings;
    vertexAttributeBindings.resize(vs->attributes.size());
    for (uint32_t j = 0; j<vs->attributes.size(); j++) {
        auto& attr = vs->attributes[j];
        auto mtlAttr = vertexDescriptor.attributes[attr.location - numVSDescriptors];
        mtlAttr.bufferIndex = attr.location;
        auto mtlLayout = vertexDescriptor.layouts[mtlAttr.bufferIndex];
        uint32_t stride = attr.elementSize;
        auto mtlVertexFormat = attr.format;
        mtlAttr.format = ::MTLVertexFormat(mtlVertexFormat);
        mtlAttr.offset = vertexAttributes.empty() ? 0 : vertexAttributes[j].offset;
        mtlLayout.stride = stride;
        if (instanceAttributes.find(attr.name) != instanceAttributes.end())
            mtlLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        vertexAttributeBindings[j] = attr.location;
    }
    
    MTLGraphicsPipeline::Shaders shaders;
    shaders.VS = mtl(vs)->mtlFunction;
    shaders.PS = mtl(fs)->mtlFunction;
    
    mtlGraphicsPipeline->create(mtl(graphicsContext), state, vertexDescriptor, shaders, ::MTLPixelFormat(colorFormat),
        ::MTLPixelFormat(depthStencilFormat));
    [vertexDescriptor release];
    return mtlGraphicsPipeline;
}
