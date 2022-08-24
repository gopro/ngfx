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
#include "ImageData.h"
namespace ngfx {

/** \class ImageUtil
 *
 *  This class provides various functionality to import and export 
 *  images as JPEGs and PNGs.  JPEG is a lossy format best suited for 
 *  photos, while PNG is a lossless format best suited for computer graphics content.
 */

class ImageUtil {
public:
  /** Load the image from file into an ImageData structure */
  static void load(std::string filename, ImageData &v);
  /** Store an image as a JPEG to file */
  static void storeJPEG(std::string filename, const ImageData& v, int quality = 90);
  /** Store an image as a PNG to file */
  static void storePNG(std::string filename, const ImageData &v);
};
} // namespace ngfx
