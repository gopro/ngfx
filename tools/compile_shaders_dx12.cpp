#include <vector>
#include <string>
#include "ngfx/core/FileUtil.h"
#include "ngfx/graphics/ShaderTools.h"
using namespace std;
using namespace ngfx;

int main(int argc, char** argv) {
    const vector<string> paths = { "data/shaders" };
    const vector<string> extensions = {".vert", ".frag", ".comp"};
    auto glslFiles = FileUtil::findFiles(paths, extensions);
    if (argc == 2) glslFiles = FileUtil::filterFiles(glslFiles, argv[1]);
    string outDir = "cmake-build-debug";
    ShaderTools shaderTools(true);
    auto spvFiles = shaderTools.compileShaders(glslFiles, outDir, ShaderTools::FORMAT_GLSL);
    auto spvMapFiles = shaderTools.generateShaderMaps(glslFiles, outDir, ShaderTools::FORMAT_GLSL);
    auto hlslFiles = shaderTools.convertShaders(spvFiles, outDir, ShaderTools::FORMAT_HLSL);
    auto dxcFiles = shaderTools.compileShaders(hlslFiles, outDir, ShaderTools::FORMAT_HLSL);
    auto hlslMapFiles = shaderTools.generateShaderMaps(dxcFiles, outDir, ShaderTools::FORMAT_HLSL);
    return 0;
}

