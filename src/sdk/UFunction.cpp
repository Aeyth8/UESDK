#include <spdlog/spdlog.h>
#include <utility/String.hpp>

#include "UObjectArray.hpp"

#include "UFunction.hpp"

namespace sdk {
sdk::UClass* UFunction::static_class() {
    return (UClass*)sdk::find_uobject(L"Class /Script/CoreUObject.Function");
}

void UFunction::update_offsets() {
    if (s_attempted_update_offsets) {
        return;
    }

    s_attempted_update_offsets = true;

    SPDLOG_INFO("[UFunction] Updating offsets...");

    const auto cheat_manager_class = sdk::find_uobject<sdk::UClass>(L"Class /Script/Engine.CheatManager");

    if (cheat_manager_class == nullptr) {
        SPDLOG_ERROR("[UFunction] Failed to find CheatManager class");
        return;
    }
    
    const auto fly_fn = cheat_manager_class->find_function(L"Fly");

    if (fly_fn == nullptr) {
        SPDLOG_ERROR("[UFunction] Failed to find CheatManager::Fly function");
        return;
    }

    const auto start_raw = std::max(UStruct::s_properties_size_offset, UStruct::s_super_struct_offset) + sizeof(uint32_t);

    constexpr auto func_exec = 0x200;
    constexpr auto func_native = 0x400;
    constexpr auto func_public = 0x20000;

    const auto wanted_flags = func_exec | func_public | func_native;

    for (auto i = start_raw; i < 0x200; i+= sizeof(uint32_t)) try {
        const auto fn_flags = *(uint32_t*)((uintptr_t)fly_fn + i);

        if ((fn_flags & wanted_flags) == wanted_flags) {
            s_function_flags_offset = i;
            SPDLOG_INFO("[UFunction] Found function flags offset: 0x{:X}", s_function_flags_offset);
            SPDLOG_INFO("[UFunction] Flags: 0x{:X}", fn_flags);
            break;
        }
    } catch(...) {
        continue;
    }

    if (s_function_flags_offset == 0) {
        SPDLOG_ERROR("[UFunction] Failed to find function flags offset");
    } 
#ifdef UFUNCTION_TESTING
    else {
        // Walk the list of UFunctions and print out the exec ones for testing
        const auto uobjectarray = sdk::FUObjectArray::get();
        const auto ufunction_t = UFunction::static_class();

        for (auto i = 0; i < uobjectarray->get_object_count(); ++i) {
            const auto object_entry = uobjectarray->get_object(i);

            if (object_entry == nullptr) {
                continue;
            }

            const auto object = (sdk::UObject*)object_entry->object;

            if (object == nullptr) {
                continue;
            }

            if (!object->is_a(ufunction_t)) {
                continue;
            }

            const auto fn = (UFunction*)object;


            if ((fn->get_function_flags() & func_exec) == func_exec) {
                SPDLOG_INFO("[UFunction] Found exec function: {}", utility::narrow(fn->get_full_name()));
            }
        }
    }
#endif

    SPDLOG_INFO("[UFunction] Done");
}
}