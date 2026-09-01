# AGENTS.md — JoystickMax

This file guides AI agents (and humans) working in this repo. Follow it for builds, hardware tests, and commits.

## Project overview

* **JoystickMax** — unified Arduino USB HID joystick library, successor to `controllercustom/JoystickWin`.
* **Provenance:** Ported and proven in `~/Giga_Joystick_Max` (Giga R1 mbed `arduino::USBHID` subclass). That directory is the reference; do not modify it without clear reason.
* **Purpose:** One library for all original JoystickWin boards **plus** every mbed board. TinyUSB boards are intentionally split into `JoystickMax-TinyUSB` (follow-up).
* **HID profile (Windows-safe):** 32 buttons (`0x29 0x20`), 2 hats (4-bit each, 315°), 11 axes (6× Generic Desktop `0x30-0x35` X/Y/Z/Rx/Ry/Rz +5× Simulation `0xBA/BB/C4/C5/C8` Rudder/Throttle/Accelerator/Brake/Steering). Payload 27 B +1 ID =28 B, `wDescriptorLength 254` dual Application collections Report ID 4 (`JoystickMax`) + ID 5 (`JoystickMax2`). `JoystickWin` is a reference alias to the same objects.

## Directory layout

```
~/JoystickMax/                # this repo (also installed to ~/Arduino/libraries/JoystickMax)
  library.properties          # name=JoystickMax, architectures=*, MIT
  LICENSE                     # MIT (upstream JoystickWin + JoystickMax contributors)
  keywords.txt
  README.md
  src/
    JoystickMax.h             # public API, JoystickMax_Report_t, JoystickMax/Max2 globals, JoystickWin refs
    JoystickWin.h             # shim: #include "JoystickMax.h"
    JoystickMax.cpp           # conditional backends (see below)
    descriptor.h              # 254 B HID descriptor (shared)
  examples/
    JoystickMax/JoystickMax.ino            # single ID 4, ~10 Hz
    JoystickMax_Dual/JoystickMax_Dual.ino  # dual ID 4+5 ~20 Hz
    JoystickWin/JoystickWin.ino            # alias drop-in test
    KeyboardMouseJoystick/KeyboardMouseJoystick.ino  # composite KB+Mouse+Joystick (IDs 1/2/4) ~15 Hz, uses built-in Keyboard/Mouse
~/Giga_Joystick_Max/          # proof reference — Giga R1 mbed port, captures, upstream verbatim
```

## Supported boards / cores

| Board | FQBN | Core | Backend |
|-------|------|------|---------|
| Leonardo / Pro Micro / Micro | `arduino:avr:leonardo` | `arduino:avr` 1.8.8 | `HID.h` legacy |
| Due | `arduino:sam:due` (→ `arduino:sam:arduino_due_x` / `_x_dbg`) | `arduino:sam` | `HID.h` legacy (same as AVR/SAMD; not available on Pi ARM host) |
| MKR / Zero / Nano 33 IoT | `arduino:samd:mkr1000:mkrzero:nano_33_iot` | `arduino:samd` 1.8.12 | `HID.h` legacy |
| Uno R4 Minima / Nano R4 | `arduino:renesas_uno:unor4wifi:nanor4:minima` | `arduino:renesas_uno` 1.6.0 | `HID.h` legacy |
| Giga R1 | `arduino:mbed_giga:giga` | `arduino:mbed_giga` 4.6.0 | `arduino::USBHID` mbed |
| Portenta H7 / Nano RP2040 / Opta | `arduino:mbed_*` | `arduino:mbed_*` | same mbed backend |

TinyUSB (`adafruit:samd/nrf52`, `esp32:esp32`, `rp2040:rp2040`) → separate `JoystickMax-TinyUSB`.

## Build & upload

Library lives at `~/JoystickMax` and is mirrored to `~/Arduino/libraries/JoystickMax` (keep in sync: `rsync -a ~/JoystickMax/ ~/Arduino/libraries/JoystickMax/ --delete`).

```bash
# compile (library auto-discovered when installed)
arduino-cli compile --fqbn arduino:avr:leonardo ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:samd:nano_33_iot ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:mbed_giga:giga ~/JoystickMax/examples/JoystickMax

# compile with explicit path (if not installed)
arduino-cli compile --fqbn arduino:mbed_giga:giga --library ~/JoystickMax ~/JoystickMax/examples/JoystickMax
```

