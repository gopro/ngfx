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
#include "ngfx/compute/ComputeApplication.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/Timer.h"
#include "ngfx/graphics/Graphics.h"
#include <memory>
using namespace ngfx;
using namespace glm;
using namespace std;

ComputeApplication::ComputeApplication(const std::string &name, int w, int h)
    : appName(name), w(w), h(h) {}

void ComputeApplication::init() {
  graphicsContext.reset(GraphicsContext::create(appName.c_str()));
  if (w == 0 && h == 0)
    graphicsContext->setSurface(nullptr);
  else {
    window.reset(Window::create(
        graphicsContext.get(), appName.c_str(),
        [&](Window *thiz) { graphicsContext->setSurface(thiz->surface); }));
  }
  graphics.reset(Graphics::create(graphicsContext.get()));
}

void ComputeApplication::recordCommandBuffer(CommandBuffer *commandBuffer) {
  commandBuffer->begin();
  onRecordCommandBuffer(commandBuffer);
  commandBuffer->end();
}

void ComputeApplication::run() {
  init();
  onInit();
  auto commandBuffer = graphicsContext->computeCommandBuffer();
  recordCommandBuffer(commandBuffer);
  onUpdate();
  doCompute(commandBuffer);
  close();
}

void ComputeApplication::close() {}

void ComputeApplication::doCompute(CommandBuffer *commandBuffer) {
  Timer timer;
  graphicsContext->submit(commandBuffer);
  graphics->waitIdle(commandBuffer);
  timer.update();
  NGFX_LOG("compute elapsed: %f", timer.elapsed);
  onComputeFinished();
}
