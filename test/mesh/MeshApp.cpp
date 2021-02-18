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
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "ngfx/MeshApp.h"
#include "ngfx/graphics/MeshUtil.h"
#include "ngfx/graphics/ShaderModule.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include "ngfx/core/DebugUtil.h"
#include <memory>
using namespace ngfx;
using namespace std;
using namespace glm;

MeshApp::MeshApp(): Application("Mesh", Window::DISPLAY_WIDTH, Window::DISPLAY_HEIGHT, true)  {}

void MeshApp::onInit() {
    MeshData meshData;
    MeshUtil::importMesh("bunny.bin", meshData);
    drawMeshOp.reset(new DrawMeshOp(graphicsContext.get(), meshData));
    camera.reset(new Camera());
    camera->zoom = -2.0f;
    initModelMat(meshData);
}

void MeshApp::initModelMat(MeshData& meshData) {
    auto& b = meshData.bounds;
    vec3 dim = b[1] - b[0];
    vec3 center = 0.5f * (b[0] + b[1]);
    float maxDim = glm::compMax(dim);
    modelMat = scale(vec3(1.0f / maxDim)) * translate(-center);
}

void MeshApp::onRecordCommandBuffer(CommandBuffer* commandBuffer) {
    graphicsContext->beginRenderPass(commandBuffer, graphics.get());
    drawMeshOp->draw(commandBuffer, graphics.get());
    graphicsContext->endRenderPass(commandBuffer, graphics.get());
}

void MeshApp::onKey(KeyCode code, InputAction action) {
    camera->onKey(code, action);
}

void MeshApp::onScroll(double xoffset, double yoffset) {
    camera->onScroll(xoffset, yoffset);
}

void MeshApp::onCursorPos(double x, double y) {
    camera->onCursorPos(x, y);
}

void MeshApp::onMouseButton(MouseButton button, InputAction action) {
    camera->onMouseButton(button, action);
}

void MeshApp::onUpdate() {
    camera->update();
    modelViewMat = camera->viewMat * modelMat;
    modelViewInverseTransposeMat = transpose(inverse(modelViewMat));
    float aspect = 1920.0f / 1080.0f; //TODO: float(window->w) / float(window->h);
    projMat = perspective(radians(60.0f), aspect, 0.1f, 100.0f);
    modelViewProjMat = projMat * modelViewMat;
    DrawMeshOp::LightData lightData;
    drawMeshOp->update(modelViewMat, modelViewInverseTransposeMat, modelViewProjMat, lightData);
}

int main() {
    MeshApp app;
    app.run();
    return 0;
}
