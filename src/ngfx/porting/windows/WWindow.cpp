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
#include "ngfx/porting/windows/WWindow.h"
#include <windowsx.h>
#include "ngfx/core/StringUtil.h"
#include "ngfx/core/DebugUtil.h"
using namespace ngfx;
#define WINDOW_CLASS_NAME L"WWindow"

WWindow::Monitor WWindow::getPrimaryMonitor() {
    DWORD adapterIndex, displayIndex;
    DISPLAY_DEVICEW adapter, display;
    //get primary adapter
    for (adapterIndex = 0; ; adapterIndex++) {
        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);
        if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0))
            break;
        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;
        if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) break;
    }
    //get primary display
    for (displayIndex = 0; ; displayIndex++) {
        ZeroMemory(&display, sizeof(display));
        display.cb = sizeof(display);
        if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0))
            break;
        if (display.StateFlags & DISPLAY_DEVICE_ACTIVE) break;
    }
    return { adapter, display };
}

WWindow::VideoMode WWindow::getVideoMode(const Monitor& monitor) {
    DEVMODEW dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsW(monitor.adapter.DeviceName, ENUM_CURRENT_SETTINGS, &dm);
    return { int(dm.dmPelsWidth), int(dm.dmPelsHeight) };
}

LRESULT windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WWindow* thiz = reinterpret_cast<WWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (message) {
    case WM_CREATE: {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        return 0;
    }
    case WM_KEYDOWN: { 
        if (thiz->onKey) thiz->onKey(KeyCode(wParam), PRESS); return 0; 
    }
    case WM_KEYUP: { 
        if (thiz->onKey) thiz->onKey(KeyCode(wParam), RELEASE); return 0; 
    }
    case WM_MOUSEWHEEL: { 
        if (thiz->onScroll) thiz->onScroll(0.0, GET_WHEEL_DELTA_WPARAM(wParam) / double(WHEEL_DELTA)); return 0;
    }
    case WM_LBUTTONDOWN: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_LEFT, InputAction(PRESS)); return 0; }
    case WM_MBUTTONDOWN: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_MIDDLE, InputAction(PRESS)); return 0; }
    case WM_RBUTTONDOWN: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_RIGHT, InputAction(PRESS)); return 0; }
    case WM_LBUTTONUP: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_LEFT, InputAction(RELEASE)); return 0; }
    case WM_MBUTTONUP: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_MIDDLE, InputAction(RELEASE)); return 0; }
    case WM_RBUTTONUP: { if (thiz->onMouseButton) thiz->onMouseButton(MOUSE_BUTTON_RIGHT, InputAction(RELEASE)); return 0; }
    case WM_MOUSEMOVE: {
        if (thiz->onCursorPos) thiz->onCursorPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }
    case WM_PAINT: {
        thiz->onUpdate();
        thiz->onPaint();
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void WWindow::registerWindowClass() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)windowProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIcon = HICON(LoadImageW(NULL, IDI_APPLICATION, IMAGE_ICON,
            0, 0, LR_DEFAULTSIZE | LR_SHARED));
    if (!RegisterClassExW(&wc)) NGFX_ERR("");
}

void WWindow::createWindow(int w, int h, const char* title) {
    const wchar_t* wTitle = StringUtil::toWString(title).c_str();
    auto& hWnd = d3dSurface.v;
    RECT rect = { 0, 0, w, h };
    DWORD style = WS_POPUP; // WS_OVERLAPPED_WINDOW;
    AdjustWindowRect(&rect, style, FALSE);
    hWnd = CreateWindow(
        WINDOW_CLASS_NAME, wTitle,
        style, 
        0, 0, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, 
        GetModuleHandleW(NULL), 
        this);
    assert(hWnd);
    ShowWindow(hWnd, SW_SHOWNA);
}

void WWindow::getFramebufferSize(int* w, int* h) {
    RECT area;
    auto& hWnd = d3dSurface.v;
    GetClientRect(hWnd, &area);
    if (w) *w = area.right;
    if (h) *h = area.bottom;
}
void WWindow::create(GraphicsContext* graphicsContext, const char* title, 
		std::function<void(Window * thiz)> onWindowCreated, int w, int h) {
    this->w = w; this->h = h;
    auto monitor = getPrimaryMonitor();
    auto videoMode = getVideoMode(monitor);
    if (w == Window::DISPLAY_WIDTH) w = videoMode.width;
    if (h == Window::DISPLAY_HEIGHT) h = videoMode.height;
    registerWindowClass();
    createWindow(w, h, title);
    getFramebufferSize(&w, &h);
    this->w = w; this->h = h;
    d3dSurface.w = w;
    d3dSurface.h = h;
    surface = &d3dSurface;
    onWindowCreated(this);
}

bool WWindow::shouldClose() { 
    return (msg.message == WM_QUIT);
}

void WWindow::pollEvents() {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
WWindow::~WWindow() {}
void WWindow::destroySurface() {}

Window* Window::create(GraphicsContext* graphicsContext, const char* title,
        std::function<void(Window * thiz)> onWindowCreated, int w, int h) {
    auto window = new WWindow();
    window->create(graphicsContext, title, onWindowCreated, w, h);
    return window;
}
