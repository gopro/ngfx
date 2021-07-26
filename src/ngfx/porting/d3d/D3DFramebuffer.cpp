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
#include "ngfx/porting/d3d/D3DFramebuffer.h"
#include "ngfx/porting/d3d/D3DTexture.h"
#include "ngfx/porting/d3d/D3DSwapchain.h"
using namespace ngfx;
using D3DAttachment = D3DFramebuffer::D3DAttachment;

void D3DAttachment::create(D3DTexture* texture, uint32_t level, uint32_t baseLayer, uint32_t layerCount) {
    this->texture = texture;
    this->level = level;
    this->baseLayer = baseLayer;
    this->layerCount = layerCount;
    resource = texture->v.Get();
    bool depthStencilAttachment =
        texture->imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    cpuDescriptor = depthStencilAttachment
        ? texture->dsvDescriptor.cpuHandle
        : texture
        ->getRtvDescriptor(level, baseLayer, layerCount)
        .cpuHandle;
    subresourceIndex = baseLayer * texture->mipLevels + level;
    imageUsageFlags = texture->imageUsageFlags;
    numSamples = texture->numSamples;
    format = DXGI_FORMAT(texture->format);
}

void D3DFramebuffer::create(std::vector<D3DAttachment> &d3dAttachments,
                            int32_t w, uint32_t h, uint32_t layers) {
  this->d3dAttachments = d3dAttachments;
  this->numAttachments = uint32_t(d3dAttachments.size());
  this->w = w;
  this->h = h;
  auto it = this->d3dAttachments.begin();
  while (it != this->d3dAttachments.end()) {
    if (it->imageUsageFlags & IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
      if (it->numSamples > 1) {
        colorAttachments.push_back(&(*it++));
        resolveAttachments.push_back(&(*it++));
      } else {
        colorAttachments.push_back(&(*it++));
      }
    } else if (it->imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      if (it->numSamples > 1) {
        depthStencilAttachment = &(*it++);
        if (it != this->d3dAttachments.end())
          depthResolve = &(*it++);
      } else {
        depthStencilAttachment = &(*it++);
      }
    }
  }
}

void D3DAttachment::createFromSwapchainImage(D3DSwapchain *d3dSwapchain, uint32_t index) {
    resource = d3dSwapchain->renderTargets[index].Get();
    cpuDescriptor = d3dSwapchain->renderTargetDescriptors[index].cpuHandle;
    subresourceIndex = 0;
    imageUsageFlags = IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    numSamples = 1;
    format = DXGI_FORMAT(d3dSwapchain->format);
}

void D3DAttachment::createFromDepthStencilAttachment(D3DTexture* d3dDepthStencilAttachment) {
    resource = d3dDepthStencilAttachment->v.Get();
    cpuDescriptor = d3dDepthStencilAttachment->dsvDescriptor.cpuHandle;
    subresourceIndex = 0;
    imageUsageFlags = IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    numSamples = 1;
    format = DXGI_FORMAT(d3dDepthStencilAttachment->format);
    texture = d3dDepthStencilAttachment;
}

Framebuffer *Framebuffer::create(Device *device, RenderPass *renderPass,
                                 const std::vector<Attachment> &attachments,
                                 uint32_t w, uint32_t h, uint32_t layers) {
  D3DFramebuffer *d3dFramebuffer = new D3DFramebuffer();
  d3dFramebuffer->attachments = attachments;
  std::vector<D3DFramebuffer::D3DAttachment> d3dAttachments(attachments.size());
  for (uint32_t j = 0; j < attachments.size(); j++) {
    auto &attachment = attachments[j];
    auto &d3dAttachment = d3dAttachments[j];
    auto d3dTexture = d3d(attachment.texture);
    bool depthStencilAttachment =
        d3dTexture->imageUsageFlags & IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    d3dAttachment = {
        d3dTexture->v.Get(),
        depthStencilAttachment
            ? d3dTexture->dsvDescriptor.cpuHandle
            : d3dTexture
                  ->getRtvDescriptor(attachment.level, attachment.layer, layers)
                  .cpuHandle,
        attachment.layer * d3dTexture->mipLevels + attachment.level,
        d3dTexture->imageUsageFlags,
        d3dTexture->numSamples,
        DXGI_FORMAT(d3dTexture->format),
        d3dTexture,
        attachment.level,
        attachment.layer,
        1
    };
  }
  d3dFramebuffer->create(d3dAttachments, w, h, layers);
  return d3dFramebuffer;
}
