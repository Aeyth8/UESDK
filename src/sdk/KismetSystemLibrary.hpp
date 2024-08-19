#pragma once

#include "UClass.hpp"

namespace sdk {
class UKismetSystemLibrary {
public:
    static UClass* static_class();

public:
    static void execute_console_command(UObject* world_context_object, std::wstring_view command, UObject* specific_player = nullptr);

private:
};
}