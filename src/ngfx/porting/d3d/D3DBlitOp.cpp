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
#include "ngfx/porting/d3d/D3DBlitOp.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphics.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/porting/d3d/D3DTexture.h"
#include <glm/gtc/type_ptr.hpp>
using namespace ngfx;
using namespace glm;

D3DBlitOp::D3DBlitOp(D3DGraphicsContext *ctx, D3DTexture *srcTexture,
                     uint32_t srcLevel, D3DTexture *dstTexture,
                     uint32_t dstLevel, Region srcRegion, Region dstRegion,
                     uint32_t srcBaseLayer, uint32_t srcLayerCount,
                     uint32_t dstBaseLayer, uint32_t dstLayerCount)
    : ctx(ctx), srcTexture(srcTexture), srcLevel(srcLevel),
      dstTexture(dstTexture), dstLevel(dstLevel), srcRegion(srcRegion),
      dstRegion(dstRegion), srcBaseLayer(srcBaseLayer),
      srcLayerCount(srcLayerCount), dstBaseLayer(dstBaseLayer),
      dstLayerCount(dstLayerCount) {
  outputFramebuffer.reset((D3DFramebuffer *)Framebuffer::create(
      ctx->device, ctx->defaultOffscreenRenderPass, {{dstTexture, dstLevel, 0}},
      dstTexture->w >> dstLevel, dstTexture->h >> dstLevel));
  std::vector<vec2> pos = {vec2(-1, 1), vec2(-1, -1), vec2(1, 1), vec2(1, -1)};
  std::vector<vec2> texCoord = {vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1)};
  bPos.reset((D3DBuffer *)createVertexBuffer<vec2>(ctx, pos));
  bTexCoord.reset((D3DBuffer *)createVertexBuffer<vec2>(ctx, texCoord));
  numVerts = uint32_t(pos.size());
  UBOData uboData = {srcLevel};
  bUbo.reset((D3DBuffer *)createUniformBuffer(ctx, &uboData, sizeof(uboData)));
  createPipeline();
  graphicsPipeline->getBindings({&U_UBO, &U_TEXTURE}, {&B_POS, &B_TEXCOORD});
}

void D3DBlitOp::createPipeline() {
  const std::string key = "d3dBlitOp";
  graphicsPipeline = (D3DGraphicsPipeline *)ctx->pipelineCache->get(key);
  if (graphicsPipeline)
    return;
  GraphicsPipeline::State state;
  state.renderPass = ctx->defaultOffscreenRenderPass;
  state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  auto device = ctx->device;
  graphicsPipeline = (D3DGraphicsPipeline *)GraphicsPipeline::create(
      ctx, state,
      VertexShaderModule::create(device, NGFX_DATA_DIR "/d3dBlitOp.vert").get(),
      FragmentShaderModule::create(device, NGFX_DATA_DIR "/d3dBlitOp.frag")
          .get(),
      dstTexture->format, ctx->depthFormat);
  ctx->pipelineCache->add(key, graphicsPipeline);
}

void D3DBlitOp::draw(D3DCommandList *cmdList, D3DGraphics *graphics) {
  graphics->bindGraphicsPipeline(cmdList, graphicsPipeline);
  graphics->bindVertexBuffer(cmdList, bPos.get(), B_POS, sizeof(vec2));
  graphics->bindUniformBuffer(cmdList, bUbo.get(), U_UBO,
                              SHADER_STAGE_FRAGMENT_BIT);
  graphics->bindVertexBuffer(cmdList, bTexCoord.get(), B_TEXCOORD,
                             sizeof(vec2));
  D3D_TRACE(cmdList->v->SetGraphicsRootDescriptorTable(
      U_TEXTURE, srcTexture->getSrvDescriptor(srcLevel, 1).gpuHandle));
  D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  D3D_TRACE(cmdList->v->SetGraphicsRootDescriptorTable(
      U_TEXTURE + 1, srcTexture->getSamplerDescriptor(filter).gpuHandle));
  graphics->draw(cmdList, 4);
}

void D3DBlitOp::apply(D3DGraphicsContext *ctx, D3DCommandList *cmdList,
                      D3DGraphics *graphics) {
  ctx->beginOffscreenRenderPass(cmdList, graphics, outputFramebuffer.get());
  draw(cmdList, graphics);
  ctx->endOffscreenRenderPass(cmdList, graphics);
}
