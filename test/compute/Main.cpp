#include <iostream>
#include "ngfx/graphics/GraphicsContext.h"
#include "ngfx/computeOps/MatrixMultiplyGPUOp.h"
#include "ngfx/computeOps/MatrixMultiplyCPUOp.h"
#include "test/common/UnitTest.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/graphics/ImageData.h"
#include "ngfx/graphics/ImageUtil.h"
#include "ngfx/graphics/FilterUtil.h"
#include "ngfx/compute/ComputeUtil.h"
#include "ngfx/graphics/TextureUtil.h"
#include <array>
using namespace ngfx;
using namespace ngfx::ComputeUtil;
using namespace ngfx::FilterUtil;
using namespace std;
using namespace glm;

enum ComputeTest { MATRIX_MULTIPLY, GAUSSIAN, SUM, PARTICLES, COLORSPACE_CONVERSION, LENS_CORRECTION };

static const map<string, ComputeTest> computeTestMap = {
        { "matrix_multiply", MATRIX_MULTIPLY },
        { "gaussian", GAUSSIAN },
        { "sum", SUM },
        { "particles", PARTICLES },
        { "colorspace_conversion", COLORSPACE_CONVERSION },
        { "lens_correction", LENS_CORRECTION }
};
static ComputeTest toComputeTest(string computeTestStr) {
    return computeTestMap.at(computeTestStr);
}
static string toString(ComputeTest computeTest) {
    for (auto p : computeTestMap)
        if (p.second == computeTest)
            return p.first;
    return "";
}

static int compareData(float* v0, float* v1, int size) {
    for (int j = 0; j < size; j++) {
        if (abs(v0[j] - v1[j]) > FLT_EPSILON)
            return 1;
    }
    return 0;
}

static int testMatrixMultiply() {
    unique_ptr<GraphicsContext> ctx;
    ctx.reset(GraphicsContext::create("compute_matrix_multiply", false));
    ctx->setSurface(nullptr);
    unique_ptr<Graphics> graphics;
    graphics.reset(Graphics::create(ctx.get()));
    using MatrixParam = MatrixMultiplyOp::MatrixParam;
    const int DIM = 16;
    vector<float> srcData[2], dstData[2];
    for (int j = 0; j < 2; j++) {
        srcData[j] = vector<float>(DIM * DIM);
        dstData[j] = vector<float>(DIM * DIM);
    }
    for (int j = 0; j < (DIM * DIM); j++) {
        srcData[0][j] = (rand() % 1024) / 1024.0f;
        srcData[1][j] = (rand() % 1024) / 1024.0f;
    }
    MatrixParam src0 = { DIM, DIM, srcData[0].data() };
    MatrixParam src1 = { DIM, DIM, srcData[1].data() };
    MatrixParam dst0 = { DIM, DIM, nullptr };
    //test matrix multiply on GPU
    auto op = make_unique<MatrixMultiplyGPUOp>(ctx.get(),
        src0, src1, dst0);
    auto commandBuffer = ctx->computeCommandBuffer();
    commandBuffer->begin();
    op->apply(commandBuffer, graphics.get());
    commandBuffer->end();
    ctx->submit(commandBuffer);
    ctx->queue->waitIdle();
    op->bDst->download(dstData[0].data(), dstData[0].size() * sizeof(dstData[0][0]));

    //compare against CPU
    MatrixParam dst1 = { DIM, DIM, dstData[1].data() };
    auto cpuOp = make_unique<MatrixMultiplyCPUOp>(src0, src1, dst1);
    cpuOp->apply();
    return compareData(dstData[0].data(), dstData[1].data(), DIM * DIM);
}

