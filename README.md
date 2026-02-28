<div align="center">
  <h1>Hyprtasking</h1>
  <p>Powerful workspace management plugin, packed with features.</p>
</div>

> [!Important]
> - This fork supports Hyprland releases `v0.46.2-v0.54.0`.
> - Hyprland `v0.54.0` compatibility is provided by this fork at `mscharstrom/hyprtasking`.
> - This repository is a fork of `raymondbian/hyprtasking` and remains distributed under the BSD-3-Clause license. See [LICENSE](./LICENSE).

> [!Note]
> This repository is a personal fork used for development, testing, and local workflow changes.
> It exists so changes can be iterated independently without affecting the original upstream project.
> Behavior and feature set may differ from upstream at any time.

> [!Warning]
> This fork is intentionally opinionated and may contain workflow-specific behavior.
> If you want the original project behavior, use the upstream repository instead of this fork.

https://github.com/user-attachments/assets/8d6cdfd2-2b17-4240-a117-1dbd2231ed4e

#### [Jump To Installation](#Installation)

#### [See Configuration](#Configuration)

## Roadmap

- [ ] Modular Layouts
    - [x] Grid layout
    - [x] Linear layout
    - [ ] Minimap layout
- [x] Mouse controls
    - [x] Exit into workspace (hover, click)
    - [x] Drag and drop windows
- [ ] Keyboard controls
    - [x] Switch workspaces with direction
    - [ ] Switch workspaces with absolute number
- [x] Multi-monitor support (tested)
- [x] Monitor scaling support (tested)
- [x] Animation support
- [x] Configurability
    - [x] Overview exit behavior
    - [x] Number of visible workspaces
    - [x] Custom workspace layouts
    - [x] Toggle behavior
    - [x] Toggle keybind
- [ ] Touch and gesture support
- [ ] Overview layers

## Installation

### Fork Notes

- This fork is intended for personal and experimental use.
- Releases, behavior, and compatibility may diverge from upstream.
- Configuration examples in this README reflect this fork, not necessarily the original project.
- If you are testing local changes, prefer loading the plugin directly from your local build output instead of relying on `hyprpm`.

### Hyprpm

``` 
hyprpm add https://github.com/mscharstrom/hyprtasking
hyprpm enable hyprtasking
```

### Nix

Add hyprtasking to your flake inputs
```nix
# flake.nix
{
  inputs = {
    hyprland.url = "github:hyprwm/Hyprland/v0.49.0";

    hyprtasking = {
      url = "github:mscharstrom/hyprtasking";
      inputs.hyprland.follows = "hyprland";
    };
  };
  # ...
}

```

Include the plugin in the hyprland home manager options

```nix
# home.nix
{ inputs, ... }:
{
  wayland.windowManager.hyprland = {
    plugins = [
      inputs.hyprtasking.packages.${pkgs.system}.hyprtasking
    ];
  }
}
```

### Manual

To build, have hyprland headers installed on the system and then:

```
meson setup build
cd build && meson compile
```

Then use `hyprctl plugin load` to load the absolute path to the `.so` file:

```
hyprctl plugin load "$(realpath libhyprtasking.so)"
```

If you are developing this fork locally and want Hyprland to keep loading your local build across reloads/reboots, add this to your Hyprland config instead:

```ini
plugin = /absolute/path/to/build/libhyprtasking.so
```

and disable the `hyprpm`-managed copy so only one version is loaded.

## Fork Differences

Current notable differences in this fork:

- Hyprland `v0.54.0` compatibility work is maintained here.
- Drag preview behavior was adjusted for safer rendering on newer Hyprland versions.
- Dragging windows between workspaces restores tiled state more aggressively than upstream.
- Floating windows dropped onto a workspace that already contains tiled windows are converted into tiled windows.

These details may continue to change as this fork is used for ongoing development.

## Known Issues

- Some application previews may still behave differently depending on how the client renders its surfaces.
- Browser windows and other complex clients may require further preview/rendering adjustments as Hyprland internals evolve.
- This fork prioritizes local workflow behavior over strict parity with upstream.

## Development Notes

- This fork is maintained primarily for local use and experimentation.
- Parts of the development and debugging work in this fork were done with assistance from OpenAI Codex.

## Usage

### Opening Overview

