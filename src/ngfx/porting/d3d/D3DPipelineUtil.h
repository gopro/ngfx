#pragma once
#include "D3DShaderModule.h"
#include <map>
#include <functional>

namespace ngfx {
    struct D3DPipelineUtil {
        enum PipelineType { PIPELINE_TYPE_COMPUTE, PIPELINE_TYPE_GRAPHICS };
        using IsReadOnly = std::function<bool(const ShaderModule::DescriptorInfo& info)>;
        static void
            parseDescriptors(std::map<uint32_t, ShaderModule::DescriptorInfo>& uniforms,
                std::vector<uint32_t>& uniformBindings,
                std::vector<CD3DX12_ROOT_PARAMETER1>& d3dRootParams,
                std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>>
                & d3dDescriptorRanges,
                PipelineType pipelineType,
                IsReadOnly isReadOnly);
    };
};