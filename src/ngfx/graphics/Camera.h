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
#include "ngfx/input/InputListener.h"
#include <glm/glm.hpp>
using namespace glm;

/** \class Camera
 * 
 *  This class supports 2D and 3D camera operations.
 *  It supports, panning, zooming, and rotation.
 *  It also supports user input via keyboard, mouse, and touch events.
 */

namespace ngfx {
class Camera : public InputListener {
public:
  /** Destroy the camera */
  virtual ~Camera() {}
  /** Handle key press input event 
   *  @param keyCode The key code 
   *  @param keyAction The key action */
  void onKey(KeyCode keyCode, InputAction keyAction) override;
  /** Handle scroll input event
   *  @param xoffset The relative x offset
   *  @param yoffset The relative y offset */
  void onScroll(double xoffset, double yoffset) override;
  /** Handle cursor move event
   *  @param x The relative x movement
   *  @param y The relative y movement
   */
  void onCursorPos(double x, double y) override;
  /** Handle mouse button event
   *  @param button The mouse button
   *  @param action The mouse button event
   */
  void onMouseButton(MouseButton button, InputAction action) override;
  /** Update camera parameters
   *  This function is called once per frame
   */
  void update();
  float panX = 0.0f, /**< The camera pan along the x axis */
        panY = 0.0f, /**< The camera pan along the y axis */ 
        zoom = 0.0f, /**< The camera zoom */
        yaw = 0.0f, /**< The camera yaw (radians) */ 
        pitch = 0.0f, /**< The camera pitch (radians) */
        roll = 0.0f; /**< The camera roll (radians) */
  /**< The camera view matrix */
  mat4 viewMat;

private:
  struct {
    uint32_t state = 0;
    double x = -1.0, y = -1.0;
  } inputState; /**< The current input state */
};
}; // namespace ngfx
