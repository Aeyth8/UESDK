#include <spdlog/spdlog.h>
#include <utility/Scan.hpp>
#include <tracy/Tracy.hpp>

#include "EngineModule.hpp"

#include "UGameEngine.hpp"

namespace sdk {
std::optional<uintptr_t> UGameEngine::get_tick_address() {
    static auto addr = []() -> uintptr_t {
        ZoneScopedN("sdk::UGameEngine::get_tick_address static init");
        SPDLOG_INFO("UGameEngine::get_tick_address: scanning for address");

        const auto module = sdk::get_ue_module(L"Engine");
        auto result = utility::find_virtual_function_from_string_ref(module, L"causeevent=", true); // present for a very long time

        auto fallback_search_slow = [&]() -> std::optional<uintptr_t> {
            SPDLOG_ERROR("Failed to find UGameEngine::Tick using normal method, trying fallback");
            const auto engine = UEngine::get();

            if (engine != nullptr) {
                // Not find_virtual_function_from_string_ref because it can fail on obfuscated executables
                // where the virtual func is actually a jmp wrapper to the real function that this string is in.
                auto uengine_tick_pure_virtual = utility::find_function_from_string_ref(module, L"UEngine::Tick", true);

                if (!uengine_tick_pure_virtual) {
                    // If we couldn't use the exception table to find the function then
                    // we can use a "dumb" fallback method
                    // Have only seen the need for this fallback method on modular builds of the engine
                    // where compiler settings are strange
                    SPDLOG_ERROR("Failed to find UEngine::Tick pure virtual, trying fallback");

                    const auto uengine_tick_string = utility::scan_string(module, L"UEngine::Tick");

                    if (!uengine_tick_string) {
                        SPDLOG_ERROR("Failed to find UEngine::Tick string for fallback");
                        return std::nullopt;
                    }

                    // TODO: Find the nearest valid instruction or something to this string and disassemble forward...?
                    const auto uengine_tick_string_ref = utility::scan_relative_reference_strict(module, *uengine_tick_string, "4C 8D 05");

                    if (!uengine_tick_string_ref) {
                        SPDLOG_ERROR("Failed to find UEngine::Tick string reference for fallback");
                        return std::nullopt;
                    }

                    uengine_tick_pure_virtual = *uengine_tick_string_ref - 3;
                    SPDLOG_INFO("Made guess for UEngine::Tick pure virtual (fallback): {:x}", (uintptr_t)*uengine_tick_pure_virtual);
                }

                SPDLOG_INFO("UEngine::Tick: {:x} (pure virtual)", (uintptr_t)*uengine_tick_pure_virtual);

                auto uengine_tick_vtable_middle = utility::scan_ptr(module, *uengine_tick_pure_virtual);

                // If this has happened then we are dealing with an obfuscated executable
                // it's not the end of the world, we can still find the address
                // through additional legwork.
                if (!uengine_tick_vtable_middle) {
                    uint32_t insn_size = 0;
                    auto func_call = utility::scan_displacement_reference(module, *uengine_tick_pure_virtual);

                    if (!func_call) {
                        func_call = utility::scan_relative_reference_strict(module, *uengine_tick_pure_virtual, "E9"); // jmp

                        if (!func_call) {
                            SPDLOG_ERROR("Failed to find UEngine::Tick vtable middle");
                            return std::nullopt;
                        }

                        insn_size = 1;
                    } else {
                        insn_size = *func_call - utility::resolve_instruction(*func_call)->addr;
                    }

                    SPDLOG_INFO("UEngine::Tick reference (call or jmp): {:x}", (uintptr_t)*func_call);

                    auto potential_pure_virtual = *func_call - insn_size;
                    auto potential_vtable_middle = utility::scan_ptr(module, potential_pure_virtual);
                    
                    // If this happens, there's some random instructions at the top of the function wrapper
                    // instead of it just being a jmp only, so we have to bruteforce backwards
                    if (!potential_vtable_middle) {
                        SPDLOG_ERROR("Failed to find vtable middle even after finding a reference to it");
                        SPDLOG_INFO("Performing bruteforce backwards search");

                        for (auto i = 0; i < 10; ++i) {
                            potential_pure_virtual--;
                            potential_vtable_middle = utility::scan_ptr(module, potential_pure_virtual);

                            if (potential_vtable_middle) {
                                SPDLOG_INFO("Found pure virtual at {:x}", (uintptr_t)potential_pure_virtual);
                                break;
                            }
                        }

                        if (!potential_vtable_middle) {
                            SPDLOG_ERROR("Failed to find pure virtual after bruteforce backwards search");
                        }
                    }

                    uengine_tick_vtable_middle = potential_vtable_middle;

                    if (!uengine_tick_vtable_middle) {
                        SPDLOG_ERROR("Failed to find UEngine::Tick vtable middle even after finding a reference to it");
                        return std::nullopt;
                    }
                }

                SPDLOG_INFO("UEngine::Tick: {:x} (vtable middle)", (uintptr_t)*uengine_tick_vtable_middle);

                // Keep going backwards through the vtable until we find an instruction that references the pointer
                for (auto i = 1; i < 200; ++i) {
                    const auto& fn = *(uintptr_t*)(*uengine_tick_vtable_middle - (i * sizeof(void*)));

                    if (fn == 0 || IsBadReadPtr((void*)fn, sizeof(void*))) {
                        SPDLOG_ERROR("Reached end of vtable during backwards search");
                        break;
                    }

                    // If a reference is found to this address, AND it's a valid instruction
                    // then we have found the start of vtable.
                    if (utility::scan_displacement_reference(module, (uintptr_t)&fn)) {
                        SPDLOG_INFO("UGameEngine::Tick: found at vtable index {}", i);
                        const auto real_engine_vtable = *(uintptr_t**)engine;
                        const auto actual_fn = real_engine_vtable[i];
                        SPDLOG_INFO("UGameEngine::Tick: {:x}", actual_fn);
                        return actual_fn;
                    }
                }
            }

            SPDLOG_ERROR("UGameEngine::Tick: failed to find address via fallback");
            return std::nullopt;
        };

        auto fallback_search_fast = [&]() -> std::optional<uintptr_t> {
            SPDLOG_INFO("Attempting fast fallback scan for UGameEngine::Tick");

            const auto string = utility::scan_string(module, L"causeevent=");

            if (!string) {
                return std::nullopt;
            }

            const auto string_ref = utility::scan_displacement_reference(module, *string);

            if (!string_ref) {
                return std::nullopt;
            }
            
            SPDLOG_INFO("Found causeevent= string ref: {:x}", *string_ref);

            const auto engine = UEngine::get();
            if (engine == nullptr) {
                return std::nullopt;
            }

            const auto vtable = *(uintptr_t*)engine;
            const auto vfunc = utility::find_encapsulating_virtual_function(vtable, 200, *string_ref);

            if (!vfunc) {
                return std::nullopt;
            }

            return *vfunc;
        };

        if (!result) {
            result = fallback_search_fast();

            if (result) {
                SPDLOG_INFO("Found UGameEngine::Tick via fast fallback scan");
                return *result;
            }

            SPDLOG_ERROR("Failed to find UGameEngine::Tick via fast fallback scan");

            return fallback_search_slow().value_or(0);
        }

        const auto engine = UEngine::get();

        if (engine != nullptr) {
            SPDLOG_INFO("Checking if {:x} resides within the vtable at {:x}", *result, *(uintptr_t*)engine);
            
            // Double check via the vtable that this function is actually UGameEngine::Tick
            const auto vtable = *(uintptr_t**)engine;
            bool exists = false;

            if (vtable != nullptr && !IsBadReadPtr(vtable, sizeof(void*))) {
                SPDLOG_INFO("Double checking UGameEngine::Tick via vtable...");

                for (auto i = 0; i < 200; ++i) {
                    if (IsBadReadPtr(&vtable[i], sizeof(void*))) {
                        break;
                    }

                    const auto fn = vtable[i];

                    if (fn == *result) {
                        SPDLOG_INFO("UGameEngine::Tick: found at vtable index {}", i);
                        exists = true;
                        break;
                    }
                }
            }

            if (!exists) {
                SPDLOG_ERROR("UGameEngine::Tick: vtable check failed!");

                if (auto fast_result = fallback_search_fast(); fast_result.has_value()) {
                    SPDLOG_INFO("Found UGameEngine::Tick via fast fallback scan");
                    return *fast_result;
                }
                
                SPDLOG_ERROR("Failed to find UGameEngine::Tick via fast fallback scan, falling back to slow search");
                return fallback_search_slow().value_or(0);
            }
        }

        SPDLOG_INFO("UGameEngine::Tick: {:x}", (uintptr_t)*result);
        return *result;
    }();

    if (addr == 0) {
        return std::nullopt;
    }

    return addr;
}
}