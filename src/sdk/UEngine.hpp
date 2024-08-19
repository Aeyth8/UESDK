#pragma once

#include "UObject.hpp"
#include "UWorld.hpp"

namespace sdk {
class UWorld;
class ULocalPlayer;
class APawn;
class APlayerController;

class UEngine : public sdk::UObject {
public:
    static UEngine** get_lvalue();
    static UEngine* get();

    ULocalPlayer* get_localplayer(int32_t index = 0);
    APawn* get_localpawn(int32_t index = 0);
    UWorld* get_world();

    struct EngineOutputDevice {
        virtual ~EngineOutputDevice() {};
        virtual void Serialize(const wchar_t* text, size_t verbosity, void* category, void* r9) {
        };
        virtual size_t a2(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a3(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a4(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a5(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a6(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a7(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a8(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a9(void* rcx, void* rdx, void* r8, void* r9) {return 0;}
        virtual size_t a10(void* rcx, void* rdx, void* r8, void* r9) {return 0;}

    private:
        char unk_padding[0x100]{}; // just in case
    };

    void exec(std::wstring_view command) {
        exec(get_world(), command);
    }
    void exec(UWorld* world, std::wstring_view command) {
        EngineOutputDevice output_device{};

        exec(world, command, &output_device);
    }
    void exec(UWorld* world, std::wstring_view command, void* output_device);

public:
    void initialize_hmd_device();
    static std::optional<uintptr_t> get_initialize_hmd_device_address();
    static std::optional<uintptr_t> get_emulatestereo_string_ref_address();
    static std::optional<uintptr_t> get_stereo_rendering_device_offset();
};
}