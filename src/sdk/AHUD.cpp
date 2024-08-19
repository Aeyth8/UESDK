#include <windows.h>
#include <spdlog/spdlog.h>

#include <utility/Scan.hpp>
#include <utility/Module.hpp>

#include "UObjectArray.hpp"

#include "AHUD.hpp"

namespace sdk {
UClass* AHUD::static_class() {
    static auto result1 = sdk::find_uobject<UClass>(L"Class /Script/Engine.HUD");
    static auto result2 = sdk::find_uobject<UClass>(L"Class /Script/Engine.hud");

    return result1 != nullptr ? result1 : result2;
}

std::optional<size_t> AHUD::get_post_render_index() {
    static auto result = []() -> std::optional<size_t> {
        SPDLOG_INFO("[AHUD] Finding post render index...");

        const auto hud_class = static_class();

        if (hud_class == nullptr) {
            SPDLOG_ERROR("[AHUD] Failed to find HUD class");
            return std::nullopt;
        }

        const auto default_object = hud_class->get_class_default_object();

        if (default_object == nullptr) {
            SPDLOG_ERROR("[AHUD] Failed to find HUD default object");
            return std::nullopt;
        }

        const auto vtable = *(void***)default_object;

        if (vtable == nullptr || IsBadReadPtr(vtable, sizeof(void*) * 100)) {
            SPDLOG_ERROR("[AHUD] Failed to find valid HUD vtable");
            return std::nullopt;
        }

        // It starts pretty high up
        // Try to locate the last index first
        const auto mod_vtable_within = utility::get_module_within((uintptr_t)vtable).value_or(nullptr);
        size_t vtable_size = 300;
        for (size_t i = 100; i < 300; ++i) {
            const auto& fn = vtable[i];

            // Reached the end
            if (fn == nullptr || IsBadReadPtr(fn, sizeof(void*))) {
                vtable_size = i;
                break;
            }
        }

        SPDLOG_INFO("[AHUD] First pass VTable size: {}", vtable_size);

        // Now do a second pass to find the true size
        for (size_t i = vtable_size; i > 100; --i) {
            const auto& fn = vtable[i];

            // Reached the end
            if (fn == nullptr || IsBadReadPtr(fn, sizeof(void*))) {
                vtable_size = i;
                break;
            }

            const auto mod_fn_within = utility::get_module_within((uintptr_t)fn).value_or(nullptr);

            if (mod_fn_within != nullptr && mod_fn_within == mod_vtable_within) {
                if (utility::scan_displacement_reference(mod_vtable_within, (uintptr_t)&fn)) {
                    vtable_size = i;
                    break;
                }
            }
        }

        SPDLOG_INFO("[AHUD] Second pass VTable size: {}", vtable_size);

        for (size_t i = vtable_size - 1; i > 100; --i) try {
            const auto& fn = vtable[i];

            // Reached the end
            if (fn == nullptr || IsBadReadPtr(fn, sizeof(void*))) {
                break;
            }

            if (utility::find_string_reference_in_path((uintptr_t)fn, L"nullrhi", true)) {
                SPDLOG_INFO("[AHUD] Found post render index: {}", i);
                return i;
            }
        } catch (...) {
            SPDLOG_ERROR("[AHUD] Failed to find post render index, exception occurred during traversal");
            return std::nullopt;
        }

        return std::nullopt;
    }();

    return result;
}

std::optional<uintptr_t> AHUD::get_post_render_fn_static(UClass* custom_hud_class) {
    const auto index = AHUD::get_post_render_index();

    if (!index.has_value()) {
        return std::nullopt;
    }

    const auto hud_class = custom_hud_class != nullptr ? custom_hud_class : static_class();

    if (hud_class == nullptr) {
        return std::nullopt;
    }

    const auto default_object = hud_class->get_class_default_object();

    if (default_object == nullptr) {
        return std::nullopt;
    }

    const auto vtable = *(void***)default_object;
    
    return (uintptr_t)vtable[index.value()];
}

sdk::UObject*& AHUD::get_canvas() const {
    return this->get_property<sdk::UObject*>(L"Canvas");
}

sdk::UObject*& AHUD::get_debug_canvas() const {
    return this->get_property<sdk::UObject*>(L"DebugCanvas");
}
}