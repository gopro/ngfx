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
#include "ngfx/drawOps/DrawTextureOp.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/graphics/ShaderModule.h"
using namespace ngfx;
using namespace glm;
using namespace std;

DrawTextureOp::DrawTextureOp(GraphicsContext *ctx, Texture *texture,
                             const std::vector<glm::vec2> &pos,
                             const std::vector<glm::vec2> &texCoord,
                             OnGetPipelineState onGetPipelineState)
    : DrawOp(ctx, onGetPipelineState), texture(texture) {
    bPos.reset(createVertexBuffer<vec2>(ctx, pos));
    bTexCoord.reset(createVertexBuffer<vec2>(ctx, texCoord));
    numVerts = uint32_t(pos.size());
    getPipeline();
    graphicsPipeline->getBindings({ &U_TEXTURE }, { &B_POS, &B_TEXCOORD });
}

void DrawTextureOp::draw(CommandBuffer *commandBuffer, Graphics *graphics) {
    graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline);
    graphics->bindVertexBuffer(commandBuffer, bPos.get(), B_POS, sizeof(vec2));
    graphics->bindVertexBuffer(commandBuffer, bTexCoord.get(), B_TEXCOORD,
                                sizeof(vec2));
    graphics->bindTexture(commandBuffer, texture, U_TEXTURE);
    graphics->draw(commandBuffer, numVerts);
}
void DrawTextureOp::getPipeline() {
    GraphicsPipeline::State state = getPipelineState();
    const string key = "drawTextureOp_" + to_string(state.key());
    graphicsPipeline = (GraphicsPipeline *)ctx->pipelineCache->get(key);
    if (graphicsPipeline)
      return;
    auto device = ctx->device;
    graphicsPipeline = GraphicsPipeline::create(
        ctx, state,
        VertexShaderModule::create(device, NGFX_DATA_DIR "/drawTexture.vert")
            .get(),
        FragmentShaderModule::create(device, NGFX_DATA_DIR "/drawTexture.frag")
            .get(),
        ctx->surfaceFormat, ctx->depthStencilFormat);
    ctx->pipelineCache->add(key, graphicsPipeline);
}

GraphicsPipeline::State DrawTextureOp::getPipelineState() {
    GraphicsPipeline::State state;
    state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    if (onGetPipelineState)
        onGetPipelineState(state);
    return state;
}
