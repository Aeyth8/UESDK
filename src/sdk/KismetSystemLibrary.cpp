#include <spdlog/spdlog.h>

#include "UObjectArray.hpp"

#include "KismetSystemLibrary.hpp"

namespace sdk {
UClass* UKismetSystemLibrary::static_class() {
    static auto result = (UClass*)sdk::find_uobject(L"Class /Script/Engine.KismetSystemLibrary");
    return result;
}

void UKismetSystemLibrary::execute_console_command(UObject* world_context_object, std::wstring_view command, UObject* specific_player) {
    const auto c = static_class();
    if (c == nullptr) {
        return;
    }

    const auto dfo = c->get_class_default_object();

    if (dfo == nullptr) {
        return;
    }

    static auto fn = c->find_function(L"ExecuteConsoleCommand");

    if (fn == nullptr) {
        SPDLOG_ERROR("[KismetSystemLibrary] Failed to find ExecuteConsoleCommand function");
        return;
    }

    struct {
        UObject* world_context_object;
        sdk::TArrayLite<wchar_t> command{};
        UObject* specific_player;
    } params;

    params.world_context_object = world_context_object;
    //params.command = command;
    params.specific_player = specific_player;

    params.command.data = (wchar_t*)command.data();
    params.command.count = command.size();
    params.command.capacity = command.size();

    dfo->process_event(fn, &params);
}
}