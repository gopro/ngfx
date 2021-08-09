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

/** \class ShaderTools
 *
 *  This class supports shader compilation and conversion, along with with reflection.
 *  It uses shaderc to compile GLSL files to SPIRV, and spirv-cross to convert SPIRV to HLSL / MSL,
 *  and also uses spirv-cross to do shader reflection.
 */

class ShaderTools {
public:
  ShaderTools(bool verbose = false);
  enum { 
      /**
      * Patch the descriptor layout definitions.
      * For example: convert 
            layout (binding = 0) in vec3 position;
            layout (binding = 1) in vec2 texcoord;
        to
            layout (set = 0, binding = 0) in vec3 position;
            layout (set = 1, binding = 0) in vec2 texcoord;
      */
      PATCH_SHADER_LAYOUTS_GLSL = 1,
      /**
      * Patch the HLSL descriptor layout definitions.
      * Use a different register space for each descriptor, with base register 0.
      * For example: convert
      *     register(t1)
      *     register(b0)
      * to
      *     register(t0, space0)
      *     register(b0, space1)
      */
      PATCH_SHADER_LAYOUTS_HLSL = 2,
      REMOVE_UNUSED_VARIABLES = 4, /*!< Remove unused input variables */
      FLIP_VERT_Y = 8 /*!< Flip vertex y output in NDC space */
  };
  enum Format { 
      FORMAT_GLSL, /*!< GLSL shading language input format */
      FORMAT_HLSL, /*!< HLSL shading language input format */
      FORMAT_MSL   /*!< MSL shading language input format */
  };
  struct MacroDefinition {
    std::string name, /*!< The macro name */
                value; /*!< The macro value */
  };
  typedef std::vector<MacroDefinition> MacroDefinitions; /*!< A collection of macro definitions */

  /** Compile shader files.
      If the output files already exist, and are newer than the input files, then this function 
      will skip re-compilation and return immediately.
   *  @param files The shader input files
   *  @param outDir The output directory
   *  @param fmt The shader input format
   *  @param defines The preprocessor macro definitions
   *  @param flags Additional compile flags
   *  @return The compiled shader filenames
   */
  std::vector<std::string> compileShaders(const std::vector<std::string> &files,
                                          std::string outDir,
                                          Format fmt = FORMAT_GLSL,
                                          const MacroDefinitions &defines = {},
                                          int flags = 0);
  /** Convert the SPIRV bytecode files to shaders.
      If the output files already exist, and are newer than the input files, then this function
      will skip conversion and return immediately.
   *  @param files The SPIRV bytecode input files
   *  @param outDir The output directory
   *  @param fmt The shader output format
   *  @return The converted shader filenames
   */
  std::vector<std::string> convertShaders(const std::vector<std::string> &files,
                                          std::string outDir, Format fmt);
  /** Generate shader reflection maps
  *   If the output files already exist, and are newer than the input files, then this function
  *   will skip processing and return immediately.
  *   @param files The shader input files
  *   @param outDir The output directory
  *   @param fmt The shader input format
   */
  std::vector<std::string>
  generateShaderMaps(const std::vector<std::string> &files, std::string outDir,
                     Format fmt);

private:
  void applyPatches(const std::vector<std::string> &patchFiles,
                    std::string outDir);
  int cmd(std::string str);
  int compileShaderToSPV(
      const std::string& src,
      shaderc_source_language sourceLanguage,
      shaderc_shader_kind shaderKind,
      const MacroDefinitions& defines, std::string& spv, bool verbose = true,
      shaderc_optimization_level optimizationLevel = shaderc_optimization_level_performance,
      std::string parentPath = "");
  inline int compileShaderGLSL(const std::string& src, shaderc_shader_kind shaderKind,
          const MacroDefinitions& defines, std::string& spv,
          bool verbose = true,
          shaderc_optimization_level optimizationLevel =
          shaderc_optimization_level_performance) {
      return compileShaderToSPV(src, shaderc_source_language_glsl, shaderKind, defines, spv, verbose, optimizationLevel);
  }
  int compileShaderGLSL(std::string filename, const MacroDefinitions &defines,
                        const std::string &outDir,
                        std::vector<std::string> &outFiles, int flags = 0);
  int compileShaderHLSL(const std::string &file,
                        const MacroDefinitions &defines, std::string outDir,
                        std::vector<std::string> &outFiles, int flags = 0);
  int compileShaderMSL(const std::string &file, const MacroDefinitions &defines,
                       std::string outDir, std::vector<std::string> &outFiles, int flags = 0);
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
  int patchShaderLayoutsHLSL(const std::string& src, std::string& dst);
  int removeUnusedVariablesGLSL(const std::string &src,
                                shaderc_shader_kind shaderKind,
                                const MacroDefinitions &defines,
                                std::string &dst);
  bool verbose = false;
  std::vector<std::string> defaultIncludePaths;
};
}; // namespace ngfx
