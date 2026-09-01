# JoystickMax — USB HID joystick for Arduino

JoystickMax is a drop-in successor to [controllercustom/JoystickWin](https://github.com/controllercustom/JoystickMax). It turns a wide range of Arduino boards into a USB HID joystick that appears on Windows and Linux as a standard game controller: **32 buttons, 2 eight-way hats (315°), and 11 axes** (6 Generic Desktop `X/Y/Z/Rx/Ry/Rz` plus 5 Simulation `Rudder/Throttle/Accelerator/Brake/Steering`). Each report is 27 bytes plus one Report ID byte (28 bytes total), and the HID descriptor is 254 bytes long with two Application collections. The first collection is Report ID 4 (`JoystickMax`), the second is Report ID 5 (`JoystickMax2`), so a single board can present as one or two independent joysticks on the same USB interface. This stays within the Windows-safe limit of 32 buttons and is the same report layout that was proven on the Giga R1 in `~/Giga_Joystick_Max`.

## Features

* Drop-in replacement for JoystickWin — same `press`, `release`, `hat`, `x/y/z` and `sendReport` API.
* Two instances on one interface: `JoystickMax` (ID 4) and `JoystickMax2` (ID 5). Use one for a single stick, or drive both for a dual-stick or collective/throttle setup.
* Windows 10/11 and Linux (evdev/joystick) compatible. Reports are read by `hid-generic` as a normal joystick and appear as `/dev/input/js0` and `/dev/hidraw0` on Linux, and as a game controller on Windows.
* No extra dependencies. The report struct `JoystickMax_Report_t` is 27 bytes and fits the HID 64-byte limit with room for CDC ACM `Serial` on the same USB composite.
* Maintains `JoystickWin` as a reference alias, so existing sketches continue to compile without edits.

## Supported boards

The library supports all boards that JoystickWin supported, plus every mbed board. Pick the entry that matches your hardware; the FQBN column is what you select in the IDE or pass to `arduino-cli`.

| Family | Example boards | Core (version tested) | Board selection in IDE |
|---|---|---|---|
| Leonardo / Pro Micro / Micro | ATmega32U4 32u4 | `arduino:avr` 1.8.8 | `Arduino AVR Boards → Arduino Leonardo` `arduino:avr:leonardo` |
| Due | ATSAM3X8E | `arduino:sam` | `Arduino SAM Boards → Arduino Due (Programming Port)` `arduino:sam:due` |
| MKR, Zero, Nano 33 IoT | SAMD21 | `arduino:samd` 1.8.12 | `Arduino SAMD Boards → Arduino MKR1000 / Zero / NANO 33 IoT` `arduino:samd:mkr1000:mkrzero:nano_33_iot` |
| Uno R4 Minima / Nano R4 | RA4M1 | `arduino:renesas_uno` 1.6.0 | `Arduino UNO R4 Boards → UNO R4 Minima / Nano R4` `arduino:renesas_uno:unor4wifi:nanor4:minima` |
| Giga R1 | STM32H747 | `arduino:mbed_giga` 4.6.0 | `Arduino Mbed OS Giga Boards → Arduino Giga R1` `arduino:mbed_giga:giga` |
| Portenta H7 / Nano RP2040 / Opta / Nicla | STM32H7 / RP2040 | `arduino:mbed_*` | Corresponding `Arduino Mbed OS ...` entry |

If your board is not listed but uses one of these cores, select the closest FQBN for that core and it will use the same HID backend.

## Installation and first upload — Arduino IDE 2.x (recommended)

Most users should follow these IDE steps. The library is installed once and then available for every board above.

### 1. Download the library from GitHub

1. Open `https://github.com/controllercustom/JoystickMax` in a browser.
2. Click the green **Code** button, then **Download ZIP**. Your browser will save `JoystickMax-main.zip` (or `JoystickMax-1.0.0.zip` from the **Releases** page).
3. Keep the ZIP as-is — do not unzip it.

### 2. Install the library in IDE 2.x

1. Open Arduino IDE 2.x.
2. Choose **Sketch → Include Library → Add .ZIP Library…**.
3. In the file dialog, select the ZIP you downloaded and click **Open**.
4. Wait for the IDE notification `Library added to your libraries`. Verify with **File → Examples** — you should now see **JoystickMax** with four entries: `JoystickMax`, `JoystickMax_Dual`, `JoystickWin`, and `KeyboardMouseJoystick`.
5. No additional dependencies are needed (`Keyboard@1.0.7`/`Mouse@1.0.1` are used automatically on AVR/Renesas via the library manager; `mbed` uses built-in `USBKeyboard`/`USBMouse`). Restart the IDE only if the examples do not appear.

After installation the library lives at `~/Arduino/libraries/JoystickMax` (on Windows `Documents\Arduino\libraries\JoystickMax`). You can update it later by repeating the ZIP install; the IDE will overwrite the old folder.

### 3. Select your board and port

1. Connect your board via USB. For boards with two ports (Zero, Due), connect the **Native** or **Programming** port as noted below and check `Tools → Port` for the new entry.
2. Choose **Tools → Board** and pick the entry from the table above. For example, for a Leonardo choose `Arduino AVR Boards → Arduino Leonardo`; for a Giga R1 choose `Arduino Mbed OS Giga Boards → Arduino Giga R1`.
3. Choose **Tools → Port** and select the port that appeared after plugging in. On Linux it is normally `/dev/ttyACM0` (Giga), `/dev/ttyACM0` or `/dev/ttyACM1` (Leonardo), `/dev/ttyACM1` EDBG + `/dev/ttyACM2` Native (Zero/Due), or `1-2` DFU for Uno R4. On Windows it is `COM3`, `COM4`, etc. If no port appears, unplug and replug and check the board-specific notes below.

### 4. Open and upload an example

1. Choose **File → Examples → JoystickMax → JoystickMax**. This is the single-joystick example that walks buttons 0–31, sweeps hats 0–7, and randomises all 11 axes at about 10 Hz (Report ID 4).
2. Click **Verify** (checkmark) to compile. You should see `Done compiling` with no errors.
3. Click **Upload** (right arrow). The IDE will compile again and then write to the board.

Board-specific upload notes:

* **Leonardo / Pro Micro / Micro (ATmega32U4):** After clicking Upload, quickly double-tap the **RESET** button. The board will enter the Caterina bootloader `2a03:0036` for about 8 seconds on `ttyACM1` at 57600 baud and the IDE will finish the write. If upload fails with `no upload port`, double-tap again and immediately retry Upload without recompiling.
* **Zero / Due:** HID appears on the **Native** port (`2341:004d/804d` or `2341:003e`), but upload via the **Programming / EDBG** port (`03EB:2157` `ttyACM1`, `arduino:samd:arduino_zero_edbg` or `arduino:sam:arduino_due_x_dbg`). Keep both cables connected if you want Serial and HID at the same time.
* **Uno R4 Minima / Nano R4 (Renesas):** The board enters `Santiago DFU` `2341:0369` / `0374` DFU automatically after Upload. No manual reset is needed; `dfu-util` will detach and reattach. For Nano R4 (HID-only) a double-tap may be required if `ttyACM` is not present.
* **Giga R1:** Upload uses `dfu-util` at `2341:0366` address `0x08040000`. If the port is busy, double-tap RESET to force DFU.

4. After a successful upload the board re-enumerates. Open **Tools → Serial Monitor** at 115200 baud to see the `tick` heartbeat, and open your operating system’s game-controller panel (Windows: `joy.cpl`, Linux: `jstest --normal /dev/input/js0`) to see the stick move.

### 5. Try the other examples

* **JoystickMax_Dual** — drives both `JoystickMax` (ID 4) and `JoystickMax2` (ID 5) at about 20 reports per second total. On the host the HID reports are interleaved `04` / `05` and 28 bytes each.
* **JoystickWin** — proves the alias. It includes `JoystickWin.h` and uses `JoystickWin` instead of `JoystickMax` but is otherwise identical to `JoystickMax`.
* **KeyboardMouseJoystick** — composite `Keyboard ID2 + Mouse ID1 + Joystick ID4` at ~15 Hz. Uses built-in `Keyboard`/`Mouse` on AVR/Renesas (`355` merged), per-ID split `47/54/254` on SAMD (`SAMD_Keyboard_`/`SAMD_Mouse_` separate `PluggableUSBModule` ifaces to avoid `355>256 _pack_buffer` timeout), and `USBKeyboard`/`USBMouse` separate ifaces `104/52/254` on mbed. Host sees 3 input nodes (e.g. Leonardo `event7 mouse / event8 kbd / event9 js1`; Zero/Nano33IoT `event5 kbd / event6 mouse / event7 js0`; Giga `event5 kbd / event6 mouse / js0`).

<details>
<summary>Advanced: arduino-cli for power users</summary>

```bash
# Install once (if you prefer the CLI over the IDE ZIP flow)
arduino-cli core install arduino:avr arduino:samd arduino:renesas_uno arduino:mbed_giga
arduino-cli lib install --zip-path JoystickMax-1.0.0.zip  # or copy ~/JoystickMax to ~/Arduino/libraries/

# Compile examples (library auto-discovered when installed)
arduino-cli compile --fqbn arduino:avr:leonardo ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:samd:nano_33_iot ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi ~/JoystickMax/examples/JoystickMax
arduino-cli compile --fqbn arduino:mbed_giga:giga ~/JoystickMax/examples/JoystickMax

# Upload (replace port as shown in arduino-cli board list)
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:mbed_giga:giga ~/JoystickMax/examples/JoystickMax
arduino-cli upload -p /dev/ttyACM1 --fqbn arduino:avr:leonardo ~/JoystickMax/examples/JoystickMax
```

For Leonardo’s 8-second bootloader you can pre-compile and then flash directly:

```bash
arduino-cli compile --fqbn arduino:avr:leonardo ~/JoystickMax/examples/JoystickMax
HEX=$(find ~/.cache/arduino/sketches -name "JoystickMax.ino.hex" -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
avrdude -C ~/.arduino15/packages/arduino/tools/avrdude/8.0.0-arduino1/etc/avrdude.conf -v -p atmega32u4 -c avr109 -P /dev/ttyACM1 -b 57600 -D -U flash:w:$HEX:i
```

</details>

## Usage

Include one header and use the same API as JoystickWin. `JoystickMax` is Report ID 4, `JoystickMax2` is Report ID 5 on the same interface.

```cpp
#include <JoystickMax.h>

void setup() {
  JoystickMax.begin();   // ID 4 — start with hats centered (8) and axes at AXIS_MID
  JoystickMax2.begin();  // ID 5 — optional second joystick
}

void loop() {
  JoystickMax.press(0);          // hold button 0
  JoystickMax.hat1(0);           // hat 1 north
  JoystickMax.x(AXIS_MID);       // center X
  JoystickMax.rudder(AXIS_MAX);  // full rudder
  JoystickMax.sendReport();      // send ID 4

  JoystickMax2.steering(AXIS_MIN);
  JoystickMax2.sendReport();     // send ID 5
  delay(10);
}
```

API summary:

* `begin()` / `end()` — initialize report (buttons 0, hats 8, axes `AXIS_MID`) and send one report.
* `press(b)` / `release(b)` / `releaseAll()` / `isPressed(b)` — `b` in `0 … BUTTONS_MAX-1` (`BUTTONS_MAX` is 32).
* `hat1(d)` / `hat2(d)` — `d` 0–7 is direction (0°–315° in 45° steps), 8 is centered/neither.
* `x(a)` `y(a)` `z(a)` `rx(a)` `ry(a)` `rz(a)` `rudder(a)` `throttle(a)` `accelerator(a)` `brake(a)` `steering(a)` — each `a` in `AXIS_MIN` 0 … `AXIS_MAX` 65535, `AXIS_MID` 32767.
* `sendReport()` — transmit the current report for that instance’s ID.
* Types: `JoystickMax_Report_t` (27 bytes packed) and constants `AXIS_MIN`, `AXIS_MAX`, `AXIS_MID`, `BUTTONS_MAX`.

Global instances: `JoystickMax` (ID 4) and `JoystickMax2` (ID 5). Both share the 254-byte descriptor.

## JoystickWin compatibility — in detail

If you have existing sketches that use `controllercustom/JoystickWin`, JoystickMax is a drop-in replacement and you do not need to edit your sketch to keep it working.

* **Header shim:** `src/JoystickWin.h` is a one-line shim: `#pragma once` + `#include "JoystickMax.h"`. Any sketch that does `#include <JoystickWin.h>` will therefore include JoystickMax without changes.
* **Type alias:** `JoystickWin_Report_t` is a `typedef` to `JoystickMax_Report_t`, and `JoystickWin_` is a `typedef` to `JoystickMax_`. Fields `buttons`, `hat1`, `hat2`, `x`, `y`, `z`, `rx`, `ry`, `rz`, `rudder`, `throttle`, `accelerator`, `brake`, `steering` are identical and keep the same packed layout (27 bytes) andReport ID 4.
* **Object references, not copies:** The library defines `extern JoystickMax_ JoystickMax;` and `extern JoystickMax_ JoystickMax2;` and then `extern JoystickMax_ &JoystickWin = JoystickMax;` and `extern JoystickMax_ &JoystickWin2 = JoystickMax2;`. `JoystickWin` is a C++ reference to `JoystickMax`, so `JoystickWin.press(0)` and `JoystickMax.press(0)` modify the same report and send the same 28-byte HID report on ID 4. There is no duplication and no extra descriptor cost.
* **Migration:** To move a sketch to the new name, change one line from `#include <JoystickWin.h>` to `#include <JoystickMax.h>` and from `JoystickWin.` to `JoystickMax.`; or leave it as-is — both compile. The example `examples/JoystickWin/JoystickWin.ino` is the same walk/hat/sweep as `JoystickMax` but uses the `JoystickWin` name to prove the alias.

Recommended for new code:

```cpp
#include <JoystickMax.h>   // prefer this
JoystickMax.begin();
```

Kept for legacy:

```cpp
#include <JoystickWin.h>   // shim → JoystickMax.h, still works
JoystickWin.begin();
```

## Verification — for developers and maintainers only

> **Note:** This section is for library developers and maintainers validating a new backend or core version. Normal users do not need these steps — a successful IDE Upload plus the operating system’s game-controller test (`joy.cpl` on Windows, `jstest` on Linux) is sufficient.

The reference rig is a Pi 5 host with `evtest`, `jstest`, `hidraw`, `lsusb`, and `dmesg`. The proven reference is `~/Giga_Joystick_Max` on the Giga R1 (`arduino:mbed_giga:giga` 4.6.0) and was used to validate every board family. The expected HID on the wire is `wDescriptorLength 254`, 28-byte reports `data[0]` `0x04` or `0x05` interleaved at 10 Hz (single) / 20 Hz (dual), and `/dev/input/js0` with 32 buttons and 13 axes (11 physical + 2 virtual hat axes).

On the Pi 5 host after uploading an example:

```bash
lsusb -v -d 2341:0266 | grep wDescriptorLength  # single joystick: 254
lsusb -v -d 2341:8036 | grep wDescriptorLength  # single joystick: 254
lsusb -v -d 2341:804d | grep wDescriptorLength  # single joystick: 254 (Zero Native 804d/004d)
lsusb -v -d 2341:8057 | grep wDescriptorLength  # single joystick: 254 (Nano 33 IoT)
# composite KeyboardMouseJoystick:
lsusb -v -d 2341:0266 | grep wDescriptorLength  # Giga: 104 (KB) + 52 (mouse) + 254 (joystick) 3 HID ifaces
lsusb -v -d 2341:804d | grep wDescriptorLength  # Zero SAMD split: 47 (KB ID2) + 54 (mouse ID1) + 254 (joystick) 3 HID ifaces (avoids 355>256)
lsusb -v -d 2341:8057 | grep wDescriptorLength  # Nano 33 IoT SAMD split: 47 + 54 + 254 (same)
lsusb -v -d 2341:0074 | grep wDescriptorLength  # Nano R4: 355 (merged ID2+ID1+ID4) Renesas OK
lsusb -v -d 2341:8036 | grep wDescriptorLength  # Leonardo: 355 merged (AVR OK) — SAMD would timeout -110
ls -l /dev/hidraw* /dev/input/js* /dev/ttyACM*
# Giga hidraw0/js0, Zero hidraw2/js1, Leonardo hidraw1/js1
# composite adds: Zero event5 kbd / event6 mouse / event7 js0; Leonardo event7 mouse / event8 kbd / event9 js1; Nano R4 event9 kbd / event10 mouse / js1

# Raw HID (developer): report ID + 27-byte payload
sudo hexdump -C /dev/hidraw0 | head  # data[0]=0x04 (or 0x05 for JoystickMax2)

# Parsed HID (Python, developer)
sudo python3 -c "import os,select; fd=os.open('/dev/hidraw0',0); ..."
jstest --normal /dev/input/js0  # 32 buttons, 13 axes (11+2 hats virtual)
evtest /dev/input/event5        # ABS_X/Y/Z/RX/RY/RZ/THROTTLE/RUDDER/WHEEL/GAS/BRAKE
# For Dual examples hidraw is interleaved ids {4,5} at ~20 Hz
# For composite: evtest event7 (REL_X/BTN_LEFT) + event8 (KEY_*) concurrently with jstest js0/js1
```

Keep `~/Giga_Joystick_Max` untouched as the proof reference; do not modify it without clear reason.

## Troubleshooting

* **Library not found or `JoystickMax.h: No such file`:** Ensure the ZIP was installed via **Sketch → Include Library → Add .ZIP Library** and that `~/Arduino/libraries/JoystickMax/src/JoystickMax.h` exists. Restart the IDE and try **File → Examples → JoystickMax** again. The library declares `architectures=*` to avoid core warnings.
* **No port after plugging in:** Check `Tools → Port` after replugging. For Zero/Due HID is on the **Native** port, upload via **Programming/EDBG** port. For Giga/Portenta HID is on the single USB-C port. Run `arduino-cli board list` / `lsusb` / `dmesg -w` to see which `ttyACM` appeared.
* **Leonardo upload fails `no upload port`:** Double-tap **RESET** (two quick presses) to enter Caterina `2a03:0036` bootloader at 57600 baud. You have about 8 seconds — click **Upload** immediately after the double-tap, or pre-compile (`Verify`) and then use the `avrdude` one-liner in the Advanced collapsed block above.
* **HID not appearing (no `/dev/hidraw*` or `wDescriptorLength 47` instead of 254):** The board is running an old sketch (for Leonardo a Keyboard-only 47-byte descriptor). Re-upload a JoystickMax example and re-check `lsusb -v -d <vid:pid> | grep wDescriptorLength`.
* **Uno R4 DFU `can't set config #1, error -71`:** Normal for the Renesas DFU transition `045e:028e` → `2341:0369`/`0374` on the Pi; the upload will continue via `dfu-util`.

## License

MIT — see `LICENSE`. Copyright (c) 2026 controllercustom@myyahoo.com

## References

* Upstream FROM: https://github.com/controllercustom/JoystickMax — JoystickWin upstream for HID layout provenance
* Proof reference: `~/Giga_Joystick_Max/README.md`, `captures/lsusb.txt`, `report_descriptor.bin` (254 B)
* Cores used for testing: `~/.arduino15/packages/arduino/hardware/*` (`arduino:avr` 1.8.8, `arduino:samd` 1.8.12, `arduino:renesas_uno` 1.6.0, `arduino:mbed_giga` 4.6.0)
