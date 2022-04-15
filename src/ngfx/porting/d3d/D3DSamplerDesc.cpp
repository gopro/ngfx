#include "D3DSamplerDesc.h"
#include "ngfx/graphics/SamplerDesc.h"
#include <vector>
using namespace ngfx;
using namespace std;

static const vector<D3D12_FILTER> filterMap = {
    D3D12_FILTER_MIN_MAG_MIP_POINT,
    D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
    D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
    D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
    D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    D3D12_FILTER_MIN_MAG_MIP_LINEAR
};

D3DSamplerDesc::D3DSamplerDesc(const SamplerDesc* samplerDesc) {
    if (samplerDesc) {
        uint32_t filterIndex = samplerDesc->minFilter << 2 | samplerDesc->magFilter << 1 | samplerDesc->mipFilter;
        Filter = filterMap.at(filterIndex);
        AddressU = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc->addressModeU);
        AddressV = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc->addressModeV);
        AddressW = D3D12_TEXTURE_ADDRESS_MODE(samplerDesc->addressModeW);
    }
    else {
        Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    }
    MipLODBias = 0;
    MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
    ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    MinLOD = 0.0f;
    MaxLOD = D3D12_FLOAT32_MAX;
}