- Bind `hyprtasking:toggle, all` to a keybind to open/close the overlay on all monitors.
- Bind `hyprtasking:toggle, cursor` to a keybind to open the overlay on one monitor and close on all monitors.
- Swipe up/down on a touchpad device to open/close the overlay on one monitor.
- See [below](#Configuration) for configuration options.

### Interaction

- Workspace Transitioning:
    - Open the overlay, then use **right click** to switch to a workspace
    - Use the directional dispatchers `hyprtasking:move` to switch to a workspace
- Window management:
    - **Left click** to drag and drop windows around

## Configuration

Example below:

```
bind = SUPER, tab, hyprtasking:toggle, cursor
bind = SUPER, space, hyprtasking:toggle, all
# NOTE: the lack of a comma after hyprtasking:toggle!
bind = , escape, hyprtasking:if_active, hyprtasking:toggle cursor


bind = SUPER, X, hyprtasking:killhovered

bind = SUPER, H, hyprtasking:move, left
bind = SUPER, J, hyprtasking:move, down
bind = SUPER, K, hyprtasking:move, up
bind = SUPER, L, hyprtasking:move, right

plugin {
    hyprtasking {
        layout = grid

        gap_size = 20
        bg_color = 0xff26233a
        border_size = 4
        exit_on_hovered = false
        warp_on_move_window = 1
        close_overview_on_reload = true

        drag_button = 0x110 # left mouse button
        select_button = 0x111 # right mouse button
        # for other mouse buttons see <linux/input-event-codes.h>

        gestures {
            enabled = true
            move_fingers = 3
            move_distance = 300
            open_fingers = 4
            open_distance = 300
            open_positive = true
        }

        grid {
            rows = 3
            cols = 3
            loop = false
            gaps_use_aspect_ratio = false
        }

        linear {
            height = 400
            scroll_speed = 1.0
            blur = false
        }
    }
}
```

### Dispatchers

- `hyprtasking:if_active, ARG` takes in a dispatch command (one that would be used after `hyprctl dispatch ...`) that will be dispatched only if the cursor overview is active.
    - Allows you to use e.g. `escape` to close the overview when it is active. See the [example config](#configuration) for more info.

- `hyprtasking:if_not_active, ARG` same as above, but if the overview is not active.

- `hyprtasking:toggle, ARG` takes in 1 argument that is either `cursor` or `all`
    - if the argument is `all`, then
        - if all overviews are hidden, then all overviews will be shown
        - otherwise all overviews will be hidden
    - if the argument is `cursor`, then
        - if current monitor's overview is hidden, then it will be shown
        - otherwise all overviews will be hidden

- `hyprtasking:move, ARG` takes in 1 argument that is one of `up`, `down`, `left`, `right`
    - when dispatched, hyprtasking will switch workspaces with a nice animation

- `hyprtasking:movewindow, ARG` takes in 1 argument that is one of `up`, `down`, `left`, `right`
    - when dispatched, hyprtasking will 1. move the hovered window to the workspace in the given direction relative to the window, and 2. switch to that workspace.

- `hyprtasking:killhovered` behaves similarly to the standard `killactive` dispatcher with focus on hover
    - when dispatched, hyprtasking will the currently hovered window, useful when the overview is active.
    - this dispatcher is designed to **replace** killactive, it will work even when the overview is **not active**.

Example conditional bind:

```
bind = $mod, S, hyprtasking:if_active, hyprtasking:killhovered
bind = $mod, S, hyprtasking:if_not_active, killactive
```

### Config Options

All options should are prefixed with `plugin:hyprtasking:`.

| Option | Type | Description | Default |
| --- | --- | --- | --- |
| `layout` | `Hyprlang::STRING` | The layout to use, either `grid` or `linear` | `grid` |
| `bg_color` | `Hyprlang::INT` | The color of the background of the overlay | `0x000000FF` |
| `gap_size` | `Hyprlang::FLOAT` | The width in logical pixels of the gaps between workspaces | `8.f` |
| `border_size` | `Hyprlang::FLOAT` | The width in logical pixels of the borders around workspaces | `4.f` |
| `exit_on_hovered` | `Hyprlang::INT` | If true, hiding the workspace will exit to the hovered workspace instead of the active workspace. | `false` |
| `warp_on_move_window` | `Hyprlang::INT` | Works the same as `cursor:warp_on_change_workspace` (see [wiki](https://wiki.hypr.land/Configuring/Variables/#cursor)) but with `hyprtasking:movewindow` dispathcer. <br> `cursor:warp_on_change_workspace` works only with `hyprtasking:move` dispathcer | `1` |
| `close_overview_on_reload ` | `Hyprlang::INT` | Whether to close the overview if its type didn't type didn't change after hyprland config reload | `true` |
| `drag_button` | `Hyprlang::INT` | The mouse button to use to drag windows around | `0x110` |
| `select_button` | `Hyprlang::INT` | The mouse button to use to select a workspace | `0x111` |
| `gestures:enabled` | `Hyprlang::INT` | Whether or not to enable gestures | `true` |
| `gestures:move_fingers` | `Hyprlang::INT` | The number of fingers to use for the "move" gesture | `3` |
| `gestures:move_distance` | `Hyprlang::FLOAT` | How large of a swipe on the touchpad corresponds to the width of a workspace | `300.f` |
| `gestures:open_fingers` | `Hyprlang::INT` | The number of fingers to use for the "open" gesture | `4` |
| `gestures:open_distance` | `Hyprlang::FLOAT` | How large of a swipe on the touchpad is needed for the "open" gesture | `300.f` |
| `gestures:open_positive` | `Hyprlang::INT` | `true` if swiping up should open the overlay, `false` otherwise | `true` |
| `grid:rows` | `Hyprlang::INT` | The number of rows to display on the grid overlay | `3` |
| `grid:cols` | `Hyprlang::INT` | The number of columns to display on the grid overlay | `3` |
| `grid:loop` | `Hyprlang::INT` | When enabled, moving right at the far right of the grid will wrap around to the leftmost workspace, etc. | `false` |
| `grid:gaps_use_aspect_ratio` | `Hyprlang::INT` | When enabled, vertical gaps will be scaled to match the monitor's aspect ratio | `false` |
| `linear:blur` | `Hyprlang::INT` | Whether or not to blur the dimmed area | `false` |
| `linear:height` | `Hyprlang::FLOAT` | The height of the linear overlay in logical pixels | `300.f` |
| `linear:scroll_speed` | `Hyprlang::FLOAT` | Scroll speed modifier. Set negative to flip direction | `1.f` |