class ConvolveGPUOp : public FilterOp {
public:
    ConvolveGPUOp(GraphicsContext* ctx) : FilterOp(ctx) {}
    ConvolveGPUOp(GraphicsContext* ctx, Texture* srcTexture, Texture* dstTexture, kernel_t kernel)
        : ConvolveGPUOp(ctx) {
        setKernel(kernel);
        this->srcTexture = srcTexture;
        this->dstTexture = dstTexture;
        createPipeline();
    }
    virtual ~ConvolveGPUOp() {}
    void draw(CommandBuffer* commandBuffer, Graphics* graphics) override {
        graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO, SHADER_STAGE_COMPUTE_BIT);
        graphics->bindComputePipeline(commandBuffer, computePipeline);
    }
    void setKernel(kernel_t kernel) {
        float* kernelData = uboData.kernel_data;
        uboData.kernel_w = kernel.w;
        uboData.kernel_h = kernel.h;
        memcpy(uboData.kernel_data, kernel.data, kernel.w * kernel.h * sizeof(float));
        bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));

    }
    std::unique_ptr<Buffer> bUbo;
    Texture* srcTexture = nullptr;
    Texture* dstTexture = nullptr;

protected:
    void createPipeline() {
        computePipeline = ComputePipeline::create(
            ctx,
            ComputeShaderModule::create(ctx->device, NGFX_TEST_DATA_DIR "/shaders/testConvolve.comp").get());
    }
    static const int MAX_KERNEL_SIZE = 64;
    struct ConvolveUboData {
        float kernel_data[MAX_KERNEL_SIZE];
        int kernel_w, kernel_h;
    };
    ConvolveUboData uboData;
    ComputePipeline* computePipeline;
    uint32_t U_UBO = 0, U_SRC_IMAGE = 1, U_DST_IMAGE = 2;
};

class GaussianOp : public ComputeOp {
public:
    GaussianOp(GraphicsContext* ctx) : ComputeOp(ctx) {}
    virtual ~GaussianOp() {}
    virtual void setRadius(int radius) {
        this->radius = radius;
        gaussianData.resize(radius + 1);
        for (int j = 0; j < (radius + 1); j++) {
            gaussianData[j] = gaussian(j);
        }
    }
    vector<float> gaussianData;
    int radius = 0;
};

class GaussianGPUOp : public GaussianOp {
public:
    GaussianGPUOp(GraphicsContext* ctx) : GaussianOp(ctx) {}
    GaussianGPUOp(GraphicsContext* ctx, Texture *srcTexture, Texture *dstTexture, int radius)
        : GaussianGPUOp(ctx) {
        setRadius(radius);
        this->srcTexture = srcTexture;
        this->dstTexture = dstTexture;
        createPipeline();
    }
    virtual ~GaussianGPUOp() {}
    void apply(CommandBuffer* commandBuffer = nullptr,
            Graphics* graphics = nullptr) override {
    }
    void setRadius(int radius) override {
        GaussianOp::setRadius(radius);
        float* kernelData = uboData.kernel_data;
        uboData.kernel_w = 2 * radius + 1;
        uboData.kernel_h = 1;
        int kernelSize = uboData.kernel_w;
        for (int j = 0; j < kernelSize; j++) {
            kernelData[j] = gaussianData[abs(j - radius)];
        }
        bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));

    }
    std::unique_ptr<Buffer> bUbo;
    Texture* srcTexture = nullptr;
    Texture* dstTexture = nullptr;

protected:
    void createPipeline() {
        const std::string key = "gaussianGPUOp";
        computePipeline = ComputePipeline::create(
            ctx,
            ComputeShaderModule::create(ctx->device, NGFX_TEST_DATA_DIR "/shaders/testGaussian.comp").get());
    }
    static const int MAX_KERNEL_SIZE = 64;
    struct ConvolveUboData {
        float kernel_data[MAX_KERNEL_SIZE];
        int kernel_w, kernel_h;
    };
    ConvolveUboData uboData;
    ComputePipeline* computePipeline;
    uint32_t U_CONVOLVE_UBO = 0, U_CONVOLE_SRC_IMAGE = 1, U_CONVOLE_DST_IMAGE = 2;
    uint32_t U_TRANSPOSE_SRC_IMAGE = 0, U_TRANSPOSE_DST_IMAGE = 1;
};

