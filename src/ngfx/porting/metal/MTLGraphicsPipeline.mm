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
    for (uint32_t j = 0; j<state.numColorAttachments; j++) {
        auto colorAttachment = pipelineStateDescriptor.colorAttachments[j];
        colorAttachment.pixelFormat = colorFormat;
        colorAttachment.blendingEnabled = state.blendEnable;
        colorAttachment.sourceRGBBlendFactor = ::MTLBlendFactor(state.srcColorBlendFactor);
        colorAttachment.sourceAlphaBlendFactor = ::MTLBlendFactor(state.srcAlphaBlendFactor);
        colorAttachment.destinationRGBBlendFactor = ::MTLBlendFactor(state.dstColorBlendFactor);
        colorAttachment.destinationAlphaBlendFactor = ::MTLBlendFactor(state.dstAlphaBlendFactor);
        colorAttachment.rgbBlendOperation = ::MTLBlendOperation(state.colorBlendOp);
        colorAttachment.alphaBlendOperation = ::MTLBlendOperation(state.alphaBlendOp);
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
        if (state.stencilEnable) {
            MTLStencilDescriptor *frontFaceStencil = [[MTLStencilDescriptor alloc] init];
            frontFaceStencil.readMask = state.stencilReadMask;
            frontFaceStencil.writeMask = state.stencilWriteMask;
            frontFaceStencil.stencilFailureOperation = ::MTLStencilOperation(state.frontStencilFailOp);
            frontFaceStencil.depthFailureOperation = ::MTLStencilOperation(state.frontStencilDepthFailOp);
            frontFaceStencil.depthStencilPassOperation = ::MTLStencilOperation(state.frontStencilPassOp);
            frontFaceStencil.stencilCompareFunction = ::MTLCompareFunction(state.frontStencilFunc);
            depthStencilDesc.frontFaceStencil = frontFaceStencil;

            MTLStencilDescriptor *backFaceStencil = [[MTLStencilDescriptor alloc] init];
            backFaceStencil.readMask = state.stencilReadMask;
            backFaceStencil.writeMask = state.stencilWriteMask;
            backFaceStencil.stencilFailureOperation = ::MTLStencilOperation(state.backStencilFailOp);
            backFaceStencil.depthFailureOperation = ::MTLStencilOperation(state.backStencilDepthFailOp);
            backFaceStencil.depthStencilPassOperation = ::MTLStencilOperation(state.backStencilPassOp);
            backFaceStencil.stencilCompareFunction = ::MTLCompareFunction(state.backStencilFunc);
            depthStencilDesc.backFaceStencil = backFaceStencil;

            stencilRef = state.stencilRef;
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
