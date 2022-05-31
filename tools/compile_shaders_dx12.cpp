#include <vector>
#include <string>
#include "ngfx/core/FileUtil.h"
#include "ngfx/graphics/ShaderTools.h"
using namespace std;
using namespace ngfx;

int main(int argc, char** argv) {
    string dataDir = NGFX_DATA_DIR, outDir = dataDir;
    string filter = (argc > 1) ? argv[1] : "";
    const vector<string> paths = { dataDir };
    const vector<string> extensions = {".vert", ".frag", ".comp"};
    auto glslFiles = FileUtil::findFiles(paths, extensions);
    if (filter != "") glslFiles = FileUtil::filterFiles(glslFiles, filter);
    ShaderTools shaderTools(true);
    auto spvFiles = shaderTools.compileShaders(glslFiles, outDir, ShaderTools::FORMAT_GLSL);
    auto spvMapFiles = shaderTools.generateShaderMaps(glslFiles, outDir, ShaderTools::FORMAT_GLSL);
    auto hlslFiles = shaderTools.convertShaders(spvFiles, outDir, ShaderTools::FORMAT_HLSL);
    auto dxcFiles = shaderTools.compileShaders(hlslFiles, outDir, ShaderTools::FORMAT_HLSL);
    auto hlslMapFiles = shaderTools.generateShaderMaps(hlslFiles, outDir, ShaderTools::FORMAT_HLSL);
    return 0;
}
