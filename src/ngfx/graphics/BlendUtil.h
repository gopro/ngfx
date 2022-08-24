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
#pragma once
#include "GraphicsCore.h"

namespace ngfx {
	struct BlendParams;
	enum BlendMode {
		DARKEN, LIGHTEN, MULTIPLY, SCREEN, OVERLAY,
		SRC, SRC_OVER, SRC_IN, DST, DST_IN, CLEAR, SRC_OUT, DST_OUT, DST_OVER
	};
	/** \struct BlendUtil
 *
 *  This module provides various blending utility functions
 */
	struct BlendUtil {
		/** Get blend parameters for a given porter duff blend mode */
		static BlendParams getBlendParams(BlendMode mode);
		/** Convert blend mode string to enum */
		static BlendMode toBlendMode(std::string str);
		/** Convert blend mode enum to string */
		static std::string toString(BlendMode blendMode);
	};
}