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

#include "ngfx/porting/metal/MTLShaderModule.h"
#include "ngfx/porting/metal/MTLDevice.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/File.h"
using namespace ngfx;
using namespace std;

void MTLShaderModule::initFromFile(id<MTLDevice> device, const std::string &filename) {
    File file;
#ifdef USE_PRECOMPILED_SHADERS
    file.read(filename + ".metallib");
    initFromByteCode(device, file.data.get(), file.size);
#else
    NGFX_ERR("TODO: Support runtime shader compilation");
#endif
}

void MTLShaderModule::initFromByteCode(id<MTLDevice> device, void* data, uint32_t size) {
    NSError* error;
    dispatch_data_t dispatch_data = dispatch_data_create(data, size, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    mtlLibrary = [device newLibraryWithData:dispatch_data error:&error];
    NSCAssert(mtlLibrary, @"Failed to load metal library");
    mtlFunction = [mtlLibrary newFunctionWithName:@"main0"];
}

template <typename T>
static std::unique_ptr<T> createShaderModule(Device* device, const std::string& filename) {
    auto mtlShaderModule = make_unique<T>();
    mtlShaderModule->initFromFile(mtl(device)->v, filename);
    mtlShaderModule->initBindings(filename+".metal.map");
    return mtlShaderModule;
}

unique_ptr<VertexShaderModule> VertexShaderModule::create(Device* device, const std::string& filename) {
    return createShaderModule<MTLVertexShaderModule>(device, filename);
}

unique_ptr<FragmentShaderModule> FragmentShaderModule::create(Device* device, const std::string& filename) {
    return createShaderModule<MTLFragmentShaderModule>(device, filename);
}

unique_ptr<ComputeShaderModule> ComputeShaderModule::create(Device* device, const std::string& filename) {
    return createShaderModule<MTLComputeShaderModule>(device, filename);
}
