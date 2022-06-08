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
#include "ngfx/drawOps/DrawMeshOp.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/graphics/ShaderModule.h"
using namespace ngfx;
using namespace glm;

DrawMeshOp::DrawMeshOp(GraphicsContext *ctx, MeshData &meshData) : DrawOp(ctx) {
  bPos.reset(createVertexBuffer<vec3>(ctx, meshData.pos));
  bNormals.reset(createVertexBuffer<vec3>(ctx, meshData.normal));
  bFaces.reset(createIndexBuffer<ivec3>(ctx, meshData.faces));
  bUboVS.reset(createUniformBuffer(ctx, nullptr, sizeof(UBO_VS_Data)));
  bUboFS.reset(createUniformBuffer(ctx, nullptr, sizeof(UBO_FS_Data)));
  numVerts = uint32_t(meshData.pos.size());
  numNormals = uint32_t(meshData.normal.size());
  numFaces = uint32_t(meshData.faces.size());
  createPipeline();
  graphicsPipeline->getBindings({&U_UBO_VS, &U_UBO_FS}, {&B_POS, &B_NORMALS});
}

void DrawMeshOp::draw(CommandBuffer *commandBuffer, Graphics *graphics) {
  graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline);
  graphics->bindVertexBuffer(commandBuffer, bPos.get(), B_POS, sizeof(vec3));
  graphics->bindVertexBuffer(commandBuffer, bNormals.get(), B_NORMALS,
                             sizeof(vec3));
  graphics->bindIndexBuffer(commandBuffer, bFaces.get());
  graphics->bindUniformBuffer(commandBuffer, bUboVS.get(), U_UBO_VS,
                              SHADER_STAGE_VERTEX_BIT);
  graphics->bindUniformBuffer(commandBuffer, bUboFS.get(), U_UBO_FS,
                              SHADER_STAGE_FRAGMENT_BIT);
  graphics->drawIndexed(commandBuffer, numFaces * 3);
}

void DrawMeshOp::update(mat4 &modelView, mat4 &modelViewInverseTranspose,
                        mat4 &modelViewProj, LightData &lightData) {
  UBO_VS_Data uboVSData = {modelView, modelViewInverseTranspose, modelViewProj};
  UBO_FS_Data uboFSData = {lightData};
  bUboVS->upload(&uboVSData, sizeof(uboVSData));
  bUboFS->upload(&uboFSData, sizeof(uboFSData));
}

void DrawMeshOp::createPipeline() {
  const std::string key = "drawMeshOp";
  graphicsPipeline = (GraphicsPipeline *)ctx->pipelineCache->get(key);
  if (graphicsPipeline)
    return;
  GraphicsPipeline::State state;
  state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  state.depthTestEnable = true;
  state.depthWriteEnable = true;
  auto device = ctx->device;
  graphicsPipeline = GraphicsPipeline::create(
      ctx, state,
      VertexShaderModule::create(device, NGFX_DATA_DIR "/drawMesh.vert").get(),
      FragmentShaderModule::create(device, NGFX_DATA_DIR "/drawMesh.frag")
          .get(),
      ctx->surfaceFormat, ctx->depthStencilFormat);
  ctx->pipelineCache->add(key, graphicsPipeline);
}
