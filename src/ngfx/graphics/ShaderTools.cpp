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

#include "ShaderTools.h"
#include "ngfx/core/DebugUtil.h"
#include "ngfx/core/FileUtil.h"
#include "ngfx/core/StringUtil.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_hlsl.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <sstream>
using namespace std;
using namespace ngfx;
static auto readFile = FileUtil::readFile;
static auto writeFile = FileUtil::writeFile;
static auto toLower = StringUtil::toLower;
namespace fs = std::filesystem;
#define V(func)                                                                \
  {                                                                            \
    ret = func;                                                                \
    if (ret != 0)                                                              \
      return ret;                                                              \
  }

#ifdef _WIN32
#define PATCH string("patch.exe")
#else
#define PATCH string("patch")
#endif

static string getEnv(const string &name) {
  char *value = getenv(name.c_str());
  return (value ? value : "");
}
static json *getEntry(const json &data, const string &key) {
  auto it = data.find(key);
  if (it == data.end())
    return nullptr;
  return (json *)&it.value();
}
ShaderTools::ShaderTools(bool verbose) : verbose(verbose) {
  defaultIncludePaths = {"ngfx/data/shaders", "nodegl/data/shaders"};
}

int ShaderTools::cmd(string str) {
  if (verbose) {
    NGFX_LOG(">> %s", str.c_str());
  } else
    str += " >> /dev/null 2>&1";
  return system(str.c_str());
}

class FileIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    FileIncluder(const vector<string>& includePaths)
        : includePaths(includePaths) {}
    ~FileIncluder() override {}
    shaderc_include_result* GetInclude(const char* requested_source,
        shaderc_include_type type,
        const char* requesting_source,
        size_t include_depth) override {
        auto r = new shaderc_include_result;
        string contents = readFile(fs::path(includePaths[0] + "/" + requested_source).string());
        r->source_name = strdup(requested_source);
        r->source_name_length = strlen(requested_source);
        r->content = strdup(contents.c_str());
        r->content_length = contents.size();
        r->user_data = nullptr;
        return r;
    }
    void ReleaseInclude(shaderc_include_result* r) override {
        free((void*)r->source_name);
        free((void*)r->content);
        free(r);
    }
    vector<string> includePaths;
};

int ShaderTools::compileShaderToSPV(
    const string &src,
    shaderc_source_language sourceLanguage,
    shaderc_shader_kind shaderKind,
    const MacroDefinitions &defines, string &spv, bool verbose,
    shaderc_optimization_level optimizationLevel,
    std::string parentPath) {
  shaderc::Compiler compiler;
  shaderc::CompileOptions compileOptions;
  for (const MacroDefinition &define : defines) {
    compileOptions.AddMacroDefinition(define.name, define.value);
  }
  compileOptions.SetOptimizationLevel(optimizationLevel);
  compileOptions.SetGenerateDebugInfo();
  compileOptions.SetSourceLanguage(sourceLanguage);
  vector<string> includePaths = { parentPath };
  auto fileIncluder = make_unique<FileIncluder>(includePaths);
  compileOptions.SetIncluder(std::move(fileIncluder));
  auto preprocessResult = compiler.PreprocessGlsl(src, shaderKind, "", compileOptions);
  if (preprocessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
      NGFX_ERR("cannot preprocess file: %s", preprocessResult.GetErrorMessage().c_str());
      return 1;
  }
  string preprocessedSrc(preprocessResult.begin());
  auto result = compiler.CompileGlslToSpv(preprocessedSrc, shaderKind, "", compileOptions);
  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    NGFX_ERR("cannot compile file: %s", result.GetErrorMessage().c_str());
    return 1;
  }
  spv = string((const char *)result.cbegin(),
               sizeof(uint32_t) * (result.cend() - result.cbegin()));
  return 0;
}

