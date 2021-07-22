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
#include "ngfx/core/DebugUtil.h"
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

namespace ngfx {
class FileUtil {
public:
  static bool getmtime(const std::string &filename,
                       std::filesystem::file_time_type &mtime);
  static bool srcFileNewerThanOutFile(const std::string &srcFileName,
                                      const std::string &targetFileName);
  static std::string tempDir();
  struct Lock {
    Lock(const std::string &path, uint32_t timeoutMs = 3000);
    ~Lock();
    std::string lockPath;
    uint32_t timeoutMs;
  };
  static std::string readFile(const std::string &path);
  static void writeFile(const std::string &path, const std::string &contents);
  static std::vector<std::string> splitExt(const std::string &filename);
  static std::vector<std::string> findFiles(const std::string &path);
  static std::vector<std::string> findFiles(const std::string &path,
                                            const std::string &ext);
  static std::vector<std::string>
  filterFiles(const std::vector<std::string> &files,
              const std::string &fileFilter);
  static std::vector<std::string>
  findFiles(const std::vector<std::string> &paths,
            const std::vector<std::string> &extensions);
  static void copyFiles(const std::vector<std::string>& files,
      const std::string& outDir);
};
}; // namespace ngfx
