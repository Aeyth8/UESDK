#include "UObjectArray.hpp"

#include "ScriptVector.hpp"

namespace sdk {
UScriptStruct* ScriptVector::static_struct() {
    static auto modern_class = sdk::find_uobject<UScriptStruct>(L"ScriptStruct /Script/CoreUObject.Vector");
    static auto old_class = modern_class == nullptr ? sdk::find_uobject<UScriptStruct>(L"ScriptStruct /Script/CoreUObject.Object.Vector") : nullptr;

    return modern_class != nullptr ? modern_class : old_class;
}
}