int ShaderTools::patchShaderLayoutsGLSL(const string &src, string &dst) {
  dst = "";
  istringstream sstream(src);
  string line;
  while (std::getline(sstream, line)) {
    // Patch GLSL shader layouts
    smatch g;

    bool matchLayout = regex_search(line, g,
                                    regex("^(.*)"
                                          "layout\\s*\\("
                                          "([^)]*)"
                                          "binding[\\s]*=[\\s]*"
                                          "([\\d]+)"
                                          "([^)]*)"
                                          "\\)"
                                          "(.*)\r*$"));
    if (matchLayout) {
      dst += g[1].str() + "layout(" + g[2].str() + "set = " + g[3].str() +
             ", binding = 0" + g[4].str() + ")" + g[5].str() + "\n";
    } else {
      dst += line + "\n";
    }
  }
  return 0;
}

int ShaderTools::patchShaderLayoutsHLSL(const string& src, string& dst) {
    dst = "";
    istringstream sstream(src);
    string line;
    int registerSpace = 0;
    while (std::getline(sstream, line)) {
        // Patch HLSL shader layouts
        smatch g;

        bool matchLayout = regex_search(line, g,
            regex("^(.*)"
                "register\\s*\\("
                "\\s*"
                "([bstu])"
                "\\d"
                "\\s*"
                "\\)"
                "(.*)\r*$"));
        if (matchLayout) {
           dst += g[1].str() + "register(" + g[2].str() + "0,space" +std::to_string(registerSpace++) + ")" + g[3].str() + "\n";
        }
        else {
            dst += line + "\n";
        }
    }
    return 0;
}

static shaderc_shader_kind toShaderKind(const string &ext) {
  static const map<string, shaderc_shader_kind> shaderKindMap = {
      {".vert", shaderc_vertex_shader},
      {".frag", shaderc_fragment_shader},
      {".comp", shaderc_compute_shader}};
  return shaderKindMap.at(ext);
}
int ShaderTools::compileShaderGLSL(string filename,
                                   const MacroDefinitions &defines,
                                   const string &outDir,
                                   vector<string> &outFiles, int flags) {
  string parentPath = fs::path(filename).parent_path().string();
  filename = fs::path(filename).filename().string();
  string inFileName =
      fs::path(parentPath + "/" + filename).make_preferred().string();
  string outFileName =
      fs::path(outDir + "/" + filename + ".spv").make_preferred().string();
  if (!FileUtil::srcFileNewerThanOutFile(inFileName, outFileName)) {
    outFiles.push_back(outFileName);
    return 0;
  }
  string src, dst;
  int ret = 0;

  src = FileUtil::readFile(inFileName);
  string ext = FileUtil::splitExt(inFileName)[1];
  shaderc_shader_kind shaderKind = toShaderKind(ext);
  if ((flags & REMOVE_UNUSED_VARIABLES) || (flags & FLIP_VERT_Y) ) {
    string spv;
    V(compileShaderGLSL(src, shaderKind, defines, spv, false));
    V(convertSPVToGLSL(spv, shaderKind, dst, flags));
    src = move(dst);
  }
  if (flags & PATCH_SHADER_LAYOUTS_GLSL) {
    V(patchShaderLayoutsGLSL(src, dst));
    src = move(dst);
  }
  V(compileShaderGLSL(src, shaderKind, defines, dst));
  writeFile(outFileName, dst);
  outFiles.push_back(outFileName);
  return 0;
}

int ShaderTools::compileShaderMSL(const string &file,
                                  const MacroDefinitions &defines,
                                  string outDir, vector<string> &outFiles, int flags) {
  string strippedFilename =
      FileUtil::splitExt(fs::path(file).filename().string())[0];
  string inFileName = fs::path(outDir + "/" + strippedFilename + ".metal")
                          .make_preferred()
                          .string();
  string outFileName = fs::path(outDir + "/" + strippedFilename + ".metallib")
                           .make_preferred()
                           .string();
  if (!FileUtil::srcFileNewerThanOutFile(inFileName, outFileName)) {
    outFiles.push_back(outFileName);
    return 0;
  }

  string debugFlags = ""; //-gline-tables-only -MO";
  int result = cmd("xcrun -sdk macosx metal " + debugFlags + " -c " +
                   inFileName + " -o " + outDir + "/" + strippedFilename +
                   ".air && "
                   "xcrun -sdk macosx metallib " +
                   outDir + "/" + strippedFilename + ".air -o " + outFileName);
  if (result == 0)
    NGFX_LOG("compiled file: %s", file.c_str());
  else
    NGFX_ERR("cannot compile file: %s", file.c_str());
  outFiles.push_back(outFileName);
  return result;
}

