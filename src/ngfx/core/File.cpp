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
#include "ngfx/core/File.h"
#include "ngfx/core/DebugUtil.h"
#include <cstdlib>
#include <fstream>
using namespace ngfx;

bool File::read(const std::string &filename) {
  std::ifstream in(filename.c_str(),
                   std::ios::binary | std::ios::in | std::ios::ate);
  if (!in.is_open()) {
      //NGFX_LOG("cannot open file: %s", filename.c_str());
      return false;
  }
  size = int(in.tellg());
  in.seekg(0, std::ios::beg);
  data.reset(new char[size]);
  in.read(data.get(), size);
  in.close();
  return true;
}