#define TO_IMG(p) { (u8vec4*)p.data, p.w, p.h }

class GaussianCPUOp : public GaussianOp {
public:
    GaussianCPUOp() : GaussianOp(nullptr) {}
    GaussianCPUOp(const ImageData *srcImage, ImageData *dstImage, int radius) : GaussianOp(nullptr) {
        this->srcImage = srcImage;
        this->dstImage = dstImage;
        setRadius(radius);
    }
    virtual ~GaussianCPUOp() {}
    void apply(CommandBuffer* commandBuffer = nullptr,
            Graphics* graphics = nullptr) override {
        ImageData img0(srcImage->w, srcImage->h),
            img1(srcImage->h, srcImage->w),
            img2(srcImage->h, srcImage->w);
        vector<float> kernelData(radius * 2 + 1);
        for (int j = 0; j < kernelData.size(); j++) {
            kernelData[j] = gaussianData[abs(j - radius)];
        }
        kernel_t kernel = { kernelData.data(), radius * 2 + 1, 1 };
        convolve(TO_IMG((*srcImage)), TO_IMG(img0), kernel);
        transpose(TO_IMG(img0), TO_IMG(img1));
        convolve(TO_IMG(img1), TO_IMG(img2), kernel);
        transpose(TO_IMG(img2), TO_IMG((*dstImage)));
    }
    const ImageData *srcImage = nullptr;
    ImageData* dstImage = nullptr;
};

static int testGaussian() {
    unique_ptr<GraphicsContext> ctx;
    ctx.reset(GraphicsContext::create("compute_gaussian", false));
    ctx->setSurface(nullptr);
    unique_ptr<Graphics> graphics;
    graphics.reset(Graphics::create(ctx.get()));
    ImageData srcImage;
    ImageUtil::load(NGFX_TEST_DATA_DIR "/images/bird.jpg", srcImage);
    //test gaussian on GPU
    unique_ptr<Texture> srcTexture(TextureUtil::load(ctx.get(), graphics.get(), srcImage, IMAGE_USAGE_STORAGE_BIT));
    unique_ptr<Texture> dstTexture(Texture::create(ctx.get(), graphics.get(), nullptr, PIXELFORMAT_RGBA8_UNORM,
        srcTexture->size, srcTexture->w, srcTexture->h, 1, 1, IMAGE_USAGE_STORAGE_BIT));
    auto gpuOp = make_unique<GaussianGPUOp>(ctx.get(), srcTexture.get(), dstTexture.get(), 3);
    auto commandBuffer = ctx->computeCommandBuffer();
    commandBuffer->begin();
    gpuOp->apply(commandBuffer, graphics.get());
    commandBuffer->end();
    ctx->submit(commandBuffer);
    ctx->queue->waitIdle();
    TextureUtil::storePNG("tmp_gpu.png", dstTexture.get());
    //compare against CPU
    ImageData dstImage(srcImage.w, srcImage.h);
    auto op = make_unique<GaussianCPUOp>(&srcImage, &dstImage, 3);
    op->apply(nullptr, nullptr);
    ImageUtil::storePNG("tmp.png", dstImage);
    return 1;
}