int ShaderTools::compileShaderHLSL(const string &file,
                                   const MacroDefinitions &defines,
                                   string outDir, vector<string> &outFiles, int flags) {
  string filename = fs::path(file).filename().string();
  string inFileName =fs::path(file).make_preferred().string();
  string outFileName = fs::path(outDir + "/" + filename + ".dxc").make_preferred().string();
  if (!FileUtil::srcFileNewerThanOutFile(inFileName, outFileName)) {
    outFiles.push_back(outFileName);
    return 0;
  }
  int ret = 0;
  if (flags & PATCH_SHADER_LAYOUTS_HLSL) {
      const string &src = FileUtil::readFile(inFileName);
      string dst;
      V(patchShaderLayoutsHLSL(src, dst));
      inFileName += ".tmp";
      FileUtil::writeFile(inFileName, dst);
  }

  string shaderModel = "";
  if (strstr(inFileName.c_str(), ".vert") || strstr(inFileName.c_str(), "_vertex"))
      shaderModel = "vs_6_0";
  else if (strstr(inFileName.c_str(), ".frag") || strstr(inFileName.c_str(), "_fragment"))
      shaderModel = "ps_6_0";
  else if (strstr(inFileName.c_str(), ".comp") || strstr(inFileName.c_str(), "_compute"))
      shaderModel = "cs_6_0";
  const char* dxc_path_env = getenv("DXC_PATH");
  string dxc_path = dxc_path_env ? std::string(dxc_path_env) : "dxc.exe";
  int result = cmd(dxc_path +" /T " + shaderModel + " /Fo " + outFileName + " - D DIRECT3D12 " +
                   inFileName + " -O3 -all-resources-bound -Fc " + outFileName + ".info");
  if (flags & PATCH_SHADER_LAYOUTS_HLSL) {
      fs::remove(inFileName);
  }
  if (result == 0)
    NGFX_LOG("compiled file: %s", file.c_str());
  else
    NGFX_ERR("cannot compile file: %s", file.c_str());
  outFiles.push_back(outFileName);
  return result;
}

int ShaderTools::convertSPVToGLSL(const std::string &spv,
                                 shaderc_shader_kind shaderKind,
                                 std::string &glsl, int flags) {
  auto compilerGLSL = make_unique<spirv_cross::CompilerGLSL>(
      (const uint32_t *)spv.data(), spv.size() / sizeof(uint32_t));
  if (flags & REMOVE_UNUSED_VARIABLES) {
    auto activeVariables = compilerGLSL->get_active_interface_variables();
    compilerGLSL->set_enabled_interface_variables(move(activeVariables));
  }
  auto opts = compilerGLSL->get_common_options();
  opts.vulkan_semantics = true;
  if (flags & FLIP_VERT_Y)
    opts.vertex.flip_vert_y = true;
  compilerGLSL->set_common_options(opts);
  glsl = compilerGLSL->compile();
  return 0;
}

int ShaderTools::convertSPVToMSL(const string &spv,
                                 shaderc_shader_kind shaderKind, string &msl) {
  auto compilerMSL = make_unique<spirv_cross::CompilerMSL>(
      (const uint32_t *)spv.data(), spv.size() / sizeof(uint32_t));
  msl = compilerMSL->compile();
  return 0;
}

int ShaderTools::convertSPVToHLSL(const string &spv,
                                  shaderc_shader_kind shaderKind, string &hlsl,
                                  uint32_t shaderModel) {
  auto compilerHLSL = make_unique<spirv_cross::CompilerHLSL>(
      (const uint32_t *)spv.data(), spv.size() / sizeof(uint32_t));
  auto options = compilerHLSL->get_hlsl_options();
  options.shader_model = shaderModel;
  compilerHLSL->set_hlsl_options(options);
  hlsl = compilerHLSL->compile();
  return 0;
}

