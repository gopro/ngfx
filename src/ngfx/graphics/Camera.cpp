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
#include "ngfx/graphics/Camera.h"
#include "ngfx/core/DebugUtil.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace ngfx;

void Camera::onKey(KeyCode code, InputAction action) {
}
void Camera::onScroll(double xOffset, double yOffset) {
    zoom += float(yOffset) / 10.0f;
}
void Camera::onCursorPos(double x, double y) {
    auto& s = inputState;
    if (s.state == 1) {
        s.x = x; s.y = y;
        s.state = 2;
    }
    else if (s.state == 2) {
        double dx = x - s.x, dy = y - s.y;
        yaw += float(radians(dx * 10.0f));
        pitch += float(radians(dy * 10.0f));
        s.x = x; s.y = y;
    }
}
void Camera::onMouseButton(MouseButton button, InputAction action) {
    auto& s = inputState;
    if (button == MOUSE_BUTTON_MIDDLE) {
        if (s.state == 0 && action == PRESS) s.state = 1;
        else if (s.state != 0 && action == RELEASE) s.state = 0;
    }
}

void Camera::update() {
    viewMat = translate(vec3(panX, panY, 0)) *
        translate(vec3(0, 0, zoom)) *
        yawPitchRoll(radians(yaw), radians(pitch), radians(roll));
}
