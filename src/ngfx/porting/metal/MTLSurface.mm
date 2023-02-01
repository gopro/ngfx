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

#include "MTLSurface.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;

Surface* Surface::createFromWindowHandle(uint32_t w, uint32_t h, void* handle) {
	NGFX_ERR("not supported");
	return nullptr;
}

Surface *Surface::create() {
    MTLSurface *surface = new MTLSurface();
    surface->create(0, 0);
    return surface;
}

Surface *Surface::create(uint32_t w, uint32_t h, bool offscreen) {
    MTLSurface *surface = new MTLSurface();
    surface->create(w, h, offscreen);
    return surface;
}

void MTLSurface::create(uint32_t w, uint32_t h, bool offscreen) {
    this->w = w;
    this->h = h;
    this->offscreen = offscreen;
}