int ShaderTools::convertShader(const string &file, const string &extraArgs,
                               string outDir, Format fmt,
                               vector<string> &outFiles) {
  auto splitFilename = FileUtil::splitExt(fs::path(file).filename().string());
  string strippedFilename = splitFilename[0];
  string ext = FileUtil::splitExt(strippedFilename)[1];
  string inFileName = fs::path(outDir + "/" + strippedFilename + ".spv")
                          .make_preferred()
                          .string();
  string outFileName = fs::path(outDir + "/" + strippedFilename +
                                (fmt == FORMAT_MSL ? ".metal" : ".hlsl"))
                           .make_preferred()
                           .string();
  if (!FileUtil::srcFileNewerThanOutFile(inFileName, outFileName)) {
    outFiles.push_back(outFileName);
    return 0;
  }
  string spv = FileUtil::readFile(inFileName), dst;
  int result;
  if (fmt == FORMAT_MSL) {
    result = convertSPVToMSL(spv, toShaderKind(ext), dst);
  } else {
    result = convertSPVToHLSL(spv, toShaderKind(ext), dst);
  }
  FileUtil::writeFile(outFileName, dst);
  string args =
      (fmt == FORMAT_MSL ? "--msl" : "--hlsl --shader-model 60") + extraArgs;
  if (result == 0)
    NGFX_LOG("converted file: %s to %s", inFileName.c_str(),
             outFileName.c_str());
  else
    NGFX_ERR("cannot convert file: %s", file.c_str());
  outFiles.push_back(outFileName);
  return result;
}

bool ShaderTools::findMetalReflectData(
    const vector<RegexUtil::Match> &metalReflectData, const string &name,
    RegexUtil::Match &match) {
  for (const RegexUtil::Match &data : metalReflectData) {
    if (data.s[2] == name) {
      match = data;
      return true;
    } else if (strstr(data.s[1].c_str(), name.c_str())) {
      match = data;
      return true;
    }
  }
  return false;
}

int ShaderTools::patchShaderReflectionDataMSL(const std::string &glslReflect,
                                              const std::string &ext,
                                              const std::string &msl,
                                              std::string &mslReflect) {
  auto glslReflectJson = json::parse(glslReflect);
  MetalReflectData metalReflectData;
  if (ext == ".vert") {
    metalReflectData.attributes =
        RegexUtil::findAll(regex("([^\\s]*)[\\s]*([^\\s]*)[\\s]*\\[\\["
                                 "attribute\\(([0-9]+)\\)\\]\\]"),
                           msl);
  }
  metalReflectData.buffers = RegexUtil::findAll(
      regex("([^\\s]*)[\\s]*([^\\s]*)[\\s]*\\[\\[buffer\\(([0-9]+)\\)\\]\\]"),
      msl);
  metalReflectData.textures = RegexUtil::findAll(
      regex("([^\\s]*)[\\s]*([^\\s]*)[\\s]*\\[\\[texture\\(([0-9]+)\\)\\]\\]"),
      msl);

  json *textures = getEntry(glslReflectJson, "textures"),
       *ubos = getEntry(glslReflectJson, "ubos"),
       *ssbos = getEntry(glslReflectJson, "ssbos"),
       *images = getEntry(glslReflectJson, "images");
  uint32_t numDescriptors =
      (textures ? textures->size() : 0) + (images ? images->size() : 0) +
      (ubos ? ubos->size() : 0) + (ssbos ? ssbos->size() : 0);

  // update input bindings
  if (ext == ".vert") {
    json *inputs = getEntry(glslReflectJson, "inputs");
    if (inputs)
        for (json &input : *inputs) {
          RegexUtil::Match metalInputReflectData;
          bool foundMatch = findMetalReflectData(
              metalReflectData.attributes, input["name"], metalInputReflectData);
          if (!foundMatch) {
            return 1;
          }
          input["location"] = stoi(metalInputReflectData.s[3]) + numDescriptors;
        }
  }

  // update descriptor bindings
  if (textures)
    for (json &descriptor : *textures) {
      RegexUtil::Match metalTextureReflectData;
      bool foundMatch =
          findMetalReflectData(metalReflectData.textures, descriptor["name"],
                               metalTextureReflectData);
      assert(foundMatch);
      descriptor["set"] = stoi(metalTextureReflectData.s[3]);
    }
  if (ubos)
    for (json &descriptor : *ubos) {
      RegexUtil::Match metalBufferReflectData;
      bool foundMatch = findMetalReflectData(
          metalReflectData.buffers, descriptor["name"], metalBufferReflectData);
      assert(foundMatch);
      descriptor["set"] = stoi(metalBufferReflectData.s[3]);
    }
  if (ssbos)
    for (json &descriptor : *ssbos) {
      RegexUtil::Match metalBufferReflectData;
      bool foundMatch = findMetalReflectData(
          metalReflectData.buffers, descriptor["name"], metalBufferReflectData);
      assert(foundMatch);
      descriptor["set"] = stoi(metalBufferReflectData.s[3]);
    }
  if (images)
    for (json &descriptor : *images) {
      RegexUtil::Match metalTextureReflectData;
      bool foundMatch =
          findMetalReflectData(metalReflectData.textures, descriptor["name"],
                               metalTextureReflectData);
      assert(foundMatch);
      descriptor["set"] = stoi(metalTextureReflectData.s[3]);
    }

  mslReflect = glslReflectJson.dump(4);
  return 0;
}