Upload ports on Pi 5 (check `arduino-cli board list` / `lsusb` / `dmesg`):
* Giga R1: `/dev/ttyACM0` `2341:0266` (DFU `2341:0366` at `0x08040000` via `dfu-util`)
* Zero: EDBG `03EB:2157` `ttyACM1` (`arduino:samd:arduino_zero_edbg`) + Native `2341:004d/804d` `ttyACM2` (`arduino:samd:arduino_zero_native`) — HID on native
* Leonardo: `2341:8036` `ttyACM1` via Caterina bootloader `2a03:0036` — double-tap RESET, then `avrdude -p atmega32u4 -c avr109 -P /dev/ttyACM1 -b 57600` or `arduino-cli upload -p /dev/ttyACM1 --fqbn arduino:avr:leonardo`
* Due: Programming `2341:003d` `ttyACM1` / Native `2341:003e` `ttyACM2` — upload via Programming Port `arduino:sam:arduino_due_x_dbg`

For Leonardo/avr109: bootloader window is ~8 s. Pre-compile first, then double-tap and flash immediately:

```bash
arduino-cli compile --fqbn arduino:avr:leonardo ~/JoystickMax/examples/JoystickMax
HEX=$(find ~/.cache/arduino/sketches -name "JoystickMax.ino.hex" -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
avrdude -C ~/.arduino15/packages/arduino/tools/avrdude/8.0.0-arduino1/etc/avrdude.conf -v -p atmega32u4 -c avr109 -P /dev/ttyACM1 -b 57600 -D -U flash:w:$HEX:i
```

Giga note: `arduino:mbed_giga` reports `architectures=mbed_giga`, so `library.properties` uses `architectures=*` to silence warnings.

**Isolated builds (avoid cache overwrite):** `~/.cache/arduino/sketches/<hash>/` is shared per sketch path — sequential `--fqbn` without `--build-path` overwrites `.bin`/`.elf` (e.g. Giga 120344 B overwritten by Renesas 61676 B ⇒ brick at `0x08040000`). Use `--build-path /tmp/<board>` per FQBN:
```bash
arduino-cli compile --fqbn arduino:mbed_giga:giga --build-path /tmp/giga_kmj ~/JoystickMax/examples/KeyboardMouseJoystick
arduino-cli compile --fqbn arduino:avr:leonardo --build-path /tmp/leo_kmj ~/JoystickMax/examples/KeyboardMouseJoystick
arduino-cli upload -p 1-2 --fqbn arduino:mbed_giga:giga --input-dir /tmp/giga_kmj   # DFU 2341:0366 → 0x08040000
arduino-cli upload -p /dev/ttyACM1 --fqbn arduino:avr:leonardo --input-dir /tmp/leo_kmj  # 1200bps touch → avr109 CATERIN 57600
```
Leonardo auto-reset: `arduino-cli upload` does 1200 bps touch; manual `avrdude -c avr109 -P /dev/ttyACM1 -b 57600` requires double-tap RESET → `2a03:0036` window ~8 s.

## Verification (Pi 5 host)

```bash
lsusb -v -d 2341:0266 | grep wDescriptorLength  # single joystick: 254
lsusb -v -d 2341:8036 | grep wDescriptorLength  # single joystick: 254
# composite KeyboardMouseJoystick:
lsusb -v -d 2341:0266 | grep wDescriptorLength  # Giga: 104 (KB) + 52 (mouse) + 254 (joystick) across 3 HID ifaces
lsusb -v -d 2341:8057 | grep wDescriptorLength  # Nano33IoT SAMD split: 47 (KB ID2) + 54 (mouse ID1) + 254 (joystick) 3 HID ifaces (mbed-style, avoids 355>256 pack buf)
lsusb -v -d 2341:8036 | grep wDescriptorLength  # Leonardo: 355 (merged Keyboard ID2 + Mouse ID1 + Joystick ID4 + CDC) — AVR/Renesas OK
lsusb -v -d 2341:0069 | grep wDescriptorLength  # Uno R4 Minima: 355 merged (Renesas USB stack pack>256)
ls -l /dev/hidraw* /dev/input/js* /dev/ttyACM*
# Giga hidraw0/js0, Zero hidraw2/js1, Leonardo hidraw1/js1
# composite adds: Leonardo mouse event7 / kbd event8 / js event9+js1 (see /proc/bus/input/devices)
sudo hexdump -C /dev/hidraw0 | head  # 28 B reports, data[0]=0x04 or 0x05
sudo python3 -c "import os,select; fd=os.open('/dev/hidraw0',0); ..."
jstest --normal /dev/input/js0  # 32 buttons, 13 axes (11+2 hats)
evtest /dev/input/event5        # ABS_X/Y/Z/RX/RY/RZ/THROTTLE/RUDDER/WHEEL/GAS/BRAKE
# Second joystick: hidraw interleaved ids {4,5} @20 Hz for Dual examples
# Composite example: evtest event7 (REL_X/BTN_LEFT) + event8 (KEY_K/M/A...) concurrently with jstest js1
```

