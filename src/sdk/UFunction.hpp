#pragma once

#include "UClass.hpp"

namespace sdk {
class UFunction : public UStruct {
public:
    static UClass* static_class();
    static void update_offsets();

    using NativeFunction = void(*)(sdk::UObject*, void*, void*);
    NativeFunction& get_native_function() const {
        return *(NativeFunction*)((uintptr_t)this + s_native_function_offset);
    }

    uint32_t& get_function_flags() const {
        return *(uint32_t*)((uintptr_t)this + s_function_flags_offset);
    }

    static uint32_t get_native_function_offset() {
        return s_native_function_offset;
    }

public: // flags
    bool is_final() const {
        return (get_function_flags() & (uint32_t)0x1) != 0;
    }

    bool is_required_api() const {
        return (get_function_flags() & (uint32_t)0x2) != 0;
    }

    bool is_blueprint_authority_only() const {
        return (get_function_flags() & (uint32_t)0x4) != 0;
    }

    bool is_blueprint_cosmetic() const {
        return (get_function_flags() & (uint32_t)0x8) != 0;
    }

    bool is_net() const {
        return (get_function_flags() & (uint32_t)0x40) != 0;
    }

    bool is_net_reliable() const {
        return (get_function_flags() & (uint32_t)0x80) != 0;
    }

    bool is_net_request() const {
        return (get_function_flags() & (uint32_t)0x100) != 0;
    }

    bool is_exec() const {
        return (get_function_flags() & (uint32_t)0x200) != 0;
    }

    bool is_native() const {
        return (get_function_flags() & (uint32_t)0x400) != 0;
    }

    bool is_event() const {
        return (get_function_flags() & (uint32_t)0x800) != 0;
    }

    bool is_net_response() const {
        return (get_function_flags() & (uint32_t)0x1000) != 0;
    }

    bool is_static() const {
        return (get_function_flags() & (uint32_t)0x2000) != 0;
    }

    bool is_net_multicast() const {
        return (get_function_flags() & (uint32_t)0x4000) != 0;
    }

    bool is_ubergraph_function() const {
        return (get_function_flags() & (uint32_t)0x8000) != 0;
    }

    bool is_multicast_delegate() const {
        return (get_function_flags() & (uint32_t)0x10000) != 0;
    }

    bool is_public() const {
        return (get_function_flags() & (uint32_t)0x20000) != 0;
    }

    bool is_private() const {
        return (get_function_flags() & (uint32_t)0x40000) != 0;
    }

    bool is_protected() const {
        return (get_function_flags() & (uint32_t)0x80000) != 0;
    }

    bool is_delegate() const {
        return (get_function_flags() & (uint32_t)0x100000) != 0;
    }

    bool is_net_server() const {
        return (get_function_flags() & (uint32_t)0x200000) != 0;
    }

    bool has_out_params() const {
        return (get_function_flags() & (uint32_t)0x400000) != 0;
    }

    bool has_defaults() const {
        return (get_function_flags() & (uint32_t)0x800000) != 0;
    }

    bool is_net_client() const {
        return (get_function_flags() & (uint32_t)0x1000000) != 0;
    }

    bool is_dll_import() const {
        return (get_function_flags() & (uint32_t)0x2000000) != 0;
    }

    bool is_blueprint_callable() const {
        return (get_function_flags() & (uint32_t)0x4000000) != 0;
    }

    bool is_blueprint_event() const {
        return (get_function_flags() & (uint32_t)0x8000000) != 0;
    }

    bool is_blueprint_pure() const {
        return (get_function_flags() & (uint32_t)0x10000000) != 0;
    }

    bool is_editor_only() const {
        return (get_function_flags() & (uint32_t)0x20000000) != 0;
    }

    bool is_const() const {
        return (get_function_flags() & (uint32_t)0x40000000) != 0;
    }

    bool is_net_validate() const {
        return (get_function_flags() & (uint32_t)0x80000000) != 0;
    }

private:
    // Updated separately
    static inline uint32_t s_native_function_offset{0x0};

    static inline bool s_attempted_update_offsets{false};
    static inline uint32_t s_function_flags_offset{0x0}; // idk

    friend class UStruct;
};
}