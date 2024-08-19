#include "UObjectArray.hpp"
#include "UClass.hpp"

#include "UPrimitiveComponent.hpp"

namespace sdk {
UClass* UPrimitiveComponent::static_class() {
    static auto ptr = sdk::find_uobject<UClass>(L"Class /Script/Engine.PrimitiveComponent");
    return ptr;
}

std::expected<bool, common::UFunctionError> UPrimitiveComponent::set_render_in_main_pass(bool value) {
    const auto c = UPrimitiveComponent::static_class();
    if (c == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_CLASS);
    }

    static const auto func = c->find_function(L"SetRenderInMainPass");
    if (func == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_FUNCTION);
    }

    struct {
        bool value{};
        char pad[7]{};
    } params{};

    params.value = value;

    this->process_event(func, &params);
    return true;
}

bool UPrimitiveComponent::is_rendering_in_main_pass() const {
    return this->get_bool_property(L"bRenderInMainPass");
}

std::expected<bool, common::UFunctionError> UPrimitiveComponent::set_render_custom_depth(bool value) {
    const auto c = UPrimitiveComponent::static_class();
    if (c == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_CLASS);
    }

    static const auto func = c->find_function(L"SetRenderCustomDepth");
    if (func == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_FUNCTION);
    }

    struct {
        bool value{};
        char pad[7]{};
    } params{};

    params.value = value;

    this->process_event(func, &params);
    return true;
}

bool UPrimitiveComponent::is_rendering_custom_depth() const {
    return this->get_bool_property(L"bRenderCustomDepth");
}

std::expected<bool, common::UFunctionError> UPrimitiveComponent::set_owner_no_see(bool value) {
    const auto c = UPrimitiveComponent::static_class();
    if (c == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_CLASS);
    }

    static const auto func = c->find_function(L"SetOwnerNoSee");
    if (func == nullptr) {
        return std::unexpected(common::UFunctionError::MISSING_FUNCTION);
    }

    struct {
        bool value{};
        char pad[7]{};
    } params{};

    params.value = value;

    this->process_event(func, &params);
    return true;
}
}