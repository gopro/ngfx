/*
 * Copyright 2020-2022 GoPro Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include "RDGPUCapture.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

void RDGPUCapture::create() {
#ifdef _WIN32
    mod = LoadLibraryA("renderdoc.dll");
    if (!mod) {
        NGFX_ERR("Could not load renderdoc.dll");
    }
    pRENDERDOC_GetAPI RENDERDOC_GetAPI =
        (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
#else
    mod = dlopen("librenderdoc.so", RTLD_LAZY);
    if (!mod) {
        NGFX_ERR("Could not load renderdoc.so: %s", dlerror());
    }
    pRENDERDOC_GetAPI RENDERDOC_GetAPI =
        (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
#endif
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, (void**)&rdApi);
    if (ret == 0) {
        NGFX_ERR("Could not initialize renderdoc");
    }
    NGFX_LOG("renderdoc capture path: %s", rdApi->GetCaptureFilePathTemplate());
}

void RDGPUCapture::begin()
{
    rdApi->StartFrameCapture(NULL, NULL);
}

void RDGPUCapture::end()
{
    int ret = rdApi->EndFrameCapture(NULL, NULL);
    if (ret == 0) {
        NGFX_ERR("End frame capture failed");
    }
}

RDGPUCapture::~RDGPUCapture() {
    if (mod)
#ifdef _WIN32
        FreeLibrary(mod);
#else
        dlclose(mod);
#endif
}

GPUCapture* GPUCapture::create() {
    RDGPUCapture* gpuCapture = new RDGPUCapture();
    gpuCapture->create();
    return gpuCapture;
}
