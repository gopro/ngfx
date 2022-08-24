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

namespace ngfx {
class Framebuffer;
/** \class RenderPass
   *
   *  This class defines the base class for a render pass 
   *  which allows rendering to an onscreen or offscreen surface 
   *  via a framebuffer object.
   *  The user will typically call the getRenderPass member function 
   *  in the graphics context to get a render pass object corresponding 
   *  to a render pass configuration
   *  
   */
class RenderPass {
public:
  virtual ~RenderPass() {}
  /** The current framebuffer object */
  Framebuffer *currentFramebuffer = nullptr;
};
}; // namespace ngfx
