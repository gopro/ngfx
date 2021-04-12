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
#include "ngfx/regex/RegexUtil.h"
#include <ctime>
#include <json.hpp>
#include <map>
#include <regex>
#include <shaderc/shaderc.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;

namespace ngfx {
class ShaderTools {
public:
  ShaderTools(bool verbose = false);
  enum { PATCH_SHADER_LAYOUTS_GLSL = 1, REMOVE_UNUSED_VARIABLES = 2, FLIP_VERT_Y = 4 };
  enum Format { FORMAT_GLSL, FORMAT_HLSL, FORMAT_MSL };
  struct MacroDefinition {
    std::string name, value;
  };
  typedef std::vector<MacroDefinition> MacroDefinitions;
  std::vector<std::string> compileShaders(const std::vector<std::string> &files,
                                          std::string outDir,
                                          Format fmt = FORMAT_GLSL,
                                          const MacroDefinitions &defines = {},
                                          int flags = 0);
  std::vector<std::string> convertShaders(const std::vector<std::string> &files,
                                          std::string outDir, Format fmt);
  std::vector<std::string>
  generateShaderMaps(const std::vector<std::string> &files, std::string outDir,
                     Format fmt);

private:
  void applyPatches(const std::vector<std::string> &patchFiles,
                    std::string outDir);
  int cmd(std::string str);
  int compileShaderGLSL(const std::string &src, shaderc_shader_kind shaderKind,
                        const MacroDefinitions &defines, std::string &spv,
                        bool verbose = true,
                        shaderc_optimization_level optimizationLevel =
                            shaderc_optimization_level_performance);
  int compileShaderGLSL(std::string filename, const MacroDefinitions &defines,
                        const std::string &outDir,
                        std::vector<std::string> &outFiles, int flags = 0);
  int compileShaderHLSL(const std::string &file,
                        const MacroDefinitions &defines, std::string outDir,
                        std::vector<std::string> &outFiles);
  int compileShaderMSL(const std::string &file, const MacroDefinitions &defines,
                       std::string outDir, std::vector<std::string> &outFiles);
  int convertSPVToGLSL(const std::string &spv, shaderc_shader_kind shaderKind,
                       std::string &glsl, int flags = 0);
  int convertSPVToHLSL(const std::string &spv, shaderc_shader_kind shaderKind,
                       std::string &hlsl, uint32_t shaderModel = 60);
  int convertSPVToMSL(const std::string &spv, shaderc_shader_kind shaderKind,
                      std::string &msl);
  int convertShader(const std::string &file, const std::string &extraArgs,
                    std::string outDir, Format fmt,
                    std::vector<std::string> &outFiles);
  bool findIncludeFile(const std::string &includeFilename,
                       const std::vector<std::string> &includePaths,
                       std::string &includeFile);
  struct MetalReflectData {
    std::vector<RegexUtil::Match> attributes, buffers, textures;
  };
  struct HLSLReflectData {
    std::vector<RegexUtil::Match> attributes, buffers, textures;
  };
  bool
  findMetalReflectData(const std::vector<RegexUtil::Match> &metalReflectData,
                       const std::string &name, RegexUtil::Match &match);
  int genShaderReflectionGLSL(const std::string &glsl, const std::string &ext,
                              const std::string &spv, std::string &glslMap);
  int genShaderReflectionHLSL(const std::string &hlsl, const std::string &ext,
                              const std::string &spv, std::string &hlslMap);
  int genShaderReflectionMSL(const std::string &msl, const std::string &ext,
                             const std::string &spv, std::string &mslMap);
  int generateShaderMapGLSL(const std::string &file, std::string outDir,
                            std::vector<std::string> &outFiles);
  int generateShaderMapHLSL(const std::string &file, std::string outDir,
                            std::vector<std::string> &outFiles);
  int generateShaderMapMSL(const std::string &file, std::string outDir,
                           std::vector<std::string> &outFiles);
  std::string parseReflectionData(const json &reflectData, std::string ext);
  int patchShaderReflectionDataMSL(const std::string &glslReflect,
                                   const std::string &ext,
                                   const std::string &msl,
                                   std::string &mslReflect);
  int patchShaderReflectionDataHLSL(const std::string &glslReflect,
                                    const std::string &ext,
                                    const std::string &hlsl,
                                    std::string &hlslReflect);
  int patchShaderLayoutsGLSL(const std::string &src, std::string &dst);
  int preprocess(const std::string &src, const std::string &dataPath,
                 std::string &dst);
  int removeUnusedVariablesGLSL(const std::string &src,
                                shaderc_shader_kind shaderKind,
                                const MacroDefinitions &defines,
                                std::string &dst);
  bool verbose = false;
  std::vector<std::string> defaultIncludePaths;
};
}; // namespace ngfx
