#pragma once

#include <expected>

#include "common/UFunctionError.hpp"

#include "USceneComponent.hpp"

namespace sdk {
class UPrimitiveComponent : public USceneComponent {
public:
    static UClass* static_class();

public:
    std::expected<bool, common::UFunctionError> set_render_in_main_pass(bool value);
    bool is_rendering_in_main_pass() const;

    std::expected<bool, common::UFunctionError> set_render_custom_depth(bool value);
    bool is_rendering_custom_depth() const;

    std::expected<bool, common::UFunctionError> set_owner_no_see(bool value);

    void set_overall_visibility(bool value, bool legacy = false) {
        if (const auto result = set_render_in_main_pass(value); !result.has_value() || legacy) {
            USceneComponent::set_visibility(value, false);
        }

        //set_owner_no_see(!value);
        //set_hidden_in_game(!value, false);

        if (value == false) {
            set_render_custom_depth(value);
        }
    }
};
}