#pragma once

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/helpers/time/Time.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprutils/math/Box.hpp>
#include <hyprutils/math/Vector2D.hpp>

void render_window_at_box(
    PHLWINDOW window,
    PHLMONITOR monitor,
    const Time::steady_tp& time,
    CBox box
);

void render_dragged_window_preview(
    PHLWINDOW window,
    PHLMONITOR monitor,
    const Time::steady_tp& time,
    const Vector2D& mouse_coords,
    float preview_scale
);
