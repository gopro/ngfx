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
#include <GLFW/glfw3.h>

#define GLFW(v) v = GLFW_##v

namespace ngfx {
	enum KeyCode {
		GLFW(KEY_UP), 
		GLFW(KEY_DOWN), 
		GLFW(KEY_LEFT), 
		GLFW(KEY_RIGHT)
	};
	enum InputAction {
		GLFW(RELEASE),
		GLFW(PRESS)
	};
	enum MouseButton {
		GLFW(MOUSE_BUTTON_LEFT),
		GLFW(MOUSE_BUTTON_MIDDLE),
		GLFW(MOUSE_BUTTON_RIGHT)
	};
}