#include <vector>
#include <string>
#include "core/FileUtil.h"
#include "graphics/ShaderTools.h"
using namespace std;
using namespace ngfx;

int main(int argc, char** argv) {
    const vector<string> paths = { "ngfx/data/shaders", "nodegl/data/shaders", "nodegl/pynodegl-utils/pynodegl_utils/examples/shaders" };
    const vector<string> extensions = {".vert", ".frag", ".comp"};
    auto glslFiles = FileUtil::findFiles(paths, extensions);
    if (argc == 2) glslFiles = FileUtil::filterFiles(glslFiles, argv[1]);
    string outDir = "cmake-build-debug";
    ShaderTools shaderTools;
    auto spvFiles = shaderTools.compileShaders(glslFiles, outDir, "glsl");
    auto spvMapFiles = shaderTools.generateShaderMaps(glslFiles, outDir, "glsl");
    return 0;
}
