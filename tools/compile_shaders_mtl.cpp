#include <vector>
#include <string>
#include "ngfx/core/FileUtil.h"
#include "ngfx/graphics/ShaderTools.h"
using namespace std;
using namespace ngfx;

int main(int argc, char** argv) {
    string filter = (argc > 1) ? argv[1] : "";
    string dataDir = (argc > 2) ? argv[2] : NGFX_DATA_DIR;
    string outDir = dataDir;
    const vector<string> paths = { dataDir };
    const vector<string> extensions = {".vert", ".frag", /*".comp"*/};
    auto glslFiles = FileUtil::findFiles(paths, extensions);
    if (filter != "") {
        glslFiles = FileUtil::filterFiles(glslFiles, filter);
    }

    ShaderTools shaderTools(true);
    string defines = "-DGRAPHICS_BACKEND_METAL=1";
    auto spvFiles = shaderTools.compileShaders(glslFiles, outDir, ShaderTools::FORMAT_GLSL, 
                        {}, ShaderTools::PATCH_SHADER_LAYOUTS_GLSL | ShaderTools::REMOVE_UNUSED_VARIABLES);
    //auto spvMapFiles = generateShaderMaps(glslFiles, outDir, "glsl");
    auto metalFiles = shaderTools.convertShaders(spvFiles, outDir, ShaderTools::FORMAT_MSL);
    auto metallibFiles = shaderTools.compileShaders(metalFiles, outDir, ShaderTools::FORMAT_MSL);
    auto metalMapFiles = shaderTools.generateShaderMaps(metalFiles, outDir, ShaderTools::FORMAT_MSL);
    return 0;
}

