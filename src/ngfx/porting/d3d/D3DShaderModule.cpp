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
#include "ngfx/porting/d3d/D3DShaderModule.h"
#include "ngfx/core/File.h"
#include "ngfx/core/StringUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include <d3dcompiler.h>
#include <memory>
using namespace ngfx;
using namespace std;

void D3DShaderModule::initFromFile(const std::string &filename) {
  File file;
#ifdef USE_PRECOMPILED_SHADERS
  file.read(filename + ".dxc");
  initFromByteCode(file.data.get(), file.size);
#else
  compile(filename + ".hlsl");
#endif
}

void D3DShaderModule::compile(const std::string &filename) {
  HRESULT hResult;
  UINT compileFlags = 0;
  if (DEBUG_SHADERS)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  ComPtr<ID3DBlob> byteCode;
  ComPtr<ID3DBlob> errorBlob;
  std::string target;
  if (strstr(filename.c_str(), "vert"))
    target = "vs_5_1";
  else if (strstr(filename.c_str(), "frag"))
    target = "ps_5_1";
  else if (strstr(filename.c_str(), "comp"))
    target = "cs_5_1";
  hResult = D3DCompileFromFile(StringUtil::toWString(filename).c_str(), nullptr,
                               nullptr, "main", target.c_str(), compileFlags, 0,
                               &byteCode, &errorBlob);
  if (errorBlob) {
    NGFX_ERR("%s %s", (char *)errorBlob->GetBufferPointer(), filename.c_str());
  }
  V0(hResult, "%s", filename.c_str());
  initFromByteCode(byteCode->GetBufferPointer(),
                   uint32_t(byteCode->GetBufferSize()));
}

void D3DShaderModule::initFromByteCode(void *bytecodeData,
                                       uint32_t bytecodeSize) {
  d3dShaderByteCode.pShaderBytecode = malloc(bytecodeSize);
  d3dShaderByteCode.BytecodeLength = bytecodeSize;
  memcpy((void *)d3dShaderByteCode.pShaderBytecode, bytecodeData, bytecodeSize);
}

D3DShaderModule::~D3DShaderModule() {
  if (d3dShaderByteCode.pShaderBytecode) {
    free((void *)d3dShaderByteCode.pShaderBytecode);
  }
}

template <typename T>
static std::unique_ptr<T> createShaderModule(Device *device,
                                             const std::string &filename) {
  auto d3dShaderModule = make_unique<T>();
  d3dShaderModule->initFromFile(filename);
  d3dShaderModule->initBindings(filename + ".hlsl.map");
  return d3dShaderModule;
}

unique_ptr<VertexShaderModule>
VertexShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<D3DVertexShaderModule>(device, filename);
}

unique_ptr<FragmentShaderModule>
FragmentShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<D3DFragmentShaderModule>(device, filename);
}

unique_ptr<ComputeShaderModule>
ComputeShaderModule::create(Device *device, const std::string &filename) {
  return createShaderModule<D3DComputeShaderModule>(device, filename);
}
