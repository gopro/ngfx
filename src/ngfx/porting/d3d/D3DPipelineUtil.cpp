#include "D3DPipelineUtil.h"
using namespace ngfx;

void D3DPipelineUtil::parseDescriptors(
    std::map<uint32_t, ShaderModule::DescriptorInfo>& descriptors,
    std::vector<uint32_t>& descriptorBindings,
    std::vector<CD3DX12_ROOT_PARAMETER1>& d3dRootParams,
    std::vector<std::unique_ptr<CD3DX12_DESCRIPTOR_RANGE1>>
    & d3dDescriptorRanges,
    PipelineType pipelineType) {
    int registerSpace = 0;
    for (const auto& it : descriptors) {
        auto& descriptor = it.second;
        descriptorBindings[registerSpace] = uint32_t(d3dRootParams.size());
        if (descriptor.type == DESCRIPTOR_TYPE_SAMPLER) {
            CD3DX12_ROOT_PARAMETER1 d3dSamplerDescriptor;
            auto d3dSamplerDescriptorRange =
                std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSamplerDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0,
                registerSpace);
            d3dSamplerDescriptor.InitAsDescriptorTable(
                1, d3dSamplerDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSamplerDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSamplerDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            CD3DX12_ROOT_PARAMETER1 d3dSrvDescriptor, d3dSamplerDescriptor;
            auto d3dSrvDescriptorRange =
                std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSrvDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
                registerSpace);
            d3dSrvDescriptor.InitAsDescriptorTable(1, d3dSrvDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSrvDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSrvDescriptor));

            auto d3dSamplerDescriptorRange =
                std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSamplerDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0,
                registerSpace);
            d3dSamplerDescriptor.InitAsDescriptorTable(
                1, d3dSamplerDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSamplerDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSamplerDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
            CD3DX12_ROOT_PARAMETER1 d3dSrvDescriptor;
            auto d3dSrvDescriptorRange =
                std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dSrvDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
                registerSpace);
            d3dSrvDescriptor.InitAsDescriptorTable(1, d3dSrvDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dSrvDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dSrvDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            CD3DX12_ROOT_PARAMETER1 d3dDescriptor;
            d3dDescriptor.InitAsConstantBufferView(0, registerSpace, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);
            d3dRootParams.emplace_back(std::move(d3dDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            CD3DX12_ROOT_PARAMETER1 d3dDescriptor;
            // TODO encode access flags as read-only or read-write
            if (pipelineType == PIPELINE_TYPE_GRAPHICS)
                d3dDescriptor.InitAsShaderResourceView(0, registerSpace);
            else
                d3dDescriptor.InitAsUnorderedAccessView(0, registerSpace);
            d3dRootParams.emplace_back(std::move(d3dDescriptor));
        }
        else if (descriptor.type == DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            CD3DX12_ROOT_PARAMETER1 d3dUavDescriptor;
            auto d3dUavDescriptorRange =
                std::make_unique<CD3DX12_DESCRIPTOR_RANGE1>();
            d3dUavDescriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0,
                registerSpace);
            d3dUavDescriptor.InitAsDescriptorTable(1, d3dUavDescriptorRange.get());
            d3dDescriptorRanges.emplace_back(std::move(d3dUavDescriptorRange));
            d3dRootParams.emplace_back(std::move(d3dUavDescriptor));
        }
        registerSpace++;
    }
}
