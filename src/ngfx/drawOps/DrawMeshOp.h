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
#include "ngfx/graphics/MeshData.h"
#include "ngfx/graphics/DrawOp.h"
#include "ngfx/graphics/GraphicsPipeline.h"
#include "ngfx/graphics/Buffer.h"
#include <memory>

namespace ngfx {
    class DrawMeshOp : public DrawOp {
    public:
        DrawMeshOp(GraphicsContext* ctx, MeshData& meshData);
        virtual ~DrawMeshOp() {}
        void draw(CommandBuffer* commandBuffer, Graphics* graphics) override;
        struct LightData {
            vec4 ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
            vec4 diffuse = vec4(1.0f);
            vec4 specular = vec4(1.0f);
            vec4 lightViewPos = vec4(0.0f, 0.0f, 1.0f, 1.0f);
            vec2 shininess = vec2(10.0f, 0.0f), padding;
        };
        virtual void update(
            mat4& modelView, mat4& modelViewInverseTranspose, mat4& modelViewProj,
            LightData& lightData
        );
        std::unique_ptr<Buffer> bPos, bNormals;
        std::unique_ptr<Buffer> bFaces;
        std::unique_ptr<Buffer> bUboVS, bUboFS;
    protected:
        struct UBO_VS_Data {
            mat4 modelView;
            mat4 modelViewInverseTranspose;
            mat4 modelViewProj;
        };
        struct UBO_FS_Data {
            LightData light0;
        };
        virtual void createPipeline();
        GraphicsPipeline* graphicsPipeline;
        uint32_t B_POS, B_NORMALS, U_UBO_VS, U_UBO_FS;
        uint32_t numVerts, numNormals;
        uint32_t numFaces;
    };
}
