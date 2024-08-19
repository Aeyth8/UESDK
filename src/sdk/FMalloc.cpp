#include <unordered_set>

#include <utility/Scan.hpp>
#include <utility/Module.hpp>
#include <utility/String.hpp>

#include <spdlog/spdlog.h>

#include <tracy/Tracy.hpp>

#include "EngineModule.hpp"

#include "FMalloc.hpp"

namespace sdk {
// There are many different ways to find this because the pointer is used all over.
FMalloc* FMalloc::get() {
    static FMalloc** result = []() -> FMalloc** {
        ZoneScopedN("sdk::FMalloc::get static init");
        SPDLOG_INFO("[FMalloc::get] Finding GMalloc...");

        const auto core_uobject = sdk::get_ue_module(L"CoreUObject");

        if (core_uobject == nullptr) {
            return nullptr;
        }

        auto object_base_init_fn = utility::find_function_with_string_refs(core_uobject, L"gc.MaxObjectsNotConsideredByGC", 
                                                                                         L"/Script/Engine.GarbageCollectionSettings");

        // Alternative scan for REALLY old versions of UE4
        // MaxObjectsNotConsideredByGC
        // SizeOfPermanentObjectPool
        if (!object_base_init_fn) {
            object_base_init_fn = utility::find_function_with_string_refs(core_uobject, L"MaxObjectsNotConsideredByGC", 
                                                                                        L"SizeOfPermanentObjectPool");
            
            if (object_base_init_fn) {
                SPDLOG_INFO("[FMalloc::get] Found object base init function for old UE4 versions");
            }
        }

        // ok at this point we're just using whatever function we can find...
        // because GMalloc is basically referenced everywhere we can use most functions
        // even if that function is just the CVar registration function for MaxObjectsNotConsideredByGC
        if (!object_base_init_fn) {
            object_base_init_fn = utility::find_function_from_string_ref(core_uobject, L"gc.MaxObjectsNotConsideredByGC");

            if (!object_base_init_fn) {
                object_base_init_fn = utility::find_function_from_string_ref(core_uobject, L"MaxObjectsNotConsideredByGC");
            }
        }

        if (!object_base_init_fn) {
            SPDLOG_ERROR("[FMalloc::get] Failed to find object base init function");
            return nullptr;
        }

        FMalloc** result = nullptr;

        std::unordered_set<uintptr_t> seen_displacements{};

        utility::exhaustive_decode((uint8_t*)*object_base_init_fn, 100, [&](utility::ExhaustionContext& ctx) -> utility::ExhaustionResult {
            if (result != nullptr) {
                return utility::ExhaustionResult::BREAK;
            }

            if (ctx.instrux.BranchInfo.IsBranch) {
                return utility::ExhaustionResult::CONTINUE;
            }

            const auto disp = utility::resolve_displacement(ctx.addr);

            if (!disp) {
                return utility::ExhaustionResult::CONTINUE;
            }

            if (seen_displacements.contains(*disp)) {
                return utility::ExhaustionResult::CONTINUE;
            }

            seen_displacements.insert(*disp);

            if (IsBadReadPtr((void*)*disp, sizeof(void*))) {
                return utility::ExhaustionResult::CONTINUE;
            }

            const auto obj = *(void**)*disp;

            if (IsBadReadPtr(obj, sizeof(void*))) {
                return utility::ExhaustionResult::CONTINUE;
            }

            const auto vtable = *(void***)obj;

            if (IsBadReadPtr(vtable, sizeof(void*) * 30)) {
                return utility::ExhaustionResult::CONTINUE;
            }

            if (IsBadReadPtr(vtable[0], sizeof(void*))) {
                return utility::ExhaustionResult::CONTINUE;
            }

            for (auto i = 0; i < 30; ++i) {
                const auto fn = (uintptr_t)vtable[i];

                if (IsBadReadPtr((void*)fn, sizeof(void*))) {
                    return utility::ExhaustionResult::CONTINUE;
                }

                // Look for really small functions.
                uint32_t distance_to_ret = 0;
                utility::exhaustive_decode((uint8_t*)fn, 100, [&](utility::ExhaustionContext& ctx2) -> utility::ExhaustionResult {
                    ++distance_to_ret;

                    if (ctx2.instrux.BranchInfo.IsBranch && std::string_view{ctx2.instrux.Mnemonic}.starts_with("CALL")) {
                        return utility::ExhaustionResult::STEP_OVER;
                    }

                    return utility::ExhaustionResult::CONTINUE;
                });

                if (distance_to_ret > 10) {
                    continue;
                }

                const std::vector<std::wstring> allocator_names {
                    L"binned",
                    L"binned2",
                    L"tbbmalloc",
                    L"ansimalloc",
                    L"binnedmalloc",
                    L"binnedmalloc2",
                    L"binnedmalloc3",
                    L"mimalloc",
                    L"stompmalloc",
                    L"Binned3",
                    L"TBB",
                };

                for (const auto& name : allocator_names) {
                    if (utility::find_string_reference_in_path(fn, name)) {
                        SPDLOG_INFO("[FMalloc::get] Found GMalloc \"{}\" at 0x{:x} (from vtable index {})", utility::narrow(name), *disp, i);
                        result = (FMalloc**)*disp;
                        return utility::ExhaustionResult::BREAK;
                    }
                }
            }

            return utility::ExhaustionResult::CONTINUE;
        });

        if (!result) {
            SPDLOG_ERROR("[FMalloc::get] Failed to find GMalloc");
            return nullptr;
        }

        // Since we found it now, go through the vtable and try to determine the indices
        // for Malloc, Realloc, and Free.
        const auto vtable = *(void***)*result;

        for (auto i = 1; i < 30; ++i) {
            const auto fn = (uintptr_t)vtable[i];

            if (IsBadReadPtr((void*)fn, sizeof(void*))) {
                break;
            }

            if (utility::find_pointer_in_path(fn, &VirtualAlloc)) {
                SPDLOG_INFO("[FMalloc::get] Found Malloc at index {}", i);
                s_malloc_index = i;
                break;
            }
        }

        if (!s_malloc_index.has_value()) {
            SPDLOG_ERROR("[FMalloc::get] Failed to find FMalloc::Malloc");
            return result;
        }

        /*for (auto i = *s_malloc_index + 1; i < 30; ++i) {
            const auto fn = (uintptr_t)vtable[i];

            if (IsBadReadPtr((void*)fn, sizeof(void*))) {
                break;
            }

            if (utility::find_pointer_in_path(fn, &VirtualAlloc) || utility::find_pointer_in_path(fn, &VirtualFree)) {
                SPDLOG_INFO("[FMalloc::get] Found Realloc at index {}", i);
                s_realloc_index = i;
                break;
            }
        }*/

        // The right one is whichever one doesn't return really early after malloc.
        // We can scan for VirtualAlloc/Free in SOME builds, but this doesn't always work.
        // Because in some builds, Realloc calls Malloc via an indirect call, which exhaustive_decode doesn't handle.
        // I could handle it with emulation but I don't want to.
        for (auto i = *s_malloc_index + 1; i < 30; ++i) {
            const auto fn = (uintptr_t)vtable[i];

            if (IsBadReadPtr((void*)fn, sizeof(void*))) {
                break;
            }

            uint32_t distance_from_ret = 0;
            utility::exhaustive_decode((uint8_t*)fn, 100, [&](utility::ExhaustionContext& ctx) -> utility::ExhaustionResult {
                ++distance_from_ret;
                return utility::ExhaustionResult::CONTINUE;
            });

            if (distance_from_ret > 15) {
                s_realloc_index = i;
                SPDLOG_INFO("[FMalloc::get] Found Realloc at index {}", i);
                break;
            }
        }

        if (!s_realloc_index.has_value()) {
            SPDLOG_ERROR("[FMalloc::get] Failed to find FMalloc::Realloc");
            return result;
        }

        for (auto i = *s_realloc_index + 1; i < 30; ++i) {
            const auto fn = (uintptr_t)vtable[i];

            if (IsBadReadPtr((void*)fn, sizeof(void*))) {
                break;
            }

            if (utility::find_pointer_in_path(fn, &VirtualFree)) {
                SPDLOG_INFO("[FMalloc::get] Found Free at index {}", i);
                s_free_index = i;
                break;
            }
        }

        if (!s_free_index.has_value()) {
            SPDLOG_ERROR("[FMalloc::get] Failed to find FMalloc::Free");
            return result;
        }

        return result;
    }();

    if (result == nullptr) {
        return nullptr;
    }

    return *result;
}
}