class SumGPUOp : public ComputeOp {
public:
    SumGPUOp(GraphicsContext* ctx) : ComputeOp(ctx) {}
    SumGPUOp(GraphicsContext* ctx, const std::vector<float>& data) : SumGPUOp(ctx) {
        update(data);
        createPipeline();
    }
    virtual ~SumGPUOp() {}
    void apply(CommandBuffer* commandBuffer = nullptr,
        Graphics* graphics = nullptr) override {
        graphics->bindComputePipeline(commandBuffer, computePipeline);
        graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO,
            SHADER_STAGE_COMPUTE_BIT);
        graphics->bindStorageBuffer(commandBuffer, bSrc.get(), SSBO_SRC,
            SHADER_STAGE_COMPUTE_BIT, true);
        graphics->bindStorageBuffer(commandBuffer, bDst.get(), SSBO_DST,
            SHADER_STAGE_COMPUTE_BIT, false);
        graphics->dispatch(commandBuffer, 1, uboData.src_h, 1);
    }
    virtual void update(const std::vector<float>& data) {
        const int src_w = 1024;
        int src_h = (data.size() + src_w - 1) / src_w;
        int dst_w = 1, dst_h = src_h;
        uboData = { src_w, src_h };
        bUbo.reset(createUniformBuffer(ctx, &uboData, sizeof(uboData)));
        bSrc.reset(createStorageBuffer(ctx, data.data(), src_w * src_h * sizeof(float)));
        bDst.reset(createStorageBuffer(ctx, nullptr, dst_w * dst_h * sizeof(float)));
    }
    std::unique_ptr<Buffer> bUbo;
    std::unique_ptr<Buffer> bSrc, bDst;

protected:
    struct UboData {
        int32_t src_w, src_h;
    };
    UboData uboData;
    void createPipeline() {
        const std::string key = "sumGPUOp";
        computePipeline = ComputePipeline::create(
            ctx,
            ComputeShaderModule::create(ctx->device, NGFX_TEST_DATA_DIR "/shaders/testComputeSum.comp").get());
    }
    ComputePipeline* computePipeline;
    uint32_t U_UBO = 0, SSBO_SRC = 1, SSBO_DST = 2;
};

static int testSum() {
    unique_ptr<GraphicsContext> ctx;
    ctx.reset(GraphicsContext::create("compute_sum", false));
    ctx->setSurface(nullptr);
    unique_ptr<Graphics> graphics;
    graphics.reset(Graphics::create(ctx.get()));
    vector<float> src(1024 * 1024);
    for (int j = 0; j < src.size(); j++) {
        src[j] = (j % 123) * (j % 2 == 0) ? -1 : 1;
    }
    //test sum on GPU
    auto op = make_unique<SumGPUOp>(ctx.get(), src);
    auto commandBuffer = ctx->computeCommandBuffer();
    commandBuffer->begin();
    op->apply(commandBuffer, graphics.get());
    commandBuffer->end();
    ctx->submit(commandBuffer);
    ctx->queue->waitIdle();
    vector<float> dst(1024);
    op->bDst->download(dst.data(), dst.size() * sizeof(dst[0]));
    float sum = 0.0f;
    for (const float &v : dst)
        sum += v;
    //compare against CPU
    float sum1 = 0.0f;
    for (const float& v : src) {
        sum1 += v;
    }
    return compareData(&sum, &sum1, 1);
}

static int testParticles() {
    //TODO
    return 1;
}

static int testColorspaceConversion() {
    //TODO
    return 1;
}

static int testLensCorrection() {
    //TODO
    return 1;
}


static int run(ComputeTest computeTest) {
    switch (computeTest) {
    case MATRIX_MULTIPLY:
        return testMatrixMultiply();
        break;
    case GAUSSIAN:
        return testGaussian();
        break;
    case SUM:
        return testSum();
        break;
    case PARTICLES:
        return testParticles();
        break;
    case COLORSPACE_CONVERSION:
        return testColorspaceConversion();
        break;
    case LENS_CORRECTION:
        return testLensCorrection();
        break;
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        vector<ComputeTest> computeTests = {
            MATRIX_MULTIPLY,
            GAUSSIAN,
            SUM,
            PARTICLES,
            COLORSPACE_CONVERSION,
            LENS_CORRECTION,
        };
        int r = 0;
        for (ComputeTest m : computeTests)
            r |= run(m);
        return r;
    }
    string computeTestStr = argv[1];
    return run(toComputeTest(computeTestStr));
}