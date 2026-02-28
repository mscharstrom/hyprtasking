#include "render.hpp"

#include <exception>
#include <variant>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/protocols/core/Compositor.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/pass/RendererHintsPassElement.hpp>
#include <hyprland/src/render/pass/TexPassElement.hpp>
#include <hyprutils/math/Vector2D.hpp>

#include "globals.hpp"
#include "types.hpp"

static bool should_render_dragged_window_preview(PHLWINDOW window, PHLMONITOR monitor) {
    if (!window || !monitor)
        return false;

    if (!g_layoutManager)
        return false;

    const auto& drag_controller = g_layoutManager->dragController();
    if (!drag_controller || drag_controller->mode() != MBIND_MOVE)
        return false;
    if (!drag_controller->dragThresholdReached())
        return false;

    if (window->m_workspace == nullptr || window->m_monitor == nullptr)
        return false;

    if (window->m_workspace->m_monitor != monitor)
        return false;

    return true;
}

static SP<CTexture> get_dragged_window_preview_texture(PHLWINDOW window) {
    if (!window)
        return nullptr;

    const auto surface = window->resource();
    if (!surface)
        return nullptr;

    return surface->m_current.texture;
}

struct SSurfacePreviewContext {
    PHLWINDOW window;
    CBox window_box;
    CBox clipped_box;
    float preview_scale = 1.F;
};

static void add_surface_tree_preview(SP<CWLSurfaceResource> surface, const Vector2D& offset, void* data) {
    if (!surface || data == nullptr)
        return;

    const auto* ctx = static_cast<SSurfacePreviewContext*>(data);
    const auto texture = surface->m_current.texture;
    if (!texture)
        return;

    Vector2D surface_size = surface->m_current.size;
    if (surface_size.x <= 0.F || surface_size.y <= 0.F)
        surface_size = texture->m_size;
    if (surface_size.x <= 0.F || surface_size.y <= 0.F)
        return;

    const Vector2D scaled_pos = ctx->window_box.pos() + offset * ctx->preview_scale;
    const Vector2D scaled_size = surface_size * ctx->preview_scale;
    const CBox surface_box = {scaled_pos, scaled_size};

    CTexPassElement::SRenderData pass;
    pass.tex = texture;
    pass.box = surface_box;
    pass.a = ctx->window->m_alpha ? ctx->window->m_alpha->value() : 1.F;
    pass.blurA = pass.a;
    pass.round = offset == Vector2D() ? sc<int>(ctx->window->rounding()) : 0;
    pass.roundingPower = ctx->window->roundingPower();
    pass.clipBox = ctx->clipped_box;
    pass.blockBlurOptimization = true;

    g_pHyprRenderer->m_renderPass.add(makeUnique<CTexPassElement>(pass));
}

// Note: box is relative to (0, 0), not monitor
void render_window_at_box(
    PHLWINDOW window,
    PHLMONITOR monitor,
    const Time::steady_tp& time,
    CBox box
) {
    if (!window || !monitor)
        return;

    box.x -= monitor->m_position.x;
    box.y -= monitor->m_position.y;

    if (box.w <= 0.F || box.h <= 0.F)
        return;

    bool added_render_hint = false;
    auto clear_render_hint = [&] {
        if (!added_render_hint)
            return;

        g_pHyprRenderer->m_renderPass.add(makeUnique<CRendererHintsPassElement>(
            CRendererHintsPassElement::SData {SRenderModifData {}}
        ));
        added_render_hint = false;
    };

    try {
        const Vector2D real_size = window->m_realSize->value();
        if (real_size.x <= 0.F || real_size.y <= 0.F)
            return;

        const float scale = box.w / real_size.x;
        if (scale <= 0.F)
            return;

        const Vector2D transform =
            (monitor->m_position - window->m_realPosition->value() + box.pos() / scale)
            * monitor->m_scale;

        SRenderModifData data {};
        data.modifs.push_back({SRenderModifData::eRenderModifType::RMOD_TYPE_TRANSLATE, transform});
        data.modifs.push_back({SRenderModifData::eRenderModifType::RMOD_TYPE_SCALE, scale});
        g_pHyprRenderer->m_renderPass.add(
            makeUnique<CRendererHintsPassElement>(CRendererHintsPassElement::SData {data})
        );
        added_render_hint = true;

        g_pHyprRenderer->damageWindow(window);
        ((render_window_t)render_window)(
            g_pHyprRenderer.get(),
            window,
            monitor,
            time,
            true,
            RENDER_PASS_MAIN,
            false,
            false
        );
    } catch (const std::exception& e) {
        Log::logger->log(
            Log::ERR,
            "[Hyprtasking] skipped dragged-window preview render after exception: {}",
            e.what()
        );
    }

    clear_render_hint();
}

void render_dragged_window_preview(
    PHLWINDOW window,
    PHLMONITOR monitor,
    const Time::steady_tp& time,
    const Vector2D& mouse_coords,
    float preview_scale
) {
    if (preview_scale <= 0.F || !should_render_dragged_window_preview(window, monitor))
        return;

    try {
        const auto surface = window->resource();
        if (!surface || !get_dragged_window_preview_texture(window))
            return;

        const CBox window_box = window->getWindowMainSurfaceBox()
                                    .translate(-mouse_coords)
                                    .scale(preview_scale)
                                    .translate(mouse_coords);
        const CBox clipped_box = window_box.intersection(monitor->logicalBox());
        if (clipped_box.empty())
            return;

        SSurfacePreviewContext ctx {
            .window = window,
            .window_box = window_box,
            .clipped_box = clipped_box,
            .preview_scale = preview_scale,
        };

        surface->breadthfirst(add_surface_tree_preview, &ctx);
    } catch (const std::exception& e) {
        Log::logger->log(
            Log::ERR,
            "[Hyprtasking] skipped dragged-window surface preview after exception: {}",
            e.what()
        );
    }
}
