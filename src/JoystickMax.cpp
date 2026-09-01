/*
  JoystickMax.cpp — unified backend for
  AVR (Leonardo/Micro), SAM (Due), SAMD (MKR/Zero/Nano33IoT),
  Renesas UNO R4 (Minima/NanoR4), and Mbed (Giga/Portenta/Nano RP2040).

  Mbed uses arduino::USBHID (PluggableUSBHID.h) subclass.
  All others use legacy HID.h / HIDSubDescriptor / HID().SendReport().
  Descriptor is shared (descriptor.h, 254 B dual ID 4+5, Windows-safe 32b).
*/

#include "JoystickMax.h"
#include "descriptor.h"
#include <string.h>

#if defined(ARDUINO_ARCH_MBED)
// ---- Mbed backend (Giga R1, Portenta H7, Nano RP2040, Opta, etc.) ----
#include "PluggableUSBHID.h"  // arduino::USBHID, HID_REPORT

#define JOYSTICKMAX_REPORT_BYTES (sizeof(JoystickMax_Report_t))

class JoystickMax_HID : public arduino::USBHID {
public:
  JoystickMax_HID()
      : arduino::USBHID(true, MAX_HID_REPORT_SIZE, 0, 0x1235, 0x0006, 0x0001) {}
  const uint8_t *report_desc() override {
    reportLength = _joystickMaxDescriptorLength;
    return _joystickMaxDescriptor;
  }
  void push(const uint8_t *data, uint16_t len) {
    HID_REPORT r;
    r.length = (len > MAX_HID_REPORT_SIZE) ? MAX_HID_REPORT_SIZE : len;
    memcpy(r.data, data, r.length);
    send_nb(&r);
  }
};

static JoystickMax_HID _joystickMaxHid;

#else
// ---- Legacy HID backend (AVR, SAM, SAMD, Renesas) ----
#include "HID.h"

#if !defined(_USING_HID)
#warning "Using legacy HID core (non pluggable) — JoystickMax may not enumerate correctly on this core"
#endif

#define JOYSTICKMAX_REPORT_BYTES (sizeof(JoystickMax_Report_t))

static HIDSubDescriptor _joystickMaxNode(_joystickMaxDescriptor, _joystickMaxDescriptorLength);
static bool _joystickMaxHidInited = false;

#endif // ARDUINO_ARCH_MBED

// ——— common state handling ———

JoystickMax_::JoystickMax_(uint8_t reportId) : _reportId(reportId) {
#if !defined(ARDUINO_ARCH_MBED)
  if (!_joystickMaxHidInited) {
    HID().AppendDescriptor(&_joystickMaxNode);
    _joystickMaxHidInited = true;
  }
#endif
}

void JoystickMax_::begin(void) {
  _hid_report.buttons = 0;
  _hid_report.hat1 = 8;
  _hid_report.hat2 = 8;
  _hid_report.x = AXIS_MID;
  _hid_report.y = AXIS_MID;
  _hid_report.z = AXIS_MID;
  _hid_report.rx = AXIS_MID;
  _hid_report.ry = AXIS_MID;
  _hid_report.rz = AXIS_MID;
  _hid_report.rudder = AXIS_MID;
  _hid_report.throttle = AXIS_MID;
  _hid_report.accelerator = AXIS_MID;
  _hid_report.brake = AXIS_MID;
  _hid_report.steering = AXIS_MID;
  sendReport();
}

void JoystickMax_::end(void) { begin(); }

void JoystickMax_::sendReport() {
#if defined(ARDUINO_ARCH_MBED)
  uint8_t buf[1 + JOYSTICKMAX_REPORT_BYTES];
  buf[0] = _reportId;
  memcpy(&buf[1], &_hid_report, JOYSTICKMAX_REPORT_BYTES);
  _joystickMaxHid.push(buf, (uint16_t)sizeof(buf));
#else
  HID().SendReport(_reportId, &_hid_report, sizeof(_hid_report));
#endif
}

void JoystickMax_::press(int b) {
  b = constrain(b, 0, BUTTONS_MAX - 1);
  _hid_report.buttons |= (uint32_t)(1UL << b);
}

void JoystickMax_::release(int b) {
  b = constrain(b, 0, BUTTONS_MAX - 1);
  _hid_report.buttons &= ~(uint32_t)(1UL << b);
}

void JoystickMax_::releaseAll() { _hid_report.buttons = 0; }

bool JoystickMax_::isPressed(int b) {
  b = constrain(b, 0, BUTTONS_MAX - 1);
  return ((_hid_report.buttons & (uint32_t)(1UL << b)) != 0);
}

void JoystickMax_::hat1(int d) {
  d = constrain(d, 0, 15);
  _hid_report.hat1 = d;
}

void JoystickMax_::hat2(int d) {
  d = constrain(d, 0, 15);
  _hid_report.hat2 = d;
}

// Globals — primary and aliases
JoystickMax_ JoystickMax(4);
JoystickMax_ JoystickMax2(5);
JoystickMax_ &JoystickWin = JoystickMax;
JoystickMax_ &JoystickWin2 = JoystickMax2;
