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
#include "FileUtil.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/File.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
using namespace std;
namespace fs = std::filesystem;
using namespace ngfx;
using namespace std::chrono;

bool FileUtil::getmtime(const string &filename, fs::file_time_type &mtime) {
  if (!fs::exists(filename)) {
    return false;
  }
  mtime = fs::last_write_time(filename);
  return true;
}

bool FileUtil::srcFileNewerThanOutFile(const string &srcFileName,
                                       const string &targetFileName) {
  fs::file_time_type srcTimeStamp, targetTimeStamp;
  getmtime(srcFileName, srcTimeStamp);
  if (!getmtime(targetFileName, targetTimeStamp))
    return true;
  if (srcTimeStamp > targetTimeStamp)
    return true;
  return false;
}

string FileUtil::tempDir() {
  return fs::canonical(fs::temp_directory_path()).string();
}

#define RETRY_WITH_TIMEOUT(fn, t) \
const uint32_t timeoutMs = t; \
auto t0 = system_clock::now(); \
std::string err = ""; \
while (true) { \
    try { \
        fn; \
        break; \
    } \
    catch (std::exception e) { \
        err = e.what(); \
        std::this_thread::sleep_for(milliseconds(10)); \
    } \
    auto t1 = system_clock::now(); \
    if (duration_cast<milliseconds>(t1 - t0).count() > timeoutMs) { \
        NGFX_ERR("%s: %s timeoutMS: %d", err.c_str(), path.c_str(), timeoutMs); \
    } \
}

bool FileUtil::exists(const fs::path& path) {
    bool r;
    RETRY_WITH_TIMEOUT(r = fs::exists(path), 3000);
    return r;
}

void FileUtil::remove(const fs::path& path) {
    RETRY_WITH_TIMEOUT(fs::remove(path), 3000);
}

FileUtil::Lock::Lock(const std::string &path, uint32_t timeoutMs)
    : lockPath(path + ".lock"), timeoutMs(timeoutMs) {
  fs::path fpath(path);
  auto t0 = system_clock::now();
  while (FileUtil::exists(lockPath)) {
    std::this_thread::sleep_for(milliseconds(10));
    auto t1 = system_clock::now();
    if (duration_cast<milliseconds>(t1 - t0).count() > timeoutMs) {
      NGFX_ERR("file locked: %s, timeoutMs: %d", path.c_str(), timeoutMs);
    }
  }
  writeFile(lockPath, "");
}
FileUtil::Lock::~Lock() {
    FileUtil::remove(lockPath);
}

string FileUtil::readFile(const string &path) {
  File file;
  file.read(path);
  return string(file.data.get(), file.size);
}

void FileUtil::writeFile(const string &path, const string &contents) {
  ofstream out(path, ofstream::binary);
  assert(out);
  out.write(contents.data(), contents.size());
  out.close();
}

vector<string> FileUtil::splitExt(const string &filename) {
  auto it = filename.find_last_of('.');
  return {filename.substr(0, it), filename.substr(it)};
}

vector<string> FileUtil::findFiles(const string &path) {
  vector<string> files;
  for (auto &entry : fs::directory_iterator(path)) {
    fs::path filePath = entry.path();
    files.push_back(filePath.make_preferred().string());
  }
  return files;
}

vector<string> FileUtil::findFiles(const string &path, const string &ext) {
  vector<string> files;
  for (auto &entry : fs::directory_iterator(path)) {
    fs::path filePath = entry.path();
    if (filePath.extension() != ext)
      continue;
    files.push_back(filePath.make_preferred().string());
  }
  return files;
}

vector<string> FileUtil::filterFiles(const vector<string> &files,
                                     const string &fileFilter) {
  vector<string> filteredFiles;
  for (const string &file : files) {
    if (strstr(file.c_str(), fileFilter.c_str()))
      filteredFiles.push_back(file);
  }
  return filteredFiles;
}

vector<string> FileUtil::findFiles(const vector<string> &paths,
                                   const vector<string> &extensions) {
  vector<string> files;
  for (const string &path : paths) {
    for (const string &ext : extensions) {
      vector<string> filteredFiles = findFiles(path, ext);
      files.insert(files.end(), filteredFiles.begin(), filteredFiles.end());
    }
  }
  return files;
}

void FileUtil::copyFiles(const vector<string>& files,
    const string& outDir) {
    for (const string& file : files) {
        string filename = fs::path(file).filename().string();
        FileUtil::writeFile(fs::path(outDir + "/" + filename).string(), FileUtil::readFile(file));
    }
}
