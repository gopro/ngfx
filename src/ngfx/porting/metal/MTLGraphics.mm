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

#include "ngfx/porting/metal/MTLGraphics.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/porting/metal/MTLFramebuffer.h"
#include "ngfx/porting/metal/MTLRenderPass.h"
#include "ngfx/porting/metal/MTLTexture.h"
#include "ngfx/porting/metal/MTLCommandBuffer.h"
#include "ngfx/porting/metal/MTLComputePipeline.h"
#include "ngfx/porting/metal/MTLGraphicsPipeline.h"
#include "ngfx/porting/metal/MTLComputeCommandEncoder.h"
#include "ngfx/porting/metal/MTLRenderCommandEncoder.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

void MTLGraphics::create() {}

void MTLGraphics::beginComputePass(CommandBuffer* commandBuffer) {
    auto mtlCommandBuffer = mtl(commandBuffer);
    currentComputeCommandEncoder.v = [mtlCommandBuffer->v computeCommandEncoder];
    currentCommandEncoder = &currentComputeCommandEncoder;
}
void MTLGraphics::endComputePass(CommandBuffer* commandBuffer) {
    [currentComputeCommandEncoder.v endEncoding];
    currentComputeCommandEncoder.v = nullptr;
}

struct MTLGraphicsUtil {
    static void setViewport(MTLGraphics* g, Rect2D& r) {
        [g->currentRenderCommandEncoder.v setViewport:(MTLViewport){ double(r.x), double(r.y), double(r.w), double(r.h), 0.0, 1.0 }];
    }

    static void setScissor(MTLGraphics* g, Rect2D &r) {
        #ifdef ORIGIN_BOTTOM_LEFT
            Rect2D &vp = g->viewport;
            MTLScissorRect mtlScissorRect = { NSUInteger(r.x), vp.h - r.y - r.h ,  r.w, r.h };
        #else
            MTLScissorRect mtlScissorRect = { NSUInteger(r.x), NSUInteger(r.y), r.w, r.h };
        #endif
            [g->currentRenderCommandEncoder.v setScissorRect:mtlScissorRect];
    }
};

void MTLGraphics::beginRenderPass(CommandBuffer* commandBuffer, RenderPass* renderPass, Framebuffer* framebuffer,
          glm::vec4 clearColor, float clearDepth, uint32_t clearStencil) {
    MTLRenderPassDescriptor* mtlRenderPassDescriptor = mtl(renderPass)->getDescriptor(mtl(ctx), mtl(framebuffer), clearColor, clearDepth, clearStencil);
    currentRenderCommandEncoder.v = [mtl(commandBuffer)->v renderCommandEncoderWithDescriptor:mtlRenderPassDescriptor];
    currentCommandEncoder = &currentRenderCommandEncoder;
}

void MTLGraphics::endRenderPass(CommandBuffer* commandBuffer) {
    [currentRenderCommandEncoder.v endEncoding];
    currentRenderCommandEncoder.v = nullptr;
}

