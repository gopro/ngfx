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
#include "ngfx/TriangleApp.h"
#include "ngfx/graphics/ShaderModule.h"
#include <memory>
using namespace ngfx;
using namespace glm;
using namespace std;

TriangleApp::TriangleApp(): Application("Triangle")  {}

void TriangleApp::onInit() {
    drawColorOp.reset(new DrawColorOp(graphicsContext.get(),
        { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f) },
        vec4(1.0, 0.0, 0.0, 1.0))
    );
}

void TriangleApp::onRecordCommandBuffer(CommandBuffer* commandBuffer) {
    graphicsContext->beginRenderPass(commandBuffer, graphics.get());
    drawColorOp->draw(commandBuffer, graphics.get());
    graphicsContext->endRenderPass(commandBuffer, graphics.get());
}

int main() {
    TriangleApp app;
    app.run();
    return 0;
}
