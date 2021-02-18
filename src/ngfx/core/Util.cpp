#include "ngfx/core/Util.h"
using namespace ngfx;

uint64_t Util::hash(const std::string & s) {
    uint64_t result = 2166136261U;
    for (const char& c : s) {
        result = 127 * result + uint8_t(c);
    }
    return result;
};
