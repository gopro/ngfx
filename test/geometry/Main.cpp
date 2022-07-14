#include <iostream>
#include "test/common/UnitTest.h"
#include "ngfx/drawOps/DrawColorOp.h"
using namespace ngfx;
using namespace std;
using namespace glm;

#define FN0(p0, p1) case p0: return p1; break
static string toString(PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        FN0(PRIMITIVE_TOPOLOGY_POINT_LIST, "point_list");
        FN0(PRIMITIVE_TOPOLOGY_LINE_LIST, "line_list");
        FN0(PRIMITIVE_TOPOLOGY_LINE_STRIP, "line_strip");
        FN0(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, "triangle_list");
        FN0(PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, "triangle_strip");
        default: return ""; break;
    }
}

#define FN1(p0, p1) if (v == p1) return p0
static PrimitiveTopology toPrimitiveTopology(string v) {
    FN1(PRIMITIVE_TOPOLOGY_POINT_LIST, "point_list");
    else FN1(PRIMITIVE_TOPOLOGY_LINE_LIST, "line_list");
    else FN1(PRIMITIVE_TOPOLOGY_LINE_STRIP, "line_strip");
    else FN1(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, "triangle_list");
    else
        return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

class GeometryTestOp : public FilterOp {
public:
    GeometryTestOp(GraphicsContext* ctx, Graphics* graphics, int w, int h, const vector<vec2> &pos,
        PrimitiveTopology primitiveTopology)
        : FilterOp(ctx, graphics, w, h), primitiveTopology(primitiveTopology) {
        auto it = pos.begin();
        op = make_unique<DrawColorOp>(ctx, pos, vec4(1, 0, 0, 1),
            [&](GraphicsPipeline::State& state) {
                state.primitiveTopology = primitiveTopology;
            });
    }
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        op->draw(commandBuffer, graphics);
    }
    std::unique_ptr<DrawColorOp> op;
    PrimitiveTopology primitiveTopology;
};

int run(PrimitiveTopology primitiveTopology) {
    UnitTest test("geometry_" + toString(primitiveTopology));
    using GetPositions = std::function<vector<vec2>()>;

    GetPositions getPointsList = []() -> vector<vec2> {
        int numSteps = 100;
        vector<vec2> p(numSteps);
        float r = 0.5f, a0 = 45.0f, a1 = 225.0f;
        for (int j = 0; j < numSteps; j++) {
            float theta = radians(mix(a0, a1, float(j) / float(numSteps)));
            p[j] = vec2(r * cos(theta), r * sin(theta));
        }
        return p;
    };
    GetPositions getLineList = []() -> vector<vec2> {
        int numSteps = 20;
        vector<vec2> p(numSteps * 2);
        auto pIter = p.begin();
        float r0 = 0.5f, r1 = 0.9f;
        for (int j = 0; j < numSteps; j++) {
            float t0 = float(j) / float(numSteps);
            float r = mix(r0, r1, t0);
            float theta = radians(360.0f * t0);
            *pIter++ = vec2(0.0f);
            *pIter++ = vec2(r * cos(theta), r * sin(theta));
        }
        return p;
    };
    GetPositions getLineStrip = []() -> vector<vec2> {
        int numSteps = 100;
        vector<vec2> p(numSteps);
        auto pIter = p.begin();
        float r0 = 0.1f, r1 = 0.9f;
        for (int j = 0; j < numSteps; j++) {
            float t0 = float(j) / float(numSteps);
            float r = mix(r0, r1, t0);
            float theta = radians(360.0f * t0);
            *pIter++ = vec2(r * cos(theta), r * sin(theta));
        }
        return p;
    };
    GetPositions getTriangleList = []() -> vector<vec2> {
        return {
            vec2(0.0f), vec2(1.0f, 0.5f), vec2(0.5f, 1.0f),
            vec2(0.0f), vec2(-.8f, 1.0f), vec2(-1.0f, 0.8f),
        };
    };
    GetPositions getTriangleStrip = []() -> vector<vec2> {
        int numSteps = 20;
        vector<vec2> p(2 * numSteps);
        auto pIter = p.begin();
        float r0 = 0.5f, r1 = 1.0f;
        for (int j = 0; j < numSteps; j++) {
            float theta = radians(360.0f * float(j) / float(numSteps - 1));
            *pIter++ = vec2(r0 * cos(theta), r0 * sin(theta));
            *pIter++ = vec2(r1 * cos(theta), r1 * sin(theta));
        }
        return p;
    };
    map<PrimitiveTopology, GetPositions> posMap = {
        { PRIMITIVE_TOPOLOGY_POINT_LIST, getPointsList},
        { PRIMITIVE_TOPOLOGY_LINE_LIST, getLineList},
        { PRIMITIVE_TOPOLOGY_LINE_STRIP, getLineStrip},
        { PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, getTriangleList},
        { PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, getTriangleStrip}
    };
    test.op = make_unique<GeometryTestOp>(test.ctx.get(), test.graphics.get(), test.outputWidth, test.outputHeight,
        posMap[primitiveTopology](), primitiveTopology);
    return test.run();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<PrimitiveTopology> testModes = {
            PRIMITIVE_TOPOLOGY_POINT_LIST, PRIMITIVE_TOPOLOGY_LINE_LIST, PRIMITIVE_TOPOLOGY_LINE_STRIP,
            PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
        };
        int r = 0;
        for (PrimitiveTopology m : testModes)
            r |= run(m);
        return r;
    }
    string primitiveTopologyStr = argv[1];
    return run(toPrimitiveTopology(primitiveTopologyStr));
}
