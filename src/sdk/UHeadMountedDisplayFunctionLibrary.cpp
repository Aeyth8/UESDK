#include "UObjectArray.hpp"

#include "UHeadMountedDisplayFunctionLibrary.hpp"

namespace sdk {
UClass* UHeadMountedDisplayFunctionLibrary::static_class() {
    static auto c = sdk::find_uobject<UClass>(L"Class /Script/HeadMountedDisplay.HeadMountedDisplayFunctionLibrary");
    return c;
}
}