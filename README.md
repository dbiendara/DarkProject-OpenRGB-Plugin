# Dark Project ALU87A OpenRGB Plugin

An OpenRGB (v1.1 / Plugin API v5) plugin for controlling **Dark Project ALU87A** keyboards (`DPKB_MIDNIGHT_87_ISO`, USB VID `0x342D`, PID `0xE40F`) natively in OpenRGB, based on reverse-engineering the WebHID protocol of `https://software.darkproject.eu/`.

## Features

- **Full 88-key ISO TKL matrix** (`6x17` layout in OpenRGB's LED View)
- **All 15 hardware lighting modes:**
  - `Direct` (per-key custom RGB control)
  - `Static` (single color)
  - `Wave` (directional: Right, Left, Up, Down; single or random color)
  - `Spiral Wave` (clockwise / counter-clockwise)
  - `Spectrum Cycle`
  - `Breathing`
  - `Random`
  - `Star`
  - `Footprint`
  - `River`
  - `Ripples`
  - `Trigger`
  - `Color Dischange`
  - `Sine Wave`
  - `Rain`
- **Brightness** (`0`–`4`), **Speed** (`1`–`5`), **Direction**, and **Color** controls
- Reads the active keyboard profile, firmware version, and custom LED colors on startup
- Works in the OpenRGB GUI and via the OpenRGB SDK Server

## Protocol Summary

The Dark Project ALU87A (`CommonKeyboardSerie`) exposes a vendor-defined HID interface (`usagePage: 0xFF01`, `usage: 0x01`) using **256-byte HID Feature Reports** with **Report ID `0x07`**:

| Command | ID | Description |
| --- | --- | --- |
| Get Active Profile | `0x81` (`129`) | Returns active profile index (`1`–`3`) at byte `2` |
| Get Profile Data | `0x82` (`130`) | Returns 256-byte profile config (effect at `9`, brightness at `10 + id`, speed at `24 + id`, multicolor at `38 + id`, direction at `52`/`84`, custom slot at `53`, RGB at `59`/`67`/`75`) |
| Set Profile Data | `0x02` (`2`) | Writes updated 256-byte profile config back to the keyboard |
| Get Firmware Version | `0x88` (`136`) | Returns firmware version bytes at `14`–`15` |
| Get Custom RGB | `0x8A` (`138`) | Reads per-key R/G/B arrays (2 chunks of 192 bytes for 96-LED matrix) |
| Set Custom RGB | `0x0A` (`10`) | Writes per-key R/G/B arrays (2 chunks of 192 bytes) |

## Requirements

- **OpenRGB 1.1** (Plugin API v5)
- **Qt 6** (`Core`, `Widgets`, `Gui`)
- **hidapi-hidraw** (`libhidapi-dev` / `hidapi`)
- **CMake** (`>= 3.16`) & C++17 compiler

## Build Instructions

1. Clone this repository and shallow-clone the OpenRGB `release_1.1` headers:

   ```bash
   git clone git@github.com:dbiendara/DarkProject-OpenRGB-Plugin.git
   cd DarkProject-OpenRGB-Plugin
   git clone --depth 1 --branch release_1.1 https://gitlab.com/CalcProgrammer1/OpenRGB.git
   ```

2. Configure and build:

   ```bash
   cmake -S . -B build
   cmake --build build -j$(nproc)
   ```

   *(If your OpenRGB `release_1.1` checkout is in a different location, pass `-DOPENRGB_DIR=/path/to/OpenRGB`.)*

3. Install the plugin:

   ```bash
   mkdir -p ~/.config/OpenRGB/plugins
   cp build/libOpenRGBDarkProjectPlugin.so ~/.config/OpenRGB/plugins/
   ```

4. *(Optional)* To expose the keyboard over the OpenRGB SDK Server (port `6742`), enable **"Share all controllers"** in OpenRGB Settings → SDK Server (or set `"all_controllers": true` under `"Server"` in `~/.config/OpenRGB/OpenRGB.json`).

5. Restart OpenRGB. **Dark Project ALU87A (Midnight ISO)** will appear in your device list.

## License

GPL-2.0-or-later (matching OpenRGB's license).
