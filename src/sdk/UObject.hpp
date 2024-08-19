#pragma once

#include "UObjectBase.hpp"

#include "nlohmann/json.hpp"

namespace sdk {
class UClass;

class UObject : public UObjectBase {
public:
    static UClass* static_class();

    void* get_property_data(std::wstring_view name) const;
    template <typename T>
    T& get_property(std::wstring_view name) const {
        return *(T*)get_property_data(name);
    }

    bool get_bool_property(std::wstring_view name) const;
    void set_bool_property(std::wstring_view name, bool value);

    void* get_uproperty_data(std::wstring_view name) const;
    template <typename T>
    T& get_uproperty(std::wstring_view name) const {
        return *(T*)get_uproperty_data(name);
    }

    template <typename T>
    bool is_a() const {
        return is_a(T::static_class());
    }

    bool is_a(UClass* cmp) const;

    nlohmann::json to_json(const std::vector<std::wstring>& properties = {}) const;
    void from_json(const nlohmann::json& j);
};
}