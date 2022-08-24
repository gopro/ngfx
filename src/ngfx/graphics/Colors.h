/*
 * Copyright 2022 GoPro Inc.
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
#include <glm/glm.hpp>

namespace ngfx {

/** \struct Color
    *
    *  This module defines various colors as RGBA float values
    */

struct Color {
    static constexpr glm::vec4 Transparent = { 0, 0, 0, 0 };
    static constexpr glm::vec4 Black = { 0, 0, 0, 1 };
    static constexpr glm::vec4 Gray = { 0.5f, 0.5f, 0.5f, 1 };
    static constexpr glm::vec4 White = { 1, 1, 1, 1 };
    static constexpr glm::vec4 Red = { 1, 0, 0, 1 };
    static constexpr glm::vec4 Green = { 0, 1, 0, 1 };
    static constexpr glm::vec4 Blue = { 0, 0, 1, 1 };
    static constexpr glm::vec4 Yellow = { 1, 1, 0, 1 };
    static constexpr glm::vec4 Cyan = { 0, 1, 1, 1 };
    static constexpr glm::vec4 Magenta = { 1, 0, 1, 1 };
};
}; // namespace ngfx
