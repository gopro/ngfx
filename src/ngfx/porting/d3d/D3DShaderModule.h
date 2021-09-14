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
#include "ngfx/graphics/ShaderModule.h"
#include "ngfx/porting/d3d/D3DUtil.h"

namespace ngfx {
class D3DShaderModule {
public:
  bool initFromFile(const std::string &filename);
  virtual ~D3DShaderModule();
  D3D12_SHADER_BYTECODE d3dShaderByteCode{};
  void initFromByteCode(void *bytecodeData, uint32_t bytecodeSize);
  bool compile(const std::string &filename);
};
D3D_CAST(ShaderModule);

class D3DVertexShaderModule : public VertexShaderModule,
                              public D3DShaderModule {
public:
  virtual ~D3DVertexShaderModule() {}
};
D3D_CAST(VertexShaderModule);

class D3DFragmentShaderModule : public FragmentShaderModule,
                                public D3DShaderModule {
public:
  virtual ~D3DFragmentShaderModule() {}
};
D3D_CAST(FragmentShaderModule);

class D3DComputeShaderModule : public ComputeShaderModule,
                               public D3DShaderModule {
public:
  virtual ~D3DComputeShaderModule() {}
};
D3D_CAST(ComputeShaderModule);
} // namespace ngfx