int ShaderTools::patchShaderReflectionDataHLSL(const std::string &glslReflect,
                                               const std::string &ext,
                                               const std::string &hlsl,
                                               std::string &hlslReflect) {
  auto glslReflectJson = json::parse(glslReflect);
  HLSLReflectData hlslReflectData;

  // parse semantics
  if (ext == ".vert") {
    json *inputs = getEntry(glslReflectJson, "inputs");
    if (inputs)
        for (json &input : *inputs) {
          regex p(input["name"].get<string>() + "\\s*:\\s*([^;]*);");
          vector<RegexUtil::Match> hlslReflectData = RegexUtil::findAll(p, hlsl);
          input["semantic"] = hlslReflectData[0].s[1];
        }
  }
  hlslReflect = glslReflectJson.dump(4);
  return 0;
}

int ShaderTools::genShaderReflectionGLSL(const string &, const string &ext,
                                         const string &spv, string &glslMap) {
  spirv_cross::CompilerReflection compilerReflection(
      (const uint32_t *)spv.data(), spv.size() / sizeof(uint32_t));
  auto reflectOutput = compilerReflection.compile();
  glslMap = json::parse(reflectOutput).dump(4);
  return 0;
}

int ShaderTools::genShaderReflectionMSL(const string &msl, const string &ext,
                                        const string &spv, string &mslMap) {
  string glslReflect;
  genShaderReflectionGLSL("", ext, spv, glslReflect);
  return patchShaderReflectionDataMSL(glslReflect, ext, msl, mslMap);
}

int ShaderTools::genShaderReflectionHLSL(const string &hlsl, const string &ext,
                                         const string &spv, string &hlslMap) {
  string glslReflect;
  genShaderReflectionGLSL("", ext, spv, glslReflect);
  return patchShaderReflectionDataHLSL(glslReflect, ext, hlsl, hlslMap);
}

