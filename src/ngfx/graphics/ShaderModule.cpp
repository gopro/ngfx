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
#include "ngfx/graphics/ShaderModule.h"
#include "ngfx/core/DebugUtil.h"
#include <fstream>
#include <map>
using namespace ngfx;
using namespace std;

#define VF_ITEM(s, count, elementSize)                                         \
  {                                                                            \
#s, { s, count, elementSize }                                              \
  }
struct VertexFormatInfo {
  VertexFormat format;
  uint32_t count, elementSize;
};
static const map<string, VertexFormatInfo> vertexFormatMap = {
    VF_ITEM(VERTEXFORMAT_FLOAT, 1, 4),
    VF_ITEM(VERTEXFORMAT_FLOAT2, 1, 8),
    VF_ITEM(VERTEXFORMAT_FLOAT3, 1, 12),
    VF_ITEM(VERTEXFORMAT_FLOAT4, 1, 16),
    {"VERTEXFORMAT_MAT4", {VERTEXFORMAT_FLOAT4, 4, 16}}};

#define ITEM(s)                                                                \
  { #s, s }

static const map<string, VertexInputRate> vertexInputRateMap = {
    ITEM(VERTEX_INPUT_RATE_VERTEX),
    ITEM(VERTEX_INPUT_RATE_INSTANCE),
};

static const map<string, DescriptorType> descriptorTypeMap = {
    ITEM(DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
    ITEM(DESCRIPTOR_TYPE_STORAGE_IMAGE), ITEM(DESCRIPTOR_TYPE_UNIFORM_BUFFER),
    ITEM(DESCRIPTOR_TYPE_STORAGE_BUFFER)};

static void
parseAttributes(ifstream &in,
                vector<VertexShaderModule::AttributeDescription> &attrs) {
  string token;
  uint32_t numAttributes;
  in >> token >> numAttributes;
  attrs.resize(numAttributes);
  for (uint32_t j = 0; j < numAttributes; j++) {
    auto &attr = attrs[j];
    string formatStr;
    in >> attr.name >> attr.semantic >> attr.location >> formatStr;
    auto formatInfo = vertexFormatMap.at(formatStr);
    attr.format = formatInfo.format;
    attr.count = formatInfo.count;
    attr.elementSize = formatInfo.elementSize;
  }
}

static void parseDescriptors(ifstream &in,
                             vector<ShaderModule::DescriptorInfo> &descs) {
  string token;
  int numDescriptors;
  in >> token >> numDescriptors;
  descs.resize(numDescriptors);
  for (uint32_t j = 0; j < uint32_t(numDescriptors); j++) {
    auto &desc = descs[j];
    string descriptorTypeStr;
    in >> desc.name >> descriptorTypeStr >> desc.set;
    desc.type = descriptorTypeMap.at(descriptorTypeStr);
  }
}

static void
parseBufferMemberInfos(ifstream &in,
                       ShaderModule::BufferMemberInfos &memberInfos) {
  uint32_t numMemberInfos;
  in >> numMemberInfos;
  for (uint32_t j = 0; j < numMemberInfos; j++) {
    ShaderModule::BufferMemberInfo memberInfo;
    string memberName;
    in >> memberName >> memberInfo.offset >> memberInfo.size >>
        memberInfo.arrayCount >> memberInfo.arrayStride;
    memberInfos[memberName] = memberInfo;
  }
}

static void parseBufferInfos(ifstream &in, string key,
                             ShaderModule::BufferInfos &bufferInfos,
                             ShaderStageFlags shaderStages) {
  string token;
  in >> token;
  if (token != key)
    return;
  uint32_t numUniformBufferInfos;
  in >> numUniformBufferInfos;
  for (uint32_t j = 0; j < numUniformBufferInfos; j++) {
    ShaderModule::BufferInfo bufferInfo;
    in >> bufferInfo.name >> bufferInfo.set;
    bufferInfo.shaderStages = shaderStages;
    parseBufferMemberInfos(in, bufferInfo.memberInfos);
    bufferInfos[bufferInfo.name] = std::move(bufferInfo);
  }
}

void ShaderModule::initBindings(std::ifstream &in,
                                ShaderStageFlags shaderStages) {
  parseDescriptors(in, descriptors);
  parseBufferInfos(in, "UNIFORM_BUFFER_INFOS", uniformBufferInfos,
                   shaderStages);
  parseBufferInfos(in, "SHADER_STORAGE_BUFFER_INFOS", shaderStorageBufferInfos,
                   shaderStages);
}

void ShaderModule::initBindings(const std::string &filename,
                                ShaderStageFlags shaderStages) {
  ifstream in(filename);
  if (!in.is_open())
    NGFX_ERR("cannot open file: %s", filename.c_str());
  initBindings(in, shaderStages);
  in.close();
}

void VertexShaderModule::initBindings(const std::string &filename) {
  ifstream in(filename);
  if (!in.is_open())
    NGFX_ERR("cannot open file: %s", filename.c_str());
  parseAttributes(in, attributes);
  ShaderModule::initBindings(in, SHADER_STAGE_VERTEX_BIT);
  in.close();
}
