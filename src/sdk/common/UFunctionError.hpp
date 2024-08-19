#pragma once

#include <cstdint>

namespace sdk{
namespace common {
enum class UFunctionError : int64_t {
    OK,
    MISSING_FUNCTION,
    MISSING_CLASS,
    INTERNAL_ERROR,
};
}
}