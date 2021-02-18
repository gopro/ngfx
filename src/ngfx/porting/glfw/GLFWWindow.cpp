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
#include "ngfx/porting/glfw/GLFWWindow.h"
#include "ngfx/input/InputMap.h"
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
#include "ngfx/porting/vulkan/VKGraphicsContext.h"
#include "ngfx/porting/vulkan/VKDebugUtil.h"
#endif
using namespace ngfx;

static void glfwOnError(int errorCode, const char * msg) {
    NGFX_LOG("[%d]: %s", errorCode, msg);
}
static void glfwOnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFWWindow* thiz = (GLFWWindow*)glfwGetWindowUserPointer(window);
    if (thiz->onKey) thiz->onKey(ngfx::KeyCode(key), InputAction(action));
}

static void glfwOnScroll(GLFWwindow* window, double xoffset, double yoffset) {
    GLFWWindow* thiz = (GLFWWindow*)glfwGetWindowUserPointer(window);
    if (thiz->onScroll) thiz->onScroll(xoffset, yoffset);
}

static void glfwOnCursorPos(GLFWwindow* window, double x, double y) {
    GLFWWindow* thiz = (GLFWWindow*)glfwGetWindowUserPointer(window);
    if (thiz->onCursorPos) thiz->onCursorPos(x, y);
}

static void glfwOnMouseButton(GLFWwindow* window, int button, int action, int mods) {
    GLFWWindow* thiz = (GLFWWindow*)glfwGetWindowUserPointer(window);
    if (thiz->onMouseButton) thiz->onMouseButton(MouseButton(button), InputAction(action));
}

void GLFWWindow::create(GraphicsContext* graphicsContext, const char* title, 
        std::function<void(Window* thiz)> onWindowCreated, int w, int h) {
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
    this->instance = vk(graphicsContext)->vkInstance.v;
    VkResult vkResult;
#endif
    glfwSetErrorCallback(glfwOnError);
    int result = glfwInit();
    if (result != GLFW_TRUE) NGFX_ERR("glfwInit failed");
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    auto monitor = glfwGetPrimaryMonitor();
    auto videoMode = glfwGetVideoMode(monitor);
    if (w == Window::DISPLAY_WIDTH) w = videoMode->width;
    if (h == Window::DISPLAY_HEIGHT) h = videoMode->height;
    v = glfwCreateWindow(w, h, title, nullptr, nullptr);
    glfwGetFramebufferSize(v, &w, &h);
    glfwSetWindowUserPointer(v, this);
    glfwSetKeyCallback(v, ::glfwOnKey);
    glfwSetScrollCallback(v, ::glfwOnScroll);
    glfwSetCursorPosCallback(v, ::glfwOnCursorPos);
    glfwSetMouseButtonCallback(v, ::glfwOnMouseButton);
    this->w = w; this->h = h;
#ifdef NGFX_GRAPHICS_BACKEND_VULKAN    
    V(glfwCreateWindowSurface(instance, v, NULL, &vkSurface.v));
    vkSurface.instance = instance;
    vkSurface.w = w; vkSurface.h = h;
    surface = &vkSurface;
#endif
    onWindowCreated(this);
}

GLFWWindow::~GLFWWindow() {}

ngfx::Window* ngfx::Window::create(GraphicsContext* graphicsContext, const char* title, std::function<void(ngfx::Window* thiz)> setWindow, int w, int h) {
    auto window = new GLFWWindow();
    window->create(graphicsContext, title, setWindow, w, h);
    return window;
}

bool GLFWWindow::shouldClose() {
    return glfwWindowShouldClose(v);
}
void GLFWWindow::pollEvents() {
    glfwPollEvents();
    onUpdate();
    onPaint();
}

