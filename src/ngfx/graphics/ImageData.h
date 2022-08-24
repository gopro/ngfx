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
#include <string>

namespace ngfx {

/** \struct ImageData
 *
 *  This struct provides a container for storing image data
 */

struct ImageData {
    ImageData() {}
    /** Create ImageData struct and allocate the data
     *  @param w The image width
        @ param h The image height
        @param numChannels The number of color channels */
    ImageData(int w, int h, int numChannels = 4) : w(w), h(h), numChannels(numChannels) {
        size = w * h * numChannels;
        data = malloc(size);
    }
    /** Destroy the struct and free the data */
    ~ImageData() {
        free(data);
    }
    /** The image width */
    int w = 0,
    /** The image height */
        h = 0,
    /** The number of channels */
        numChannels = 4;
    /** The image data */
    void* data = nullptr;
    /** The data size (in bytes) */
    int size = 0;
};
} // namespace ngfx
