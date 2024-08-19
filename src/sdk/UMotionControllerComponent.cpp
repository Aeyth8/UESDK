#include "UObjectArray.hpp"
#include "FProperty.hpp"

#include "UMotionControllerComponent.hpp"

namespace sdk {
UClass* UMotionControllerComponent::static_class() {
    static auto result = sdk::find_uobject<UClass>(L"Class /Script/HeadMountedDisplay.MotionControllerComponent");
    return result;
}

EControllerHand UMotionControllerComponent::get_hand() const {
    static sdk::FProperty* hand_prop = []() -> sdk::FProperty* {
        const auto c = UMotionControllerComponent::static_class();
        if (c == nullptr) {
            return nullptr;
        }

        return c->find_property(L"Hand");
    }();

    if (hand_prop == nullptr) {
        return EControllerHand::AnyHand;
    }

    return *hand_prop->get_data<EControllerHand>(this);
}

int32_t UMotionControllerComponent::get_player_index() const {
    static sdk::FProperty* player_index_prop = []() -> sdk::FProperty* {
        const auto c = UMotionControllerComponent::static_class();
        if (c == nullptr) {
            return nullptr;
        }

        return c->find_property(L"PlayerIndex");
    }();

    if (player_index_prop == nullptr) {
        return 0;
    }

    return *player_index_prop->get_data<int32_t>(this);
}

bool UMotionControllerComponent::has_motion_source() const {
    static sdk::FProperty* motion_source_prop = []() -> sdk::FProperty* {
        const auto c = UMotionControllerComponent::static_class();
        if (c == nullptr) {
            return nullptr;
        }

        return c->find_property(L"MotionSource");
    }();

    return motion_source_prop != nullptr;
}

FName& UMotionControllerComponent::get_motion_source() const {
    static sdk::FProperty* motion_source_prop = []() -> sdk::FProperty* {
        const auto c = UMotionControllerComponent::static_class();
        if (c == nullptr) {
            return nullptr;
        }

        return c->find_property(L"MotionSource");
    }();

    if (motion_source_prop == nullptr) {
        static FName result{};
        return result;
    }

    return *motion_source_prop->get_data<FName>(this);
}
}