string ShaderTools::parseReflectionData(const json &reflectData, string ext) {
  string contents = "";
  if (ext == ".vert") {
    json *inputs = getEntry(reflectData, "inputs");
    contents += "INPUT_ATTRIBUTES " + to_string(inputs ? inputs->size() : 0) + "\n";
    if (inputs) 
        for (const json &input : *inputs) {
          string inputName = input["name"];
          string inputSemantic = "";
          string inputNameLower = toLower(inputName);
          inputSemantic = "UNDEFINED";
          if (input.find("semantic") != input.end())
            inputSemantic = input["semantic"];
          map<string, string> inputTypeMap = {
              {"float", "VERTEXFORMAT_FLOAT"}, {"vec2", "VERTEXFORMAT_FLOAT2"},
              {"vec3", "VERTEXFORMAT_FLOAT3"}, {"vec4", "VERTEXFORMAT_FLOAT4"},
              {"ivec2", "VERTEXFORMAT_INT2"},  {"ivec3", "VERTEXFORMAT_INT3"},
              {"ivec4", "VERTEXFORMAT_INT4"},  {"mat2", "VERTEXFORMAT_MAT2"},
              {"mat3", "VERTEXFORMAT_MAT3"},   {"mat4", "VERTEXFORMAT_MAT4"}};
          string inputType = inputTypeMap[input["type"]];
          contents += "\t" + inputName + " " + inputSemantic + " " +
                      to_string(input["location"].get<int>()) + " " + inputType +
                      "\n";
    }
  }
  json *textures = getEntry(reflectData, "textures"),
       *ubos = getEntry(reflectData, "ubos"),
       *ssbos = getEntry(reflectData, "ssbos"),
       *images = getEntry(reflectData, "images"),
       *types = getEntry(reflectData, "types");
  json uniformBufferInfos;
  json shaderStorageBufferInfos;

  std::function<void(const json &, json &, uint32_t, string)> parseMembers =
      [&](const json &membersData, json &members, uint32_t baseOffset = 0,
          string baseName = "") {
        for (const json &memberData : membersData) {
          const map<string, int> typeSizeMap = {
              {"int", 4},    {"uint", 4},  {"float", 4},  {"vec2", 8},
              {"vec3", 12},  {"vec4", 16}, {"ivec2", 8},  {"ivec3", 12},
              {"ivec4", 16}, {"uvec2", 8}, {"uvec3", 12}, {"uvec4", 16},
              {"mat2", 16},  {"mat3", 36}, {"mat4", 64}};
          string memberType = memberData["type"];
          if (typeSizeMap.find(memberType) != typeSizeMap.end()) {
            json member = memberData;
            member["name"] = baseName + member["name"].get<string>();
            member["size"] = typeSizeMap.at(memberType);
            member["offset"] = member["offset"].get<int>() + baseOffset;
            member["array_count"] = (member.find("array") != member.end())
                                        ? member["array"][0].get<int>()
                                        : 0;
            member["array_stride"] =
                (member.find("array_stride") != member.end())
                    ? member["array_stride"].get<int>()
                    : 0;
            members.push_back(member);
          } else if (types->find(memberType) != types->end()) {
            const json &type = (*types)[memberType];
            parseMembers(type["members"], members,
                         baseOffset + memberData["offset"].get<int>(),
                         baseName + memberData["name"].get<string>() + ".");
          } else
            NGFX_ERR("unrecognized type: {memberType}");
        }
      };
  enum BufferType { UBO, SSBO };
  auto parseBuffers = [&](const json &buffers, json &bufferInfos, BufferType type) {
    for (const json &buffer : buffers) {
      const json &bufferType = (*types)[buffer["type"].get<string>()];
      json bufferMembers = {};
      parseMembers(bufferType["members"], bufferMembers, 0, "");
      json bufferInfo = { {"name", buffer["name"].get<string>()},
                         {"set", buffer["set"].get<int>()},
                         {"binding", buffer["binding"].get<int>()},
                         {"members", bufferMembers} };
      auto it = buffer.find("readonly");
      if (it != buffer.end())
          bufferInfo["readonly"] = it->get<bool>();
      else bufferInfo["readonly"] = type == UBO ? true : false;
      bufferInfos.push_back(bufferInfo);
    }
  };
  if (ubos)
    parseBuffers(*ubos, uniformBufferInfos, UBO);
  if (ssbos)
    parseBuffers(*ssbos, shaderStorageBufferInfos, SSBO);

  json textureDescriptors = {};
  json bufferDescriptors = {};
  if (textures)
    for (const json &texture : *textures) {
      textureDescriptors[to_string(texture["set"].get<int>())] = {
          {"type", texture["type"]},
          {"name", texture["name"]},
          {"set", texture["set"]},
          {"binding", texture["binding"]}};
    }
  if (images)
    for (const json &image : *images) {
      textureDescriptors[to_string(image["set"].get<int>())] = {
          {"type", image["type"]},
          {"name", image["name"]},
          {"set", image["set"]},
          {"binding", image["binding"]}};
    }
  if (ubos)
    for (const json &ubo : *ubos) {
      bufferDescriptors[to_string(ubo["set"].get<int>())] = {
          {"type", "uniformBuffer"},
          {"name", ubo["name"]},
          {"set", ubo["set"]},
          {"binding", ubo["binding"]} };
    }
  if (ssbos)
    for (const json &ssbo : *ssbos) {
      bufferDescriptors[to_string(ssbo["set"].get<int>())] = {
          {"type", "shaderStorageBuffer"},
          {"name", ssbo["name"]},
          {"set", ssbo["set"]},
          {"binding", ssbo["binding"]} };
    }
  contents += "DESCRIPTORS " +
              to_string(textureDescriptors.size() + bufferDescriptors.size()) +
              "\n";
  map<string, string> descriptorTypeMap = {
      {"sampler2D", "DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"},
      {"sampler3D", "DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"},
      {"samplerCube", "DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"},
      {"image2D", "DESCRIPTOR_TYPE_STORAGE_IMAGE"},
      {"uniformBuffer", "DESCRIPTOR_TYPE_UNIFORM_BUFFER"},
      {"shaderStorageBuffer", "DESCRIPTOR_TYPE_STORAGE_BUFFER"}};
  for (auto &[key, val] : textureDescriptors.items()) {
    string descriptorType = descriptorTypeMap[val["type"]];
    contents += "\t" + val["name"].get<string>() + " " + descriptorType + " " +
                to_string(val["set"].get<int>()) + "\n";
  }
  for (auto &[key, val] : bufferDescriptors.items()) {
    string descriptorType = descriptorTypeMap[val["type"]];
    contents += "\t" + val["name"].get<string>() + " " + descriptorType + " " +
                to_string(val["set"].get<int>()) + "\n";
  }
  auto processBufferInfos = [&](const json &bufferInfo) -> string {
    string contents = "";
    const json &memberInfos = bufferInfo["members"];
    contents += bufferInfo["name"].get<string>() + " " +
                to_string(bufferInfo["set"].get<int>()) + " " +
                to_string(bufferInfo["readonly"].get<bool>()) + " " +
                to_string(memberInfos.size()) + "\n";
    for (const json &m : memberInfos) {
      contents += m["name"].get<string>() + " " +
                  to_string(m["offset"].get<int>()) + " " +
                  to_string(m["size"].get<int>()) + " " +
                  to_string(m["array_count"].get<int>()) + " " +
                  to_string(m["array_stride"].get<int>()) + "\n";
    }
    return contents;
  };
  contents +=
      "UNIFORM_BUFFER_INFOS " + to_string(uniformBufferInfos.size()) + "\n";
  for (const json &bufferInfo : uniformBufferInfos) {
    contents += processBufferInfos(bufferInfo);
  }

  contents += "SHADER_STORAGE_BUFFER_INFOS " +
              to_string(shaderStorageBufferInfos.size()) + "\n";
  for (const json &bufferInfo : shaderStorageBufferInfos) {
    contents += processBufferInfos(bufferInfo);
  }
  return contents;
}

