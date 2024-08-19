#pragma once

#include "UClass.hpp"

namespace sdk {
class AHUD : public sdk::UObject {
public:
    static UClass* static_class();

public:
    static std::optional<size_t> get_post_render_index();
    static std::optional<uintptr_t> get_post_render_fn_static(UClass* custom_hud_class = nullptr);

    std::optional<uintptr_t> get_post_render_fn_this() const {
        const auto index = get_post_render_index();

        if (!index.has_value()) {
            return std::nullopt;
        }

        const auto vtable = *(void***)this;

        return (uintptr_t)vtable[index.value()];
    }

    sdk::UObject*& get_canvas() const;
    sdk::UObject*& get_debug_canvas() const;

private:
};
}