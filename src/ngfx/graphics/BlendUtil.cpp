/*
 * Copyright 2022 GoPro Inc.
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
#include "BlendUtil.h"
#include "ngfx/core/HashUtil.h"
#include "ngfx/graphics/GraphicsPipeline.h"
#include <algorithm>
#include <map>
using namespace ngfx;
using namespace std;
#define PD(x,y) { x, y, x, y, BLEND_OP_ADD, BLEND_OP_ADD }

BlendParams BlendUtil::getBlendParams(BlendMode mode) {
	const map<BlendMode, BlendParams> blendParamsMap = {
		{ DARKEN, { /* TODO */ }},
		{ LIGHTEN, { /* TODO */ }},
		{ MULTIPLY, { /* TODO */ }},
		{ SCREEN, { /* TODO */ }},
		{ OVERLAY, { /* TODO */ }},
		{ SRC, PD(BLEND_FACTOR_ONE, BLEND_FACTOR_ZERO) },
		{ SRC_OVER, PD(BLEND_FACTOR_ONE, BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) },
		{ SRC_IN, PD(BLEND_FACTOR_DST_ALPHA, BLEND_FACTOR_ZERO) },
		{ DST, PD(BLEND_FACTOR_ZERO, BLEND_FACTOR_ONE) },
		{ DST_IN, PD(BLEND_FACTOR_ZERO, BLEND_FACTOR_SRC_ALPHA) },
		{ CLEAR, PD(BLEND_FACTOR_ZERO, BLEND_FACTOR_ZERO) },
		{ SRC_OUT, PD(BLEND_FACTOR_ONE_MINUS_DST_ALPHA, BLEND_FACTOR_ZERO) },
		{ DST_OUT, PD(BLEND_FACTOR_ZERO, BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) },
		{ DST_OVER, PD(BLEND_FACTOR_ONE_MINUS_DST_ALPHA, BLEND_FACTOR_ONE) }
	};
	return blendParamsMap.at(mode);
}

static const vector<string> BlendModeStr = { "darken", "lighten", "multiply", "screen", "overlay",
"src", "src_over", "src_in", "dst", "dst_in", "clear", "src_out", "dst_out", "dst_over" };

BlendMode BlendUtil::toBlendMode(string str) {
	return BlendMode(find(BlendModeStr.begin(), BlendModeStr.end(), str) - BlendModeStr.begin());
}

string BlendUtil::toString(BlendMode blendMode) {
	return BlendModeStr[blendMode];
}