void MTLGraphics::bindVertexBuffer(CommandBuffer* cmdBuffer, Buffer* buffer, uint32_t location, uint32_t stride) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    [renderEncoder->v setVertexBuffer:mtl(buffer)->v offset:0 atIndex:location];
}
void MTLGraphics::bindIndexBuffer(CommandBuffer* cmdBuffer, Buffer* buffer, IndexFormat indexFormat) {
    currentIndexBuffer = mtl(buffer);
    currentIndexFormat = indexFormat;
}
void MTLGraphics::bindUniformBuffer(CommandBuffer* cmdBuffer, Buffer* buffer, uint32_t binding, ShaderStageFlags shaderStageFlags) {
    if (MTLGraphicsPipeline* graphicsPipeline = dynamic_cast<MTLGraphicsPipeline*>(currentPipeline)) {
        auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
        if (shaderStageFlags & SHADER_STAGE_VERTEX_BIT) {
            [renderEncoder->v setVertexBuffer:mtl(buffer)->v offset:0 atIndex:binding];
        }
        if (shaderStageFlags & SHADER_STAGE_FRAGMENT_BIT) {
            [renderEncoder->v setFragmentBuffer:mtl(buffer)->v offset:0 atIndex:binding];
        }
    }
    else if (MTLComputePipeline* computePipeline = dynamic_cast<MTLComputePipeline*>(currentPipeline)) {
        auto computeEncoder = (MTLComputeCommandEncoder*)currentCommandEncoder;
        [computeEncoder->v setBuffer:mtl(buffer)->v offset:0 atIndex:binding];
    }
}
void MTLGraphics::bindStorageBuffer(CommandBuffer* cmdBuffer, Buffer* buffer, uint32_t binding, ShaderStageFlags shaderStageFlags) {
    if (MTLGraphicsPipeline* graphicsPipeline = dynamic_cast<MTLGraphicsPipeline*>(currentPipeline)) {
        auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
        if (shaderStageFlags & SHADER_STAGE_VERTEX_BIT) [renderEncoder->v setVertexBuffer:mtl(buffer)->v offset:0 atIndex:binding];
        if (shaderStageFlags & SHADER_STAGE_FRAGMENT_BIT) [renderEncoder->v setFragmentBuffer:mtl(buffer)->v offset:0 atIndex:binding];
    }
    else if (MTLComputePipeline* computePipeline = dynamic_cast<MTLComputePipeline*>(currentPipeline)) {
        auto computeEncoder = (MTLComputeCommandEncoder*)currentCommandEncoder;
        [computeEncoder->v setBuffer:mtl(buffer)->v offset:0 atIndex:binding];
    }
}
void MTLGraphics::bindComputePipeline(CommandBuffer* cmdBuffer, ComputePipeline* computePipeline) {
    auto computeEncoder = (MTLComputeCommandEncoder*)currentCommandEncoder;
    [computeEncoder->v setComputePipelineState: mtl(computePipeline)->mtlPipelineState];
    currentPipeline = computePipeline;
}
void MTLGraphics::bindGraphicsPipeline(CommandBuffer* cmdBuffer, GraphicsPipeline* graphicsPipeline) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    auto mtlPipeline = mtl(graphicsPipeline);
    [renderEncoder->v setRenderPipelineState: mtlPipeline->mtlPipelineState];
    [renderEncoder->v setCullMode: mtlPipeline->mtlCullMode];
    [renderEncoder->v setFrontFacingWinding: mtlPipeline->mtlFrontFaceWinding];
    if (mtlPipeline->mtlDepthStencilState) {
        [renderEncoder->v setDepthStencilState: mtlPipeline->mtlDepthStencilState];
    }
    currentPrimitiveType = mtl(graphicsPipeline)->mtlPrimitiveType;
    currentPipeline = graphicsPipeline;
}
void MTLGraphics::bindTexture(CommandBuffer* cmdBuffer, Texture* texture, uint32_t set) {
    if (MTLRenderCommandEncoder* renderEncoder = dynamic_cast<MTLRenderCommandEncoder*>(currentCommandEncoder)) {
        [renderEncoder->v setFragmentTexture: mtl(texture)->v atIndex: set];
        [renderEncoder->v setFragmentSamplerState: mtl(texture)->mtlSamplerState atIndex: set];
    }
    else if (MTLComputeCommandEncoder* computeEncoder = dynamic_cast<MTLComputeCommandEncoder*>(currentCommandEncoder)) {
        [computeEncoder->v setTexture: mtl(texture)->v atIndex: set];
        [computeEncoder->v setSamplerState: mtl(texture)->mtlSamplerState atIndex: set];
    }
}

void MTLGraphics::dispatch(CommandBuffer* cmdBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
       uint32_t threadsPerGroupX, uint32_t threadsPerGroupY, uint32_t threadsPerGroupZ) {
    auto computeEncoder = (MTLComputeCommandEncoder*)currentCommandEncoder;
    [computeEncoder->v dispatchThreadgroups:MTLSizeMake(groupCountX, groupCountY, groupCountZ)
                      threadsPerThreadgroup:MTLSizeMake(threadsPerGroupX, threadsPerGroupY, threadsPerGroupZ)];
}

void MTLGraphics::draw(CommandBuffer* cmdBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    [renderEncoder->v drawPrimitives: currentPrimitiveType vertexStart:firstVertex vertexCount:vertexCount
        instanceCount: instanceCount baseInstance: firstInstance];
}
void MTLGraphics::drawIndexed(CommandBuffer* cmdBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    [renderEncoder->v drawIndexedPrimitives:currentPrimitiveType indexCount:indexCount
        indexType: (currentIndexFormat == INDEXFORMAT_UINT16 ? ::MTLIndexTypeUInt16 : ::MTLIndexTypeUInt32)
        indexBuffer:currentIndexBuffer->v indexBufferOffset:0
        instanceCount: instanceCount baseVertex: 0 baseInstance: firstInstance];
}
void MTLGraphics::setViewport(CommandBuffer* cmdBuffer, Rect2D r) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    if (renderEncoder) MTLGraphicsUtil::setViewport(this, r);
    viewport = r;
}
void MTLGraphics::setScissor(CommandBuffer* cmdBuffer, Rect2D r) {
    auto renderEncoder = (MTLRenderCommandEncoder*)currentCommandEncoder;
    if (renderEncoder) MTLGraphicsUtil::setScissor(this, r);
    scissorRect = r;
}

void MTLGraphics::waitIdle(CommandBuffer* cmdBuffer) {
    mtl(cmdBuffer)->waitUntilCompleted();
}

Graphics* Graphics::create(GraphicsContext* ctx) {
    MTLGraphics* mtlGraphics = new MTLGraphics();
    mtlGraphics->ctx = ctx;
    mtlGraphics->create();
    return mtlGraphics;
}