## Code style & backends

* `src/JoystickMax.h` declares `JoystickMax_Report_t`, `JoystickMax_` with `reportId` param, `JoystickMax`/`Max2` and `JoystickWin` refs (`extern JoystickMax_ &JoystickWin`).
* `src/JoystickMax.cpp` — conditional:
  * `#if defined(ARDUINO_ARCH_MBED)` → `#include "PluggableUSBHID.h"` subclass `arduino::USBHID`, `report_desc()` returns `254 B`, `push()` via `HID_REPORT` `send_nb`.
  * `#else` → `#include "HID.h"` `_USING_HID` `HIDSubDescriptor` `_joystickMaxDescriptor` `HID().AppendDescriptor` + `HID().SendReport(_reportId, &_hid_report, sizeof)`. Guarded single `AppendDescriptor` via `static bool`.
* `examples/KeyboardMouseJoystick` — uses built-in managed `Keyboard@1.0.7` (`HID().SendReport 2`) / `Mouse@1.0.1` (`SendReport 1`) on `HID.h` legacy cores (AVR/Renesas), `arduino::USBKeyboard`/`USBMouse` on mbed (each a `PluggableUSBModule` iface), and SAMD per-ID split (`SAMD_Keyboard_`/`SAMD_Mouse_` custom `PluggableUSBModule` `47/54 B` + `JoystickMax 254`) to avoid `SAMD _pack_buffer[256]` overflow on merged `355` (`HID 355 >256` → `-110` timeout). No hand-rolled `kbDescriptor`/`mouseDescriptor` on AVR/Renesas/mbed.
* Keep `descriptor.h` as single source; both backends share it. No TinyUSB code in this repo.

## Development guidelines

* Keep `BUTTONS_MAX 32` (Windows limit). Do not reintroduce 74b/14a without discussion — last revert was `9e649e7`.
* Preserve `JoystickWin` alias — many sketches use `#include <JoystickWin.h>`.
* After editing `~/JoystickMax/src/*`, run compile matrix (Giga at minimum, plus one AVR, one SAMD, one Renesas). Do not commit if Giga fails (proven reference).
* Sync library: `rsync -a ~/JoystickMax/ ~/Arduino/libraries/JoystickMax/ --delete`.
* Do not add TinyUSB dependency here; use separate repo.

## Git & commits

* Repo at `~/JoystickMax` is standalone (`main` branch, `0a45b1b` v1.0.0). Configure `user.email`/`user.name` before committing.
* Messages: concise, prefix `hid:` or `feat:` or `fix:`.
* Keep `~/Giga_Joystick_Max` history separate (proof repo, `9e649e7` revert point).

## Hardware tips

* Giga: 3.3 V only, double-tap RESET for DFU `2341:0366` if upload fails; app at `0x08040000`.
* Zero/Due: HID on **Native** port, upload via **Programming/EDBG** port.
* Leonardo: HID+CDC composite `wTotalLength 0x0064`; upload via Caterina `2a03:0036` `57600`.
* Use `dmesg -w`, `lsusb -t`, `arduino-cli board list`, `evtest`, `jstest` for triage.

## References

* Upstream: https://github.com/controllercustom/JoystickWin
* Proof: `~/Giga_Joystick_Max/README.md`, `captures/lsusb.txt`, `report_descriptor.bin` (254 B), `hidraw` samples.
* Cores: `~/.arduino15/packages/arduino/hardware/*`
