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
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <stdexcept>
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
//#define LOG_TO_DEBUG_CONSOLE

#if defined(WIN32) && defined(LOG_TO_DEBUG_CONSOLE)
#include <Windows.h>
inline void debugMessage(FILE* filenum, const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    OutputDebugStringA((buffer));
}
#define LOG_FN debugMessage
#else
#define LOG_FN fprintf
#endif

#define NGFX_LOG(fmt, ...) LOG_FN(stderr, fmt "\n", ##__VA_ARGS__)
#define NGFX_ERR(fmt, ...) \
{ \
    char buffer[4096]; \
    snprintf(buffer, sizeof(buffer), "ERROR: [%s][%s][%d] " fmt "\n", __FILE__, \
        __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); \
    LOG_FN(stderr, "%s", buffer); \
    throw std::runtime_error(buffer); \
}
#define NGFX_LOG_TRACE(fmt, ...)                                               \
  NGFX_LOG("[%s][%s][%d] " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__,       \
           ##__VA_ARGS__)
#define NGFX_TODO(fmt, ...)                                                    \
  NGFX_LOG("[%s][%s][%d] TODO: " fmt, __FILE__, __FUNCTION__, __LINE__,        \
           ##__VA_ARGS__)

struct DebugUtil {
  static inline void Exit(uint32_t code) {
      exit(code);
  };
};
