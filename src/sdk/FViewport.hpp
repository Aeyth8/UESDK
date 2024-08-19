#pragma once

#include <cstdint>
#include <optional>

namespace sdk {
class FCanvas;
}

namespace sdk {
class FViewport {
public:
    static std::optional<size_t> get_debug_canvas_index();
    static std::optional<size_t> get_viewport_size_xy_index();

public:
    FCanvas* get_debug_canvas() const {
        const auto vtable = *(FCanvas* (***)(const FViewport*))this;
        if (vtable == nullptr) {
            return nullptr;
        }

        const auto index = get_debug_canvas_index();

        if (!index.has_value()) {
            return nullptr;
        }

        const auto func = vtable[index.value()];
        return func(this);
    }

    struct IntPoint {
        int32_t x;
        int32_t y;
    };

    IntPoint get_viewport_size_xy() const {
        const auto vtable = *(IntPoint* (***)(const FViewport*, IntPoint*))this;
        if (vtable == nullptr) {
            return {0, 0};
        }

        const auto index = get_viewport_size_xy_index();

        if (!index.has_value()) {
            return {0, 0};
        }

        IntPoint result{};
        const auto func = vtable[index.value()];
        func(this, &result);
        return result;
    }

private:
};
}