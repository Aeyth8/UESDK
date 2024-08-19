#include <array>
#include <spdlog/spdlog.h>

#include <bdshemu.h>

#include <utility/Emulation.hpp>
#include <utility/Module.hpp>

#include "UGameViewportClient.hpp"

#include "FViewport.hpp"

namespace sdk {
namespace detail {
std::optional<size_t> debug_canvas_index{};
std::optional<size_t> viewport_size_xy_index{};
bool already_updated{false};

bool all_indices_found() {
    return debug_canvas_index.has_value() && viewport_size_xy_index.has_value();
}

// Not a lambda because it makes the try catch block cleaner, less indentation
void update() try {
    if (already_updated) {
        return;
    }

    already_updated = true;

    SPDLOG_INFO("[FViewport] Finding GetDebugCanvas and GetViewportSizeXY indices...");

    // The code that calls InViewport->GetDebugCanvas() is in UGameViewportClient::Draw, so we need to find that function first
    // as well as GetViewportSizeXY
    const auto viewport_draw_fn = UGameViewportClient::get_draw_function();

    if (!viewport_draw_fn) {
        return;
    }

    const auto module_within = utility::get_module_within(*viewport_draw_fn);

    if (!module_within) {
        SPDLOG_ERROR("[FViewport] Failed to find module that FViewport::Draw is in");
        return;
    }

    utility::ShemuContext ctx{*module_within};
    
    // Set up our fake registers
    std::array<uint8_t, 0x200> fake_rcx{};
    std::array<uint8_t, 0x200> fake_rdx{};
    std::array<uint8_t, 0x200> fake_r8{};
    std::array<uint8_t, 0x200> fake_r9{};
    std::array<uintptr_t, 100> fake_vtable{};

    // Set up our fake RDX's vtable so we don't cause any access violations
    *(uintptr_t*)fake_rdx.data() = (uintptr_t)fake_vtable.data();

    // Set up our registers
    ctx.ctx->Registers.RegRip = *viewport_draw_fn;
    ctx.ctx->Registers.RegRcx = (ND_UINT64)fake_rcx.data();
    ctx.ctx->Registers.RegRdx = (ND_UINT64)fake_rdx.data();
    ctx.ctx->Registers.RegR8 = (ND_UINT64)fake_r8.data();
    ctx.ctx->Registers.RegR9 = (ND_UINT64)fake_r9.data();
    ctx.ctx->MemThreshold = 100;

    // Emulate until we find something that looks like a call [reg+disp]
    // but also double check that the This pointer (RCX) is equal to our initialized fake_rdx (which is the FViewport* pointer)
    // TODO: Maybe we could handle stuff that calls registers directly if there's some math on it beforehand like add rdx, 0x10
    utility::emulate(*module_within, *viewport_draw_fn, 100, ctx, [&](const utility::ShemuContextExtended& ctx) -> utility::ExhaustionResult {
        SPDLOG_INFO("Emulating instruction: {:x}", ctx.ctx->ctx->Registers.RegRip);

        if (all_indices_found()) {
            return utility::ExhaustionResult::BREAK;
        }

        if (ctx.next.writes_to_memory) {
            // just do not care
            return utility::ExhaustionResult::STEP_OVER;
        }

        if (ctx.ctx->ctx->Registers.RegRcx == (uintptr_t)fake_rdx.data()) {
            SPDLOG_INFO("[FViewport] rip: {:x}, rcx: {:x}", ctx.ctx->ctx->Registers.RegRip, ctx.ctx->ctx->Registers.RegRcx);
        }

        if (std::string_view{ctx.next.ix.Mnemonic}.starts_with("CALL")) {
            SPDLOG_INFO("[FViewport] Examining CALL at {:x}", ctx.ctx->ctx->Registers.RegRip);

            if (ctx.next.ix.IsRipRelative) {
                // Null out RCX so we don't accidentally find something else on the next iteration
                // We can always assume that calls trample RCX so this is safe
                ctx.ctx->ctx->Registers.RegRcx = 0;

                SPDLOG_INFO("[FViewport] Skipping RIP-relative call");
                return utility::ExhaustionResult::STEP_OVER;
            }

            // Check if any opcode is a register
            for (auto i = 0; i < ctx.next.ix.OperandsCount; ++i) {
                const auto& op = ctx.next.ix.Operands[i];

                if (op.Type == ND_OPERAND_TYPE::ND_OP_MEM && op.Info.Memory.HasBase && op.Info.Memory.HasDisp) {
                    if (ctx.ctx->ctx->Registers.RegRcx == (ND_UINT64)fake_rdx.data()) {
                        const auto offset = op.Info.Memory.Disp;
                        const auto index = offset / sizeof(void*);

                        // First one is always GetDebugCanvas
                        // Second one is always GetViewportSizeXY
                        if (!debug_canvas_index) {
                            debug_canvas_index = index;   
                            SPDLOG_INFO("[FViewport] Found GetDebugCanvas index: {} (offset {:x})", *debug_canvas_index, *debug_canvas_index * sizeof(void*));
                        } else if (!viewport_size_xy_index) {
                            viewport_size_xy_index = index;
                            SPDLOG_INFO("[FViewport] Found GetViewportSizeXY index: {} (offset {:x})", *viewport_size_xy_index, *viewport_size_xy_index * sizeof(void*));
                        } else {
                            SPDLOG_INFO("[FViewport] Found all indices");
                            return utility::ExhaustionResult::BREAK;
                        }
                    }
                }
            }

            // Null out RCX so we don't accidentally find something else on the next iteration
            // We can always assume that calls trample RCX so this is safe
            ctx.ctx->ctx->Registers.RegRcx = 0;
            
            // just do not care
            return utility::ExhaustionResult::STEP_OVER;
        }

        return utility::ExhaustionResult::CONTINUE;
    });

    if (!debug_canvas_index) {
        SPDLOG_ERROR("[FViewport] Failed to find GetDebugCanvas index");
    }

    if (!viewport_size_xy_index) {
        SPDLOG_ERROR("[FViewport] Failed to find GetViewportSizeXY index");
    }
} catch(...) {
    SPDLOG_ERROR("[FViewport] Failed to find GetDebugCanvas index, exception occurred during scan");
}
}

std::optional<size_t> FViewport::get_debug_canvas_index() {
    detail::update();

    return detail::debug_canvas_index;
}

std::optional<size_t> FViewport::get_viewport_size_xy_index() {
    detail::update();

    return detail::viewport_size_xy_index;
}
}