int ShaderTools::generateShaderMapGLSL(const string &file, string outDir,
                                       vector<string> &outFiles, int flags) {
  string filename = fs::path(file).filename().string();
  string ext = FileUtil::splitExt(filename)[1];

  string glslFileName =
      fs::path(outDir + "/" + filename).make_preferred().string();
  string spvFileName =
      fs::path(outDir + "/" + filename + ".spv").make_preferred().string();
  string glslMapFileName =
      fs::path(outDir + "/" + filename + ".map").make_preferred().string();
  if (!FileUtil::srcFileNewerThanOutFile(glslFileName, glslMapFileName)) {
    outFiles.push_back(glslMapFileName);
    return 0;
  }

  string glsl = "", spv = readFile(spvFileName), glslReflect;
  genShaderReflectionGLSL(glsl, ext, spv, glslReflect);
  auto glslReflectJson = json::parse(glslReflect);
  string glslMap = parseReflectionData(glslReflectJson, ext);

  writeFile(glslMapFileName, glslMap);
  outFiles.push_back(glslMapFileName);
  return 0;
}

int ShaderTools::generateShaderMapMSL(const string &file, string outDir,
                                      vector<string> &outFiles, int flags) {
  auto splitFilename = FileUtil::splitExt(fs::path(file).filename().string());
  string glslFilename = splitFilename[0];
  string ext = FileUtil::splitExt(splitFilename[0])[1];

  string mslFileName = fs::path(outDir + "/" + glslFilename + ".metal")
                           .make_preferred()
                           .string();
  string spvFileName =
      fs::path(outDir + "/" + glslFilename + ".spv").make_preferred().string();
  string mslMapFileName = fs::path(outDir + "/" + glslFilename + ".metal.map")
                              .make_preferred()
                              .string();
  if (!FileUtil::srcFileNewerThanOutFile(mslFileName, mslMapFileName)) {
    outFiles.push_back(mslMapFileName);
    return 0;
  }

  string msl = readFile(mslFileName), spv = readFile(spvFileName), mslReflect;
  genShaderReflectionMSL(msl, ext, spv, mslReflect);
  auto mslReflectJson = json::parse(mslReflect);
  string mslMap = parseReflectionData(mslReflectJson, ext);

  writeFile(mslMapFileName, mslMap);
  outFiles.push_back(mslMapFileName);
  return 0;
}

