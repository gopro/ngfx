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

#include "RegexUtil.h"
#include "ngfx/core/DebugUtil.h"
using namespace std;
using namespace ngfx;

static RegexUtil::Match toMatch(smatch &m) {
  RegexUtil::Match match;
  match.s.resize(m.size());
  for (uint32_t j = 0; j < m.size(); j++)
    match.s[j] = m.str(j);
  return match;
}

vector<RegexUtil::Match> RegexUtil::findAll(const regex &p, string contents) {
  vector<Match> matches;
  smatch m;
  while (regex_search(contents, m, p)) {
    matches.push_back(toMatch(m));
    contents = m.suffix().str();
  }
  return matches;
}
