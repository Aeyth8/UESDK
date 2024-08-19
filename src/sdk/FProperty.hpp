#pragma once

#include <cstdint>

#include "FField.hpp"
#include "UClass.hpp"

namespace sdk {
class UProperty;
class FStructProperty;

class FProperty : public FField {
public:
    int32_t get_offset() const {
        return *(int32_t*)((uintptr_t)this + s_offset_offset);
    }

    template<typename T>
    T* get_data(void* object) const {
        return (T*)((uintptr_t)object + get_offset());
    }

    template<typename T>
    T* get_data(const void* object) const {
        return (T*)((uintptr_t)object + get_offset());
    }

    uint64_t get_property_flags() const {
        return *(uint64_t*)((uintptr_t)this + s_property_flags_offset);
    }

    bool is_param() const {
        return (get_property_flags() & (uint64_t)0x80) != 0;
    }

    bool is_out_param() const {
        return (get_property_flags() & (uint64_t)0x100) != 0;
    }

    bool is_return_param() const {
        return (get_property_flags() & (uint64_t)0x400) != 0;
    }

    bool is_reference_param() const {
        return (get_property_flags() & (uint64_t)0x8000000) != 0;
    }

    bool is_pod() const {
        return (get_property_flags() & (uint64_t)0x40000000) != 0;
    }

    // Given xyz props from FVector, find the offset which matches up with all of them
    static void bruteforce_fproperty_offset(FProperty* x_prop, FProperty* y_prop, FProperty* z_prop);
    static void update_offsets(); // for other offsets like PropertyFlags

protected:
    static inline bool s_attempted_update_offsets{false};
    static inline uint32_t s_offset_offset{0x0}; // idk
    static inline uint32_t s_property_flags_offset{0x0}; // idk

    friend class UStruct;
    friend class UProperty;
    friend class FStructProperty;
};
}