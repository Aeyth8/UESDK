#pragma once

#include "UPrimitiveComponent.hpp"
#include "FName.hpp"

namespace sdk {
enum EControllerHand : uint8_t {
    Left = 0,
    Right = 1,
    AnyHand = 2,
};

class UMotionControllerComponent : public UPrimitiveComponent {
public:
    static UClass* static_class();

    int32_t get_player_index() const;
    EControllerHand get_hand() const;
    bool has_motion_source() const;
    FName& get_motion_source() const;

private:
};
}