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

#include "ngfx/porting/metal/MTLRenderPass.h"
#include "ngfx/porting/metal/MTLGraphicsContext.h"
#include "ngfx/porting/metal/MTLSurface.h"
using namespace ngfx;
using namespace std;

void MTLRenderPass::create(MTLGraphicsContext *ctx,
                           AttachmentLoadOp colorLoadOp,
                           AttachmentStoreOp colorStoreOp,
                           AttachmentLoadOp depthLoadOp,
                           AttachmentStoreOp depthStoreOp) {
  this->ctx = ctx;
  this->colorLoadOp = colorLoadOp;
  this->colorStoreOp = colorStoreOp;
  this->depthLoadOp = depthLoadOp;
  this->depthStoreOp = depthStoreOp;
}

MTLRenderPassDescriptor* MTLRenderPass::getDescriptor(MTLGraphicsContext* mtlCtx, MTLFramebuffer* mtlFramebuffer,
       glm::vec4 clearColor, float clearDepth, uint32_t clearStencil) {
    MTLRenderPassDescriptor* mtlRenderPassDescriptor = nullptr;
    vector<MTLRenderPassColorAttachmentDescriptor*> colorAttachments;
    if (mtlFramebuffer->colorAttachments.empty()) {
        MTLSurface *surface = (MTLSurface*)mtlCtx->surface;
        NSView *view = surface->view;
        MTKView *mtkView = nullptr;
        if ([view class] == [MTKView class]) {
            mtkView = (MTKView*)view;
        }
        if (mtkView) {
            mtlRenderPassDescriptor = mtkView.currentRenderPassDescriptor;
            colorAttachments.push_back(mtlRenderPassDescriptor.colorAttachments[0]);
        }
        else {
            mtlRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            auto colorAttachment = mtlRenderPassDescriptor.colorAttachments[0];
            if (mtlCtx->numSamples == 1) {
                colorAttachment.texture = surface->drawable.texture;
            } else {
                colorAttachment.texture = surface->msaaColorTexture.get()->v;
                colorAttachment.resolveTexture = surface->drawable.texture;
            }
            colorAttachments.push_back(colorAttachment);
            auto depthAttachment = mtlRenderPassDescriptor.depthAttachment;
            if (mtlCtx->numSamples == 1) {
                depthAttachment.texture = surface->depthStencilTexture.get()->v;
            } else {
                depthAttachment.resolveTexture = surface->depthStencilTexture.get()->v;
                depthAttachment.texture = surface->msaaDepthStencilTexture.get()->v;
            }
            auto stencilAttachment = mtlRenderPassDescriptor.stencilAttachment;
            if (mtlCtx->numSamples == 1) {
                stencilAttachment.texture = surface->depthStencilTexture.get()->v;
            } else {
                stencilAttachment.texture = surface->msaaDepthStencilTexture.get()->v;
                stencilAttachment.resolveTexture = surface->depthStencilTexture.get()->v;
            }
        }
    } else {
        mtlRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        for (uint32_t j = 0; j<mtlFramebuffer->colorAttachments.size(); j++) {
            auto colorAttachment = mtlRenderPassDescriptor.colorAttachments[j];
            auto& fbColorAttachment = mtlFramebuffer->colorAttachments[j];
            colorAttachment.texture = fbColorAttachment.texture;
            colorAttachment.resolveTexture = fbColorAttachment.resolveTexture;
            colorAttachment.slice = fbColorAttachment.slice;
            colorAttachment.level = fbColorAttachment.level;
            colorAttachment.resolveSlice = fbColorAttachment.resolveSlice;
            colorAttachment.resolveLevel = fbColorAttachment.resolveLevel;
            colorAttachments.push_back(colorAttachment);
        }
    }
    for (auto& colorAttachment : colorAttachments) {
        colorAttachment.clearColor = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
        colorAttachment.loadAction = ::MTLLoadAction(colorLoadOp);
        if (colorAttachment.resolveTexture)
            colorAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
        else
            colorAttachment.storeAction = ::MTLStoreAction(colorStoreOp);
    }
    auto depthAttachment = mtlRenderPassDescriptor.depthAttachment;
    if (mtlFramebuffer->depthAttachment) {
        depthAttachment.clearDepth = clearDepth;
        depthAttachment.loadAction = ::MTLLoadAction(depthLoadOp);
        depthAttachment.resolveTexture = mtlFramebuffer->depthAttachment.resolveTexture;
        depthAttachment.texture = mtlFramebuffer->depthAttachment.texture;
        if (depthAttachment.resolveTexture)
            depthAttachment.storeAction = ::MTLStoreActionMultisampleResolve;
        else
            depthAttachment.storeAction = ::MTLStoreAction(depthStoreOp);
    }
    auto stencilAttachment = mtlRenderPassDescriptor.stencilAttachment;
    if (mtlFramebuffer->stencilAttachment) {
        stencilAttachment.texture = mtlFramebuffer->stencilAttachment.texture;
    }
    if (mtlRenderPassDescriptor.stencilAttachment)
        mtlRenderPassDescriptor.stencilAttachment.clearStencil = clearStencil;
    return mtlRenderPassDescriptor;
}
