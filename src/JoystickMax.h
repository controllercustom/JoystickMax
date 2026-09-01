/*
  JoystickMax.h — drop-in successor to controllercustom/JoystickWin
  for all Arduino HID-capable boards.

  Supported boards (tested via arduino-cli):
   - arduino:avr  Leonardo, Pro Micro, Micro (ATmega32U4)
   - arduino:sam  Due (ATSAM3X8E) — requires arduino:sam core
   - arduino:samd MKR, Zero, Nano 33 IoT (SAMD21)
   - arduino:renesas_uno Uno R4 Minima, Nano R4 (RA4M1)
   - arduino:mbed_giga Giga R1, plus all mbed boards (Portenta H7, Nano RP2040, etc.)

  API mirrors JoystickWin: 32 buttons, 2 hats, 11 axes, Report ID 4 (JoystickMax)
  and Report ID 5 (JoystickMax2) on the same HID interface. Windows/Linux compatible.
  JoystickWin alias is provided for drop-in compatibility.
*/

#pragma once
#include "Arduino.h"

#define AXIS_MIN    (0)
#define AXIS_MAX    (0xFFFFUL)
#define AXIS_MID    (AXIS_MAX / 2)
#define BUTTONS_MAX (32)

// 32b (4) + 1 hat byte (2x4) + 11*16 axes = 27 bytes per report +1 ID =28
typedef struct __attribute__((__packed__)) {
  uint32_t buttons;
  uint8_t  hat1:4;
  uint8_t  hat2:4;
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t rx;
  uint16_t ry;
  uint16_t rz;
  uint16_t rudder;
  uint16_t throttle;
  uint16_t accelerator;
  uint16_t brake;
  uint16_t steering;
} JoystickMax_Report_t;

// Also expose legacy name for JoystickWin compatibility
typedef JoystickMax_Report_t JoystickWin_Report_t;

class JoystickMax_ {
private:
  JoystickMax_Report_t _hid_report;
  uint8_t _reportId;
public:
  explicit JoystickMax_(uint8_t reportId = 4);
  void begin(void);
  void end(void);
  void press(int b);
  void release(int b);
  void releaseAll();
  bool isPressed(int b);
  void hat1(int d);
  void hat2(int d);
  void x(uint16_t a)              { _hid_report.x = a; }
  void y(uint16_t a)              { _hid_report.y = a; }
  void z(uint16_t a)              { _hid_report.z = a; }
  void rx(uint16_t a)             { _hid_report.rx = a; }
  void ry(uint16_t a)             { _hid_report.ry = a; }
  void rz(uint16_t a)             { _hid_report.rz = a; }
  void rudder(uint16_t a)         { _hid_report.rudder = a; }
  void throttle(uint16_t a)       { _hid_report.throttle = a; }
  void accelerator(uint16_t a)    { _hid_report.accelerator = a; }
  void brake(uint16_t a)          { _hid_report.brake = a; }
  void steering(uint16_t a)       { _hid_report.steering = a; }
  void sendReport();
  uint8_t reportId() const { return _reportId; }
};

// Primary (ID 4) and secondary (ID 5) instances
extern JoystickMax_ JoystickMax;
extern JoystickMax_ JoystickMax2;

// JoystickWin drop-in aliases (Q1) — references to same objects, not copies
typedef JoystickMax_ JoystickWin_;
extern JoystickMax_ &JoystickWin;
extern JoystickMax_ &JoystickWin2;
#ifndef JOYSTICKWIN_H_
#define JOYSTICKWIN_H_ // allow #include "JoystickWin.h" after JoystickMax.h
#endif
