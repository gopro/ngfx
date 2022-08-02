#include <iostream>
#include <vector>
#include "ngfx/graphics/GraphicsCore.h"
#include "ngfx/graphics/FilterOp.h"
#include "ngfx/drawOps/DrawMeshOp.h"
#include "ngfx/graphics/MeshUtil.h"
#include "ngfx/graphics/Camera.h"
#include "ngfx/graphics/Colors.h"
#include "test/common/UnitTest.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>
using namespace ngfx;
using namespace std;

static string toString(PixelFormat v) {
    static const map<PixelFormat, string> formatMap = {
        { PIXELFORMAT_D32_SFLOAT_S8_UINT, "d32_s8" },
        { PIXELFORMAT_D32_SFLOAT, "d32" },
        { PIXELFORMAT_D24_UNORM_S8_UINT, "d24_s8" },
        { PIXELFORMAT_D16_UNORM, "d16" }
    };
    return formatMap.at(v);
}

static PixelFormat toPixelFormat(const string& s) {
    static const map<string, PixelFormat> formatMap = {
        { "d32_s8", PIXELFORMAT_D32_SFLOAT_S8_UINT },
        { "d32", PIXELFORMAT_D32_SFLOAT },
        { "d24_s8", PIXELFORMAT_D24_UNORM_S8_UINT },
        { "d16", PIXELFORMAT_D16_UNORM }
    };
    return formatMap.at(s);
}

class DepthModeTestOp : public FilterOp {
public:
    DepthModeTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h, PixelFormat depthMode)
        : FilterOp(ctx, graphics, w, h, true), depthMode(depthMode) {
        auto meshData = make_unique<MeshData>();
        MeshUtil::importMesh(NGFX_DATA_DIR "/models/" "bunny.bin", *meshData);
        drawMeshOp = make_unique<DrawMeshOp>(ctx, *meshData);
        camera.reset(new Camera());
        camera->zoom = -1.0f;
        initModelMat(*meshData);
        update();
        ctx->clearColor = Color::Black;
    }
    void update() {
        camera->update();
        modelViewMat = camera->viewMat * modelMat;
        modelViewInverseTransposeMat = transpose(inverse(modelViewMat));
        float aspect = float(outputTexture->w) / outputTexture->h;
        projMat = perspective(radians(60.0f), aspect, 0.1f, 100.0f);
        modelViewProjMat = projMat * modelViewMat;
        DrawMeshOp::LightData lightData;
        drawMeshOp->update(modelViewMat, modelViewInverseTransposeMat, modelViewProjMat, lightData);
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        drawMeshOp->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawMeshOp> drawMeshOp;
    std::unique_ptr<Camera> camera;
    PixelFormat depthMode;
    mat4 modelMat, modelViewMat, modelViewInverseTransposeMat, projMat, modelViewProjMat;
private:
    void initModelMat(MeshData& meshData) {
        auto& b = meshData.bounds;
        vec3 dim = b[1] - b[0];
        vec3 center = 0.5f * (b[0] + b[1]);
        float maxDim = compMax(dim);
        modelMat = scale(vec3(1.0f / maxDim)) * translate(-center);
    }
};

int run(PixelFormat depthFormat) {
    UnitTest test("depth_" + toString(depthFormat), 256, 256, true);
    test.op = make_unique<DepthModeTestOp>(test.ctx.get(), test.graphics.get(),
        test.outputWidth, test.outputHeight, depthFormat);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<PixelFormat> depthModes = {
            PIXELFORMAT_D32_SFLOAT_S8_UINT, PIXELFORMAT_D32_SFLOAT,
            PIXELFORMAT_D24_UNORM_S8_UINT, PIXELFORMAT_D16_UNORM,
        };
        int r = 0;
        for (PixelFormat m : depthModes)
            r |= run(m);
        return r;
    }
    string depthModeStr = argv[1];
    return run(toPixelFormat(depthModeStr));
}