int ShaderTools::generateShaderMapHLSL(const string &file, string outDir,
                                       vector<string> &outFiles, int flags) {
  auto splitFilename = FileUtil::splitExt(fs::path(file).filename().string());
  string glslFilename = splitFilename[0];
  string ext = FileUtil::splitExt(splitFilename[0])[1];

  string hlslFileName =
      fs::path(outDir + "/" + glslFilename + ".hlsl").make_preferred().string();
  string spvFileName =
      fs::path(outDir + "/" + glslFilename + ".spv").make_preferred().string();
  string hlslMapFileName = fs::path(outDir + "/" + glslFilename + ".hlsl.map")
                               .make_preferred()
                               .string();
  if (!FileUtil::srcFileNewerThanOutFile(hlslFileName, hlslMapFileName)) {
    outFiles.push_back(hlslMapFileName);
    return 0;
  }

  string hlsl = readFile(hlslFileName), spv = readFile(spvFileName),
         hlslReflect;
  genShaderReflectionHLSL(hlsl, ext, spv, hlslReflect);
  auto hlslReflectJson = json::parse(hlslReflect);
  string hlslMap = parseReflectionData(hlslReflectJson, ext);

  writeFile(hlslMapFileName, hlslMap);
  outFiles.push_back(hlslMapFileName);
  return 0;
}

vector<string> ShaderTools::convertShaders(const vector<string> &files,
                                           string outDir, Format fmt) {
  vector<string> outFiles;
  for (const string &file : files)
    convertShader(file, "", outDir, fmt, outFiles);
  return outFiles;
}

vector<string> ShaderTools::compileShaders(const vector<string> &files,
                                           string outDir, Format fmt,
                                           const MacroDefinitions &defines,
                                           int flags) {
  vector<string> outFiles;
  for (const string &file : files) {
    if (fmt == FORMAT_GLSL)
      compileShaderGLSL(file, defines, outDir, outFiles, flags);
    else if (fmt == FORMAT_MSL)
      compileShaderMSL(file, defines, outDir, outFiles, flags);
    else if (fmt == FORMAT_HLSL)
      compileShaderHLSL(file, defines, outDir, outFiles, flags);
  }
  return outFiles;
}

void ShaderTools::applyPatches(const vector<string> &patchFiles,
                               string outDir) {
  for (const string &patchFile : patchFiles) {
    string filename = FileUtil::splitExt(fs::path(patchFile).string())[0];
    NGFX_LOG("filename: %s", filename.c_str());
    string outFile =
        fs::path(outDir + "/" + filename).make_preferred().string();
    if (fs::exists(outFile)) {
      NGFX_LOG("applying patch: {patchFile}");
      string cmdStr = PATCH + " -N -u " + outFile + " -i " + patchFile;
      cmd(cmdStr);
    }
  }
}

vector<string> ShaderTools::generateShaderMaps(const vector<string> &files,
                                               string outDir, Format fmt, int flags) {
  vector<string> outFiles;
  for (const string &file : files) {
    if (fmt == FORMAT_GLSL)
      generateShaderMapGLSL(file, outDir, outFiles, flags);
    else if (fmt == FORMAT_MSL)
      generateShaderMapMSL(file, outDir, outFiles, flags);
    else if (fmt == FORMAT_HLSL)
      generateShaderMapHLSL(file, outDir, outFiles, flags);
  }
  return outFiles;
}
