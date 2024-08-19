#pragma once

#include <optional>
#include <cstdint>
#include <string_view>

#include "TArray.hpp"

namespace sdk {
enum EFindName {
    Find,
    Add
};

struct FName {
    using ConstructorFn = void* (*)(FName*, const wchar_t*, uint32_t);
    static std::optional<ConstructorFn> get_constructor();

    using ToStringFn = TArray<wchar_t>* (*)(const FName*, TArray<wchar_t>*);
    static std::optional<ToStringFn> get_to_string();

    static inline bool s_is_case_preserving{false};
    static inline bool s_checked_case_preserving{false};

    FName() 
    {
        
    }
    FName(std::wstring_view name, EFindName find_type = EFindName::Add);
    std::wstring to_string() const;

    // Version meant to be used when bruteforcing through memory
    // usually through UObjectArray initialization
    // Has some extra checks to make sure we don't crash when calling to_string
    // In some games ToString crashes even with a try-catch so this is the best we can do
    // this doesnt remove the numbers, just doesnt allow FNames with numbers to be converted
    std::wstring to_string_no_numbers() const {
        if (this->a1 == 0) {
            return L"None";
        }

        // Fix for case preserving?
        if (this->a1 != this->a2) {
            if (this->a2 != 0) {
                return L"None";
            }
        }

        const auto block_index = (uint32_t)(this->a1 >> 16);
        
        // uhh if this happens we will cause a crash that is unrecoverable
        // even with a try-catch
        if (block_index > (1 << 13)) {
            return L"None";
        }

        return to_string();
    }

    std::wstring to_string_remove_numbers() const;

    int32_t get_number() const {
        if (s_is_case_preserving) {
            return *(int32_t*)((uintptr_t)&a2 + 4);
        }

        return a2;
    }

    void set_number(int32_t number) {
        if (s_is_case_preserving) {
            *(int32_t*)((uintptr_t)&a2 + 4) = number;
        } else {
            a2 = number;
        }
    }

    int32_t a1{0};
    int32_t a2{0};
};

struct FNameCasePreserving : public FName {
    int32_t a3{0};
};
}