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
#pragma once
#include "ngfx/Application.h"
#include "ngfx/graphics/Camera.h"
#include "ngfx/drawOps/DrawMeshOp.h"
#include <memory>

namespace ngfx {
    class MeshApp : public Application {
    public:
        MeshApp();
        void onInit() override;
        void onRecordCommandBuffer(CommandBuffer* commandBuffer) override;
        void onUpdate() override;
        void onKey(KeyCode code, InputAction action) override;
        void onScroll(double xoffset, double yoffset) override;
        void onCursorPos(double x, double y) override;
        void onMouseButton(MouseButton button, InputAction action) override;
    private:
        void initModelMat(MeshData& meshData);
        std::unique_ptr<Camera> camera;
        std::unique_ptr<DrawMeshOp> drawMeshOp;
        mat4 modelMat, modelViewMat, modelViewInverseTransposeMat, projMat, modelViewProjMat;
    };
};
